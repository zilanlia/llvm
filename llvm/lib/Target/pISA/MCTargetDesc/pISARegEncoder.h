//===-- pISARegEncoder.h - Output pISA MCInsts as ASM -------*- C++ -*-==----=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_MCTARGETDESC_pISAREGENCODER_H
#define LLVM_LIB_TARGET_pISA_MCTARGETDESC_pISAREGENCODER_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

namespace llvm {
namespace pISA {

class RegEncoder {
public:
  enum RegType {
    NONE,
    // Keep in sync with TSFlags in pISARegisterInfo.td
    REG,
    PRED,
    NUM_TYPE
  };
public:
  static const char* getRegPrefix(const TargetRegisterClass* RC);
  static const char* getRegPrefix(RegType Type);
  static unsigned encodeVirtualRegister(unsigned Idx, RegType Type);
  static std::pair<const char*, unsigned> decodeVirtualRegister(MCRegister Reg);
protected:
  static RegEncoder::RegType getRegType(uint8_t TSFlags);
  static RegEncoder::RegType getRegType(const TargetRegisterClass* RC);
};

} // namespace pISA
} // namespace llvm

#endif // LLVM_LIB_TARGET_pISA_MCTARGETDESC_pISAREGENCODER_H
