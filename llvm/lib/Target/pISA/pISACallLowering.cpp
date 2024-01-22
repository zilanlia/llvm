//===--- pISACallLowering.cpp - Call lowering ------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the lowering of LLVM calls to machine code calls for
// GlobalISel.
//
//===----------------------------------------------------------------------===//

#include "pISACallLowering.h"
#include "MCTargetDesc/pISABaseInfo.h"
#include "pISA.h"
#include "pISAISelLowering.h"
#include "pISARegisterInfo.h"
#include "pISASubtarget.h"
#include "pISAUtils.h"
#include "llvm/CodeGen/FunctionLoweringInfo.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/Support/ModRef.h"

using namespace llvm;

pISACallLowering::pISACallLowering(const pISATargetLowering &TLI)
    : CallLowering(&TLI) {}

bool pISACallLowering::lowerReturn(MachineIRBuilder &MIRBuilder,
                                    const Value *Val, ArrayRef<Register> VRegs,
                                    FunctionLoweringInfo &FLI,
                                    Register SwiftErrorVReg) const {
  // FIXME: Currently the return support is only for registers.
  // Pending:
  //  - return immediates: fold immediates to return operand
  if (VRegs.size() > 1)
    return false;
  if (Val) {
    auto& DL = MIRBuilder.getDataLayout();
    const auto &STI = MIRBuilder.getMF().getSubtarget();
    unsigned Op = 0;
    auto Ty = Val->getType();
    if (Ty->isVectorTy()) {
      auto *VTy = cast<FixedVectorType>(Ty);
      unsigned NumElts = VTy->getNumElements();
      unsigned EltSize = DL.getTypeSizeInBits(Ty->getScalarType());
      switch (EltSize) {
      case 8:
        switch (NumElts) {
        case 2:
          Op = pISA::retValue_v2_8b_r;
          break;
        case 4:
          Op = pISA::retValue_v4_8b_r;
          break;
        default:
          llvm_unreachable("Unknown return vector size!");
          break;
        }
        break;
      case 16:
        switch (NumElts) {
        case 2:
          Op = pISA::retValue_v2_16b_r;
          break;
        case 3:
          Op = pISA::retValue_v3_16b_r;
          break;
        case 4:
          Op = pISA::retValue_v4_16b_r;
          break;
        default:
          llvm_unreachable("Unknown return vector size!");
          break;
        }
        break;
      case 32:
        switch (NumElts) {
        case 2:
          Op = pISA::retValue_v2_32b_r;
          break;
        case 3:
          Op = pISA::retValue_v3_32b_r;
          break;
        case 4:
          Op = pISA::retValue_v4_32b_r;
          break;
        default:
          llvm_unreachable("Unknown return vector size!");
          break;
        }
        break;
      case 64:
        switch (NumElts) {
        case 2:
          Op = pISA::retValue_v2_64b_r;
          break;
        case 3:
          Op = pISA::retValue_v3_64b_r;
          break;
        case 4:
          Op = pISA::retValue_v4_64b_r;
          break;
        default:
          llvm_unreachable("Unknown return vector size!");
          break;
        }
        break;
      default:
        llvm_unreachable("Unknown return size!");
        break;
      }
    } else {
      unsigned BitSize = DL.getTypeSizeInBits(Ty);
      switch (BitSize) {
      case 1:
        Op = pISA::retValue_p;
        break;
      case 8:
        Op = pISA::retValue_8b_r;
        break;
      case 16:
        Op = pISA::retValue_16b_r;
        break;
      case 32:
        Op = pISA::retValue_32b_r;
        break;
      case 64:
        Op = pISA::retValue_64b_r;
        break;
      default:
        llvm_unreachable("Unknown return size!");
        break;
      }
    }
    return MIRBuilder
        .buildInstr(Op)
        .addUse(VRegs[0])
        .constrainAllUses(MIRBuilder.getTII(), *STI.getRegisterInfo(),
                          *STI.getRegBankInfo());
  }
  MIRBuilder.buildInstr(pISA::ret);
  return true;
}

