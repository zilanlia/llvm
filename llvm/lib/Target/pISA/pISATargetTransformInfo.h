//===- pISATargetTransformInfo.h - pISA specific TTI ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// \file
// This file contains a TargetTransformInfo::Concept conforming object specific
// to the pISA target machine. It uses the target's detailed information to
// provide more precise answers to certain TTI queries, while letting the
// target independent and default TTI implementations handle the rest.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_pISATARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_pISA_pISATARGETTRANSFORMINFO_H

#include "pISA.h"
#include "pISATargetMachine.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/BasicTTIImpl.h"

namespace llvm {
class pISATTIImpl : public BasicTTIImplBase<pISATTIImpl> {
  using BaseT = BasicTTIImplBase<pISATTIImpl>;

  friend BaseT;

  const pISASubtarget *ST;
  const pISATargetLowering *TLI;

  const TargetSubtargetInfo *getST() const { return ST; }
  const pISATargetLowering *getTLI() const { return TLI; }

public:
  explicit pISATTIImpl(const pISATargetMachine *TM, const Function &F)
      : BaseT(TM, F.getParent()->getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_pISA_pISATARGETTRANSFORMINFO_H
