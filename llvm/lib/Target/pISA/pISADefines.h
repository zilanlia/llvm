//===-- pISADefines.h - Common defines for pISA -*- C++ -*-----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_pISADEFINES_H
#define LLVM_LIB_TARGET_pISA_pISADEFINES_H

namespace llvm {
namespace pISA {
enum class AddressSpace : unsigned {
  PRIVATE  = 0,
  GLOBAL   = 1,
  CONSTANT = 2,
  SHARED   = 3,
  GENERIC  = 4
};

enum class Swizzle : unsigned { X, Y, Z, W, XYZW, XY, ZW, NONE };

enum class FenceScope : unsigned {
  workgroup,
  local,
  tile,
  gpu,
  system
};

constexpr unsigned SIMD_SIZE = 32;
} // namespace pISA
} // namespace llvm

#endif // LLVM_LIB_TARGET_pISA_pISADEFINES_H
