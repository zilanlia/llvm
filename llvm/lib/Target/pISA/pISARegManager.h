//===-- pISARegManager.h - Output pISA MCInsts as ASM -------*- C++ -*-==----=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_pISAREGMANAGER_H
#define LLVM_LIB_TARGET_pISA_pISAREGMANAGER_H

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/ADT/MapVector.h"
#include "MCTargetDesc/pISARegEncoder.h"

namespace llvm {
namespace pISA {

class RegManager : public RegEncoder {
public:
  enum Usage {
    None          = 0,
    NoEmissionDef = (1 << 0)
  };
  struct RegInfo {
    RegType Type;
    unsigned Idx;
    Usage Flags;
  };
public:
  RegManager(const MachineFunction& MF);
  unsigned getRegIdx(Register Reg) const;
  unsigned encodeVirtualRegister(Register Reg) const;
  auto mapping() const {
    return make_range(Mapping.begin(), Mapping.end());
  }
  bool exists(Register Reg) const { return Mapping.count(Reg) > 0; }

private:
  MapVector<Register, RegInfo> Mapping;
  const MachineFunction &MF;
  const MachineRegisterInfo& MRI;
  void computeMapping();
};

} // namespace pISA
} // namespace llvm

#endif // LLVM_LIB_TARGET_pISA_pISAREGMANAGER_H
