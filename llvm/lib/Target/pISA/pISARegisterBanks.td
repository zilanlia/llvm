//===-- pISARegisterBanks.td - Describe pISA RegBanks ---*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

def GRFRegBank : RegisterBank<"GRF",
  [Pred, Reg8b, Reg16b, Reg32b, Reg64b,
  RegV2_8b, RegV4_8b,
  RegV2_16b, RegV3_16b, RegV4_16b,
  RegV2_32b, RegV3_32b, RegV4_32b,
  RegV2_64b, RegV3_64b, RegV4_64b]
>;
