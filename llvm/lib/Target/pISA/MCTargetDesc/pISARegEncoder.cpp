//===-- pISARegEncoder.cpp - Output pISA MCInsts as ASM -----*- C++ -*-==----=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Contains methods for encoding and decoding virtual registers for printing
//
//===----------------------------------------------------------------------===//

#include "pISARegEncoder.h"

using namespace llvm;
using namespace pISA;

RegEncoder::RegType RegEncoder::getRegType(uint8_t TSFlags) {
  return static_cast<RegEncoder::RegType>(TSFlags & 0x3);
}

RegEncoder::RegType RegEncoder::getRegType(const TargetRegisterClass* RC) {
  return getRegType(RC->TSFlags);
}

const char* RegEncoder::getRegPrefix(RegType Type) {
  switch (Type) {
  case RegType::REG:
    return "%r";
  case RegType::PRED:
    return "%p";
  default:
    llvm_unreachable("unhandled regclass!");
  }
}

const char* RegEncoder::getRegPrefix(const TargetRegisterClass* RC) {
  return getRegPrefix(getRegType(RC));
}

static constexpr unsigned NumRegBits = 29;
static_assert(RegEncoder::NUM_TYPE < 4, "need to update encoding");

unsigned RegEncoder::encodeVirtualRegister(unsigned Idx, RegType Type) {
  return Register::index2VirtReg(
    (Type << NumRegBits) | (Idx & ((1U << NumRegBits) - 1)));
}

std::pair<const char*, unsigned>
RegEncoder::decodeVirtualRegister(MCRegister Reg) {
  const char* Name = getRegPrefix(getRegType(Reg >> NumRegBits));
  unsigned Num = Reg & ((1U << NumRegBits) - 1);
  return std::make_pair(Name, Num);
}

