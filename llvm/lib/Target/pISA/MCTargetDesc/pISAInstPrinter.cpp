//===-- pISAInstPrinter.cpp - Output pISA MCInsts as ASM -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class prints a pISA MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "pISAInstPrinter.h"
#include "MCTargetDesc/pISARegEncoder.h"
#include "pISA.h"
#include "pISABaseInfo.h"
#include "pISAMCExpr.h"
#include "pISAMCInstLower.h"
#include "pISAUtils.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;
using namespace llvm::pISA;

#define DEBUG_TYPE "asm-printer"

// Include the auto-generated portion of the assembly writer.
#include "pISAGenAsmWriter.inc"

void pISAInstPrinter::printDebugLoc(const MCInst *MI, raw_ostream &OS) {
  const auto *pISAInst = static_cast<const pISAMCInst *>(MI);
  const auto &DbgLoc = pISAInst->getDebugLoc();
  if (!DbgLoc)
    return;

  OS << " !dbg !" << pISAInst->getId();
}

void pISAInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                StringRef Annot, const MCSubtargetInfo &STI,
                                raw_ostream &OS) {
  printInstruction(MI, Address, OS);
  printAnnotation(OS, Annot);
  printDebugLoc(MI, OS);
}

void pISAInstPrinter::printRegName(raw_ostream &OS, MCRegister Reg) const {
  if (MCRegister::isPhysicalRegister(Reg)) {
    OS << getRegisterName(Reg);
  } else {
    auto [Prefix, Idx] = RegEncoder::decodeVirtualRegister(Reg);
    OS << Prefix << Idx;
  }
}

void pISAInstPrinter::printImm1Opnd(const MCInst *MCI, unsigned OpNo,
                                    raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  OS << formatImm(MCOp.getImm());
}

void pISAInstPrinter::printImm8Opnd(const MCInst *MCI, unsigned OpNo,
                                    raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  OS << formatImm(MCOp.getImm());
}

void pISAInstPrinter::printImm16Opnd(const MCInst *MCI, unsigned OpNo,
                                     raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  OS << formatImm(MCOp.getImm());
}

void pISAInstPrinter::printImm32Opnd(const MCInst *MCI, unsigned OpNo,
                                     raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  OS << formatImm(MCOp.getImm());
}

void pISAInstPrinter::printImm64Opnd(const MCInst *MCI, unsigned OpNo,
                                     raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  OS << formatImm(MCOp.getImm());
}

void pISAInstPrinter::printFpImm16Opnd(const MCInst *MCI, unsigned OpNo,
                                       raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  OS << format_hex(MCOp.getImm(), /*Width=*/4, /*Upper=*/true);
}

void pISAInstPrinter::printFpImm32Opnd(const MCInst *MCI, unsigned OpNo,
                                       raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  OS << format_hex(MCOp.getSFPImm(), /*Width=*/8, /*Upper=*/true);
}

void pISAInstPrinter::printFpImm64Opnd(const MCInst *MCI, unsigned OpNo,
                                       raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  OS << format_hex(MCOp.getDFPImm(), /*Width=*/16, /*Upper=*/true);
}

void pISAInstPrinter::printPredOpnd(const MCInst *MCI, unsigned OpNo,
                                    raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  printRegName(OS, MCOp.getReg());
  printSwizzle(MCI, OpNo, OS);
}

void pISAInstPrinter::printReg8bOpnd(const MCInst *MCI, unsigned OpNo,
                                     raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  printRegName(OS, MCOp.getReg());
  printSwizzle(MCI, OpNo, OS);
}

void pISAInstPrinter::printReg16bOpnd(const MCInst *MCI, unsigned OpNo,
                                      raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  printRegName(OS, MCOp.getReg());
  printSwizzle(MCI, OpNo, OS);
}

void pISAInstPrinter::printReg32bOpnd(const MCInst *MCI, unsigned OpNo,
                                      raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  printRegName(OS, MCOp.getReg());
  printSwizzle(MCI, OpNo, OS);
}

void pISAInstPrinter::printReg64bOpnd(const MCInst *MCI, unsigned OpNo,
                                      raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  printRegName(OS, MCOp.getReg());
  printSwizzle(MCI, OpNo, OS);
}

