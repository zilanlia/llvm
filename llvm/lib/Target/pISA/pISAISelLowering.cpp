//===- pISAISelLowering.cpp - pISA DAG Lowering Impl ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the pISATargetLowering class.
//
//===----------------------------------------------------------------------===//

#include "pISAISelLowering.h"
#include "pISASubtarget.h"
#include "pISA.h"
#include "llvm/IR/IntrinsicspISA.h"

#define DEBUG_TYPE "pISA-lower"

using namespace llvm;

unsigned pISATargetLowering::getNumRegistersForCallingConv(
    LLVMContext &Context, CallingConv::ID CC, EVT VT) const {
  // This code avoids CallLowering fail inside getVectorTypeBreakdown
  // on v3i1 arguments. Maybe we need to return 1 for all types.
  // TODO: remove it once this case is supported by the default implementation.
  if (VT.isVector() && VT.getVectorNumElements() == 3 &&
      (VT.getVectorElementType() == MVT::i1 ||
       VT.getVectorElementType() == MVT::i8))
    return 1;
  return getNumRegisters(Context, VT);
}

MVT pISATargetLowering::getRegisterTypeForCallingConv(LLVMContext &Context,
                                                       CallingConv::ID CC,
                                                       EVT VT) const {
  // This code avoids CallLowering fail inside getVectorTypeBreakdown
  // on v3i1 arguments. Maybe we need to return i32 for all types.
  // TODO: remove it once this case is supported by the default implementation.
  if (VT.isVector() && VT.getVectorNumElements() == 3) {
    if (VT.getVectorElementType() == MVT::i1)
      return MVT::v4i1;
    else if (VT.getVectorElementType() == MVT::i8)
      return MVT::v4i8;
  }
  return getRegisterType(Context, VT);
}

bool pISATargetLowering::getTgtMemIntrinsic(IntrinsicInfo &Info,
                                            const CallInst &I,
                                            MachineFunction &MF,
                                            unsigned Intrinsic) const {
  return false;
}

LLT pISATargetLowering::getOptimalMemOpLLT(
    const MemOp &Op, const AttributeList &FuncAttributes) const {
  if (Op.size() >= 8 && Op.isAligned(Align(8)))
    return LLT::scalar(64);
  if (Op.size() >= 4 && Op.isAligned(Align(4)))
    return LLT::scalar(32);
  if (Op.size() >= 2 && Op.isAligned(Align(2)))
    return LLT::scalar(16);
  return LLT();
}

bool pISATargetLowering::useFTZ(const MachineFunction &MF) const {
  return MF.getDenormalMode(APFloat::IEEEsingle()).Output ==
         DenormalMode::PreserveSign;
}

pISATargetLowering::ConstraintType
pISATargetLowering::getConstraintType(StringRef Constraint) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    default:
      break;
    case 'b':
    case 'r':
    case 'h':
    case 'c':
    case 'l':
    case 'f':
    case 'd':
    case '0':
    case 'N':
      return C_RegisterClass;
    }
  }
  return TargetLowering::getConstraintType(Constraint);
}

std::pair<unsigned, const TargetRegisterClass *>
pISATargetLowering::getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                                                  StringRef Constraint,
                                                  MVT VT) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    case 'b':
      return std::make_pair(0U, &pISA::PredRegClass);
    case 'c':
      return std::make_pair(0U, &pISA::Reg16bRegClass);
    case 'h':
      return std::make_pair(0U, &pISA::Reg16bRegClass);
    case 'r':
      return std::make_pair(0U, &pISA::Reg32bRegClass);
    case 'l':
    case 'N':
      return std::make_pair(0U, &pISA::Reg64bRegClass);
    case 'f':
      return std::make_pair(0U, &pISA::Reg32bRegClass);
    case 'd':
      return std::make_pair(0U, &pISA::Reg64bRegClass);
    }
  }
  return TargetLowering::getRegForInlineAsmConstraint(TRI, Constraint, VT);
}

void pISATargetLowering::LowerAsmOperandForConstraint(SDValue Op,
                                                    StringRef Constraint,
                                                    std::vector<SDValue> &Ops,
                                                    SelectionDAG &DAG) const {
    TargetLowering::LowerAsmOperandForConstraint(Op, Constraint, Ops, DAG);
}
