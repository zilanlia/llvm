//===-- pISAEmitIntrinsics.cpp - emit pISA intrinsics ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// The pass emits pISA intrinsics keeping essential high-level information for
// the translation of LLVM IR to pISA.
//
//===----------------------------------------------------------------------===//

#include "pISA.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/IntrinsicspISA.h"

using namespace llvm;

namespace llvm {
void initializepISAEmitIntrinsicsPass(PassRegistry &);
} // namespace llvm

namespace {
class pISAEmitIntrinsics
    : public FunctionPass,
      public InstVisitor<pISAEmitIntrinsics> {

  IRBuilder<> *IRB = nullptr;
  bool Changed = false;

  CallInst *insertAssignRndMode(Value *Op, RoundingMode RnMode);
public:
  static char ID;
  pISAEmitIntrinsics() : FunctionPass(ID) {
    initializepISAEmitIntrinsicsPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
  }

  void visitIntrinsicInst(IntrinsicInst &I);
};
} // namespace

char pISAEmitIntrinsics::ID = 0;

INITIALIZE_PASS(pISAEmitIntrinsics, "pisa-emit-intrinsics", "pISA emit intrinsics",
                false, false)

void pISAEmitIntrinsics::visitIntrinsicInst(IntrinsicInst &I) {
  IRB->SetInsertPoint(&I);
  switch (I.getIntrinsicID()) {
  case Intrinsic::experimental_constrained_fadd: {
    // Rouning mode from LLVM constrained FP intrinsics (except from ftrunc) are
    // not passed to GMIR. Since we don't control LLVM IRBuilder, we transtale
    // these intrinsics into:
    //   LLVM instruction + pISA intrinsic assign_rndmode
    // which are later pattern matched by tablegen and selected to a pISA
    // instruction with rounding mode.
    auto *CII = cast<ConstrainedFPIntrinsic>(&I);
    auto *FAdd = IRB->CreateFAdd(CII->getOperand(0), CII->getOperand(1));
    auto *AssignRnd = insertAssignRndMode(FAdd, CII->getRoundingMode().value());
    I.replaceAllUsesWith(AssignRnd);
    I.eraseFromParent();
    Changed = true;
    break;
  }
  case Intrinsic::experimental_constrained_fmul: {
    auto *CII = cast<ConstrainedFPIntrinsic>(&I);
    auto *FMul = IRB->CreateFMul(CII->getOperand(0), CII->getOperand(1));
    auto *AssignRnd = insertAssignRndMode(FMul, CII->getRoundingMode().value());
    I.replaceAllUsesWith(AssignRnd);
    I.eraseFromParent();
    Changed = true;
    break;
  }
  case Intrinsic::experimental_constrained_fdiv: {
    auto *CII = cast<ConstrainedFPIntrinsic>(&I);
    auto *FDiv = IRB->CreateFDiv(CII->getOperand(0), CII->getOperand(1));
    auto *AssignRnd = insertAssignRndMode(FDiv, CII->getRoundingMode().value());
    I.replaceAllUsesWith(AssignRnd);
    I.eraseFromParent();
    Changed = true;
    break;
  }
  case Intrinsic::experimental_constrained_uitofp: {
    auto *CII = cast<ConstrainedFPIntrinsic>(&I);
    auto *UIToFP = IRB->CreateUIToFP(CII->getOperand(0), CII->getType());
    auto *AssignRnd =
        insertAssignRndMode(UIToFP, CII->getRoundingMode().value());
    I.replaceAllUsesWith(AssignRnd);
    I.eraseFromParent();
    Changed = true;
    break;
  }
  case Intrinsic::experimental_constrained_sitofp: {
    auto *CII = cast<ConstrainedFPIntrinsic>(&I);
    auto *SItoFP = IRB->CreateSIToFP(CII->getOperand(0), CII->getType());
    auto *AssignRnd =
        insertAssignRndMode(SItoFP, CII->getRoundingMode().value());
    I.replaceAllUsesWith(AssignRnd);
    I.eraseFromParent();
    Changed = true;
    break;
  }
  case Intrinsic::pisa_fptoui: {
    auto *MD = cast<MetadataAsValue>(I.getArgOperand(1))->getMetadata();
    auto RoundMode = convertStrToRoundingMode(cast<MDString>(MD)->getString());
    auto *AssignRnd = insertAssignRndMode(I.getArgOperand(0), *RoundMode);
    auto *FPToUI = IRB->CreateFPToUI(AssignRnd, I.getType());
    I.replaceAllUsesWith(FPToUI);
    I.eraseFromParent();
    Changed = true;
    break;
  }
  case Intrinsic::pisa_fptosi: {
    auto *MD = cast<MetadataAsValue>(I.getArgOperand(1))->getMetadata();
    auto RoundMode = convertStrToRoundingMode(cast<MDString>(MD)->getString());
    auto *AssignRnd = insertAssignRndMode(I.getArgOperand(0), *RoundMode);
    auto *FPToSI = IRB->CreateFPToSI(AssignRnd, I.getType());
    I.replaceAllUsesWith(FPToSI);
    I.eraseFromParent();
    Changed = true;
    break;
  }
  case Intrinsic::experimental_constrained_sqrt: {
    auto *CII = cast<ConstrainedFPIntrinsic>(&I);
    auto *Sqrt =
        IRB->CreateUnaryIntrinsic(Intrinsic::sqrt, CII->getOperand(0), CII);
    auto *AssignRnd =
        insertAssignRndMode(Sqrt, CII->getRoundingMode().value());
    I.replaceAllUsesWith(AssignRnd);
    I.eraseFromParent();
    Changed = true;
    break;
  }
  case Intrinsic::experimental_constrained_fmuladd: {
    auto *CI = cast<ConstrainedFPIntrinsic>(&I);
    auto *FMulAdd = IRB->CreateIntrinsic(
        Intrinsic::fmuladd, CI->getType(),
        {CI->getArgOperand(0), CI->getArgOperand(1), CI->getArgOperand(2)});
    auto *AssignRnd =
        insertAssignRndMode(FMulAdd, CI->getRoundingMode().value());
    I.replaceAllUsesWith(AssignRnd);
    I.eraseFromParent();
    Changed = true;
    break;
  }
  default:
    break;
  }
}

