//===-- pISAAsmPrinter.cpp - pISA LLVM assembly writer ------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the pISA assembly language.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/pISAInstPrinter.h"
#include "MCTargetDesc/pISARegEncoder.h"
#include "MCTargetDesc/pISATargetStreamer.h"
#include "TargetInfo/pISATargetInfo.h"
#include "pISA.h"
#include "pISADebugInfo.h"
#include "pISAInstrInfo.h"
#include "pISAMCInstLower.h"
#include "pISARegManager.h"
#include "pISASubtarget.h"
#include "pISATargetMachine.h"
#include "pISAUtils.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

namespace {
class pISAAsmPrinter : public AsmPrinter {
public:
  const DebugInfoMetadata &getDebugInfoMetadata() const { return DI; }

  pISATargetStreamer &getTargetStreamer() const {
    return static_cast<pISATargetStreamer &>(*OutStreamer->getTargetStreamer());
  }

private:
  void emitRegDcls(raw_ostream &OS);
  // emit private and shared vars
  void emitStackVariableDcls(raw_ostream &OS);

  void collectRegDcls(pISA::RegDcls &);
  void collectStackVariableDcls(pISA::StackVariableDcls &);

  void outputInstruction(const MachineInstr *MI);
  void printOperand(const MachineInstr *MI, int OpNum, raw_ostream &O);

  unsigned int getTypeBitSize(const MachineInstr &MI, unsigned OpNo) const;

  static StringRef CallingConvRepr(CallingConv::ID CC);

  void emitFunctionSignature(raw_ostream &OS);
  void emitFunctionParameters(raw_ostream &OS);
  void emitKernelParameters(raw_ostream &OS);
  void emitFunctionDirectiveAndName(const Function &F, raw_ostream &OS);
  void emitLinkage(const GlobalValue &V, raw_ostream &OS);

  void collectFunctionSignature(pISA::FunctionSignature &);
  void collectFunctionParameters(pISA::FunctionSignature &);
  void collectKernelParameters(pISA::FunctionSignature &);
  void collectFunctionDirectiveAndName(pISA::FunctionDirectiveAndName &DN,
                                       const Function &F);
  std::optional<bool> collectLinkage(const GlobalValue &V);
  std::optional<unsigned> collectDbgBinding(const Function &F);

  // helper functions for type string printing, e.g. ".32b", ".v2.8b"
  void emitTypeString(const LLT &T, raw_ostream &OS);
  // NumElts: number of elements in a vector type. 1 indicates scalar type
  void emitTypeString(uint32_t TypeSizeInBits, uint32_t NumElts,
                      raw_ostream &OS);

  // emit module scope variables and functions
  void emitGlobalFunctionDecls(const Module &M, raw_ostream &OS);
  // use our own emit method for GV instead of override emitGlobalVariable
  // since emitGlobalVariable is called after function emit.
  void emitGlobalVariables(const Module &M, raw_ostream &OS);
  void emitDbgBinding(const Function &F, raw_ostream &OS);

  const pISASubtarget *ST = nullptr;
  const pISAInstrInfo *TII = nullptr;
  const pISARegisterInfo *TRI = nullptr;
  const pISA::RegManager *RegMgr = nullptr;

  DebugInfoMetadata DI;

protected:
  bool doInitialization(Module &M) override;

public:
  explicit pISAAsmPrinter(TargetMachine &TM,
                          std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}

  StringRef getPassName() const override { return "pISA Assembly Printer"; }
  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &O) override;

  void emitInstruction(const MachineInstr *MI) override;
  void emitFunctionHeader() override;
  void emitFunctionBodyStart() override;
  void emitFunctionBodyEnd() override;
  void emitEndOfAsmFile(Module &) override;

  void emitFunctionEntryLabel() override {}
  void emitBasicBlockEnd(const MachineBasicBlock &MBB) override {}
  void emitGlobalVariable(const GlobalVariable *GV) override {}