static ConstantInt *getConstInt(MDNode *MD, unsigned NumOp) {
  if (MD->getNumOperands() > NumOp) {
    auto *CMeta = dyn_cast<ConstantAsMetadata>(MD->getOperand(NumOp));
    if (CMeta)
      return dyn_cast<ConstantInt>(CMeta->getValue());
  }
  return nullptr;
}

// This code restores function args/retvalue types for composite cases
// because the final types should still be aggregate whereas they're i32
// during the translation to cope with aggregate flattening etc.
static FunctionType *getOriginalFunctionType(const Function &F) {
  auto *NamedMD = F.getParent()->getNamedMetadata("pisa.cloned_funcs");
  if (NamedMD == nullptr)
    return F.getFunctionType();

  Type *RetTy = F.getFunctionType()->getReturnType();
  SmallVector<Type *, 4> ArgTypes;
  for (auto &Arg : F.args())
    ArgTypes.push_back(Arg.getType());

  auto ThisFuncMDIt =
      std::find_if(NamedMD->op_begin(), NamedMD->op_end(), [&F](MDNode *N) {
        return isa<MDString>(N->getOperand(0)) &&
               cast<MDString>(N->getOperand(0))->getString() == F.getName();
      });
  // TODO: probably one function can have numerous type mutations,
  // so we should support this.
  if (ThisFuncMDIt != NamedMD->op_end()) {
    auto *ThisFuncMD = *ThisFuncMDIt;
    MDNode *MD = dyn_cast<MDNode>(ThisFuncMD->getOperand(1));
    assert(MD && "MDNode operand is expected");
    ConstantInt *Const = getConstInt(MD, 0);
    if (Const) {
      auto *CMeta = dyn_cast<ConstantAsMetadata>(MD->getOperand(1));
      assert(CMeta && "ConstantAsMetadata operand is expected");
      assert(Const->getSExtValue() >= -1);
      // Currently -1 indicates return value, greater values mean
      // argument numbers.
      if (Const->getSExtValue() == -1)
        RetTy = CMeta->getType();
      else
        ArgTypes[Const->getSExtValue()] = CMeta->getType();
    }
  }

  return FunctionType::get(RetTy, ArgTypes, F.isVarArg());
}

