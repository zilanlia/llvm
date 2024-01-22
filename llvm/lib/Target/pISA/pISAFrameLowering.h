//===-- pISAFrameLowering.h - Define frame lowering for pISA -*- C++-*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class implements pISA-specific bits of TargetFrameLowering class.
// The target uses only virtual registers. It does not operate with stack frame
// explicitly and does not generate prologues/epilogues of functions.
// As a result, we are not required to implemented the frame lowering
// functionality substantially.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_pISAFRAMELOWERING_H
#define LLVM_LIB_TARGET_pISA_pISAFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Support/Alignment.h"

namespace llvm {
class pISASubtarget;

class pISAFrameLowering : public TargetFrameLowering {
public:
  explicit pISAFrameLowering(const pISASubtarget &sti)
      : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(8), 0) {}

  void emitPrologue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override {}
  void emitEpilogue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override {}

  bool hasFP(const MachineFunction &MF) const override { return false; }

  bool isSupportedStackID(TargetStackID::Value ID) const override {
    switch (ID) {
    default:
      return false;
    case TargetStackID::Default:
    case TargetStackID::pISAShared:
      return true;
    }
  }
};
} // namespace llvm
#endif // LLVM_LIB_TARGET_pISA_pISAFRAMELOWERING_H
