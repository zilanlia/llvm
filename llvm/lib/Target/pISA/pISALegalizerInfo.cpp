//===- pISALegalizerInfo.cpp --- pISA Legalization Rules ------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the targeting of the Machinelegalizer class for pISA.
//
//===----------------------------------------------------------------------===//

#include "pISALegalizerInfo.h"
#include "pISA.h"
#include "pISASubtarget.h"
#include "llvm/CodeGen/GlobalISel/LegalizerHelper.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetOpcodes.h"

using namespace llvm;
using namespace llvm::LegalizeActions;
using namespace llvm::LegalityPredicates;

pISALegalizerInfo::pISALegalizerInfo(const pISASubtarget &ST) {
  using namespace TargetOpcode;

  this->ST = &ST;

  const LLT s1 = LLT::scalar(1);
  const LLT s8 = LLT::scalar(8);
  const LLT s16 = LLT::scalar(16);
  const LLT s32 = LLT::scalar(32);
  const LLT s64 = LLT::scalar(64);

  const LLT v3s8 = LLT::fixed_vector(3, 8);
  const LLT v3s16 = LLT::fixed_vector(3, 16);
  const LLT v3s32 = LLT::fixed_vector(3, 32);
  const LLT v3s64 = LLT::fixed_vector(3, 64);

  const LLT v2s8 = LLT::fixed_vector(2, 8);
  const LLT v2s16 = LLT::fixed_vector(2, 16);
  const LLT v2s32 = LLT::fixed_vector(2, 32);
  const LLT v2s64 = LLT::fixed_vector(2, 64);

  const LLT v4s8 = LLT::fixed_vector(4, 8);
  const LLT v4s16 = LLT::fixed_vector(4, 16);
  const LLT v4s32 = LLT::fixed_vector(4, 32);
  const LLT v4s64 = LLT::fixed_vector(4, 64);

  auto& TM = ST.getTargetLowering()->getTargetMachine();

  auto getPointerLLT = [&](pISA::AddressSpace Addrspace) {
    uint32_t NumBits = TM.getPointerSizeInBits(static_cast<unsigned>(Addrspace));
    return LLT::pointer(static_cast<unsigned>(Addrspace), NumBits);
  };

  const LLT PrivatePtr  = getPointerLLT(pISA::AddressSpace::PRIVATE);
  const LLT GlobalPtr   = getPointerLLT(pISA::AddressSpace::GLOBAL);
  const LLT ConstantPtr = getPointerLLT(pISA::AddressSpace::CONSTANT);
  const LLT SharedPtr   = getPointerLLT(pISA::AddressSpace::SHARED);
  const LLT GenericPtr  = getPointerLLT(pISA::AddressSpace::GENERIC);

  // TODO: remove copy-pasting here by using concatenation in some way.
  auto allPtrsScalarsAndVectors = {
      PrivatePtr,    GlobalPtr,    ConstantPtr,  SharedPtr,    GenericPtr,
      s1,       s8,    s16,   s32,   s64,
              v2s8,  v2s16, v2s32, v2s64,
              v3s8,  v3s16, v3s32, v3s64,
              v4s8,  v4s16, v4s32, v4s64 };

  auto allVectors = { v2s8,  v2s16, v2s32, v2s64,
                      v3s8,  v3s16, v3s32, v3s64,
                      v4s8,  v4s16, v4s32, v4s64 };

  auto allScalarsAndVectors = {
      s1,       s8,    s16,   s32,   s64,
              v2s8,  v2s16, v2s32, v2s64,
              v3s8,  v3s16, v3s32, v3s64,
              v4s8,  v4s16, v4s32, v4s64 };

  auto allIntScalarsAndVectors = {
                s8,    s16,   s32,   s64,
              v2s8,  v2s16, v2s32, v2s64,
              v3s8,  v3s16, v3s32, v3s64,
              v4s8,  v4s16, v4s32, v4s64 };

  auto allScalars = {s8, s16, s32, s64};

  auto allFloatScalarsAndVectors = {
      s16,   s32,   s64,   v2s16, v2s32, v2s64, v3s16,  v3s32,  v3s64,
      v4s16, v4s32, v4s64 };

  auto allPtrs = {PrivatePtr, GlobalPtr, ConstantPtr, SharedPtr, GenericPtr};
  auto allWritablePtrs = {PrivatePtr, GlobalPtr, SharedPtr, GenericPtr};

  getActionDefinitionsBuilder({G_FADD, G_ADD, G_SUB, G_FSUB, G_MUL, G_FMUL,
                               G_SDIV, G_UDIV, G_SREM, G_UREM, G_AND, G_OR,
                               G_XOR, G_FCONSTANT, G_FNEG, G_FMINNUM, G_FMAXNUM})
      .legalFor({s16, s32, s64})
      .clampScalar(0, s16, s64)
      .scalarize(0);

  getActionDefinitionsBuilder({ G_SHL, G_LSHR, G_ASHR })
    .legalFor({ {s16, s32}, {s32, s32}, {s64, s32} })
    .clampScalar(0, s16, s64)
    .clampScalar(1, s32, s32)
    .scalarize(0);

  getActionDefinitionsBuilder(G_TRUNC)
    .legalFor({ {s8, s16}, {s16, s32}, {s8, s32},
                {s32, s64}, {s16, s64}, {s8, s64} })
    .scalarize(0)
    .customIf([=](const LegalityQuery& Query) {
      return Query.Types[0].getScalarSizeInBits() == 1;
    })
    .clampScalar(0, s8, s32)
    .clampScalar(1, s16, s64);

  getActionDefinitionsBuilder({ G_SEXT, G_ZEXT, G_ANYEXT })
    .legalFor({ {s16, s8},
                {s32, s8},
                {s64, s8},
                {s32, s16},
                {s64, s16},
                {s64, s32}})
    .scalarize(0)
    .customIf([=](const LegalityQuery& Query) {
      return Query.Types[1].getScalarSizeInBits() == 1;
    })
    .clampScalar(0, s16, s64)
    .clampScalar(1, s8, s32);

  getActionDefinitionsBuilder(G_SEXT_INREG).lower();

  getActionDefinitionsBuilder(G_FPTRUNC)
      .legalFor({{s16, s32}, {s16, s64}, {s32, s64}})
      .clampScalar(0, s16, s32)
      .clampScalar(1, s16, s64)
      .scalarize(0);
  
  getActionDefinitionsBuilder(G_INTRINSIC_FPTRUNC_ROUND)
      .legalFor({{s16, s32}, {s16, s64}, {s32, s64}})
      .clampScalar(0, s16, s32)
      .clampScalar(1, s16, s64)
      .scalarize(0);

  getActionDefinitionsBuilder(G_FPEXT)
      .legalFor({{s32, s16}, {s64, s16}, {s64, s32}})
      .clampScalar(0, s32, s64)
      .clampScalar(1, s16, s32)
      .scalarize(0);

  getActionDefinitionsBuilder(G_CTPOP)
    .legalFor({ {s16, s16}, {s32, s32}, {s64,s64} })
    .clampScalar(0, s16, s64)
    .clampScalar(1, s16, s64)
    .scalarize(0);

  getActionDefinitionsBuilder({ G_CTTZ, G_CTLZ,
                                G_CTTZ_ZERO_UNDEF, G_CTLZ_ZERO_UNDEF})
    .legalFor({ {s16, s16}, {s32, s32}, {s64,s64} })
    .clampScalar(0, s16, s64)
    .clampScalar(1, s16, s64)
    .scalarize(0);

  getActionDefinitionsBuilder({ G_BITREVERSE, G_FDIV, G_FREM })
    .legalFor({ s32, s64 })
    .clampScalar(0, s32, s64)
    .scalarize(0);

  getActionDefinitionsBuilder(G_CONSTANT)
    .legalFor({ s8, s16, s32, s64 })
    .clampScalar(0, s8, s64)
    .scalarize(0);

  getActionDefinitionsBuilder(G_PTR_ADD)
    .legalIf(all(isPointer(0), sameSize(0, 1)))
    .scalarize(0)
    .scalarSameSizeAs(1, 0);

  getActionDefinitionsBuilder({ G_FSHR, G_FSHL })
    .legalFor({ {s32, s32} })
    .scalarize(0)
    .clampScalar(0, s32, s32)
    .clampScalar(1, s32, s32);

  /////////////////////////////////////////////////////////////////////////

  getActionDefinitionsBuilder(G_GLOBAL_VALUE).alwaysLegal();

  getActionDefinitionsBuilder(G_SHUFFLE_VECTOR).lower();

  getActionDefinitionsBuilder({G_MEMCPY, G_MEMMOVE, G_MEMSET}).lower();

  getActionDefinitionsBuilder(G_ADDRSPACE_CAST)
      .legalForCartesianProduct(allPtrs, allPtrs);

  getActionDefinitionsBuilder({G_LOAD, G_STORE})
      .fewerElementsIf(LegalityPredicate(([=](const LegalityQuery &Query) {
                         unsigned int BitSize =
                             Query.Types[0].getScalarSizeInBits();
                         return Query.Types[0].isVector() &&
                                Query.MMODescrs[0].AlignInBits < BitSize;
                       })),
                       LegalizeMutation(([=](const LegalityQuery &Query) {
                         assert(Query.Types[0].isVector() &&
                                "expecting to see vector type");
                           LLT EltTy = Query.Types[0].getScalarType();
                           return std::make_pair(0, EltTy);
                       })))
      .clampMaxNumElements(0,  s8, 4)
      .clampMaxNumElements(0, s16, 4)
      .clampMaxNumElements(0, s32, 4)
      .clampMaxNumElements(0, s64, 2)
      .customIf([=](const LegalityQuery& Query) {
        return Query.Types[0].getScalarType().isPointer();
      })
      .legalIf(typeInSet(1, allPtrs));

  getActionDefinitionsBuilder(G_FMA)
      .legalFor({s16,s32, s64})
      .scalarize(0);

  getActionDefinitionsBuilder({G_FPTOSI, G_FPTOUI})
      .legalForCartesianProduct(allIntScalarsAndVectors,
                                allFloatScalarsAndVectors);

  getActionDefinitionsBuilder({G_SITOFP, G_UITOFP})
      .legalForCartesianProduct(allFloatScalarsAndVectors,
                                allScalarsAndVectors);

  getActionDefinitionsBuilder({G_SMIN, G_SMAX, G_UMIN, G_UMAX, G_ABS})
      .legalFor(allIntScalarsAndVectors);

  getActionDefinitionsBuilder(G_PHI).legalFor(allPtrsScalarsAndVectors);

  getActionDefinitionsBuilder(G_BITCAST)
    .legalFor({ {s16, v2s8}, {s32, v2s16}, {s32, v4s8}, {s64, v2s32},
                {s64, v4s16}, {v2s8, s16}, {v4s8, s32}, {v2s16, s32},
                {v4s16, s64}, {v2s32, s64}, {v4s8, v2s16}, {v2s16, v4s8},
                {v4s16, v2s32}, {v2s32, v4s16} })
    .lower();

  getActionDefinitionsBuilder(G_EXTRACT_VECTOR_ELT).lower();
  getActionDefinitionsBuilder(G_INSERT_VECTOR_ELT).lower();

  getActionDefinitionsBuilder(G_UNMERGE_VALUES)
    .legalIf(all(typeInSet(0, allIntScalarsAndVectors),
                 typeInSet(1, allVectors)))
    .clampMaxNumElements(1, s8, 4)
    .clampMaxNumElements(1, s16, 4)
    .clampMaxNumElements(1, s32, 4)
    .clampMaxNumElements(1, s64, 4);

  getActionDefinitionsBuilder(G_BUILD_VECTOR).alwaysLegal();

  getActionDefinitionsBuilder({G_IMPLICIT_DEF, G_FREEZE})
      .legalFor({s8, s16, s32, s64})
      .scalarize(0);

  getActionDefinitionsBuilder(G_INTTOPTR)
    .legalForCartesianProduct(allPtrs, allScalars)
    .scalarize(0);

  getActionDefinitionsBuilder(G_PTRTOINT)
    .legalForCartesianProduct(allScalars, allPtrs)
    .scalarize(0);

  getActionDefinitionsBuilder(G_ICMP)
    .legalForCartesianProduct({ s1 }, { s16, s32, s64 })
    .scalarize(0)
    .scalarize(1)
    .clampScalar(1, s16, s64);

  getActionDefinitionsBuilder(G_FCMP)
    .clampScalar(1, s16, s64)
    .scalarize(0)
    .scalarize(1)
    .custom();

  getActionDefinitionsBuilder(G_SELECT)
    .legalForCartesianProduct({ s16, s32, s64 }, { s1 })
    .scalarize(0)
    .scalarize(1)
    .clampScalar(0, s16, s64);

  getActionDefinitionsBuilder(
        {G_ATOMICRMW_OR, G_ATOMICRMW_ADD, G_ATOMICRMW_AND, G_ATOMICRMW_MAX,
         G_ATOMICRMW_MIN, G_ATOMICRMW_SUB, G_ATOMICRMW_XOR, G_ATOMICRMW_UMAX,
         G_ATOMICRMW_UMIN, G_ATOMICRMW_UINC_WRAP, G_ATOMICRMW_UDEC_WRAP,
         G_ATOMICRMW_FADD, G_ATOMICRMW_FSUB, G_ATOMICRMW_FMIN,
         G_ATOMICRMW_FMAX})
        .legalForCartesianProduct({s16, s32, s64},
                                  {GlobalPtr, SharedPtr, GenericPtr});

  getActionDefinitionsBuilder(G_ATOMICRMW_XCHG)
      .legalForCartesianProduct(allScalars, allWritablePtrs);

  getActionDefinitionsBuilder(G_ATOMIC_CMPXCHG_WITH_SUCCESS).lower();
  // TODO: add proper legalization rules.
  getActionDefinitionsBuilder(G_ATOMIC_CMPXCHG).alwaysLegal();

  getActionDefinitionsBuilder(
      {G_UADDO, G_USUBO, G_UADDE, G_SADDE, G_USUBE, G_SSUBE})
      .legalFor({{s32, s1}, {s64, s1}})
      .clampScalar(0, s32, s64)
      .scalarize(0);

  // Pointer-handling.
  getActionDefinitionsBuilder(G_FRAME_INDEX).legalFor(
    { PrivatePtr, SharedPtr });

  // Control-flow. In some cases (e.g. constants) s1 may be promoted to s32.
  getActionDefinitionsBuilder(G_BRCOND).legalFor({s1, s32});

  getActionDefinitionsBuilder({G_FCOS, G_FSIN, G_FEXP2, G_FLOG2})
      .legalFor({s32})
      .clampScalar(0, s32, s32)
      .scalarize(0);

  getActionDefinitionsBuilder({G_FEXP, G_FLOG, G_FLOG10}).customFor({s32});

  getActionDefinitionsBuilder({G_FPOW,
                               G_FABS,
                               G_FCEIL,
                               G_FSQRT,
                               G_FFLOOR,
                               G_FRINT,
                               G_FNEARBYINT,
                               G_INTRINSIC_ROUND,
                               G_INTRINSIC_TRUNC,
                               G_FMINIMUM,
                               G_FMAXIMUM,
                               G_INTRINSIC_ROUNDEVEN})
      .legalFor(allFloatScalarsAndVectors);

  getActionDefinitionsBuilder(G_FCOPYSIGN)
      .legalForCartesianProduct(allFloatScalarsAndVectors,
                                allFloatScalarsAndVectors);

  getActionDefinitionsBuilder(G_FPOWI).legalForCartesianProduct(
      allFloatScalarsAndVectors, allIntScalarsAndVectors);

  getActionDefinitionsBuilder({G_SMULH, G_UMULH}).lower();

  getLegacyLegalizerInfo().computeTables();
  verify(*ST.getInstrInfo());
}

