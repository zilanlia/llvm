//===- IntrinsicspISARaytracing.td - Defines pISA intrinsics ------------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines all of the pISA-specific intrinsics for raytracing.
//
//===----------------------------------------------------------------------===//

def int_pisa_rt_trace_async    : Intrinsic<[],
                                           [llvm_constant_i8ptr_ty, // global pointer
                                            llvm_i32_ty],         // payload
                                           []>;
def int_pisa_rt_trace_sync     : Intrinsic<[llvm_i32_ty],         // dst
                                           [llvm_constant_i8ptr_ty, // global pointer
                                            llvm_i32_ty],         // payload
                                           []>;
def int_pisa_rt_sync           : Intrinsic<[],
                                           [llvm_i32_ty],         // rt.trace_sync result
                                           []>;
def int_pisa_rt_check          : Intrinsic<[], [], []>;
def int_pisa_rt_release        : Intrinsic<[], [], []>;
def int_pisa_btd               : Intrinsic<[],
                                           [llvm_constant_i8ptr_ty, // global pointer
                                            llvm_i16_ty,          // stack id
                                            llvm_i64_ty],         // ksp pointer
                                           []>;
def int_pisa_stackid_release   : Intrinsic<[],
                                           [llvm_i16_ty],         // stack id
                                           []>;
// special registers
def int_pisa_rt_globalpointer  : Intrinsic<[llvm_constant_i8ptr_ty], [], [IntrNoMem]>;
def int_pisa_rt_ksppointer     : Intrinsic<[llvm_constant_i8ptr_ty], [], [IntrNoMem]>;
def int_pisa_rt_async_stackid  : Intrinsic<[llvm_i16_ty],          [], [IntrNoMem]>;
def int_pisa_rt_shader_type    : Intrinsic<[llvm_i16_ty],          [], [IntrNoMem]>;
