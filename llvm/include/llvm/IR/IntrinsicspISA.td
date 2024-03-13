//===- IntrinsicspISA.td - Defines pISA intrinsics ---------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines all of the pISA-specific intrinsics.
//
//===----------------------------------------------------------------------===//

// def llvm_global_ptr_ty   : LLVMQualPointerType<1>;  // (global)*
def llvm_constant_i8ptr_ty : LLVMQualPointerType<2>;  // (constant)*
// def llvm_shared_ptr_ty   : LLVMQualPointerType<3>;  // (shared)*
// def llvm_generic_ptr_ty  : LLVMQualPointerType<4>;  // (generic)*

let TargetPrefix = "pisa" in {
  def int_pisa_assign_rndmode_re : Intrinsic<[llvm_anyfloat_ty], [LLVMMatchType<0>], [IntrNoMem]>;
  def int_pisa_assign_rndmode_rd : Intrinsic<[llvm_anyfloat_ty], [LLVMMatchType<0>], [IntrNoMem]>;
  def int_pisa_assign_rndmode_ru : Intrinsic<[llvm_anyfloat_ty], [LLVMMatchType<0>], [IntrNoMem]>;
  def int_pisa_assign_rndmode_rz : Intrinsic<[llvm_anyfloat_ty], [LLVMMatchType<0>], [IntrNoMem]>;
  def int_pisa_localid           : Intrinsic<[LLVMType<v3i32>],  [],                 [IntrNoMem]>;
  def int_pisa_localid_x         : Intrinsic<[llvm_i32_ty],      [],                 [IntrNoMem]>;
  def int_pisa_localid_y         : Intrinsic<[llvm_i32_ty],      [],                 [IntrNoMem]>;
  def int_pisa_localid_z         : Intrinsic<[llvm_i32_ty],      [],                 [IntrNoMem]>;
  def int_pisa_localsize         : Intrinsic<[LLVMType<v3i32>],  [],                 [IntrNoMem]>;
  def int_pisa_localsize_x       : Intrinsic<[llvm_i32_ty],      [],                 [IntrNoMem]>;
  def int_pisa_localsize_y       : Intrinsic<[llvm_i32_ty],      [],                 [IntrNoMem]>;
  def int_pisa_localsize_z       : Intrinsic<[llvm_i32_ty],      [],                 [IntrNoMem]>;
  def int_pisa_groupid           : Intrinsic<[LLVMType<v3i32>],  [],                 [IntrNoMem]>;
  def int_pisa_groupid_x         : Intrinsic<[llvm_i32_ty],      [],                 [IntrNoMem]>;
  def int_pisa_groupid_y         : Intrinsic<[llvm_i32_ty],      [],                 [IntrNoMem]>;
  def int_pisa_groupid_z         : Intrinsic<[llvm_i32_ty],      [],                 [IntrNoMem]>;
  def int_pisa_groupcount        : Intrinsic<[LLVMType<v3i32>],  [],                 [IntrNoMem]>;
  def int_pisa_groupcount_x      : Intrinsic<[llvm_i32_ty],      [],                 [IntrNoMem]>;
  def int_pisa_groupcount_y      : Intrinsic<[llvm_i32_ty],      [],                 [IntrNoMem]>;
  def int_pisa_groupcount_z      : Intrinsic<[llvm_i32_ty],      [],                 [IntrNoMem]>;
  def int_pisa_subgroupid        : Intrinsic<[llvm_i32_ty],      [],                 [IntrNoMem]>;
  def int_pisa_laneid            : Intrinsic<[llvm_i16_ty],      [],                 [IntrNoMem]>;
  def int_pisa_logicalsubsliceid : Intrinsic<[llvm_i16_ty],      [],                 [IntrNoMem]>;
  def int_pisa_fence             : Intrinsic<[], [/* scope */ llvm_i32_ty], [IntrNoMem, IntrHasSideEffects, IntrConvergent, ImmArg<ArgIndex<0>>]>;
  def int_pisa_workgroup_barrier : Intrinsic<[], [], [IntrNoMem, IntrHasSideEffects, IntrConvergent]>;
  def int_pisa_subgroup_barrier  : Intrinsic<[], [], [IntrNoMem, IntrHasSideEffects, IntrConvergent]>;
  def int_pisa_sbarrier_signal   : Intrinsic<[], [], [IntrNoMem, IntrHasSideEffects, IntrConvergent]>;
  def int_pisa_sbarrier_wait     : Intrinsic<[], [], [IntrNoMem, IntrHasSideEffects, IntrConvergent]>;
  def int_pisa_isaddr_private    : Intrinsic<[llvm_i32_ty],      [llvm_anyptr_ty],   [IntrNoMem]>;
  def int_pisa_isaddr_shared     : Intrinsic<[llvm_i32_ty],      [llvm_anyptr_ty],   [IntrNoMem]>;
  def int_pisa_isaddr_global     : Intrinsic<[llvm_i32_ty],      [llvm_anyptr_ty],   [IntrNoMem]>;
  def int_pisa_isaddr_const      : Intrinsic<[llvm_i32_ty],      [llvm_anyptr_ty],   [IntrNoMem]>;
  def int_pisa_iavg              : Intrinsic<[llvm_anyint_ty],   [LLVMMatchType<0>, LLVMMatchType<0>], [IntrNoMem]>;
  def int_pisa_fptosi            : Intrinsic<[llvm_anyint_ty],   [llvm_anyfloat_ty, llvm_metadata_ty], [IntrNoMem]>;
  def int_pisa_fptoui            : Intrinsic<[llvm_anyint_ty],   [llvm_anyfloat_ty, llvm_metadata_ty], [IntrNoMem]>;
  def int_pisa_dp4a_uu           : Intrinsic<[llvm_i32_ty],      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], [IntrNoMem]>;
  def int_pisa_dp4a_us           : Intrinsic<[llvm_i32_ty],      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], [IntrNoMem]>;
  def int_pisa_dp4a_su           : Intrinsic<[llvm_i32_ty],      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], [IntrNoMem]>;
  def int_pisa_dp4a_ss           : Intrinsic<[llvm_i32_ty],      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], [IntrNoMem]>;
  def int_pisa_shfl_up           : Intrinsic<[llvm_i32_ty],      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], [IntrNoMem]>;
  def int_pisa_shfl_down         : Intrinsic<[llvm_i32_ty],      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], [IntrNoMem]>;
  def int_pisa_shfl_xor          : Intrinsic<[llvm_i32_ty],      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], [IntrNoMem]>;
  def int_pisa_shfl_idx          : Intrinsic<[llvm_i32_ty],      [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], [IntrNoMem]>;
  def int_pisa_cas_fatom_global  : Intrinsic<[llvm_anyfloat_ty], [LLVMQualPointerType<1>, LLVMMatchType<0>, LLVMMatchType<0>], [IntrHasSideEffects]>;
  def int_pisa_cas_fatom_shared  : Intrinsic<[llvm_anyfloat_ty], [LLVMQualPointerType<3>, LLVMMatchType<0>, LLVMMatchType<0>], [IntrHasSideEffects]>;
  def int_pisa_cas_fatom_generic : Intrinsic<[llvm_anyfloat_ty], [LLVMQualPointerType<4>, LLVMMatchType<0>, LLVMMatchType<0>], [IntrHasSideEffects]>;

  // begin intel embargo
  include "llvm/IR/IntrinsicspISARaytracing.td"
  // end intel embargo
}