// Rounding mode intrinsics are meant for assigning rounding mode information
// to FP instructions.
// This can be achieved by using LLVM constrained FP intrinsics, such as
// @llvm.experimental.constrained.fadd(%a, %b, rndmode,...), or using
// pISA intrinsics @llvm.pisa.assign.rndmode.XX(FPOp)
// The source operand of these pISA intrinsics corresponds to the FP instruction,
// e.g.
//   %c = fadd %a, %b
//   %r = @llvm.pisa.assign.rnmode.rz(%c)
// The expected generated pISA code will be
//   fadd.rz %r, %a, %b
// In particular, this function replaces the LLVM constrained FP intrinsic
// by the pISA intrinsic.
CallInst *pISAEmitIntrinsics::insertAssignRndMode(Value *Op, RoundingMode RndMode) {
  CallInst *AssignRndIntr = nullptr;
  switch (RndMode) {
  case RoundingMode::TowardZero:
    AssignRndIntr = IRB->CreateIntrinsic(Intrinsic::pisa_assign_rndmode_rz,
                                         Op->getType(), Op);
    break;
  case RoundingMode::NearestTiesToEven:
    AssignRndIntr = IRB->CreateIntrinsic(Intrinsic::pisa_assign_rndmode_re,
                                         Op->getType(), Op);
    break;
  case RoundingMode::TowardPositive:
    AssignRndIntr = IRB->CreateIntrinsic(Intrinsic::pisa_assign_rndmode_ru,
                                         Op->getType(), Op);
    break;
  case RoundingMode::TowardNegative:
    AssignRndIntr = IRB->CreateIntrinsic(Intrinsic::pisa_assign_rndmode_rd,
                                         Op->getType(), Op);
    break;
  default:
    llvm_unreachable("Rounding mode not supported for floating point inst.");
  }
  return AssignRndIntr;
}

bool pISAEmitIntrinsics::runOnFunction(Function &Func) {
  IRBuilder<> LocalIRB(Func.getContext());

  IRB = &LocalIRB;
  Changed = false;

  visit(Func);

  return Changed;
}

FunctionPass *llvm::createpISAEmitIntrinsicsPass() {
  return new pISAEmitIntrinsics();
}
