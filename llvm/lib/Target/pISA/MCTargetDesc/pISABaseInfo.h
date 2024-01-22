//===-- pISABaseInfo.h - Top level pISA definitions -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_pISABASEINFO_H
#define LLVM_LIB_TARGET_pISA_pISABASEINFO_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include <string>

namespace llvm {

// Return a string representation of the operands from startIndex onwards.
// Templated to allow both MachineInstr and MCInst to use the same logic.
template <class InstType>
std::string getpISAStringOperand(const InstType &MI, unsigned StartIndex) {
  std::string s; // Iteratively append to this string.

  const unsigned NumOps = MI.getNumOperands();
  bool IsFinished = false;
  for (unsigned i = StartIndex; i < NumOps && !IsFinished; ++i) {
    const auto &Op = MI.getOperand(i);
    if (!Op.isImm()) // Stop if we hit a register operand.
      break;
    assert((Op.getImm() >> 32) == 0 && "Imm operand should be i32 word");
    const uint32_t Imm = Op.getImm(); // Each i32 word is up to 4 characters.
    for (unsigned ShiftAmount = 0; ShiftAmount < 32; ShiftAmount += 8) {
      char c = (Imm >> ShiftAmount) & 0xff;
      if (c == 0) { // Stop if we hit a null-terminator character.
        IsFinished = true;
        break;
      }
      s += c; // Otherwise, append the character to the result string.
    }
  }
  return s;
}
} // namespace llvm
#endif // LLVM_LIB_TARGET_pISA_pISABASEINFO_H
