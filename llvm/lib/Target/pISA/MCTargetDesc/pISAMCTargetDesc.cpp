//===-- pISAMCTargetDesc.cpp - pISA Target Descriptions ----*- C++ -*---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides pISA specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "pISAMCTargetDesc.h"
#include "TargetInfo/pISATargetInfo.h"
#include "pISAInstPrinter.h"
#include "pISAMCAsmInfo.h"
#include "pISATargetStreamer.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "pISAGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "pISAGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "pISAGenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createpISAMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitpISAMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createpISAMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  return X;
}

static MCSubtargetInfo *createpISAMCSubtargetInfo(const Triple &TT,
                                                  StringRef CPU, StringRef FS) {
  return createpISAMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCInstPrinter *createpISAMCInstPrinter(const Triple &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI) {
  assert(SyntaxVariant == 0);
  return new pISAInstPrinter(MAI, MII, MRI);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializepISATargetMC() {
  for (Target *T : {&getThepISATarget()}) {
    RegisterMCAsmInfo<pISAMCAsmInfo> X(*T);
    TargetRegistry::RegisterMCInstrInfo(*T, createpISAMCInstrInfo);
    TargetRegistry::RegisterMCRegInfo(*T, createpISAMCRegisterInfo);
    TargetRegistry::RegisterMCSubtargetInfo(*T, createpISAMCSubtargetInfo);
    TargetRegistry::RegisterMCInstPrinter(*T, createpISAMCInstPrinter);
    TargetRegistry::RegisterAsmTargetStreamer(*T, createpISAAsmTargetStreamer);
    TargetRegistry::RegisterObjectTargetStreamer(
        *T, createpISAObjectTargetStreamer);
    TargetRegistry::RegisterNullTargetStreamer(*T,
                                               createpISANullTargetStreamer);
  }
}
