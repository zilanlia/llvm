//===- pISALegalizerInfo.h --- pISA Legalization Rules --------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the targeting of the MachineLegalizer class for pISA.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_pISAMACHINELEGALIZER_H
#define LLVM_LIB_TARGET_pISA_pISAMACHINELEGALIZER_H

#include "llvm/CodeGen/GlobalISel/LegalizerInfo.h"

namespace llvm {

class LLVMContext;
class pISASubtarget;

// This class provides the information for legalizing pISA instructions.
class pISALegalizerInfo : public LegalizerInfo {
  const pISASubtarget *ST;

public:
  bool legalizeCustom(LegalizerHelper &Helper, MachineInstr &MI,
                      LostDebugLocObserver &LocObserver) const override;
  pISALegalizerInfo(const pISASubtarget &ST);
};
} // namespace llvm
#endif // LLVM_LIB_TARGET_pISA_pISAMACHINELEGALIZER_H
