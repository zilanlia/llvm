//=====- pISATargetStreamer.cpp - pISATargetStreamer class ------------=====//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the pISATargetStreamer class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/pISATargetStreamer.h"
#include "MCTargetDesc/pISAMCTargetDesc.h"
#include "MCTargetDesc/pISARegEncoder.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/CodeGen/LowLevelType.h"

using namespace llvm;

static StringRef getCallingConvRepr(CallingConv::ID CC) {
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

static void emitTypeString(uint32_t TypeSizeInBits, uint32_t NumElts,
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

static void emitTypeString(const LLT &T, raw_ostream &OS) {
  auto TypeBitSize = T.getScalarType().getSizeInBits();
  auto NumElts = T.isVector() ? T.getNumElements() : 1;
  emitTypeString(TypeBitSize, NumElts, OS);
}

namespace {

class pISATargetAsmStreamer final : public pISATargetStreamer {
  formatted_raw_ostream &OS;

public:
  pISATargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS)
      : pISATargetStreamer(S), OS(OS) {}
  ~pISATargetAsmStreamer() override;

  void emitFunctionSignature(pISA::FunctionSignature &Sig) override {
    if (Sig.DN.CC == CallingConv::PISA_FUNC) {
      if (Sig.DN.Linkage)
        OS << ((*Sig.DN.Linkage) ? ".export " : ".import ");
    }

    OS << getCallingConvRepr(Sig.DN.CC) << " ";

    if (Sig.DN.CC == CallingConv::PISA_FUNC) {
      if (Sig.DN.RetLLT) {
        emitTypeString(*Sig.DN.RetLLT, OS);
      } else
        OS << "void";
    }

    OS << " @";
    // FIXME: Remove dependency on LLVM IR printer.
    printLLVMNameWithoutPrefix(OS, Sig.DN.Name);

    OS << "(";
    if (Sig.DN.CC == CallingConv::PISA_FUNC) {
      const char *Sep = "";
      for (auto &Param : Sig.FunctionParams) {
        OS << Sep << ".reg ";
        emitTypeString(Param.Ty, OS);
        OS << " " << Param.Prefix << Param.Idx;
        Sep = ", ";
      }
    } else {
      const char *Sep = "";
      unsigned i = 0;
      for (auto &Param : Sig.KernelParams) {
        OS << Sep << ".param[" << Param.Size << "] "
           << "%arg" << i++;
        Sep = ", ";
      }
    }
    OS << ")";

    if (Sig.DbgId)
      OS << " !dbg !" << *Sig.DbgId;

    OS << "\n";
  }

  void emitRegDcls(pISA::RegDcls &Dcls) override {
    constexpr unsigned DefsPerLine = 5;
    for (const auto &I : Dcls.Regs) {
      auto [NumElts, BitWidth, RegType] = I.first;
      ArrayRef<std::pair<const char *, unsigned>> Registers{I.second};

      auto nextChunk = [&](size_t N, size_t M) {
        if (N + M > Registers.size())
          M = Registers.size() - N;
        return Registers.slice(N, M);
      };

      unsigned CurLoc = 0;
      ArrayRef CurDecls = nextChunk(CurLoc, DefsPerLine);
      while (!CurDecls.empty()) {
        switch (RegType) {
        case pISA::RegEncoder::PRED:
          OS << "\t.pred ";
          break;
        case pISA::RegEncoder::REG:
          OS << "\t.reg ";
          emitTypeString(BitWidth, NumElts, OS);
          OS << " ";
          break;
        default:
          llvm_unreachable("unknown reg type!");
        }
        unsigned int Idx = 0;
        for (auto &D : CurDecls) {
          OS << D.first << D.second;
          if (++Idx < CurDecls.size())
            OS << ", ";
        }
        OS << ";\n";
        CurLoc += CurDecls.size();
        CurDecls = nextChunk(CurLoc, DefsPerLine);
      }
    }
  }

  void emitStackVariableDcls(pISA::StackVariableDcls &Dcls) override {
    unsigned i = 0;
    for (auto &V : Dcls.Vars) {
      OS << '\t' << (V.StackId ? ".shared" : ".private");
      OS << " .align " << V.Alignment.value();
      OS << " .8b @R" << i++ << '[' << V.Size << "];\n";
    }
  }

  void emitFuncBodyStart() override { OS << "{\n"; }
  void emitFuncBodyEnd() override { OS << "}\n"; }
};

class pISATargetELFStreamer final : public pISATargetStreamer {
public:
  pISATargetELFStreamer(MCStreamer &S) : pISATargetStreamer(S) {}
  ~pISATargetELFStreamer() override;

  void emitFunctionSignature(pISA::FunctionSignature &) override {
    llvm_unreachable("Not implemented yet!");
  }
  void emitRegDcls(pISA::RegDcls &) override {
    llvm_unreachable("Not implemented yet!");
  }
  void emitStackVariableDcls(pISA::StackVariableDcls &) override {
    llvm_unreachable("Not implemented yet!");
  }
  void emitFuncBodyStart() override {
    llvm_unreachable("Not implemented yet!");
  }
  void emitFuncBodyEnd() override { llvm_unreachable("Not implemented yet!"); }
};

class pISATargetNullStreamer final : public pISATargetStreamer {
public:
  pISATargetNullStreamer(MCStreamer &S) : pISATargetStreamer(S) {}
  ~pISATargetNullStreamer() override;

  void emitFunctionSignature(pISA::FunctionSignature &) override {}
  void emitRegDcls(pISA::RegDcls &) override {}
  void emitStackVariableDcls(pISA::StackVariableDcls &) override {}
  void emitFuncBodyStart() override {}
  void emitFuncBodyEnd() override {}
};

} // namespace

pISATargetStreamer::~pISATargetStreamer() = default;
pISATargetAsmStreamer::~pISATargetAsmStreamer() = default;
pISATargetELFStreamer::~pISATargetELFStreamer() = default;
pISATargetNullStreamer::~pISATargetNullStreamer() = default;

MCTargetStreamer *llvm::createpISAAsmTargetStreamer(MCStreamer &S,
                                                    formatted_raw_ostream &OS,
                                                    MCInstPrinter *InstPrinter,
                                                    bool IsVerboseAsm) {
  return new pISATargetAsmStreamer(S, OS);
}

MCTargetStreamer *
llvm::createpISAObjectTargetStreamer(MCStreamer &S,
                                     const MCSubtargetInfo &STI) {
  const Triple &TT = STI.getTargetTriple();
  if (TT.isOSBinFormatELF())
    return new pISATargetELFStreamer(S);
  return nullptr;
}

MCTargetStreamer *llvm::createpISANullTargetStreamer(MCStreamer &S) {
  return new pISATargetNullStreamer(S);
}