static Register convertPtrToInt(Register Reg, LLT ConvTy,
                                LegalizerHelper &Helper,
                                MachineRegisterInfo &MRI) {
  Register ConvReg = MRI.createGenericVirtualRegister(ConvTy);
  Helper.MIRBuilder.buildInstr(TargetOpcode::G_PTRTOINT)
      .addDef(ConvReg)
      .addUse(Reg);
  return ConvReg;
}

// flog(x) = flog2(x) * ln(2)
static bool legalizeFlog(MachineInstr &MI, MachineIRBuilder &B,
                         double Log2BaseInverted) {
  Register Dst = MI.getOperand(0).getReg();
  Register Src = MI.getOperand(1).getReg();
  LLT Ty = B.getMRI()->getType(Dst);
  unsigned Flags = MI.getFlags();

  auto Log2Operand = B.buildFLog2(Ty, Src, Flags);
  auto Log2BaseInvertedOperand = B.buildFConstant(Ty, Log2BaseInverted);

  B.buildFMul(Dst, Log2Operand, Log2BaseInvertedOperand, Flags);
  MI.eraseFromParent();
  return true;
}

// fexp(x) = fexp2(x * log2(e))
static bool legalizeFExp(MachineInstr &MI, MachineIRBuilder &B) {
  Register Dst = MI.getOperand(0).getReg();
  Register Src = MI.getOperand(1).getReg();
  unsigned Flags = MI.getFlags();
  LLT Ty = B.getMRI()->getType(Dst);

  auto K = B.buildFConstant(Ty, numbers::log2e);
  auto Mul = B.buildFMul(Ty, Src, K, Flags);
  B.buildFExp2(Dst, Mul, Flags);
  MI.eraseFromParent();
  return true;
}

