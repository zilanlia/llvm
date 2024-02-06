//===-- pISAInstrInfo.cpp - pISA Instruction Information ------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the pISA implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "pISAInstrInfo.h"
#include "pISA.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_INSTRINFO_CTOR_DTOR
#define GET_INSTRINFO_NAMED_OPS
#define GET_INSTRINFO_OPERAND_ENUM
#include "pISAGenInstrInfo.inc"

using namespace llvm;
using namespace pISA;

pISAInstrInfo::pISAInstrInfo() : pISAGenInstrInfo() {}

bool pISAInstrInfo::isNoEmissionInstr(const MachineInstr &MI) const {
  // Functions are emitted to C-style function signature.
  // These instructions are not required to be in the output.
  return isFunctionParamInstr(MI);
}

bool pISAInstrInfo::isFunctionParamInstr(const MachineInstr &MI) const {
  switch (MI.getOpcode())
  {
  case pISA::functionParameter_8b:
  case pISA::functionParameter_16b:
  case pISA::functionParameter_32b:
  case pISA::functionParameter_64b:
  case pISA::functionParameter_v2_8b:
  case pISA::functionParameter_v4_8b:
  case pISA::functionParameter_v2_16b:
  case pISA::functionParameter_v3_16b:
  case pISA::functionParameter_v4_16b:
  case pISA::functionParameter_v2_32b:
  case pISA::functionParameter_v3_32b:
  case pISA::functionParameter_v4_32b:
  case pISA::functionParameter_v2_64b:
  case pISA::functionParameter_v3_64b:
  case pISA::functionParameter_v4_64b:
    return true;
  default:
    return false;
  }
 }

bool pISAInstrInfo::isLoadParamInstr(const MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  case pISA::loadParam_8b:
  case pISA::loadParam_v2_8b:
  case pISA::loadParam_v4_8b:
  case pISA::loadParam_16b:
  case pISA::loadParam_v2_16b:
  case pISA::loadParam_v4_16b:
  case pISA::loadParam_32b:
  case pISA::loadParam_v2_32b:
  case pISA::loadParam_v3_32b:
  case pISA::loadParam_v4_32b:
  case pISA::loadParam_64b:
  case pISA::loadParam_v2_64b:
    return true;
  default:
    return false;
  }
 }

namespace llvm {
namespace pISA {
  static const MachineOperand& getMO(const MachineInstr& MI, unsigned Name) {
    int16_t Idx = getNamedOperandIdx(MI.getOpcode(), Name);
    assert(Idx >= 0 && "name not present!");

    return MI.getOperand(Idx);
  }
}
}

// See description in TargetInstrInfo.h
bool pISAInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                  MachineBasicBlock *&TBB,
                                  MachineBasicBlock *&FBB,
                                  SmallVectorImpl<MachineOperand> &Cond,
                                  bool /*AllowModify*/) const {
  // If the block has no terminators, it just falls into the block after it.
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  if (I == MBB.end() || !isUnpredicatedTerminator(*I))
    return false;

  // Get the last instruction in the block.
  MachineInstr* LastInst = &*I;

  // If there is only one terminator instruction, process it.
  if (I == MBB.begin() || !isUnpredicatedTerminator(*--I)) {
    if (LastInst->isUnconditionalBranch()) {
      if (LastInst->getOpcode() != pISA::gotolabel)
        return true;
      TBB = LastInst->getOperand(0).getMBB();
      return false;
    }
    else if (LastInst->isConditionalBranch()) {
      if (LastInst->getOpcode() != pISA::predgoto)
        return true;
      // Block ends with fall-through condbranch.
      TBB = getMO(*LastInst, OpName::label).getMBB();
      // mod, pred
      Cond.push_back(LastInst->getOperand(0));
      Cond.push_back(LastInst->getOperand(1));
      return false;
    }
    return true; // Can't handle indirect branch.
  }

  // Get the instruction before it if it is a terminator.
  MachineInstr* SecondLastInst = &*I;

  if (!SecondLastInst->isConditionalBranch() ||
      !LastInst->isUnconditionalBranch()     ||
      // triple terminator?
      (I != MBB.begin() && isUnpredicatedTerminator(*--I)))
    return true;

  if (SecondLastInst->getOpcode() != pISA::predgoto ||
      LastInst->getOpcode() != pISA::gotolabel)
    return true;

  TBB = getMO(*SecondLastInst, OpName::label).getMBB();
  FBB = LastInst->getOperand(0).getMBB();

  // mod, pred
  Cond.push_back(SecondLastInst->getOperand(0));
  Cond.push_back(SecondLastInst->getOperand(1));

  return false;
}

