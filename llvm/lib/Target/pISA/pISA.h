//===-- pISA.h - Top-level interface for pISA representation -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_pISA_H
#define LLVM_LIB_TARGET_pISA_pISA_H

#include "pISADefines.h"
#include "MCTargetDesc/pISAMCTargetDesc.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class pISATargetMachine;
class pISASubtarget;
class InstructionSelector;
class RegisterBankInfo;

ModulePass *createpISAInputTranslatorPass();
FunctionPass *createpISAExpandIntrinsicsPass();
FunctionPass *createpISAEmitIntrinsicsPass();
InstructionSelector *
createpISAInstructionSelector(const pISATargetMachine &TM,
                              const pISASubtarget &Subtarget,
                              const RegisterBankInfo &RBI);
FunctionPass* createpISADetermineStackID();
FunctionPass* createpISALegalizeSubregAccess();
FunctionPass* createpISAPreLegalizerCombiner();
FunctionPass* createpISAPostLegalizerCombiner();

void initializepISADetermineStackIDPass(PassRegistry &);
void initializepISAEmitIntrinsicsPass(PassRegistry &);
void initializepISAExpandIntrinsicsPass(PassRegistry &);
void initializepISAInputTranslatorPass(PassRegistry &);
void initializepISALegalizeSubregAccessPass(PassRegistry &);
// void initializepISAPreLegalizerCombinerPass(PassRegistry &);
// void initializepISAPostLegalizerCombinerPass(PassRegistry &);

namespace pISA {
LLVM_READONLY int16_t getNamedOperandIdx(uint16_t Opcode, uint16_t NamedIdx);
}
}

#endif // LLVM_LIB_TARGET_pISA_pISA_H