// GlobalISel doesn't currently have builtin support to legalize based on
// condition code like the SelectionDAG path does. We can move to that approach
// if and when it is available. For now, we custom legalize it based upon the
// approach in TargetLowering::LegalizeSetCCCondCode().
static bool legalizeCondCode(MachineInstr &MI, MachineIRBuilder &B) {
  auto Pred = static_cast<CmpInst::Predicate>(MI.getOperand(1).getPredicate());
  Register Dst = MI.getOperand(0).getReg();
  Register Op0 = MI.getOperand(2).getReg();
  Register Op1 = MI.getOperand(3).getReg();
  unsigned Flags = MI.getFlags();
  const LLT s1  = LLT::scalar(1);
  const LLT s32 = LLT::scalar(32);
  switch (Pred) {
  case CmpInst::FCMP_UNE:
  case CmpInst::FCMP_OEQ:
  case CmpInst::FCMP_OGT:
  case CmpInst::FCMP_OGE:
  case CmpInst::FCMP_OLT:
  case CmpInst::FCMP_OLE:
    // already legal
    break;
  case CmpInst::FCMP_ONE:
  case CmpInst::FCMP_UEQ: {
    // Without the explicit G_SEXT added here, the legalizer will typically
    // G_ANYEXT the G_FCMP compare result to s16. Given that a .reg destination
    // for fcmp is only available for 32-bit, we explicitly extend it here
    // so we can fold the resulting select into the fcmp.
    auto LHS = B.buildSExt(s32,
      B.buildFCmp(CmpInst::FCMP_OGT, s1, Op0, Op1, Flags));
    auto RHS = B.buildSExt(s32,
      B.buildFCmp(CmpInst::FCMP_OLT, s1, Op0, Op1, Flags));
    auto Result = B.buildOr(s32, LHS, RHS);
    if (Pred == CmpInst::FCMP_UEQ)
      Result = B.buildNot(s32, Result);
    B.buildICmp(CmpInst::ICMP_EQ, Dst, Result, B.buildConstant(s32, -1));
    MI.eraseFromParent();
    break;
  }
  case CmpInst::FCMP_ORD: {
    auto LHS = B.buildSExt(s32,
      B.buildFCmp(CmpInst::FCMP_OEQ, s1, Op0, Op0, Flags));
    auto RHS = B.buildSExt(s32,
      B.buildFCmp(CmpInst::FCMP_OEQ, s1, Op1, Op1, Flags));
    auto Result = B.buildAnd(s32, LHS, RHS);
    B.buildICmp(CmpInst::ICMP_EQ, Dst, Result, B.buildConstant(s32, -1));
    MI.eraseFromParent();
    break;
  }
  case CmpInst::FCMP_UNO: {
    auto LHS = B.buildSExt(s32,
      B.buildFCmp(CmpInst::FCMP_UNE, s1, Op0, Op0, Flags));
    auto RHS = B.buildSExt(s32,
      B.buildFCmp(CmpInst::FCMP_UNE, s1, Op1, Op1, Flags));
    auto Result = B.buildOr(s32, LHS, RHS);
    B.buildICmp(CmpInst::ICMP_EQ, Dst, Result, B.buildConstant(s32, -1));
    MI.eraseFromParent();
    break;
  }
  case CmpInst::FCMP_UGT:
  case CmpInst::FCMP_UGE:
  case CmpInst::FCMP_ULT:
  case CmpInst::FCMP_ULE: {
    auto Cmp = B.buildSExt(s32,
      B.buildFCmp(FCmpInst::getInversePredicate(Pred), s1, Op0, Op1, Flags));
    auto Not = B.buildNot(s32, Cmp);
    B.buildICmp(CmpInst::ICMP_EQ, Dst, Not, B.buildConstant(s32, -1));
    MI.eraseFromParent();
    break;
  }
  default:
    llvm_unreachable("unknown predicate?");
  }
  return true;
}