void pISAInstPrinter::printRegV2_8bOpnd(const MCInst *MCI, unsigned OpNo,
                                      raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  printRegName(OS, MCOp.getReg());
  printSwizzle(MCI, OpNo, OS);
}

void pISAInstPrinter::printRegV4_8bOpnd(const MCInst *MCI, unsigned OpNo,
                                      raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  printRegName(OS, MCOp.getReg());
  printSwizzle(MCI, OpNo, OS);
}

void pISAInstPrinter::printRegV2_16bOpnd(const MCInst *MCI, unsigned OpNo,
                                      raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  printRegName(OS, MCOp.getReg());
  printSwizzle(MCI, OpNo, OS);
}

void pISAInstPrinter::printRegV3_16bOpnd(const MCInst *MCI, unsigned OpNo,
                                      raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  printRegName(OS, MCOp.getReg());
  printSwizzle(MCI, OpNo, OS);
}

void pISAInstPrinter::printRegV4_16bOpnd(const MCInst *MCI, unsigned OpNo,
                                      raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  printRegName(OS, MCOp.getReg());
  printSwizzle(MCI, OpNo, OS);
}

void pISAInstPrinter::printRegV2_32bOpnd(const MCInst *MCI, unsigned OpNo,
                                      raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  printRegName(OS, MCOp.getReg());
  printSwizzle(MCI, OpNo, OS);
}

void pISAInstPrinter::printRegV3_32bOpnd(const MCInst *MCI, unsigned OpNo,
                                      raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  printRegName(OS, MCOp.getReg());
  printSwizzle(MCI, OpNo, OS);
}

void pISAInstPrinter::printRegV4_32bOpnd(const MCInst *MCI, unsigned OpNo,
                                      raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  printRegName(OS, MCOp.getReg());
  printSwizzle(MCI, OpNo, OS);
}

void pISAInstPrinter::printRegV2_64bOpnd(const MCInst *MCI, unsigned OpNo,
                                      raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  printRegName(OS, MCOp.getReg());
  printSwizzle(MCI, OpNo, OS);
}

void pISAInstPrinter::printRegV3_64bOpnd(const MCInst *MCI, unsigned OpNo,
                                      raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  printRegName(OS, MCOp.getReg());
  printSwizzle(MCI, OpNo, OS);
}

void pISAInstPrinter::printRegV4_64bOpnd(const MCInst *MCI, unsigned OpNo,
                                      raw_ostream &OS) {
  const MCOperand &MCOp = MCI->getOperand(OpNo);
  printRegName(OS, MCOp.getReg());
  printSwizzle(MCI, OpNo, OS);
}


void pISAInstPrinter::printSwizzle(const MCInst *MI, unsigned OpNo,
                                   raw_ostream &O) {
  auto Swizzle = pISAMCInstLower::getSwizzle(*MI, OpNo);
  swizzleRepr(O, static_cast<unsigned>(Swizzle));
}

void pISAInstPrinter::printRegNoSwizzle(const MCInst *MI, unsigned OpNo,
                                        raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  printRegName(O, Op.getReg());
}

void pISAInstPrinter::printFenceScope(const MCInst *MI, unsigned OpNo,
                                      raw_ostream &O) {
  auto Scope = static_cast<pISA::FenceScope>(MI->getOperand(OpNo).getImm());

#define CASE(X)                                                                \
  case FenceScope::X:                                                          \
    O << #X;                                                                   \
    break;

  switch (Scope) {
    CASE(workgroup)
    CASE(local)
    CASE(tile)
    CASE(gpu)
    CASE(system)
  }
#undef CASE
}

void pISAInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                   raw_ostream &O, const char *Modifier) {
  assert((Modifier == 0 || Modifier[0] == 0) && "No modifiers supported");
  if (pISAMCInstLower::isVariableRef(*MI, OpNo)) {
    assert(MI->getOperand(OpNo).isImm() && "expecting imm operand");
    unsigned int FrameIndex = MI->getOperand(OpNo).getImm();
    O << "@R" << FrameIndex;
  } else if (OpNo < MI->getNumOperands()) {
    const MCOperand &Op = MI->getOperand(OpNo);
    if (Op.isReg()) {
      printRegName(O, Op.getReg());
      printSwizzle(MI, OpNo, O);
    } else if (Op.isImm())
      O << formatImm((int64_t)Op.getImm());
    else if (Op.isDFPImm())
      O << formatImm((double)Op.getDFPImm());
    else if (Op.isExpr())
      Op.getExpr()->print(O, &MAI);
    else
      llvm_unreachable("Unexpected operand type");
  }
}