bool pISACallLowering::lowerFormalArguments(MachineIRBuilder &MIRBuilder,
                                            const Function &F,
                                            ArrayRef<ArrayRef<Register>> VRegs,
                                            FunctionLoweringInfo &FLI) const {

  auto MRI = MIRBuilder.getMRI();
  auto &MF = MIRBuilder.getMF();
  auto TRI = static_cast<const pISARegisterInfo *>(
      MF.getSubtarget().getRegisterInfo());

  bool IsKernel = (F.getCallingConv() != CallingConv::PISA_FUNC);
  int i = 0;
  unsigned Op = 0;
  for (const auto &Arg : F.args()) {
    assert(VRegs[i].size() == 1 && "Formal arg has multiple vregs");
    auto ArgType = Arg.getType();
    auto BitSize = ArgType->getScalarSizeInBits();
    if (ArgType->isIntegerTy()) {
      switch (BitSize) {
      case 8:
        MRI->setRegClass(VRegs[i][0], &pISA::Reg8bRegClass);
        Op = IsKernel ? pISA::loadParam_8b : pISA::functionParameter_8b;
        break;
      case 16:
        MRI->setRegClass(VRegs[i][0], &pISA::Reg16bRegClass);
        Op = IsKernel ? pISA::loadParam_16b : pISA::functionParameter_16b;
        break;
      case 32:
        MRI->setRegClass(VRegs[i][0], &pISA::Reg32bRegClass);
        Op = IsKernel ? pISA::loadParam_32b : pISA::functionParameter_32b;
        break;
      case 64:
        MRI->setRegClass(VRegs[i][0], &pISA::Reg64bRegClass);
        Op = IsKernel ? pISA::loadParam_64b : pISA::functionParameter_64b;
        break;
      default:
        assert(false && "Bit size for call arg not supported");
      }
    } else if (ArgType->isPointerTy()) {
      auto AS = ArgType->getPointerAddressSpace();
      auto pointerBitsize =
          F.getParent()->getDataLayout().getPointerSizeInBits(AS);
      if (pointerBitsize == 64) {
        MRI->setRegClass(VRegs[i][0], &pISA::Reg64bRegClass);
        Op = IsKernel ? pISA::loadParam_64b : pISA::functionParameter_64b;
      } else if (pointerBitsize == 32) {
        MRI->setRegClass(VRegs[i][0], &pISA::Reg32bRegClass);
        Op = IsKernel ? pISA::loadParam_32b : pISA::functionParameter_32b;
      } else {
        llvm_unreachable("unsupported pointer size");
      }
    } else if (ArgType->isHalfTy()) {
      MRI->setRegClass(VRegs[i][0], &pISA::Reg16bRegClass);
      Op = IsKernel ? pISA::loadParam_16b : pISA::functionParameter_16b;
    } else if (ArgType->isFloatTy()) {
      MRI->setRegClass(VRegs[i][0], &pISA::Reg32bRegClass);
      Op = IsKernel ? pISA::loadParam_32b : pISA::functionParameter_32b;
    } else if (ArgType->isDoubleTy()) {
      MRI->setRegClass(VRegs[i][0], &pISA::Reg64bRegClass);
      Op = IsKernel ? pISA::loadParam_64b : pISA::functionParameter_64b;
    } else if (ArgType->isVectorTy()) {
      auto VectorTy = cast<FixedVectorType>(ArgType);
      auto NumElts = VectorTy->getNumElements();
      BitSize = VectorTy->getScalarSizeInBits();
      MRI->setRegClass(VRegs[i][0], TRI->getVectorRegClass(NumElts, BitSize));
      switch (BitSize) {
      case 8:
        switch (NumElts) {
        case 2:
          Op = IsKernel ? pISA::loadParam_v2_8b : pISA::functionParameter_v2_8b;
          break;
        case 4:
          Op = IsKernel ? pISA::loadParam_v4_8b : pISA::functionParameter_v4_8b;
          break;
        default:
          llvm_unreachable("Vector size not supported");
        }
        break;
      case 16:
        switch (NumElts) {
        case 2:
          Op = IsKernel ? pISA::loadParam_v2_16b : pISA::functionParameter_v2_16b;
          break;
        case 3:
          assert(!IsKernel && "not supported!");
          Op = pISA::functionParameter_v3_16b;
          break;
        case 4:
          Op = IsKernel ? pISA::loadParam_v4_16b : pISA::functionParameter_v4_16b;
          break;
        default:
          llvm_unreachable("Vector size not supported");
        }
        break;
      case 32:
        switch (NumElts) {
        case 2:
          Op = IsKernel ? pISA::loadParam_v2_32b : pISA::functionParameter_v2_32b;
          break;
        case 3:
          Op = IsKernel ? pISA::loadParam_v3_32b : pISA::functionParameter_v3_32b;
          break;
        case 4:
          Op = IsKernel ? pISA::loadParam_v4_32b : pISA::functionParameter_v4_32b;
          break;
        default:
          llvm_unreachable("Vector size not supported");
        }
        break;
      case 64:
        switch (NumElts) {
        case 2:
          Op = IsKernel ? pISA::loadParam_v2_64b : pISA::functionParameter_v2_64b;
          break;
        case 3:
          assert(!IsKernel && "not supported!");
          Op = pISA::functionParameter_v3_64b;
          break;
        case 4:
          assert(!IsKernel && "not supported!");
          Op = pISA::functionParameter_v4_64b;
          break;
        default:
          llvm_unreachable("Vector size not supported");
        }
        break;
      default:
        assert(false && "Bit size for call arg not supported");
      }
    } else {
      llvm_unreachable("Arg Type not supported");
    }

    auto MIB = MIRBuilder.buildInstr(Op)
        .addDef(VRegs[i][0])
        .addImm(Arg.getArgNo());
    if (IsKernel) {
      // offset of the argument to be loaded
      // TODO: Currently support scalar argument only so must be 0
      MIB.addImm(0);
    }
    i++;
  }

  // TODO: Handle entry points and function linkage.
  return true;
}

