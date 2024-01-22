//===-- pISAMCTargetDesc.h - pISA Target Descriptions --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides pISA specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_MCTARGETDESC_pISAMCTARGETDESC_H
#define LLVM_LIB_TARGET_pISA_MCTARGETDESC_pISAMCTARGETDESC_H

#include "llvm/MC/MCInstrDesc.h"
#include "llvm/Support/DataTypes.h"
#include <memory>

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstPrinter;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCStreamer;
class MCSubtargetInfo;
class MCTargetOptions;
class MCTargetStreamer;
class Target;
class formatted_raw_ostream;

namespace pISA {
enum OperandType : unsigned {
  OPERAND_NEGATE = MCOI::OPERAND_FIRST_TARGET,
  OPERAND_SWIZZLE,
};
}

MCTargetStreamer *createpISAAsmTargetStreamer(MCStreamer &S,
                                              formatted_raw_ostream &OS,
                                              MCInstPrinter *InstPrint,
                                              bool IsVerboseAsm);
MCTargetStreamer *createpISAObjectTargetStreamer(MCStreamer &S,
                                                 const MCSubtargetInfo &STI);
MCTargetStreamer *createpISANullTargetStreamer(MCStreamer &S);

} // namespace llvm

// Defines symbolic names for pISA registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "pISAGenRegisterInfo.inc"

// Defines symbolic names for the pISA instructions.
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "pISAGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "pISAGenSubtargetInfo.inc"

#endif // LLVM_LIB_TARGET_pISA_MCTARGETDESC_pISAMCTARGETDESC_H
