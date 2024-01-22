//===-- pISAExpandIntrinsics.cpp - modify function signatures --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass substitutes some llvm intrinsic calls with their code expansions.
//
//===----------------------------------------------------------------------===//

#include "pISA.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/Utils/LowerMemIntrinsics.h"

using namespace llvm;

namespace {

class pISAExpandIntrinsics : public FunctionPass {
public:
  static char ID;

  pISAExpandIntrinsics() : FunctionPass(ID) {
    initializepISAExpandIntrinsicsPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;

  StringRef getPassName() const override { return "pISA prepare functions"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetTransformInfoWrapperPass>();
  }

private:
  bool substituteIntrinsicCalls(Function &F);
  bool memIntrinsicAsLoop(MemIntrinsic *MI);

  static constexpr unsigned MaxStaticMemIntrSize = 128;
};

} // namespace

char pISAExpandIntrinsics::ID = 0;

INITIALIZE_PASS(pISAExpandIntrinsics, "pisa-prepare-functions",
                "pISA prepare functions", false, false)

bool pISAExpandIntrinsics::memIntrinsicAsLoop(MemIntrinsic *MI) {
  // @llvm.mem* intrinsics with constant length < MaxStaticMemIntrSize are
  // lowered to a sequence of st/ld. For constant length > MaxStaticMemIntrSize
  // and dynamic length cases we expand the intrinsic to a loop
  if (ConstantInt *LenCI = dyn_cast<ConstantInt>(MI->getLength()))
    if (LenCI->getZExtValue() < MaxStaticMemIntrSize)
      return false;
  return true;
}

bool pISAExpandIntrinsics::substituteIntrinsicCalls(Function &F) {
  SmallVector<MemIntrinsic *> MemIntrs;
  for (auto& I : instructions(F)) {
    auto* II = dyn_cast<IntrinsicInst>(&I);
    if (!II)
      continue;
    if (II->getIntrinsicID() == Intrinsic::memset ||
        II->getIntrinsicID() == Intrinsic::memcpy ||
        II->getIntrinsicID() == Intrinsic::memmove) {
      MemIntrinsic* MI = cast<MemIntrinsic>(II);
      if (memIntrinsicAsLoop(MI))
        MemIntrs.push_back(MI);
    }
  }

  bool Changed = false;

  // Expand llvm.mem* intrinsics to a loop
  const TargetTransformInfo &TTI =
      getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  for (MemIntrinsic *MemCall : MemIntrs) {
    if (auto *Memcpy = dyn_cast<MemCpyInst>(MemCall))
      expandMemCpyAsLoop(Memcpy, TTI);
    else if (auto *Memmove = dyn_cast<MemMoveInst>(MemCall))
      expandMemMoveAsLoop(Memmove, TTI);
    else if (auto *Memset = dyn_cast<MemSetInst>(MemCall))
      expandMemSetAsLoop(Memset);
    Changed = true;
    MemCall->eraseFromParent();
  }

  return Changed;
}

bool pISAExpandIntrinsics::runOnFunction(Function &F) {
  return substituteIntrinsicCalls(F);
}

FunctionPass *llvm::createpISAExpandIntrinsicsPass() {
  return new pISAExpandIntrinsics();
}
