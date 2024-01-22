//===- pISARegisterBankInfo.cpp ------------------------------*- C++ -*---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the targeting of the RegisterBankInfo class for pISA.
//
//===----------------------------------------------------------------------===//

#include "pISARegisterBankInfo.h"
#include "pISARegisterInfo.h"
#include "llvm/CodeGen/RegisterBank.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

#define GET_REGINFO_ENUM
#include "pISAGenRegisterInfo.inc"

#define GET_TARGET_REGBANK_IMPL
#include "pISAGenRegisterBank.inc"

using namespace llvm;

const RegisterBankInfo::InstructionMapping&
pISARegisterBankInfo::getInstrMapping(const MachineInstr& MI) const {
  const RegisterBankInfo::InstructionMapping& Mapping = getInstrMappingImpl(MI);

  if (Mapping.isValid())
    return Mapping;

  const MachineFunction& MF = *MI.getParent()->getParent();
  const MachineRegisterInfo& MRI = MF.getRegInfo();
  const TargetRegisterInfo* TRI = MRI.getTargetRegisterInfo();

  SmallVector<const ValueMapping*, 8> OpdsMapping(MI.getNumOperands());

  for (unsigned Idx = 0; Idx < MI.getNumOperands(); ++Idx) {
    auto& MO = MI.getOperand(Idx);

    if (MO.isReg() && MO.getReg().isValid()) {
      unsigned Size = getSizeInBits(MO.getReg(), MRI, *TRI);
      OpdsMapping[Idx] = &getValueMapping(0, Size, pISA::GRFRegBank);
    }
  }

  return getInstructionMapping(DefaultMappingID, 1,
                               getOperandsMapping(OpdsMapping),
                               MI.getNumOperands());
}

const RegisterBank &
pISARegisterBankInfo::getRegBankFromRegClass(const TargetRegisterClass & /* RC */,
                                              LLT /* Ty */) const {
  return pISA::GRFRegBank;
}