// See description in TargetInstrInfo.h
unsigned pISAInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                     int *BytesRemoved) const {
  assert(!BytesRemoved && "not supported!");

  unsigned Count = 0;
  for (auto &MI : llvm::make_early_inc_range(MBB.terminators())) {
    assert(MI.isBranch() && "not a branch?");
    MI.eraseFromParent();
    Count++;
  }

  return Count;
}

// See description in TargetInstrInfo.h
unsigned pISAInstrInfo::insertBranch(
    MachineBasicBlock &MBB, MachineBasicBlock *TBB, MachineBasicBlock *FBB,
    ArrayRef<MachineOperand> Cond, const DebugLoc &DL, int *BytesAdded) const {

  assert(!BytesAdded && "not supported!");
  assert(TBB && "Should have at least one branch!");

  if (!FBB) {
    if (Cond.empty()) { // Unconditional branch
      BuildMI(&MBB, DL, get(pISA::gotolabel)).addMBB(TBB);
    }
    else { // Conditional branch
      assert(Cond.size() == 2 && "wrong number of args?");
      BuildMI(&MBB, DL, get(pISA::predgoto))
        .add(Cond[0]).add(Cond[1]).addMBB(TBB);
    }
    return 1;
  }

  // Two-way Conditional Branch.
  assert(Cond.size() == 2 && "wrong number of args?");
  BuildMI(&MBB, DL, get(pISA::predgoto))
    .add(Cond[0]).add(Cond[1]).addMBB(TBB);
  BuildMI(&MBB, DL, get(pISA::gotolabel)).addMBB(FBB);
  return 2;
}

bool pISAInstrInfo::reverseBranchCondition(
  SmallVectorImpl<MachineOperand>& Cond) const {
  if (Cond.empty())
    return true;

  uint64_t DoNegate = Cond[0].getImm();
  Cond[0].setImm(!DoNegate);

  return false;
}

// We have to override this because LowerCopy() in ExpandPostRAPseudos
// gets rid of identity copies without checking subregs. It doesn't check
// subregs because register allocation has already happend (but not for pISA)
// so there aren't any subregs.
bool pISAInstrInfo::expandPostRAPseudo(MachineInstr& MI) const {
  if (!MI.isCopy())
    return false;

  MachineOperand &DstMO = MI.getOperand(0);
  MachineOperand &SrcMO = MI.getOperand(1);

  copyPhysReg(*MI.getParent(), MI, MI.getDebugLoc(),
              DstMO.getReg(), SrcMO.getReg(), SrcMO.isKill());

  MI.eraseFromParent();

  return true;
}

