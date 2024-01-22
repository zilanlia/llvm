//===-- pISARegManager.cpp - Output pISA MCInsts as ASM -----*- C++ -*-==----=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class manages the printing of virtual registers. It has two main
// functions
// 
// 1) Determine the register prefix for each register
// 2) renumber registers so they are tightly packed
//
//===----------------------------------------------------------------------===//

#include "pISARegManager.h"
#include "pISAInstrInfo.h"
#include "pISASubtarget.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;
using namespace pISA;

void RegManager::computeMapping() {
  std::array<unsigned, RegType::NUM_TYPE> Count{};
  auto *TII  = MF.getSubtarget<pISASubtarget>().getInstrInfo();
  for (auto& MBB : MF) {
    for (auto& MI : MBB) {
      for (auto &MO : MI.operands()) {
        if (!MO.isReg() || (!MO.isDef() && !MO.isUndef()))
          continue;

        Register CurReg = MO.getReg();

        if (CurReg.isPhysical())
          continue;

        if (Mapping.count(CurReg) != 0)
          continue;

        unsigned Flags = Usage::None;
        if (TII->isNoEmissionInstr(MI))
          Flags |= Usage::NoEmissionDef;

        auto Type = getRegType(MRI.getRegClass(MO.getReg()));
        RegInfo Info{ Type, Count[Type]++, static_cast<Usage>(Flags) };
        Mapping[CurReg] = Info;
      }
    }
  }
}

unsigned RegManager::getRegIdx(Register Reg) const {
  auto I = Mapping.find(Reg);
  assert(I != Mapping.end() && "missing?");
  return I->second.Idx;
}

unsigned RegManager::encodeVirtualRegister(Register Reg) const {
  auto& MRI = MF.getRegInfo();
  unsigned Idx = getRegIdx(Reg);
  auto Type = getRegType(MRI.getRegClass(Reg));
  return RegEncoder::encodeVirtualRegister(Idx, Type);
}

RegManager::RegManager(const MachineFunction& MF) :
  MF(MF), MRI(MF.getRegInfo()) {
  computeMapping();
}
