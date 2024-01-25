//=- pISACombine.td - Define pISA Combine Rules ----------*- tablegen -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "llvm/Target/GlobalISel/Combine.td"

def extra_commute_constant_to_rhs : GICombineRule<
  (defs root:$root),
  (match (wip_match_opcode G_SMIN, G_SMAX, G_UMIN, G_UMAX, G_UADDO, G_SADDO,
                           G_UMULO, G_SMULO, G_UMULH, G_SMULH, G_UADDSAT,
                           G_SADDSAT, G_SMULFIX, G_UMULFIX, G_SMULFIXSAT,
                           G_UMULFIXSAT, G_FMINNUM, G_FMAXNUM, G_FMINNUM_IEEE,
                           G_FMAXNUM_IEEE, G_FMINIMUM, G_FMAXIMUM,
                           G_FADD, G_FMUL):$root, [{
    return getIConstantVRegVal(${root}->getOperand(1).getReg(), MRI).has_value();
  }]),
  (apply [{
    Observer.changingInstr(*${root});
    Register LHSReg = ${root}->getOperand(1).getReg();
    Register RHSReg = ${root}->getOperand(2).getReg();
    ${root}->getOperand(1).setReg(RHSReg);
    ${root}->getOperand(2).setReg(LHSReg);
    Observer.changedInstr(*${root});
  }])
>;

// def pISAPreLegalizerCombinerHelper: GICombinerHelper<
//   "pISAGenPreLegalizerCombinerHelper",
//   !listremove(all_combines.Rules, [combines_for_extload])> {
//   let DisableRuleOption = "pisapostlegalizercombiner-disable-rule";
//   let StateClass = "pISAPostLegalizerCombinerHelperState";
//   let AdditionalArguments = [];
//   let CombineAllMethodName = "tryCombineAllImpl";
// }

// def pISAPostLegalizerCombinerHelper: GICombinerHelper<
//   "pISAGenPostLegalizerCombinerHelper",
//   [commute_constant_to_rhs, extra_commute_constant_to_rhs,
//    copy_prop]> {
//   let DisableRuleOption = "pisapostlegalizercombiner-disable-rule";
//   let StateClass = "pISAPostLegalizerCombinerHelperState";
//   let AdditionalArguments = [];
//   let CombineAllMethodName = "tryCombineAllImpl";
// }

// def pISAPreLegalizerCombiner: GICombiner<
//   "pISAPreLegalizerCombinerImpl",
//   !listremove(all_combines.Rules, [combines_for_extload])> {
//   let CombineAllMethodName = "tryCombineAllImpl";
// }

def pISAPostLegalizerCombiner: GICombiner<
  "pISAPostLegalizerCombinerImpl",
  [commute_constant_to_rhs, extra_commute_constant_to_rhs,
   copy_prop]> {
  let CombineAllMethodName = "tryCombineAllImpl";
}