  bool runOnMachineFunction(MachineFunction &MF) override;
};
} // namespace

bool pISAAsmPrinter::doInitialization(Module &M) {
  bool Result = AsmPrinter::doInitialization(M);

  // Emit Module level function decl
  std::string GlobalStr;
  raw_string_ostream OS(GlobalStr);
  emitGlobalVariables(M, OS);
  emitGlobalFunctionDecls(M, OS);
  OS.flush();
  OutStreamer->emitRawText(GlobalStr);

  return Result;
}

bool pISAAsmPrinter::runOnMachineFunction(MachineFunction &MF) {
  ST = &MF.getSubtarget<pISASubtarget>();
  TII = ST->getInstrInfo();
  TRI = ST->getRegisterInfo();

  pISA::RegManager Mgr{MF};
  RegMgr = &Mgr;

  return AsmPrinter::runOnMachineFunction(MF);
}

void pISAAsmPrinter::emitFunctionHeader() {
  const Function &F = MF->getFunction();

  auto Section = getObjFileLowering().SectionForGlobal(&F, TM);
  MF->setSection(Section);
}

unsigned int pISAAsmPrinter::getTypeBitSize(const MachineInstr &MI,
                                            unsigned OpNo) const {
  auto *RC = TII->getRegClass(TII->get(MI.getOpcode()), OpNo, TRI, *MF);
  return TRI->getRegSizeInBits(*RC);
}

void pISAAsmPrinter::emitRegDcls(raw_ostream &OS) {
  using namespace pISA;
  auto &MRI = MF->getRegInfo();
  // Emit var decls
  // Map aggregates regs of same bit size. This allows
  // us to emit comma-separated dcls.
  // Map stores <numElts, bitwidth, reg type> as key and a vector of all virtual
  // registers.
  // NumElts is 1 for scalar registers.
  MapVector<std::tuple<unsigned, unsigned, unsigned>, SmallVector<Register>>
      RegPerBitSize;
  for (auto &[CurReg, Info] : RegMgr->mapping()) {
    if (Info.Flags & RegManager::NoEmissionDef)
      continue;
    auto *RC = MRI.getRegClass(CurReg);
    unsigned BitWidth = TRI->getBitSizeFromRegClass(RC);
    unsigned NumElts = TRI->getNumEltsFromRegClass(RC);
    RegPerBitSize[std::make_tuple(NumElts, BitWidth, Info.Type)].push_back(
        Info.Idx);
  }

  constexpr unsigned DefsPerLine = 5;
  // Print scalar and vector reg declarations
  for (const auto &RegsPerBitSize : RegPerBitSize) {
    auto [NumElts, BitWidth, RegType] = RegsPerBitSize.first;
    ArrayRef<Register> Registers{RegsPerBitSize.second};
    auto nextChunk = [&](size_t N, size_t M) {
      if (N + M > Registers.size())
        M = Registers.size() - N;
      return Registers.slice(N, M);
    };
    unsigned CurLoc = 0;
    ArrayRef CurDecls = nextChunk(CurLoc, DefsPerLine);
    while (!CurDecls.empty()) {
      switch (RegType) {
      case RegManager::PRED:
        OS << "\t.pred ";
        break;
      case RegManager::REG:
        OS << "\t.reg ";
        emitTypeString(BitWidth, NumElts, OS);
        OS << " ";
        break;
      default:
        llvm_unreachable("unknown reg type!");
      }
      unsigned int Idx = 0;
      for (Register Reg : CurDecls) {
        const char *Prefix =
            RegMgr->getRegPrefix(static_cast<RegManager::RegType>(RegType));
        OS << Prefix << Reg;
        if (++Idx < CurDecls.size())
          OS << ", ";
      }
      OS << ";\n";
      CurLoc += CurDecls.size();
      CurDecls = nextChunk(CurLoc, DefsPerLine);
    }
  }
}

