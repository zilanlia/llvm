//===-- pISATargetStreamer.h - pISA Target Streamer ----------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LIB_TARGET_pISA_MCTARGETDESC_pISATARGETSTREAMER_H
#define LIB_TARGET_pISA_MCTARGETDESC_pISATARGETSTREAMER_H

#include "llvm/CodeGen/LowLevelType.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/MC/MCStreamer.h"
#include <optional>
#include <tuple>
#include <utility>

namespace llvm {

namespace pISA {
// pISA function and directive name
struct FunctionDirectiveAndName {
  // Calling convention.
  CallingConv::ID CC;
  // A single bit (export or import) for the optional external linkage.
  std::optional<bool> Linkage;
  // Function name.
  std::string Name;
  // A optional return LLT for non-kernel functions.
  std::optional<LLT> RetLLT;
};
struct FunctionParameter {
  LLT Ty;
  const char *Prefix;
  unsigned Idx;
};
struct KernelParameter {
  unsigned Size;
};
// pISA function signature
struct FunctionSignature {
  FunctionDirectiveAndName DN;
  // FIXME: Need to unify kernel and function parameters.
  std::vector<FunctionParameter> FunctionParams;
  std::vector<KernelParameter> KernelParams;
  std::optional<unsigned> DbgId;
};
// pISA function register declarations
struct RegDcls {
  MapVector<std::tuple</*NumElts=*/unsigned, /*BitWidth=*/unsigned,
                       /*Type=*/unsigned>,
            std::vector<std::pair</*Prefix=*/const char *, /*Id=*/unsigned>>>
      Regs;
};
// pISA function stack variables
struct StackVariableDcl {
  unsigned StackId;
  int64_t Size;
  Align Alignment;
};
struct StackVariableDcls {
  std::vector<StackVariableDcl> Vars;
};
} // namespace pISA

class pISATargetStreamer : public MCTargetStreamer {
public:
  pISATargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}
  ~pISATargetStreamer() override;

  virtual void emitFunctionSignature(pISA::FunctionSignature &) = 0;
  virtual void emitRegDcls(pISA::RegDcls &) = 0;
  virtual void emitStackVariableDcls(pISA::StackVariableDcls &) = 0;
  virtual void emitFuncBodyStart() = 0;
  virtual void emitFuncBodyEnd() = 0;
};
} // namespace llvm

#endif // LIB_TARGET_pISA_MCTARGETDESC_pISATARGETSTREAMER_H_
