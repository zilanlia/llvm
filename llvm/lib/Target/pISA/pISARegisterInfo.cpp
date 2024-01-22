//===-- pISARegisterInfo.cpp - pISA Register Information -----*- C++ -*-===//
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

#include "pISARegisterInfo.h"
#include "pISA.h"
#include "pISASubtarget.h"
#include "llvm/CodeGen/MachineFunction.h"

#define GET_REGINFO_TARGET_DESC
#include "pISAGenRegisterInfo.inc"
using namespace llvm;

static std::array<std::optional<pISA::Swizzle>,
                  pISA::NUM_TARGET_SUBREGS> getSwizzleMap() {
  using namespace pISA;
  static_assert(NUM_TARGET_SUBREGS == 25, "updated needed!");
  std::array<std::optional<Swizzle>, NUM_TARGET_SUBREGS> Swizzles{};
  Swizzles[NoSubRegister]           = Swizzle::NONE;
  Swizzles[sub8_0]                  = Swizzle::X;
  Swizzles[sub8_1]                  = Swizzle::Y;
  Swizzles[sub8_2]                  = Swizzle::Z;
  Swizzles[sub8_3]                  = Swizzle::W;
  Swizzles[sub16_0]                 = Swizzle::X;
  Swizzles[sub16_1]                 = Swizzle::Y;
  Swizzles[sub16_2]                 = Swizzle::Z;
  Swizzles[sub16_3]                 = Swizzle::W;
  Swizzles[sub32_0]                 = Swizzle::X;
  Swizzles[sub32_1]                 = Swizzle::Y;
  Swizzles[sub32_2]                 = Swizzle::Z;
  Swizzles[sub32_3]                 = Swizzle::W;
  Swizzles[sub64_0]                 = Swizzle::X;
  Swizzles[sub64_1]                 = Swizzle::Y;
  Swizzles[sub64_2]                 = Swizzle::Z;
  Swizzles[sub64_3]                 = Swizzle::W;
  Swizzles[sub32_0_sub32_1]         = Swizzle::XY;
  Swizzles[sub16_0_sub16_1]         = Swizzle::XY;
  Swizzles[sub8_0_sub8_1]           = Swizzle::XY;
  Swizzles[sub8_0_sub8_1_sub8_2]    = std::nullopt;
  Swizzles[sub16_0_sub16_1_sub16_2] = std::nullopt;
  Swizzles[sub32_0_sub32_1_sub32_2] = std::nullopt;
  Swizzles[sub64_0_sub64_1]         = std::nullopt;
  Swizzles[sub64_0_sub64_1_sub64_2] = std::nullopt;
  return Swizzles;
}

bool pISARegisterInfo::shouldCoalesce(
  MachineInstr* MI, const TargetRegisterClass* SrcRC, unsigned SubReg,
  const TargetRegisterClass* DstRC, unsigned DstSubReg,
  const TargetRegisterClass* NewRC, LiveIntervals& LIS) const {

  if (!MI->isCopy())
    return false;

  auto isLegalSwizzle = [&](unsigned Subreg) {
    auto Swizzle = SwizzleMap[Subreg];
    return Swizzle.has_value();
  };

  return isLegalSwizzle(SubReg) && isLegalSwizzle(DstSubReg);
}

pISARegisterInfo::pISARegisterInfo() : pISAGenRegisterInfo(pISA::DummyReg) {
  // Tablegen can sometimes synthesize register classes if you don't set
  // subregs explicitly on some regs. For now at least, we probably want to
  // be explicit about what regclasses exist. If you added a register class
  // explicitly, go ahead and update this number. If not, you might want to
  // figure out what happened.
  static_assert(std::size(RegisterClasses) == 16);
  unsigned NumRCs = getNumRegClasses();
  for (unsigned i = 0; i < NumRCs; i++) {
    auto *RC = getRegClass(i);
    auto RCD = std::make_unique<RegClassDescription>();
    RCD->NumElements = RC->LaneMask.getNumLanes();
    RCD->ScalarBitSize = getScalarBitSize(RC, RCD->NumElements);
    VecRegClassMap[std::make_pair(RCD->NumElements, RCD->ScalarBitSize)] = RC;
    RegClassMap[RC] = std::move(RCD);
  }

  SwizzleMap = getSwizzleMap();
}