void pISAAsmPrinter::collectRegDcls(pISA::RegDcls &Dcls) {
  auto &MRI = MF->getRegInfo();
  for (auto &[CurReg, Info] : RegMgr->mapping()) {
    if (Info.Flags & pISA::RegManager::NoEmissionDef)
      continue;
    auto *RC = MRI.getRegClass(CurReg);
    unsigned BitWidth = TRI->getBitSizeFromRegClass(RC);
    unsigned NumElts = TRI->getNumEltsFromRegClass(RC);
    const char *Prefix =
        RegMgr->getRegPrefix(static_cast<pISA::RegManager::RegType>(Info.Type));
    Dcls.Regs[std::make_tuple(NumElts, BitWidth, Info.Type)].push_back(
        std::make_pair(Prefix, Info.Idx));
  }
}

void pISAAsmPrinter::emitStackVariableDcls(raw_ostream &OS) {
  // Collect stack objects to emit them as .private/.shared in pISA
  auto &MFI = MF->getFrameInfo();
  for (int Idx = MFI.getObjectIndexBegin(), EndIdx = MFI.getObjectIndexEnd();
       Idx != EndIdx; ++Idx) {
    auto Align = MFI.getObjectAlign(Idx).value();
    auto ByteSize = MFI.getObjectSize(Idx);

    const char *Storage = nullptr;

    switch (MFI.getStackID(Idx)) {
    case TargetStackID::Default:
      Storage = ".private";
      break;
    case TargetStackID::pISAShared:
      Storage = ".shared";
      break;
    default:
      llvm_unreachable("unknown stack ID!");
    }

    OS << "\t" << Storage << " ";
    OS << ".align " << Align << " ";
    OS << ".8b ";
    OS << "@R" << Idx << "[" << ByteSize << "];\n";
  }
}

void pISAAsmPrinter::collectStackVariableDcls(pISA::StackVariableDcls &Dcls) {
  auto &MFI = MF->getFrameInfo();
  for (int Idx = MFI.getObjectIndexBegin(), EndIdx = MFI.getObjectIndexEnd();
       Idx != EndIdx; ++Idx) {
    unsigned StackId = (MFI.getStackID(Idx) == TargetStackID::Default) ? 0 : 1;
    Dcls.Vars.push_back(
        {StackId, MFI.getObjectSize(Idx), MFI.getObjectAlign(Idx)});
  }
}

void pISAAsmPrinter::emitLinkage(const GlobalValue &V, raw_ostream &OS) {
  if (!V.hasExternalLinkage())
    return;

  // global variable linkage
  if (auto *GVar = dyn_cast<GlobalVariable>(&V)) {
    // External GV with no initializer must be .import. In llvm, global
    // variable definitions must be initialized. Though pISA allows
    // a GV definition with no initializer, we can safely determine the
    // linkage by having initializer or not here
    if (GVar->hasInitializer())
      OS << ".export ";
    else
      OS << ".import ";
    return;
  }

  // function variable linkage
  if (V.isDeclaration())
    OS << ".import ";
  else
    OS << ".export ";
}

std::optional<bool> pISAAsmPrinter::collectLinkage(const GlobalValue &V) {
  if (!V.hasExternalLinkage())
    return std::nullopt;

  // global variable linkage
  if (auto *GVar = dyn_cast<GlobalVariable>(&V)) {
    // External GV with no initializer must be .import. In llvm, global
    // variable definitions must be initialized. Though pISA allows
    // a GV definition with no initializer, we can safely determine the
    // linkage by having initializer or not here
    return GVar->hasInitializer();
  }

  // function variable linkage
  return !V.isDeclaration();
}