void pISAInstPrinter::negateRepr(const MCInst *MI, unsigned OpNo,
                                 raw_ostream &O) {
  int64_t DoNegate = MI->getOperand(OpNo).getImm();
  if (DoNegate)
    O << "!";
}

void pISAInstPrinter::swizzleRepr(raw_ostream &O, unsigned SwizzleVal) {
  switch (static_cast<Swizzle>(SwizzleVal)) {
  case Swizzle::X:
    O << ".x";
    break;
  case Swizzle::Y:
    O << ".y";
    break;
  case Swizzle::Z:
    O << ".z";
    break;
  case Swizzle::W:
    O << ".w";
    break;
  case Swizzle::XYZW:
    O << ".xyzw";
    break;
  case Swizzle::XY:
    O << ".xy";
    break;
  case Swizzle::ZW:
    O << ".zw";
    break;
  case Swizzle::NONE:
    break;
  default:
    llvm_unreachable("Wrong swizzle");
  }
}

void pISAInstPrinter::swizzleRepr(const MCInst *MI, unsigned OpNo,
                                  raw_ostream &O) {
  swizzleRepr(O, MI->getOperand(OpNo).getImm());
}

void pISAInstPrinter::printCallTargetFunc(const MCInst *MI, unsigned OpNo,
                                          raw_ostream &O) {
  printOperand(MI, OpNo, O);

  // print function args if any
  O << " (";
  for (unsigned ArgIdx = OpNo + 1; ArgIdx < MI->getNumOperands(); ++ArgIdx) {
    printOperand(MI, ArgIdx, O);
    if (ArgIdx + 1 < MI->getNumOperands())
      O << ", ";
  }
  O << ")";
}

void pISAInstPrinter::printAddrOffsetImm(const MCInst *MI, unsigned OpNo,
                                         raw_ostream &O) {
  int64_t Val = MI->getOperand(OpNo).getImm();
  // print imm offset only when it's not zero
  if (Val > 0) {
    O << " + " << formatImm(Val);
  } else if (Val < 0) {
    if (Val == std::numeric_limits<int64_t>::min())
      O << " - " << llvm::format("%" PRIu64, Val);
    else
      O << " - " << formatImm(-Val);
  }
}

void pISAInstPrinter::printStringImm(const MCInst *MI, unsigned OpNo,
                                     raw_ostream &O) {
  const unsigned NumOps = MI->getNumOperands();
  unsigned StrStartIndex = OpNo;
  while (StrStartIndex < NumOps) {
    if (MI->getOperand(StrStartIndex).isReg())
      break;

    std::string Str = getpISAStringOperand(*MI, OpNo);
    if (StrStartIndex != OpNo)
      O << ' '; // Add a space if we're starting a new string/argument.
    O << '"';
    for (char c : Str) {
      if (c == '"')
        O.write('\\'); // Escape " characters (might break for complex UTF-8).
      O.write(c);
    }
    O << '"';

    unsigned numOpsInString = (Str.size() / 4) + 1;
    StrStartIndex += numOpsInString;
  }
}

void pISAInstPrinter::printMemOperand(const MCInst *MI, int OpNo,
                                         raw_ostream &OS,
                                         const char * /*Modifier*/) {
  // [ Base OP Offset ]
  OS << "[";
  printOperand(MI, OpNo, OS);
  const MCOperand &OffsetOp = MI->getOperand(OpNo + 1);
  if (OffsetOp.isImm()) {
    auto Val = OffsetOp.getImm();
    if (Val != 0) {
      if (Val > 0)
        OS << " + " << formatImm(Val);
      else if (Val == std::numeric_limits<int64_t>::min())
        OS << " - " << llvm::format("%" PRIu64, Val);
      else
        OS << " - " << formatImm(-Val);
    }
  } else {
    assert(OffsetOp.isReg() && "Register expected");
    OS << " + ";
    printRegName(OS, OffsetOp.getReg());
  }
  OS << "]";
}
