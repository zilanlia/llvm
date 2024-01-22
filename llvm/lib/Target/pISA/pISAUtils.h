//===--- pISAUtils.h ---- pISA Utility Functions -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains miscellaneous utility functions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_pISAUTILS_H
#define LLVM_LIB_TARGET_pISA_pISAUTILS_H

#include "MCTargetDesc/pISABaseInfo.h"
#include "llvm/IR/IRBuilder.h"
#include <string>

namespace llvm {
class MachineInstr;
class MCInst;
class MachineInstrBuilder;
class StringRef;

// Add the given string as a series of integer operand, inserting null
// terminators and padding to make sure the operands all have 32-bit
// little-endian words.
void addStringImm(const StringRef &Str, MCInst &Inst);
void addStringImm(const StringRef &Str, MachineInstrBuilder &MIB);
void addStringImm(const StringRef &Str, IRBuilder<> &B,
                  std::vector<Value *> &Args);

// Read the series of integer operands back as a null-terminated string using
// the reverse of the logic in addStringImm.
std::string getStringImm(const MachineInstr &MI, unsigned StartIndex);

// Add the given numerical immediate to MIB.
void addNumImm(const APInt &Imm, MachineInstrBuilder &MIB);
} // namespace llvm
#endif // LLVM_LIB_TARGET_pISA_pISAUTILS_H