void pISAAsmPrinter::emitGlobalVariables(const Module &M, raw_ostream &OS) {
  // TODO: consider re-order global variables to avoid forward reference
  for (const GlobalVariable &GV : M.globals()) {
    emitLinkage(GV, OS);

    // globals must have .const or .global addrspace
    pISA::AddressSpace AS =
        static_cast<pISA::AddressSpace>(GV.getType()->getAddressSpace());
    switch (AS) {
    case pISA::AddressSpace::GLOBAL:
      OS << ".global ";
      break;
    case pISA::AddressSpace::CONSTANT:
      OS << ".const ";
      break;
    case pISA::AddressSpace::SHARED:
      continue;
    default:
      llvm_unreachable("Invalid address space for global variables");
    }

    // TODO: print alignment

    // print type
    Type *VTy = GV.getValueType();
    if (VTy->isArrayTy())
      OS << "." << VTy->getArrayElementType()->getScalarSizeInBits() << "b ";
    else
      emitTypeString(llvm::getLLTForType(*VTy, M.getDataLayout()), OS);

    // print name and array size
    OS << " @" << GV.getName();
    if (VTy->isArrayTy())
      OS << "[" << VTy->getArrayNumElements() << "]";

    // print initializer
    // TODO: Currently support initializer with scalar integer value only
    // Has not supported pointers, vectors, arrays, or with other constant
    // expressions. Has not supported float type.
    if (GV.hasInitializer()) {
      const Constant *Initializer = GV.getInitializer();
      if (VTy->isArrayTy() && Initializer->isZeroValue()) {
        OS << " = zeroinitializer";
      } else if (const ConstantInt *CI = dyn_cast<ConstantInt>(Initializer)) {
        OS << " = " << format_hex(CI->getValue().getZExtValue(), 1);
      }
    }
    OS << ";\n";
  }
}

void pISAAsmPrinter::emitGlobalFunctionDecls(const Module &M, raw_ostream &OS) {
  for (auto &F : M.getFunctionList()) {
    if (!F.isDeclaration() ||
        F.getCallingConv() != CallingConv::PISA_FUNC) // avoid llvm builtins
      continue;

    emitFunctionDirectiveAndName(F, OS);

    // print arguments types
    OS << "(";
    for (auto &P : F.args()) {
      if (P.getArgNo())
        OS << ", ";
      OS << ".reg ";
      auto ArgLLT =
          llvm::getLLTForType(*P.getType(), F.getParent()->getDataLayout());
      emitTypeString(ArgLLT, OS);
    }
    OS << ");\n";
  }
  OS << "\n";
}

void pISAAsmPrinter::emitKernelParameters(raw_ostream &OS) {
  llvm::SmallVector<const MachineInstr *, 8> LoadParamInsts;
  for (auto &MBB : *MF) {
    // loadParam must be contiguous and in the same BB
    // Find the iterator of the first loadParam inst and iterate from it
    // to collect all loadParam insts
    auto MIIt = find_if(
        MBB, [&](MachineInstr &MI) { return TII->isLoadParamInstr(MI); });

    for (; MIIt != MBB.end(); ++MIIt) {
      if (!TII->isLoadParamInstr(*MIIt))
        break;
      LoadParamInsts.push_back(&*MIIt);
    }
  }

  // sort loadParam by param index
  llvm::sort(LoadParamInsts, [](const MachineInstr *L, const MachineInstr *R) {
    return L->getOperand(1).getImm() < R->getOperand(1).getImm();
  });

  // print params
  for (auto *MI : LoadParamInsts) {
    unsigned TypeSize = getTypeBitSize(*MI, 0);
    assert(TypeSize / 8);
    auto ArgIdx = MI->getOperand(1).getImm();
    if (ArgIdx)
      OS << ", ";
    OS << ".param[" << (TypeSize / 8) << "] ";
    OS << "%arg" << ArgIdx;
  }
}

