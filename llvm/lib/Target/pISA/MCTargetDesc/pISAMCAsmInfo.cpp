//===-- pISAMCAsmInfo.h - pISA asm properties --------------*- C++ -*--====//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the pISAMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "pISAMCAsmInfo.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

pISAMCAsmInfo::pISAMCAsmInfo(const Triple &TT,
                               const MCTargetOptions &Options) {
  IsLittleEndian = true;

  HasSingleParameterDotFile = false;
  HasDotTypeDotSizeDirective = false;

  MinInstAlignment = 4;

  CodePointerSize = 4;
  CommentString = "//";
  HasFunctionAlignment = false;
}

bool pISAMCAsmInfo::shouldOmitSectionDirective(StringRef SectionName) const {
  return true;
}
