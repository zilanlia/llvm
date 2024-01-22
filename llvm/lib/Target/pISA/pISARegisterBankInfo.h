//===- pISARegisterBankInfo.h -----------------------------------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the targeting of the RegisterBankInfo class for pISA.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_pISAREGISTERBANKINFO_H
#define LLVM_LIB_TARGET_pISA_pISAREGISTERBANKINFO_H

#include "llvm/CodeGen/RegisterBankInfo.h"

#define GET_REGBANK_DECLARATIONS
#include "pISAGenRegisterBank.inc"

namespace llvm {

class TargetRegisterInfo;

class pISAGenRegisterBankInfo : public RegisterBankInfo {
protected:
#define GET_TARGET_REGBANK_CLASS
#include "pISAGenRegisterBank.inc"
};

// This class provides the information for the target register banks.
class pISARegisterBankInfo final : public pISAGenRegisterBankInfo {
public:
  const RegisterBank &getRegBankFromRegClass(const TargetRegisterClass &RC,
                                             LLT Ty) const override;

  const InstructionMapping &
  getInstrMapping(const MachineInstr &MI) const override;
};
} // namespace llvm
#endif // LLVM_LIB_TARGET_pISA_pISAREGISTERBANKINFO_H