void pISAAsmPrinter::collectKernelParameters(pISA::FunctionSignature &Sig) {
  llvm::SmallVector<const MachineInstr *, 8> LoadParamInsts;
  for (auto &MBB : *MF) {
    // loadParam must be contiguous and in the same BB
    // Find the iterator of the first loadParam inst and iterate from it
    // to collect all loadParam insts
    auto MIIt = find_if(
        MBB, [&](MachineInstr &MI) { return TII->isLoadParamInstr(MI); });

    for (; MIIt != MBB.end(); ++MIIt) {
      if (!TII->isLoadParamInstr(*MIIt))
        break;
      LoadParamInsts.push_back(&*MIIt);
    }
  }

  // sort loadParam by param index
  llvm::sort(LoadParamInsts, [](const MachineInstr *L, const MachineInstr *R) {
    return L->getOperand(1).getImm() < R->getOperand(1).getImm();
  });

  // print params
  for (auto *MI : LoadParamInsts) {
    unsigned TypeSize = getTypeBitSize(*MI, 0);
    assert(TypeSize / 8);
    pISA::KernelParameter Param;
    Param.Size = TypeSize / 8;
    Sig.KernelParams.push_back(Param);
  }
}

void pISAAsmPrinter::emitTypeString(const LLT &T, raw_ostream &OS) {
  auto TypeBitSize = T.getScalarType().getSizeInBits();
  auto NumElts = T.isVector() ? T.getNumElements() : 1;
  emitTypeString(TypeBitSize, NumElts, OS);
}

void pISAAsmPrinter::emitTypeString(uint32_t TypeSizeInBits, uint32_t NumElts,
                                    raw_ostream &OS) {
  assert(NumElts > 0);
  if (NumElts == 1 && TypeSizeInBits == 1) {
    OS << ".pred";
    return;
  }

  if (NumElts > 1)
    OS << ".v" << NumElts;
  OS << "." << TypeSizeInBits << "b";
}

void pISAAsmPrinter::emitFunctionParameters(raw_ostream &OS) {
  llvm::SmallVector<const MachineInstr *, 8> FuncParamInsts;
  for (auto &MBB : *MF) {
    // FunctionParam must be contiguous and in the same BB
    // Find the iterator of the first FunctionParam inst and iterate from it
    // to collect all FunctionParam insts
    auto MIIt = find_if(
        MBB, [&](MachineInstr &MI) { return TII->isFunctionParamInstr(MI); });

    for (; MIIt != MBB.end(); ++MIIt) {
      if (!TII->isFunctionParamInstr(*MIIt))
        break;
      FuncParamInsts.push_back(&*MIIt);
    }
  }

  // sort FunctionParam by param index
  llvm::sort(FuncParamInsts, [](const MachineInstr *L, const MachineInstr *R) {
    return L->getOperand(1).getImm() < R->getOperand(1).getImm();
  });

  // print params
  for (auto *MI : FuncParamInsts) {
    if (MI->getOperand(1).getImm())
      OS << ", ";

    const MachineOperand &MO = MI->getOperand(0);
    assert(MO.getSubReg() == 0 && "no swizzle allowed on args!");
    auto RC = MF->getRegInfo().getRegClass(MO.getReg());
    OS << ".reg ";
    emitTypeString(TRI->getBitSizeFromRegClass(RC),
                   TRI->getNumEltsFromRegClass(RC), OS);
    OS << " ";
    OS << RegMgr->getRegPrefix(RC);
    OS << RegMgr->getRegIdx(MO.getReg());
  }
}

void pISAAsmPrinter::collectFunctionParameters(pISA::FunctionSignature &Sig) {
  llvm::SmallVector<const MachineInstr *, 8> FuncParamInsts;
  for (auto &MBB : *MF) {
    // FunctionParam must be contiguous and in the same BB
    // Find the iterator of the first FunctionParam inst and iterate from it
    // to collect all FunctionParam insts
    auto MIIt = find_if(
        MBB, [&](MachineInstr &MI) { return TII->isFunctionParamInstr(MI); });

    for (; MIIt != MBB.end(); ++MIIt) {
      if (!TII->isFunctionParamInstr(*MIIt))
        break;
      FuncParamInsts.push_back(&*MIIt);
    }
  }

  // Sort FunctionParam by param index
  llvm::sort(FuncParamInsts, [](const MachineInstr *L, const MachineInstr *R) {
    return L->getOperand(1).getImm() < R->getOperand(1).getImm();
  });

  // Collect params
  for (auto *MI : FuncParamInsts) {
    const MachineOperand &MO = MI->getOperand(0);
    assert(MO.getSubReg() == 0 && "no swizzle allowed on args!");
    auto RC = MF->getRegInfo().getRegClass(MO.getReg());
    pISA::FunctionParameter Param;
    Param.Ty = LLT::scalarOrVector(
        ElementCount::getFixed(TRI->getNumEltsFromRegClass(RC)),
        TRI->getBitSizeFromRegClass(RC));
    Param.Prefix = RegMgr->getRegPrefix(RC);
    Param.Idx = RegMgr->getRegIdx(MO.getReg());
    Sig.FunctionParams.push_back(Param);
  }
}

