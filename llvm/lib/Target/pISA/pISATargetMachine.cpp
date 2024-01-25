//===- pISATargetMachine.cpp - Define TargetMachine for pISA -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implements the info about pISA target spec.
//
//===----------------------------------------------------------------------===//
#include "pISATargetMachine.h"
#include "pISA.h"
#include "pISACallLowering.h"
#include "pISALegalizerInfo.h"
#include "pISATargetObjectFile.h"
#include "pISATargetTransformInfo.h"
#include "TargetInfo/pISATargetInfo.h"
#include "llvm/CodeGen/GlobalISel/IRTranslator.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelect.h"
#include "llvm/CodeGen/GlobalISel/Legalizer.h"
#include "llvm/CodeGen/GlobalISel/RegBankSelect.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Pass.h"
#include "llvm/Target/TargetOptions.h"
#include <optional>

using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializepISATarget() {
  // Register the target.
  RegisterTargetMachine<pISATargetMachine> Y(getThepISATarget());

  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeGlobalISel(PR);
  initializepISADetermineStackIDPass(PR);
  initializepISAEmitIntrinsicsPass(PR);
  initializepISAExpandIntrinsicsPass(PR);
  initializepISAInputTranslatorPass(PR);
  initializepISALegalizeSubregAccessPass(PR);
  // initializepISAPreLegalizerCombinerPass(PR);
  initializepISAPostLegalizerCombinerPass(PR);
}

static std::string computeDataLayout(const Triple &TT) {
  // little endian
  std::string Ret = "e";
  // pointers
  Ret += "-p:32:32-p1:64:64-p2:64:64-p3:32:32-p4:64:64";
  // data type alignment
  Ret += "-i1:8:8-i8:8:8-i16:16:16-i32:32:32"
         "-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32"
         "-v32:32:32-v48:64:64-v64:64:64-v96:128:128"
         "-v128:128:128-v192:256:256-v256:256:256";

  return Ret;
}

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  if (!RM)
    return Reloc::PIC_;
  return *RM;
}

// Pin pISATargetObjectFile's vtables to this file.
pISATargetObjectFile::~pISATargetObjectFile() {}

pISATargetMachine::pISATargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       std::optional<Reloc::Model> RM,
                                       std::optional<CodeModel::Model> CM,
                                       CodeGenOptLevel OL, bool JIT)
    : LLVMTargetMachine(T, computeDataLayout(TT), TT, CPU, FS, Options,
                        getEffectiveRelocModel(RM),
                        getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<pISATargetObjectFile>()),
      Subtarget(TT, CPU.str(), FS.str(), *this) {
  initAsmInfo();
  setGlobalISel(true);
  setFastISel(false);
  setO0WantsFastISel(false);
  setRequiresStructuredCFG(false);
}

namespace {
// pISA Code Generator Pass Configuration Options.
class pISAPassConfig : public TargetPassConfig {
public:
  pISAPassConfig(pISATargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  pISATargetMachine &getpISATargetMachine() const {
    return getTM<pISATargetMachine>();
  }
  void addIRPasses() override;
  void addISelPrepare() override;

  bool addIRTranslator() override;
  void addPreLegalizeMachineIR() override;
  bool addLegalizeMachineIR() override;
  void addPreRegBankSelect() override;
  bool addRegBankSelect() override;
  bool addGlobalInstructionSelect() override;

  FunctionPass *createTargetRegisterAllocator(bool) override;

  bool addRegAssignAndRewriteFast() override {
    return false;
  }

  bool addRegAssignAndRewriteOptimized() override {
    return false;
  }

  void addPostRegAlloc() override;
};
} // namespace

// We do not use physical registers, and maintain virtual registers throughout
// the entire pipeline, so return nullptr to disable register allocation.
FunctionPass *pISAPassConfig::createTargetRegisterAllocator(bool) {
  return nullptr;
}

void pISAPassConfig::addPostRegAlloc() {
  addPass(createpISALegalizeSubregAccess());
  TargetPassConfig::addPostRegAlloc();
}

TargetTransformInfo
pISATargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(pISATTIImpl(this, F));
}

TargetPassConfig *pISATargetMachine::createPassConfig(PassManagerBase &PM) {
  return new pISAPassConfig(*this, PM);
}

void pISAPassConfig::addIRPasses() {
  TargetPassConfig::addIRPasses();
  addPass(createpISAInputTranslatorPass());
  addPass(createpISAExpandIntrinsicsPass());

  // Disable passes that break from assuming no virtual registers exist.
  disablePass(&PrologEpilogCodeInserterID);
  disablePass(&MachineLateInstrsCleanupID);
  disablePass(&MachineCopyPropagationID);
  disablePass(&TailDuplicateID);
  disablePass(&StackMapLivenessID);
  disablePass(&LiveDebugValuesID);
  disablePass(&PostRAMachineSinkingID);
  disablePass(&PostRASchedulerID);
  disablePass(&FuncletLayoutID);
  disablePass(&PatchableFunctionID);
  disablePass(&ShrinkWrapID);
}

void pISAPassConfig::addISelPrepare() {
  addPass(createpISAEmitIntrinsicsPass());
  TargetPassConfig::addISelPrepare();
}

bool pISAPassConfig::addIRTranslator() {
  addPass(new IRTranslator(getOptLevel()));
  return false;
}

void pISAPassConfig::addPreLegalizeMachineIR() {
  addPass(createpISADetermineStackID());
  llvm::outs() <<"\n\naddPass(createpISAPreLegalizerCombiner())\n";
  // if (getOptLevel() != CodeGenOptLevel::None)
  //   addPass(createpISAPreLegalizerCombiner());
}

// Use the default legalizer.
bool pISAPassConfig::addLegalizeMachineIR() {
  addPass(new Legalizer());
  return false;
}

void pISAPassConfig::addPreRegBankSelect() {
  llvm::outs() <<"\n\naddPass(createpISAPostLegalizerCombiner())\n";
  bool IsOptNone = getOptLevel() == CodeGenOptLevel::None;
  if (getOptLevel() != CodeGenOptLevel::None)
    addPass(createpISAPostLegalizerCombiner(IsOptNone));
}

bool pISAPassConfig::addRegBankSelect() {
  addPass(new RegBankSelect());
  return false;
}

bool pISAPassConfig::addGlobalInstructionSelect() {
  addPass(new InstructionSelect());
  return false;
}