BitVector pISARegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  return BitVector(getNumRegs(), true);
}

const MCPhysReg *
pISARegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  static const MCPhysReg CalleeSavedReg = {0};
  return &CalleeSavedReg;
}

unsigned
pISARegisterInfo::getScalarBitSize(
  const TargetRegisterClass *RC, unsigned NumElts) const {
  unsigned RegSizeInBits = getRegSizeInBits(*RC);
  assert((RegSizeInBits % NumElts == 0) && "not divisible?");
  return RegSizeInBits / NumElts;
}

unsigned pISARegisterInfo::getSubRegIdx(unsigned Size, unsigned Idx) const {
  assert(Idx < 4 && "Only 4 sub-registers supported!");

  static unsigned Subregs[4][4] = {
      {pISA::sub8_0, pISA::sub8_1, pISA::sub8_2, pISA::sub8_3},
      {pISA::sub16_0, pISA::sub16_1, pISA::sub16_2, pISA::sub16_3},
      {pISA::sub32_0, pISA::sub32_1, pISA::sub32_2, pISA::sub32_3},
      {pISA::sub64_0, pISA::sub64_1, pISA::sub64_2, pISA::sub64_3}};

  switch (Size) {
  case 8:
    return Subregs[0][Idx];
  case 16:
    return Subregs[1][Idx];
  case 32:
    return Subregs[2][Idx];
  case 64:
    return Subregs[3][Idx];
  default:
    assert(0 && "unknown type size!");
    break;
  }
  return 0;
}

pISA::Swizzle pISARegisterInfo::getSwizzle(unsigned SubReg) const {
  auto Swizzle = SwizzleMap[SubReg];
  assert(Swizzle.has_value() && "invalid swizzle!");
  return *Swizzle;
}

bool pISARegisterInfo::isSelectorSwizzle(pISA::Swizzle Swizzle) {
  switch (Swizzle) {
  case pISA::Swizzle::X:
  case pISA::Swizzle::Y:
  case pISA::Swizzle::Z:
  case pISA::Swizzle::W:
    return true;
  default:
    return false;
  }
}

const TargetRegisterClass *pISARegisterInfo::getRegClassFromLLT(LLT Ty) const {
  if (Ty.isScalar() || Ty.isPointer()) {
    switch (Ty.getSizeInBits()) {
    case 1:
      return &pISA::PredRegClass;
    case 8:
      return &pISA::Reg8bRegClass;
    case 16:
      return &pISA::Reg16bRegClass;
    case 32:
      return &pISA::Reg32bRegClass;
    case 64:
      return &pISA::Reg64bRegClass;
    default:
      break;
    }
  } else if (Ty.isVector()) {
    unsigned NumElts = Ty.getNumElements();
    unsigned BitSize = Ty.getScalarSizeInBits();
    return getVectorRegClass(NumElts, BitSize);
  }

  llvm_unreachable("unhandled LLT!");
}

unsigned
pISARegisterInfo::getNumEltsFromRegClass(const TargetRegisterClass *RC) const {
  auto I = RegClassMap.find(RC);
  assert(I != RegClassMap.end());
  auto &RCD = I->second;
  return RCD->NumElements;
}

unsigned
pISARegisterInfo::getBitSizeFromRegClass(const TargetRegisterClass *RC) const {
  auto I = RegClassMap.find(RC);
  assert(I != RegClassMap.end());
  auto &RCD = I->second;
  return RCD->ScalarBitSize;
}

const TargetRegisterClass *
pISARegisterInfo::getVectorRegClass(unsigned NumElts, unsigned BitSize) const {
  auto P = std::make_pair(NumElts, BitSize);
  auto I = VecRegClassMap.find(P);
  assert(I != VecRegClassMap.end());
  return I->second;
}