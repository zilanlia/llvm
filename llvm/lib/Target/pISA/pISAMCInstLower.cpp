//=- pISAMCInstLower.cpp - Convert pISA MachineInstr to MCInst -*- C++ -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower pISA MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "pISAMCInstLower.h"
#include "MCTargetDesc/pISAMCExpr.h"
#include "pISA.h"
#include "pISARegManager.h"
#include "pISAUtils.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/IR/Constants.h"
#include "llvm/MC/MCContext.h"

using namespace llvm;

MCOperand pISAMCInstLower::lowerSymbolOperand(const MachineOperand &MO,
                                              MCSymbol &Sym) const {
  const MCExpr *Expr = pISAGlobalAddressMCExpr::create(Sym, OutContext);
  return MCOperand::createExpr(Expr);
}

MCSymbol &
pISAMCInstLower::getGlobalAddressSymbol(const MachineOperand &MO) const {
  assert(MO.isGlobal());
  return *OutContext.getOrCreateSymbol(MO.getGlobal()->getName());
}

void pISAMCInstLower::lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());
  for (unsigned OpNo = 0, e = MI->getNumOperands(); OpNo != e; ++OpNo) {
    const MachineOperand &MO = MI->getOperand(OpNo);
    MCOperand MCOp;
    switch (MO.getType()) {
    default:
      llvm_unreachable("unknown operand type");
    case MachineOperand::MO_Metadata: {
      // Skip call to addOperand() outside the switch as MCInst doesn't
      // support metadata operand types.
      continue;
      break;
    }
    case MachineOperand::MO_FrameIndex: {
      // MachineOperand of type MO_FrameIndex is lowered to immediate operand
      // with value equal to frame index. Whether an immediate operand is
      // frame index is indicated by flags that are stored in MCInst instance.
      // During operand printing, we inspect flag value first and decide if
      // the operand has to be printed as private variable or a generic
      // immediate value.
      unsigned int FrameIndex = MO.getIndex();
      MCOp = MCOperand::createImm(FrameIndex);
      setVariableRef(OutMI, OpNo);
      break;
    }
    case MachineOperand::MO_GlobalAddress: {
      MCOp = lowerSymbolOperand(MO, getGlobalAddressSymbol(MO));
      break;
    }
    case MachineOperand::MO_MachineBasicBlock:
      MCOp = MCOperand::createExpr(
          MCSymbolRefExpr::create(MO.getMBB()->getSymbol(), OutContext));
      break;
    case MachineOperand::MO_Register: {
      Register CurReg = MO.getReg();
      unsigned EncodedVal = CurReg;
      if (CurReg.isVirtual()) {
        if (OutMI.getOpcode() == pISA::DBG_VALUE && !RegMgr.exists(CurReg)) {
          // If MachineOperand that DBG_VALUE points to was removed then
          // we emit "undef" for that variable.
          MCOp = MCOperand::createReg(MCRegister::NoRegister);
          break;
        }
        EncodedVal = RegMgr.encodeVirtualRegister(CurReg);
      }
      MCOp = MCOperand::createReg(EncodedVal);
      setSwizzle(OutMI, OpNo, MO.getSubReg());
      break;
    }
    case MachineOperand::MO_Immediate:
      MCOp = MCOperand::createImm(MO.getImm());
      break;
    case MachineOperand::MO_FPImmediate:
      // All floating point values (including bfloat and fp8) are bitcasted
      // into integer ones. Double- and single-precison ones could be specially
      // taged in MC as SFP or DFP immediate. But, in general, as each type of
      // immediate operand has its own methods, general immediate operands
      // won't loss any semantics.
      uint64_t ImmVal =
          MO.getFPImm()->getValueAPF().bitcastToAPInt().getZExtValue();
      if (MO.getFPImm()->getType()->isFloatTy())
        MCOp = MCOperand::createSFPImm(ImmVal);
      else if (MO.getFPImm()->getType()->isDoubleTy())
        MCOp = MCOperand::createDFPImm(ImmVal);
      else
        MCOp = MCOperand::createImm(ImmVal);
      break;
    }

    OutMI.addOperand(MCOp);
  }
}