static bool legalizeTrunc(MachineInstr& MI, MachineIRBuilder& B) {
  auto& MRI = *B.getMRI();
  Register Dst  = MI.getOperand(0).getReg();
  assert(MRI.getType(Dst).getSizeInBits() == 1);
  Register Op1  = MI.getOperand(1).getReg();
  LLT OpTy = MRI.getType(Op1);
  auto Zero = B.buildConstant(OpTy, 0);
  auto One  = B.buildConstant(OpTy, 1);
  auto And = B.buildAnd(OpTy, Op1, One);
  B.buildICmp(CmpInst::ICMP_NE, Dst, And, Zero);
  MI.eraseFromParent();
  return true;
}

static bool legalizeExtend(MachineInstr& MI, MachineIRBuilder& B) {
  auto& MRI = *B.getMRI();
  Register Dst  = MI.getOperand(0).getReg();
  Register Op1  = MI.getOperand(1).getReg();
  assert(MRI.getType(Op1).getSizeInBits() == 1);
  LLT DstTy = MRI.getType(Dst);
  auto Zero = B.buildConstant(DstTy, 0);
  int64_t ExtendedVal = MI.getOpcode() == TargetOpcode::G_SEXT ? -1 : 1;
  auto One = B.buildConstant(DstTy, ExtendedVal);
  B.buildSelect(Dst, Op1, One, Zero);
  MI.eraseFromParent();
  return true;
}

