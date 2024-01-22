//===-- pISATargetMachine.h - Define TargetMachine for pISA -*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the pISA specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_pISATARGETMACHINE_H
#define LLVM_LIB_TARGET_pISA_pISATARGETMACHINE_H

#include "pISASubtarget.h"
#include "llvm/Target/TargetMachine.h"
#include <optional>

namespace llvm {
class pISATargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  pISASubtarget Subtarget;

public:
  pISATargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                     StringRef FS, const TargetOptions &Options,
                     std::optional<Reloc::Model> RM,
                     std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                     bool JIT);

  const pISASubtarget *getSubtargetImpl() const { return &Subtarget; }

  const pISASubtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  bool usesPhysRegsForValues() const override { return false; }

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_pISA_pISATARGETMACHINE_H