StringRef pISAAsmPrinter::CallingConvRepr(CallingConv::ID CC) {
  switch (CC) {
  case CallingConv::PISA_KERNEL:
    return ".kernel";
  case CallingConv::PISA_FUNC:
    return ".function";
  // begin intel embargo
  case CallingConv::PISA_RT_RAYGEN:
    return ".rt.raygen";
  case CallingConv::PISA_RT_DISPATCH:
    return ".rt.dispatch";
  // end intel embargo
  default:
    llvm_unreachable("unknown calling convention!");
  }
}

void pISAAsmPrinter::emitFunctionDirectiveAndName(const Function &F,
                                                  raw_ostream &OS) {
  CallingConv::ID CC = F.getCallingConv();
  if (CC == CallingConv::PISA_FUNC)
    emitLinkage(F, OS);
  OS << CallingConvRepr(CC) << " ";

  // do not print return value for kernels
  if (CC == CallingConv::PISA_FUNC) {
    auto RetType = F.getReturnType();
    if (RetType->isVoidTy()) {
      OS << "void";
    } else {
      auto RetLLT =
          llvm::getLLTForType(*RetType, F.getParent()->getDataLayout());
      emitTypeString(RetLLT, OS);
    }
  }

  OS << " @";
  printLLVMNameWithoutPrefix(OS, F.getName());
}

void pISAAsmPrinter::collectFunctionDirectiveAndName(
    pISA::FunctionDirectiveAndName &DN, const Function &F) {
  DN.CC = F.getCallingConv();
  if (DN.CC == CallingConv::PISA_FUNC)
    DN.Linkage = collectLinkage(F);
  DN.Name = F.getName();
  if (DN.CC == CallingConv::PISA_FUNC) {
    Type *RetType = F.getReturnType();
    if (!RetType->isVoidTy()) {
      DN.RetLLT = llvm::getLLTForType(*RetType, F.getParent()->getDataLayout());
    }
  }
}

void pISAAsmPrinter::emitFunctionSignature(raw_ostream &OS) {
  Function &F = MF->getFunction();
  emitFunctionDirectiveAndName(F, OS);

  OS << "(";
  if (F.getCallingConv() != CallingConv::PISA_FUNC)
    emitKernelParameters(OS);
  else
    emitFunctionParameters(OS);
  OS << ")";

  emitDbgBinding(F, OS);

  OS << "\n{\n";
}

void pISAAsmPrinter::collectFunctionSignature(pISA::FunctionSignature &Sig) {
  Function &F = MF->getFunction();
  collectFunctionDirectiveAndName(Sig.DN, F);
  if (F.getCallingConv() != CallingConv::PISA_FUNC)
    collectKernelParameters(Sig);
  else
    collectFunctionParameters(Sig);
  Sig.DbgId = collectDbgBinding(F);
}

void pISAAsmPrinter::emitDbgBinding(const Function &F, raw_ostream &OS) {
  if (const auto MD = F.getSubprogram()) {
    auto Id = DI.getOrCreateId(MD->getUnit());
    Id = DI.getOrCreateId(MD);
    OS << " !dbg !" << Id;
  }
}

