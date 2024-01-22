//=== lib/Target/pISA/pISADetermineStackID.cpp ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass examines stack objects and determines the appropriate stackID
//
//===----------------------------------------------------------------------===//

#include "pISA.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/IR/Instructions.h"

#define DEBUG_TYPE "pisa-determine-stack-id"

using namespace llvm;

namespace {

class pISADetermineStackID : public MachineFunctionPass {
public:
  static char ID;

  pISADetermineStackID();

  StringRef getPassName() const override {
    return "pISADetermineStackID";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};
} // end anonymous namespace

void pISADetermineStackID::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  MachineFunctionPass::getAnalysisUsage(AU);
}

pISADetermineStackID::pISADetermineStackID()
  : MachineFunctionPass(ID) {
  initializepISADetermineStackIDPass(*PassRegistry::getPassRegistry());
}

bool pISADetermineStackID::runOnMachineFunction(MachineFunction &MF) {
  bool Changed = false;
  auto &MFI = MF.getFrameInfo();
  for (int Idx = MFI.getObjectIndexBegin(), EndIdx = MFI.getObjectIndexEnd();
       Idx != EndIdx; ++Idx) {
    if (auto *AI = MFI.getObjectAllocation(Idx)) {
      if (AI->getAddressSpace() == unsigned(pISA::AddressSpace::SHARED)) {
        assert(MF.getFunction().getCallingConv() != CallingConv::PISA_FUNC &&
          "shared variables are not supported in functions!");
        Changed = true;
        MFI.setStackID(Idx, TargetStackID::pISAShared);
      }
    }
  }
  return Changed;
}

char pISADetermineStackID::ID = 0;
INITIALIZE_PASS_BEGIN(pISADetermineStackID, DEBUG_TYPE,
                      "Determine stack IDs for stack objects",
                      false, false)
INITIALIZE_PASS_END(pISADetermineStackID, DEBUG_TYPE,
                    "Combine pISA machine instrs before legalization", false,
                    false)

namespace llvm {
FunctionPass *createpISADetermineStackID() {
  return new pISADetermineStackID();
}
} // end namespace llvm