static bool legalizeLoadStore(MachineInstr& MI, MachineIRBuilder& B) {
  auto& MRI = *B.getMRI();
  auto& ValMO = MI.getOperand(0);
  Register Val = ValMO.getReg();
  MachineMemOperand& MMO = **MI.memoperands_begin();
  LLT CurTy = MRI.getType(Val);
  assert(CurTy.getScalarType().isPointer());
  LLT NewTy = CurTy.changeElementType(LLT::scalar(CurTy.getScalarSizeInBits()));
  Register NewVal = MRI.createGenericVirtualRegister(NewTy);
  MMO.setType(NewTy);
  ValMO.setReg(NewVal);
  if (MI.getOpcode() == TargetOpcode::G_LOAD) {
    B.setInsertPt(B.getMBB(), ++B.getInsertPt());
    B.buildIntToPtr(Val, NewVal);
  } else {
    B.buildPtrToInt(NewVal, Val);
  }
  return true;
}

bool pISALegalizerInfo::legalizeCustom(
    LegalizerHelper &Helper, MachineInstr &MI, 
    LostDebugLocObserver &LocObserver) const {

  MachineIRBuilder &B = Helper.MIRBuilder;
  switch (MI.getOpcode()) {
  case TargetOpcode::G_FLOG:
    return legalizeFlog(MI, B, numbers::ln2f);
  case TargetOpcode::G_FLOG10:
    return legalizeFlog(MI, B, numbers::ln2f / numbers::ln10f);
  case TargetOpcode::G_FEXP:
    return legalizeFExp(MI, B);
  case TargetOpcode::G_FCMP:
    return legalizeCondCode(MI, B);
  case TargetOpcode::G_TRUNC:
    return legalizeTrunc(MI, B);
  case TargetOpcode::G_ZEXT:
  case TargetOpcode::G_SEXT:
  case TargetOpcode::G_ANYEXT:
    return legalizeExtend(MI, B);
  case TargetOpcode::G_STORE:
  case TargetOpcode::G_LOAD:
    return legalizeLoadStore(MI, B);
  }
  assert(0 && "unhandled!");
  return false;
}