std::optional<unsigned> pISAAsmPrinter::collectDbgBinding(const Function &F) {
  if (const auto MD = F.getSubprogram()) {
    auto Id = DI.getOrCreateId(MD->getUnit());
    Id = DI.getOrCreateId(MD);
    return Id;
  }
  return std::nullopt;
}

void pISAAsmPrinter::emitFunctionBodyStart() {
  pISATargetStreamer &TS = getTargetStreamer();
  pISA::FunctionSignature Sig;
  // Collect function signature.
  collectFunctionSignature(Sig);
  TS.emitFunctionSignature(Sig);
  // Emit the function body start.
  TS.emitFuncBodyStart();
  // Collect local registers.
  pISA::RegDcls Regs;
  collectRegDcls(Regs);
  TS.emitRegDcls(Regs);
  // Collect local variables.
  pISA::StackVariableDcls Vars;
  collectStackVariableDcls(Vars);
  TS.emitStackVariableDcls(Vars);

  // Emit pisa.dbg.declare instructions right before
  // executable instruction stream.
  DI.emitDbgDeclare(*MF, OutStreamer);
}

void pISAAsmPrinter::emitFunctionBodyEnd() {
  pISATargetStreamer &TS = getTargetStreamer();
  // Emit the function body end.
  TS.emitFuncBodyEnd();
}

void pISAAsmPrinter::emitEndOfAsmFile(llvm::Module &M) {
  DI.emitDebugMetadata(OutStreamer);
}

void pISAAsmPrinter::printOperand(const MachineInstr *MI, int OpNum,
                                  raw_ostream &O) {
  const MachineOperand &MO = MI->getOperand(OpNum);

  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    O << pISAInstPrinter::getRegisterName(MO.getReg());
    break;

  case MachineOperand::MO_Immediate:
    O << MO.getImm();
    break;

  case MachineOperand::MO_FPImmediate:
    O << MO.getFPImm();
    break;

  case MachineOperand::MO_MachineBasicBlock:
    O << *MO.getMBB()->getSymbol();
    break;

  case MachineOperand::MO_GlobalAddress:
    O << *getSymbol(MO.getGlobal());
    break;

  case MachineOperand::MO_BlockAddress: {
    MCSymbol *BA = GetBlockAddressSymbol(MO.getBlockAddress());
    O << BA->getName();
    break;
  }

  case MachineOperand::MO_ExternalSymbol:
    O << *GetExternalSymbolSymbol(MO.getSymbolName());
    break;

  case MachineOperand::MO_JumpTableIndex:
  case MachineOperand::MO_ConstantPoolIndex:
  default:
    llvm_unreachable("<unknown operand type>");
  }
}

bool pISAAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                     const char *ExtraCode, raw_ostream &O) {
  if (ExtraCode && ExtraCode[0])
    return true; // Invalid instruction - pISA does not have special modifiers

  printOperand(MI, OpNo, O);
  return false;
}

void pISAAsmPrinter::outputInstruction(const MachineInstr *MI) {
  pISAMCInstLower MCInstLowering{OutContext, *TRI, *RegMgr};
  pISAMCInst Inst;
  DI.attachDbgLoc(*MI, Inst);
  MCInstLowering.lower(MI, Inst);
  if (MI->getOpcode() == pISA::DBG_VALUE) {
    DI.emitDbgValue(*MI, Inst, OutStreamer);
    return;
  }
  OutStreamer->emitInstruction(Inst, *OutContext.getSubtargetInfo());
}

void pISAAsmPrinter::emitInstruction(const MachineInstr *MI) {
  pISA_MC::verifyInstructionPredicates(MI->getOpcode(),
                                       getSubtargetInfo().getFeatureBits());

  if (!TII->isNoEmissionInstr(*MI))
    outputInstruction(MI);
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializepISAAsmPrinter() {
  RegisterAsmPrinter<pISAAsmPrinter> Y(getThepISATarget());
}
