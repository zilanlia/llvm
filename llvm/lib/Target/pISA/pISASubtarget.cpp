//===-- pISASubtarget.cpp - pISA Subtarget Information ------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the pISA specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "pISASubtarget.h"
#include "pISA.h"
#include "pISALegalizerInfo.h"
#include "pISARegisterBankInfo.h"
#include "pISATargetMachine.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/CodeGen/GlobalISel/InlineAsmLowering.h"

using namespace llvm;

#define DEBUG_TYPE "pISA-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "pISAGenSubtargetInfo.inc"

pISASubtarget::pISASubtarget(const Triple &TT, const std::string &CPU,
                               const std::string &FS,
                               const pISATargetMachine &TM)
    : pISAGenSubtargetInfo(TT, CPU, /*TuneCPU=*/CPU, FS),
      pISAVersion(0),
      InstrInfo(), FrameLowering(initSubtargetDependencies(CPU, FS)),
      TLInfo(TM, *this) {

  CallLoweringInfo = std::make_unique<pISACallLowering>(TLInfo);
  InlineAsmLoweringInfo.reset(new InlineAsmLowering(getTargetLowering()));
  Legalizer = std::make_unique<pISALegalizerInfo>(*this);
  RegBankInfo = std::make_unique<pISARegisterBankInfo>();
  InstSelector.reset(
      createpISAInstructionSelector(TM, *this, *RegBankInfo.get()));
}

pISASubtarget &pISASubtarget::initSubtargetDependencies(StringRef CPU,
                                                        StringRef FS) {
  ParseSubtargetFeatures(CPU, /*TuneCPU=*/CPU, FS);
  if (pISAVersion == 0)
    pISAVersion = 10;
  return *this;
}

