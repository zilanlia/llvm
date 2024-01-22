//===-- pISAMCAsmInfo.h - pISA asm properties --------------*- C++ -*--====//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the pISAMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_MCTARGETDESC_pISAMCASMINFO_H
#define LLVM_LIB_TARGET_pISA_MCTARGETDESC_pISAMCASMINFO_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm {

class Triple;

class pISAMCAsmInfo : public MCAsmInfo {
public:
  explicit pISAMCAsmInfo(const Triple &TT, const MCTargetOptions &Options);
  bool shouldOmitSectionDirective(StringRef SectionName) const override;
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_pISA_MCTARGETDESC_pISAMCASMINFO_H