bool pISACallLowering::lowerCall(MachineIRBuilder &MIRBuilder,
                                 CallLoweringInfo &Info) const {
  // Currently call returns should have single vregs.
  // TODO: handle the case of multiple registers.
  if (Info.OrigRet.Regs.size() > 1)
    return false;

  // Callee must be a PISA_FUNC
  if (Info.CallConv != CallingConv::PISA_FUNC)
    return false;

  // TODO: Assume functions are global (could be reg for func ptr call)
  assert(Info.Callee.isGlobal());
  const Function *CF = dyn_cast_or_null<const Function>(Info.Callee.getGlobal());
  if (CF == nullptr)
    return false;

  const MachineInstrBuilder *MIB = nullptr;
  if (!Info.OrigRet.Ty->isVoidTy()) {
    // select the call op according to return type and build MI
    auto RetLLT =
        llvm::getLLTForType(*Info.OrigRet.Ty, MIRBuilder.getDataLayout());
    unsigned CallOp = 0;
    if (RetLLT.isScalar() || RetLLT.isPointer()) {
      switch (RetLLT.getSizeInBits()) {
      case 8:  CallOp = pISA::functionCall_8b_r;  break;
      case 16: CallOp = pISA::functionCall_16b_r; break;
      case 32: CallOp = pISA::functionCall_32b_r; break;
      case 64: CallOp = pISA::functionCall_64b_r; break;
      default:
        llvm_unreachable("Unsupported function return type");
        break;
      }
    } else if (RetLLT.isVector()) {
      auto TypeBitSize = RetLLT.getElementType().getSizeInBits();
      auto NumElts = RetLLT.getNumElements();
      switch (TypeBitSize) {
      case 8:
        switch(NumElts) {
        case 2: CallOp = pISA::functionCall_v2_8b_r; break;
        case 4: CallOp = pISA::functionCall_v4_8b_r; break;
        default:
          llvm_unreachable("Vector size not supported");
        }
        break;
      case 16:
        switch(NumElts) {
        case 2: CallOp = pISA::functionCall_v2_16b_r; break;
        case 3: CallOp = pISA::functionCall_v3_16b_r; break;
        case 4: CallOp = pISA::functionCall_v4_16b_r; break;
        default:
          llvm_unreachable("Vector size not supported");
        }
        break;
      case 32:
        switch(NumElts) {
        case 2: CallOp = pISA::functionCall_v2_32b_r; break;
        case 3: CallOp = pISA::functionCall_v3_32b_r; break;
        case 4: CallOp = pISA::functionCall_v4_32b_r; break;
        default:
          llvm_unreachable("Vector size not supported");
        }
        break;
      case 64:
        switch(NumElts) {
        case 2: CallOp = pISA::functionCall_v2_64b_r; break;
        case 3: CallOp = pISA::functionCall_v3_64b_r; break;
        case 4: CallOp = pISA::functionCall_v4_64b_r; break;
        default:
          llvm_unreachable("Vector size not supported");
        }
        break;
      default:
        llvm_unreachable("Unsupported function return type");
        break;
      }
    } else {
      llvm_unreachable("Unsupported call return type");
    }
    // set Ret register class
    assert(!Info.OrigRet.Regs.empty());
    Register ResVReg = Info.OrigRet.Regs[0];
    auto TRI = static_cast<const pISARegisterInfo *>(
          MIRBuilder.getMF().getSubtarget().getRegisterInfo());
    MIRBuilder.getMRI()->setRegClass(ResVReg, TRI->getRegClassFromLLT(RetLLT));

    MIB = &MIRBuilder.buildInstr(CallOp).addDef(ResVReg).add(Info.Callee);
  } else {
    // void return
    MIB = &MIRBuilder.buildInstr(pISA::functionCall_void).add(Info.Callee);
  }

  // add function args into MI if any
  for (const auto &Arg : Info.OrigArgs) {
    // Currently call args should have single vregs.
    if (Arg.Regs.size() > 1)
      return false;
    MIB->addUse(Arg.Regs[0]);
  }

  return true;
}
