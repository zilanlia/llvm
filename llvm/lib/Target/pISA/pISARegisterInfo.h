//===-- pISARegisterInfo.h - pISA Register Information -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the pISA implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_pISAREGISTERINFO_H
#define LLVM_LIB_TARGET_pISA_pISAREGISTERINFO_H

#include "pISADefines.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "pISAGenRegisterInfo.inc"

namespace llvm {

class pISARegisterInfo : public pISAGenRegisterInfo {
public:
  pISARegisterInfo();
  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  BitVector getReservedRegs(const MachineFunction &MF) const override;
  bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override {
    llvm_unreachable("unexpected execution");
  }
  Register getFrameRegister(const MachineFunction &MF) const override {
    return 0;
  }
  const TargetRegisterClass* getRegClassFromLLT(LLT Ty) const;
  unsigned getNumEltsFromRegClass(const TargetRegisterClass *RC) const;
  unsigned getBitSizeFromRegClass(const TargetRegisterClass *RC) const;
  const TargetRegisterClass *getVectorRegClass(unsigned NumElts,
                                               unsigned BitSize) const;
  unsigned getSubRegIdx(unsigned Size, unsigned Elt) const;

  pISA::Swizzle getSwizzle(unsigned SubReg) const;
  static bool isSelectorSwizzle(pISA::Swizzle Swizzle);

  bool shouldCoalesce(MachineInstr* MI, const TargetRegisterClass* SrcRC,
    unsigned SubReg, const TargetRegisterClass* DstRC,
    unsigned DstSubReg, const TargetRegisterClass* NewRC,
    LiveIntervals& LIS) const override;

private:
  unsigned getScalarBitSize(
    const TargetRegisterClass *RC, unsigned NumElts) const;

  struct RegClassDescription {
    unsigned NumElements;
    unsigned ScalarBitSize;
  };

  // Maps TargetRegisterClass -> RegClassDescription
  DenseMap<const TargetRegisterClass *, std::unique_ptr<RegClassDescription>>
      RegClassMap;
  // Maps <NumElts, BitSize> -> TargetRegisterClass (vector reg class)
  DenseMap<std::pair<unsigned, unsigned>, const TargetRegisterClass *>
      VecRegClassMap;

  // subreg -> swizzle. These are optional as some subregisters don't map
  // to valid swizzles.
  std::array<std::optional<pISA::Swizzle>, 25> SwizzleMap;
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_pISA_pISAREGISTERINFO_H