void pISAInstrInfo::copyPhysReg(MachineBasicBlock& MBB,
                                MachineBasicBlock::iterator I,
                                const DebugLoc& DL, MCRegister DestReg,
                                MCRegister SrcReg, bool KillSrc) const {

  assert(I->isCopy() && "Copy instruction is expected");
  auto& MRI = I->getMF()->getRegInfo();

  auto getSubregRC = [&](MachineOperand& MO) -> const TargetRegisterClass* {
    unsigned Subreg = MO.getSubReg();
    auto *SuperRC = MRI.getRegClass(MO.getReg());
    if (Subreg == 0)
      return SuperRC;

    return RI.getSubRegisterClass(SuperRC, Subreg);
  };

  auto getSwizzle = [&](const MachineOperand& MO) -> pISA::Swizzle {
    unsigned Subreg = MO.getSubReg();
    if (Subreg == 0) {
      auto* SuperRC = MRI.getRegClass(MO.getReg());
      unsigned NumElts = RI.getNumEltsFromRegClass(SuperRC);
      if (NumElts > 1) {
        switch (NumElts) {
        case 2: return Swizzle::XY;
        case 4: return Swizzle::XYZW;
        default: llvm_unreachable("missing swizzle case!");
        }
      } else {
        return Swizzle::NONE;
      }
    }

    return RI.getSwizzle(Subreg);
  };

  auto& DstOp = I->getOperand(0);
  auto& SrcOp = I->getOperand(1);
  auto* DstSubRC = getSubregRC(DstOp);
  auto* SrcSubRC = getSubregRC(SrcOp);

  unsigned Op = 0;
  bool IsBitcast = true;

  const unsigned DstSubNumElts   = RI.getNumEltsFromRegClass(DstSubRC);
  const unsigned DstSubEltSize   = RI.getBitSizeFromRegClass(DstSubRC);
  const unsigned SrcSubNumElts   = RI.getNumEltsFromRegClass(SrcSubRC);
  const unsigned SrcSubEltSize   = RI.getBitSizeFromRegClass(SrcSubRC);
  const bool DstSubIsVector      = DstSubNumElts > 1;
  const bool SrcSubIsVector      = SrcSubNumElts > 1;
  const bool DstSubIsScalar      = !DstSubIsVector;
  const bool SrcSubIsScalar      = !SrcSubIsVector;

  if (DstSubIsVector && SrcSubIsVector) {
    if (DstSubNumElts == 4 && DstSubEltSize == 8 && SrcSubNumElts == 2 &&
        SrcSubEltSize == 16)
      Op = pISA::bitcast_v4_8b_v2_16b;
    else if (DstSubNumElts == 2 && DstSubEltSize == 16 && SrcSubNumElts == 4 &&
             SrcSubEltSize == 8)
      Op = pISA::bitcast_v2_16b_v4_8b;
    else if (DstSubNumElts == 4 && DstSubEltSize == 16 && SrcSubNumElts == 2 &&
             SrcSubEltSize == 32)
      Op = pISA::bitcast_v4_16b_v2_32b;
    else if (DstSubNumElts == 2 && DstSubEltSize == 32 && SrcSubNumElts == 4 &&
             SrcSubEltSize == 16)
      Op = pISA::bitcast_v2_32b_v4_16b;
    else {
      assert(DstSubNumElts == SrcSubNumElts && "num elts mismatch!");
      assert(SrcOp.getSubReg() == 0 && "vector copy can't select swizzle!");
      assert(DstOp.getSubReg() == 0 && "vector copy can't select swizzle!");
      IsBitcast = false;
      if (DstSubNumElts == 2) {
        switch (DstSubEltSize) {
        case 8:  Op = pISA::mov_v2_8r;  break;
        case 16: Op = pISA::mov_v2_16r; break;
        case 32: Op = pISA::mov_v2_32r; break;
        case 64: Op = pISA::mov_v2_64r; break;
        default: llvm_unreachable("unknown elt size!");
        }
      } else if (DstSubNumElts == 3) {
        switch (DstSubEltSize) {
        case 16: Op = pISA::mov_v3_16r; break;
        case 32: Op = pISA::mov_v3_32r; break;
        case 64: Op = pISA::mov_v3_64r; break;
        default: llvm_unreachable("unknown elt size!");
        }
      } else if (DstSubNumElts == 4) {
        switch (DstSubEltSize) {
        case 8:  Op = pISA::mov_v4_8r;  break;
        case 16: Op = pISA::mov_v4_16r; break;
        case 32: Op = pISA::mov_v4_32r; break;
        case 64: Op = pISA::mov_v4_64r; break;
        default: llvm_unreachable("unknown elt size!");
        }
      } else {
        llvm_unreachable("unknown vec size!");
      }
    }
  } else if (DstSubIsVector && SrcSubIsScalar) {
    if (DstSubNumElts == 2 && DstSubEltSize == 8 && SrcSubEltSize == 16)
      Op = pISA::bitcast_v2_8b_16b;
    else if (DstSubNumElts == 4 && DstSubEltSize == 8 && SrcSubEltSize == 32)
      Op = pISA::bitcast_v4_8b_32b;
    else if (DstSubNumElts == 2 && DstSubEltSize == 16 && SrcSubEltSize == 32)
      Op = pISA::bitcast_v2_16b_32b;
    else if (DstSubNumElts == 4 && DstSubEltSize == 16 && SrcSubEltSize == 64)
      Op = pISA::bitcast_v4_16b_64b;
    else if (DstSubNumElts == 2 && DstSubEltSize == 32 && SrcSubEltSize == 64)
      Op = pISA::bitcast_v2_32b_64b;
    else if (DstSubNumElts == 2 && DstSubEltSize == 8 && SrcSubEltSize == 8)
      Op = pISA::bitcast_v2_8b_8b;
    else if (DstSubNumElts == 4 && DstSubEltSize == 8 && SrcSubEltSize == 8)
      Op = pISA::bitcast_v4_8b_8b;
    else if (DstSubNumElts == 2 && DstSubEltSize == 16 && SrcSubEltSize == 16)
      Op = pISA::bitcast_v2_16b_16b;
    else if (DstSubNumElts == 3 && DstSubEltSize == 16 && SrcSubEltSize == 16)
      Op = pISA::bitcast_v3_16b_16b;
    else if (DstSubNumElts == 4 && DstSubEltSize == 16 && SrcSubEltSize == 16)
      Op = pISA::bitcast_v4_16b_16b;
    else if (DstSubNumElts == 2 && DstSubEltSize == 32 && SrcSubEltSize == 32)
      Op = pISA::bitcast_v2_32b_32b;
    else if (DstSubNumElts == 3 && DstSubEltSize == 32 && SrcSubEltSize == 32)
      Op = pISA::bitcast_v3_32b_32b;
    else if (DstSubNumElts == 4 && DstSubEltSize == 32 && SrcSubEltSize == 32)
      Op = pISA::bitcast_v4_32b_32b;
    else if (DstSubNumElts == 2 && DstSubEltSize == 64 && SrcSubEltSize == 64)
      Op = pISA::bitcast_v2_64b_64b;
    else if (DstSubNumElts == 3 && DstSubEltSize == 64 && SrcSubEltSize == 64)
      Op = pISA::bitcast_v3_64b_64b;
    else if (DstSubNumElts == 4 && DstSubEltSize == 64 && SrcSubEltSize == 64)
      Op = pISA::bitcast_v4_64b_64b;
    else
      llvm_unreachable("wrong copy instruction");
  } else if (DstSubIsScalar && SrcSubIsVector) {
    if (DstSubEltSize == 16 && SrcSubNumElts == 2 && SrcSubEltSize == 8)
      Op = pISA::bitcast_16b_v2_8b;
    else if (DstSubEltSize == 32 && SrcSubNumElts == 2 && SrcSubEltSize == 16)
      Op = pISA::bitcast_32b_v2_16b;
    else if (DstSubEltSize == 32 && SrcSubNumElts == 4 && SrcSubEltSize == 8)
      Op = pISA::bitcast_32b_v4_8b;
    else if (DstSubEltSize == 64 && SrcSubNumElts == 2 && SrcSubEltSize == 32)
      Op = pISA::bitcast_64b_v2_32b;
    else if (DstSubEltSize == 64 && SrcSubNumElts == 4 && SrcSubEltSize == 16)
      Op = pISA::bitcast_64b_v4_16b;
    else if (DstSubEltSize == 8 && SrcSubNumElts == 2 && SrcSubEltSize == 8)
      Op = pISA::bitcast_8b_v2_8b;
    else if (DstSubEltSize == 8 && SrcSubNumElts == 4 && SrcSubEltSize == 8)
      Op = pISA::bitcast_8b_v4_8b;
    else if (DstSubEltSize == 16 && SrcSubNumElts == 2 && SrcSubEltSize == 16)
      Op = pISA::bitcast_16b_v2_16b;
    else if (DstSubEltSize == 16 && SrcSubNumElts == 3 && SrcSubEltSize == 16)
      Op = pISA::bitcast_16b_v3_16b;
    else if (DstSubEltSize == 16 && SrcSubNumElts == 4 && SrcSubEltSize == 16)
      Op = pISA::bitcast_16b_v4_16b;
    else if (DstSubEltSize == 32 && SrcSubNumElts == 2 && SrcSubEltSize == 32)
      Op = pISA::bitcast_32b_v2_32b;
    else if (DstSubEltSize == 32 && SrcSubNumElts == 3 && SrcSubEltSize == 32)
      Op = pISA::bitcast_32b_v3_32b;
    else if (DstSubEltSize == 32 && SrcSubNumElts == 4 && SrcSubEltSize == 32)
      Op = pISA::bitcast_32b_v4_32b;
    else if (DstSubEltSize == 64 && SrcSubNumElts == 2 && SrcSubEltSize == 64)
      Op = pISA::bitcast_64b_v2_64b;
    else if (DstSubEltSize == 64 && SrcSubNumElts == 3 && SrcSubEltSize == 64)
      Op = pISA::bitcast_64b_v3_64b;
    else if (DstSubEltSize == 64 && SrcSubNumElts == 4 && SrcSubEltSize == 64)
      Op = pISA::bitcast_64b_v4_64b;
    else
      llvm_unreachable("wrong bitcast operation");
  } else if (DstSubIsScalar && SrcSubIsScalar) {
    IsBitcast = false;

    switch (DstSubEltSize) {
    case 1:  Op = pISA::mov_8r;  break;
    case 8:  Op = pISA::mov_8r;  break;
    case 16: Op = pISA::mov_16r; break;
    case 32: Op = pISA::mov_32r; break;
    case 64: Op = pISA::mov_64r; break;
    default: llvm_unreachable("unknown elt size!");
    }
  }

  unsigned DstSubreg = DstOp.getSubReg();
  unsigned SrcSubreg = SrcOp.getSubReg();

  auto MIB = BuildMI(MBB, I, DL, get(Op))
    .addDef(DstOp.getReg(), 0, DstSubreg)
    .addReg(SrcOp.getReg(), getKillRegState(KillSrc), SrcSubreg);
  if (IsBitcast) {
    MIB.addImm((unsigned)getSwizzle(DstOp))
       .addImm((unsigned)getSwizzle(SrcOp));
  }
}

