//=- pISAMCInstLower.h -- Convert pISA MachineInstr to MCInst --*- C++ -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_pISAMCINSTLOWER_H
#define LLVM_LIB_TARGET_pISA_pISAMCINSTLOWER_H

#include "pISADefines.h"
#include "pISARegisterInfo.h"
#include "llvm/Support/Compiler.h"
#include "llvm/MC/MCInst.h"
#include <cassert>

namespace llvm {
class MCContext;
class MCOperand;
class MCSymbol;
class MachineInstr;
class MachineOperand;

namespace pISA {
class RegManager;
}

class pISAMCInst : public MCInst {
private:
  DebugLoc dbgLoc;
  unsigned int Id = 0;

public:
  const DebugLoc &getDebugLoc() const { return dbgLoc; }
  void setDebugLoc(const DebugLoc &dbg) { dbgLoc = dbg; }
  unsigned int getId() const { return Id; }
  void setId(unsigned int i) { Id = i; }
};

// This class is used to lower a MachineInstr into an MCInst.
class LLVM_LIBRARY_VISIBILITY pISAMCInstLower {
public:
  static constexpr unsigned MAX_ENCODED_ARGS = 4;
  union BitEncoder {
    unsigned int Value;
    struct OpBits {
      union OpEncoder {
        unsigned char Value;
        struct Fields {
          // This tells us whether the given immediate value represents the
          // index of a variable (.e.g, @R0).
          unsigned char IsVariable : 1;
          unsigned char Swizzle    : 3;
        } fields;
      } Op;
    };
    OpBits Op[MAX_ENCODED_ARGS];
  };
  static_assert(sizeof(BitEncoder) == sizeof(unsigned));

  static void setVariableRef(MCInst &MI, unsigned OpNo) {
    assert(OpNo < MAX_ENCODED_ARGS && "OpNo out of range!");

    // Use flags to indicate that ith operand of MCInst is private.
    // This is used to emit a different prefix for it.
    unsigned CurFlags = MI.getFlags();
    assert(BitEncoder{ CurFlags }.Op[OpNo].Op.Value == 0 && "already set?");
    BitEncoder BC{};
    BC.Op[OpNo].Op.fields.IsVariable = 1;

    CurFlags |= BC.Value;
    MI.setFlags(CurFlags);
  }

  static bool isVariableRef(const MCInst &MI, unsigned int OpNo) {
    if (OpNo >= MAX_ENCODED_ARGS)
      return false;
    BitEncoder BC{ MI.getFlags() };
    return BC.Op[OpNo].Op.fields.IsVariable;
  }

  void setSwizzle(MCInst &MI, unsigned OpNo, unsigned SubReg) const {
    if (OpNo >= MAX_ENCODED_ARGS)
      return;

    static_assert(static_cast<unsigned>(pISA::Swizzle::NONE) <= 7);

    auto Swizzle = static_cast<unsigned>(TRI.getSwizzle(SubReg));

    unsigned CurFlags = MI.getFlags();
    assert(BitEncoder{ CurFlags }.Op[OpNo].Op.Value == 0 && "already set?");
    BitEncoder BC{};
    BC.Op[OpNo].Op.fields.Swizzle = Swizzle;

    CurFlags |= BC.Value;
    MI.setFlags(CurFlags);
  }

  static pISA::Swizzle getSwizzle(const MCInst& MI, unsigned OpNo) {
    if (OpNo >= MAX_ENCODED_ARGS)
      return pISA::Swizzle::NONE;
    BitEncoder BC{ MI.getFlags() };

    return static_cast<pISA::Swizzle>(BC.Op[OpNo].Op.fields.Swizzle);
  }

  pISAMCInstLower(
    MCContext &ctx,
    const pISARegisterInfo &TRI,
    const pISA::RegManager &RegMgr) :
    OutContext(ctx), TRI(TRI), RegMgr(RegMgr) {}
  void lower(const MachineInstr *MI, MCInst &OutMI) const;

private:
  MCOperand lowerSymbolOperand(const MachineOperand &MO, MCSymbol &Sym) const;
  MCSymbol &getGlobalAddressSymbol(const MachineOperand &MO) const;

private:
  MCContext &OutContext;
  const pISARegisterInfo &TRI;
  const pISA::RegManager &RegMgr;
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_pISA_pISAMCINSTLOWER_H
