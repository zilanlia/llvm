//===-- pISAInstrInfo.h - pISA Instruction Information -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the pISA implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_pISAINSTRINFO_H
#define LLVM_LIB_TARGET_pISA_pISAINSTRINFO_H

#include "pISARegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "pISAGenInstrInfo.inc"

namespace llvm {

class pISAInstrInfo : public pISAGenInstrInfo {
  const pISARegisterInfo RI;

public:
  pISAInstrInfo();

  const pISARegisterInfo &getRegisterInfo() const { return RI; }
  bool isNoEmissionInstr(const MachineInstr& MI) const;
  bool isFunctionParamInstr(const MachineInstr &MI) const;
  bool isLoadParamInstr(const MachineInstr &MI) const;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify = false) const override;

  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;

  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        const DebugLoc &DL,
                        int *BytesAdded = nullptr) const override;

  bool reverseBranchCondition(
    SmallVectorImpl<MachineOperand>& Cond) const override;

  bool expandPostRAPseudo(MachineInstr& MI) const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                   const DebugLoc &DL, MCRegister DestReg, MCRegister SrcReg,
                   bool KillSrc) const override;
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_pISA_pISAINSTRINFO_H
