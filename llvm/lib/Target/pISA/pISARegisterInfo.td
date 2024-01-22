//===-- pISARegisterInfo.td - pISA Register defs --------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  Declarations that describe the pISA register file.
//
//===----------------------------------------------------------------------===//

// TODO: Newer tablegen has dynamic slicing features to make this easier
class RegSlice<int n, list<Register> Rs> {
  Register E1 = !head(Rs);
  Register E2 = !head(!tail(Rs));
  Register E3 = !head(!tail(!tail(Rs)));
  list<Register> R1 = [E1];
  list<Register> R2 = [E1, E2];
  list<Register> R3 = [E1, E2, E3];

  list<Register> R = !cond(!eq(n, 1): R1,
                           !eq(n, 2): R2,
                           !eq(n, 3): R3,
                           true: []);
  assert !not(!eq(!size(R), 0)), "unhandled size!";
}

let Namespace = "pISA" in {

  class pISARegisterClass<list<ValueType> regTypes, int alignment, dag regList>
    : RegisterClass<"pISA", regTypes, alignment,regList> {}

  // .reg style registers
  class GeneralRegisterClass<list<ValueType> regTypes, int alignment, dag regList> :
    pISARegisterClass<regTypes, alignment, regList> {
    // Keep flags in sync with RegisterEncoding in pISARegistryManager
    let TSFlags{1-0} = 1;
  }

  // .pred style registers
  class PredRegisterClass<list<ValueType> regTypes, int alignment, dag regList> :
    pISARegisterClass<regTypes, alignment, regList> {
    // Keep flags in sync with RegisterEncoding in pISARegistryManager
    let TSFlags{1-0} = 2;
  }

  def DummyReg : Register<"">;

  multiclass VectorSpecialRegSubRegs<int n, string name> {
    def X : Register<name#".x">;
    if !ge(n, 2) then {
      def Y : Register<name#".y">;
    }
    if !ge(n, 3) then {
      def Z : Register<name#".z">;
    }
    if !ge(n, 4) then {
      def W : Register<name#".w">;
    }
  }

  class VectorSpecialReg<string name, list<Register> SubRegs, int EltSize> :
    RegisterWithSubRegs<name, SubRegs> {

    // TODO: move to !range(!size(SubRegs)) when available
    list<int> indices = !filter(i, [0, 1, 2, 3], !lt(i, !size(SubRegs)));

    let SubRegIndices = !foreach(i, indices, !cast<SubRegIndex>("sub"#EltSize#"_"#i));
    let CoveredBySubRegs = 1;
    let isConstant = true;
  }

  // Registers to describe Vector-subregister relationships
  class VecReg<list<Register> subregs> : RegisterWithSubRegs<"", subregs>;

  foreach Index = 0-3 in {
    def sub8_#Index  : SubRegIndex<8,  !shl(Index, 3)>;
    def sub16_#Index : SubRegIndex<16, !shl(Index, 4)>;
    def sub32_#Index : SubRegIndex<32, !shl(Index, 5)>;
    def sub64_#Index : SubRegIndex<64, !shl(Index, 6)>;
  }

  // Special registers

  defset list<Register> LocalIdSubRegs = {
    defm SpecialReg_LocalId : VectorSpecialRegSubRegs<3, "%localid">;
  }
  def SpecialReg_LocalIdXY  : VectorSpecialReg<"%localid.xy", RegSlice<2, LocalIdSubRegs>.R, 16>;
  def SpecialReg_LocalIdXYZ : VectorSpecialReg<"%localid", LocalIdSubRegs, 16>;

  defset list<Register> LocalSizeSubRegs = {
    defm SpecialReg_LocalSize : VectorSpecialRegSubRegs<3, "%localsize">;
  }
  def SpecialReg_LocalSizeXY  : VectorSpecialReg<"%localsize.xy", RegSlice<2, LocalSizeSubRegs>.R, 16>;
  def SpecialReg_LocalSizeXYZ : VectorSpecialReg<"%localsize", LocalSizeSubRegs, 16>;

  defset list<Register> GroupIdSubRegs = {
    defm SpecialReg_GroupId : VectorSpecialRegSubRegs<3, "%groupid">;
  }
  def SpecialReg_GroupIdXY  : VectorSpecialReg<"%groupid.xy", RegSlice<2, GroupIdSubRegs>.R, 32>;
  def SpecialReg_GroupIdXYZ : VectorSpecialReg<"%groupid", GroupIdSubRegs, 32>;

  defset list<Register> GroupCountSubRegs = {
    defm SpecialReg_GroupCount : VectorSpecialRegSubRegs<3, "%groupcount">;
  }
  def SpecialReg_GroupCountXY  : VectorSpecialReg<"%groupcount.xy", RegSlice<2, GroupCountSubRegs>.R, 32>;
  def SpecialReg_GroupCountXYZ : VectorSpecialReg<"%groupcount", GroupCountSubRegs, 32>;

  def SpecialReg_SubgroupId : Register<"%subgroupid">;
  def SpecialReg_LaneId     : Register<"%laneid">;
  def SpecialReg_LogicalSubSliceId : Register<"%logicalsubsliceid">;

  // begin intel embargo
  // Raytracing Special Registers
  def SpecialReg_RTGlobalPointer : Register<"%rt.globalpointer">;
  def SpecialReg_RTKSPPointer    : Register<"%rt.ksppointer">;
  def SpecialReg_RTAsyncStackId  : Register<"%rt.async_stackid">;
  def SpecialReg_RTShaderType    : Register<"%rt.shader_type">;
  // end intel embargo

  //
  // Scalar registers
  //

  foreach Index = 0-3 in {
    def Reg8b_#Index  : Register<"">;
    def Reg16b_#Index : Register<"">;
    def Reg32b_#Index : Register<"">;
    def Reg64b_#Index : Register<"">;
  }

  def Pred : PredRegisterClass<[i1], 8, (add DummyReg)>;
  def Reg8b : GeneralRegisterClass<[i8], 8, (add (sequence "Reg8b_%u", 0, 3))>;
  def Reg16b : GeneralRegisterClass<[i16, f16], 16,
    (add LocalIdSubRegs, LocalSizeSubRegs, SpecialReg_LaneId,
         SpecialReg_LogicalSubSliceId,
         // begin intel embargo
         SpecialReg_RTAsyncStackId, SpecialReg_RTShaderType,
         // end intel embargo
         (sequence "Reg16b_%u", 0, 3))>;
  def Reg32b : GeneralRegisterClass<[i32, f32], 32,
    (add GroupIdSubRegs, GroupCountSubRegs, SpecialReg_SubgroupId,
         (sequence "Reg32b_%u", 0, 3))>;
  def Reg64b : GeneralRegisterClass<[i64, f64], 64,
    (add
      // begin intel embargo
      SpecialReg_RTGlobalPointer, SpecialReg_RTKSPPointer,
      // end intel embargo
      (sequence "Reg64b_%u", 0, 3)
      )>;

  //
  // Vector registers
  //

  // 8b subregisters
  def Reg8bx2 : VecReg<[Reg8b_0, Reg8b_1]> {
    let SubRegIndices = [sub8_0, sub8_1];
    let CoveredBySubRegs = 1;
  }

  def Reg8bx3 : VecReg<[Reg8b_0, Reg8b_1, Reg8b_2]> {
    let SubRegIndices = [sub8_0, sub8_1, sub8_2];
    let CoveredBySubRegs = 1;
  }

  def Reg8bx4 : VecReg<[Reg8b_0, Reg8b_1, Reg8b_2, Reg8b_3]> {
    let SubRegIndices = [sub8_0, sub8_1, sub8_2, sub8_3];
    let CoveredBySubRegs = 1;
  }

  // 8b vectors
  def RegV2_8b : GeneralRegisterClass<[v2i8], 8, (add Reg8bx2)>;
  def RegV4_8b : GeneralRegisterClass<[v4i8], 8, (add Reg8bx4)>;

  // 16b subregisters
  def Reg16bx2 : VecReg<[Reg16b_0, Reg16b_1]> {
    let SubRegIndices = [sub16_0, sub16_1];
    let CoveredBySubRegs = 1;
  }

  def Reg16bx3 : VecReg<[Reg16b_0, Reg16b_1, Reg16b_2]> {
    let SubRegIndices = [sub16_0, sub16_1, sub16_2];
    let CoveredBySubRegs = 1;
  }

  def Reg16bx4 : VecReg<[Reg16b_0, Reg16b_1, Reg16b_2, Reg16b_3]> {
    let SubRegIndices = [sub16_0, sub16_1, sub16_2, sub16_3];
    let CoveredBySubRegs = 1;
  }

  // 16b vectors
  def RegV2_16b : GeneralRegisterClass<[v2i16, v2f16], 16,
    (add Reg16bx2, SpecialReg_LocalIdXY, SpecialReg_LocalSizeXY)>;
  def RegV3_16b : GeneralRegisterClass<[v3i16, v3f16], 16,
    (add Reg16bx3, SpecialReg_LocalIdXYZ, SpecialReg_LocalSizeXYZ)>;
  def RegV4_16b : GeneralRegisterClass<[v4i16, v4f16], 16, (add Reg16bx4)>;

  // 32b subregisters
  def Reg32bx2 : VecReg<[Reg32b_0, Reg32b_1]> {
    let SubRegIndices = [sub32_0, sub32_1];
    let CoveredBySubRegs = 1;
  }

  def Reg32bx3 : VecReg<[Reg32b_0, Reg32b_1, Reg32b_2]> {
    let SubRegIndices = [sub32_0, sub32_1, sub32_2];
    let CoveredBySubRegs = 1;
  }

  def Reg32bx4 : VecReg<[Reg32b_0, Reg32b_1, Reg32b_2, Reg32b_3]> {
    let SubRegIndices = [sub32_0, sub32_1, sub32_2, sub32_3];
    let CoveredBySubRegs = 1;
  }

  // 32b vectors
  def RegV2_32b : GeneralRegisterClass<[v2i32, v2f32], 32,
    (add Reg32bx2, SpecialReg_GroupIdXY, SpecialReg_GroupCountXY)>;
  def RegV3_32b : GeneralRegisterClass<[v3i32, v3f32], 32,
    (add Reg32bx3, SpecialReg_GroupIdXYZ, SpecialReg_GroupCountXYZ)>;
  def RegV4_32b : GeneralRegisterClass<[v4i32, v4f32], 32, (add Reg32bx4)>;

  // 64b subregisters
  def Reg64bx2 : VecReg<[Reg64b_0, Reg64b_1]> {
    let SubRegIndices = [sub64_0, sub64_1];
    let CoveredBySubRegs = 1;
  }

  def Reg64bx3 : VecReg<[Reg64b_0, Reg64b_1, Reg64b_2]> {
    let SubRegIndices = [sub64_0, sub64_1, sub64_2];
    let CoveredBySubRegs = 1;
  }

  def Reg64bx4 : VecReg<[Reg64b_0, Reg64b_1, Reg64b_2, Reg64b_3]> {
    let SubRegIndices = [sub64_0, sub64_1, sub64_2, sub64_3];
    let CoveredBySubRegs = 1;
  }

  // 64b vectors
  def RegV2_64b : GeneralRegisterClass<[v2i64, v2f64], 64, (add Reg64bx2)>;
  def RegV3_64b : GeneralRegisterClass<[v3i64, v3f64], 64, (add Reg64bx3)>;
  def RegV4_64b : GeneralRegisterClass<[v4i64, v4f64], 64, (add Reg64bx4)>;
}
