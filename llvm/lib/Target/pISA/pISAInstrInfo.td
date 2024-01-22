//===-- pISAInstrInfo.td - Target Description for pISA Target ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the pISA instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

include "pISAInstrFormats.td"
include "llvm/TableGen/SearchableTable.td"

def VTs: pISAValueTypes;

// Templates for imm type checks
class uimmCheck<int num, ValueType VT>
  : ImmLeaf<VT, "return isUInt<"#num#">(Imm);">;

class simmCheck<int num, ValueType VT>
  : ImmLeaf<VT, "return isInt<"#num#">(Imm);">;

class immType<bit IsSigned, ValueType VT, ValueType extVT = VT> {
  ImmLeaf check = !if(IsSigned, simmCheck<VT.Size, extVT>,
                                uimmCheck<VT.Size, extVT>);

}

class PrintDispatch<string PM, ValueType Ty, string OpType> : OperandWithDefaultOps<Ty, (ops (Ty 0))> {
  let PrintMethod = PM;
  let OperandNamespace = "pISA";
  let OperandType = OpType;
}

def Negate : PrintDispatch<"negateRepr", i32, "OPERAND_NEGATE">;
def Swizzle : PrintDispatch<"swizzleRepr", i32, "OPERAND_SWIZZLE">;
class RegNoSwizzle<ValueType VT> : Operand<VT> {
  let PrintMethod="printRegNoSwizzle";
}
def FenceScope : Operand<i32> {
  let PrintMethod="printFenceScope";
}

let RecomputePerFunction = 1 in {
  def useFTZ : Predicate<"Subtarget->getTargetLowering()->useFTZ(*MF)">;
  def noFTZ : Predicate<"!Subtarget->getTargetLowering()->useFTZ(*MF)">;
}

class UnaryInst_r<string name, SDPatternOperator node, pisaVT dstVT, pisaVT srcVT>
  : pISAInst_r<name, dstVT, srcVT> {
  let Pattern = [(set dstRC:$dst, (node srcRC:$src))];
}

class UnaryInst_i<string name, SDPatternOperator node, pisaVT dstVT, pisaVT srcVT>
  : pISAInst_i<name, dstVT, srcVT> {
  let Pattern = [(set dstRC:$dst, (node (srcVT.VT immVal:$src)))];
}

multiclass UnaryInst<string name, SDPatternOperator node> {
  foreach VT = [_i16, _i32, _i64] in {
    defvar opcode = VT.opcode;
    defvar directive = VT.directive;
    def opcode#r: UnaryInst_r<name#directive, node, VT, VT>;
  }
}

class ConvInst<string name, pisaIntVT ToVT, pisaIntVT FromVT, SDNode Node>
  : UnaryInst_r<name#"."#ToVT.name#"."#FromVT.name, Node, ToVT, FromVT>;

class FextInst<pisaFloatVT ToVT, pisaFloatVT FromVT>
  : UnaryInst_r<"fext."#ToVT.name#"."#FromVT.name, fpextend, ToVT, FromVT>;

multiclass FtruncInst<pisaFloatVT ToVT, pisaFloatVT FromVT> {
  foreach ftz = [0, 1] in {
    defvar ftzOpcode = !if(ftz,"_ftz","");
    defvar ftzDirective = !if(ftz,".ftz","");

    def "r"#ftzOpcode
      : UnaryInst_r<"ftrunc."#ToVT.name#"."#FromVT.name#""#ftzDirective, fpround, ToVT, FromVT>,
        Requires<[!if(ftz,useFTZ,noFTZ)]>;

    // Definitions of opcodes with rounding mode
    foreach rndmode = [int_pisa_assign_rndmode_re, int_pisa_assign_rndmode_rd,
      int_pisa_assign_rndmode_ru, int_pisa_assign_rndmode_rz] in {
      defvar rndname = !subst("int_pisa_assign_rndmode_", "", ""#rndmode);

      def "r_"#rndname#""#ftzOpcode
        : pISAInst_r<"ftrunc."#ToVT.name#"."#FromVT.name#"."#rndname#""#ftzDirective, ToVT, FromVT>,
          Requires<[!if(ftz,useFTZ,noFTZ)]>;
    } // rounding mode
  } // ftz
} // multiclass

multiclass UnaryFloatInst<string name, SDNode node> {
  foreach VT = [_f16, _f32, _f64] in {
    defvar directive = VT.directive;
    defvar opcode = VT.opcode;
    def opcode#r : UnaryInst_r<name#directive, node, VT, VT>;
  }
}

class BinInst_rr<string name, SDNode node, pisaVT src0VT, pisaVT src1VT>
  : pISAInst_rr<name, src0VT, src0VT, src1VT> {
  let Pattern = [(set dstRC:$dst, (node src0RC:$src0, src1RC:$src1))];
}

class BinInst_ri<string name, SDNode node, pisaVT src0VT, pisaVT src1VT>
  : pISAInst_ri<name, src0VT, src0VT, src1VT> {
  let Pattern = [(set dstRC:$dst, (node src0RC:$src0, (src1VT.VT immVal1:$src1)))];
}

multiclass BinInst<string name, SDNode node> {
  foreach VT = [_i16, _i32, _i64] in {
    defvar opcode = VT.opcode;
    defvar directive = VT.directive;
    def opcode#rr: BinInst_rr<name#directive, node, VT, VT>;
    def opcode#ri: BinInst_ri<name#directive, node, VT, VT>;
  }
}

class TernaryInst_rrr<string name, SDPatternOperator node, pisaVT src0VT, pisaVT src2VT>
  : pISAInst_rrr<name, src2VT, src0VT, src0VT, src2VT> {
  let Pattern = [(set dstRC:$dst, (node src0RC:$src0, src1RC:$src1, src2RC:$src2))];
}

class TernaryInst_rri<string name, SDPatternOperator node, pisaVT src0VT, pisaVT src2VT>
  : pISAInst_rri<name, src2VT, src0VT, src0VT, src2VT> {
  let Pattern = [(set dstRC:$dst, (node src0RC:$src0, src1RC:$src1, (src2VT.VT immVal2:$src2)))];
}

class TernaryInst_rir<string name, SDPatternOperator node, pisaVT src0VT, pisaVT src2VT>
  : pISAInst_rir<name, src2VT, src0VT, src0VT, src2VT> {
  let Pattern = [(set dstRC:$dst, (node src0RC:$src0, (src0VT.VT immVal1:$src1), src2RC:$src2))];
}

class TernaryInst_rii<string name, SDPatternOperator node, pisaVT src0VT, pisaVT src2VT>
  : pISAInst_rii<name, src2VT, src0VT, src0VT, src2VT> {
  let Pattern = [(set dstRC:$dst, (node src0RC:$src0, (src0VT.VT immVal1:$src1), (src2VT.VT immVal2:$src2)))];
}

class TernaryInst_irr<string name, SDPatternOperator node, pisaVT src0VT, pisaVT src2VT>
  : pISAInst_irr<name, src2VT, src0VT, src0VT, src2VT> {
  let Pattern = [(set dstRC:$dst, (node (src0VT.VT immVal0:$src0), src1RC:$src1, src2RC:$src2))];
}

class TernaryInst_iri<string name, SDPatternOperator node, pisaVT src0VT, pisaVT src2VT>
  : pISAInst_iri<name, src2VT, src0VT, src0VT, src2VT> {
  let Pattern = [(set dstRC:$dst, (node (src0VT.VT immVal0:$src0), src1RC:$src1, (src2VT.VT immVal2:$src2)))];
}

class TernaryInst_iir<string name, SDPatternOperator node, pisaVT src0VT, pisaVT src2VT>
  : pISAInst_iir<name, src2VT, src0VT, src0VT, src2VT> {
  let Pattern = [(set dstRC:$dst, (node (src0VT.VT immVal0:$src0), (src0VT.VT immVal1:$src1), src2RC:$src2))];
}

class MadInst_rrr<string name, pisaVT bitwidth> : pISAInst_rrr<name, bitwidth, bitwidth, bitwidth, bitwidth> {
  let Pattern = [(set dstRC:$dst, (add (mul src0RC:$src0, src1RC:$src1), src2RC:$src2))];
}

class MadInst_rri<string name, pisaVT bitwidth> : pISAInst_rri<name, bitwidth, bitwidth, bitwidth, bitwidth> {
  let Pattern = [(set dstRC:$dst, (add (mul src0RC:$src0, src1RC:$src1), (bitwidth.VT imm:$src2)))];
}

class MadInst_rir<string name, pisaVT bitwidth> : pISAInst_rir<name, bitwidth, bitwidth, bitwidth, bitwidth> {
  let Pattern = [(set dstRC:$dst, (add (mul src0RC:$src0, (bitwidth.VT imm:$src1)), src2RC:$src2))];
}

class MadInst_rii<string name, pisaVT bitwidth> : pISAInst_rii<name, bitwidth, bitwidth, bitwidth, bitwidth> {
  let Pattern = [(set dstRC:$dst, (add (mul src0RC:$src0, (bitwidth.VT imm:$src1)), (bitwidth.VT imm:$src2)))];
}

class MadFullInst_rrr<string name, pisaVT bitwidth, pisaVT extBitwidth, bit IsSigned> :
  pISAInst_rrr<name, extBitwidth, bitwidth, bitwidth, extBitwidth> {
  SDNode extNode = !if(IsSigned, sext, zext);
  let Pattern = [(set dstRC:$dst, (add (mul (extNode src0RC:$src0), (extNode src1RC:$src1)), src2RC:$src2))];
}

class MadFullInst_rri<string name, pisaVT bitwidth, pisaVT extBitwidth, bit IsSigned> :
  pISAInst_rri<name, extBitwidth, bitwidth, bitwidth, extBitwidth> {
  SDNode extNode = !if(IsSigned, sext, zext);
  let Pattern = [(set dstRC:$dst, (add (mul (extNode src0RC:$src0), (extNode src1RC:$src1)), (extBitwidth.VT imm:$src2)))];
}

class MadFullInst_rir<string name, pisaVT bitwidth, pisaVT extBitwidth, bit IsSigned> :
  pISAInst_rir<name, extBitwidth, bitwidth, extBitwidth, extBitwidth> {
  ImmLeaf immCheck1 = immType<IsSigned, bitwidth.VT, extBitwidth.VT>.check;
  SDNode extNode = !if(IsSigned, sext, zext);
  let Pattern = [(set dstRC:$dst, (add (mul (extNode src0RC:$src0), (immCheck1:$src1)), src2RC:$src2))];
}

class MadFullInst_rii<string name, pisaVT bitwidth, pisaVT extBitwidth, bit IsSigned> :
  pISAInst_rii<name, extBitwidth, bitwidth, extBitwidth, extBitwidth> {
  ImmLeaf immCheck1 = immType<IsSigned, bitwidth.VT, extBitwidth.VT>.check;
  SDNode extNode = !if(IsSigned, sext, zext);
  let Pattern = [(set dstRC:$dst, (add (mul (extNode src0RC:$src0), (immCheck1:$src1)), (extBitwidth.VT imm:$src2)))];
}

multiclass MadInst<string name, bit IsSigned> {
  foreach VT = [_i16, _i32, _i64] in {
    defvar directive = VT.directive;
    defvar opcode = VT.opcode;
    def opcode#rrr : MadInst_rrr<name#directive, VT>;
    def opcode#rri : MadInst_rri<name#directive, VT>;
    def opcode#rir : MadInst_rir<name#directive, VT>;
    def opcode#rii : MadInst_rii<name#directive, VT>;
  }

  foreach VT = [_i16, _i32] in {
    defvar extBitwidth = !if(!eq(VT, _i16), _i32, _i64);
    defvar directive = name#".full"#VT.directive;
    defvar opcode = "full"#VT.opcode;
    def opcode#rrr : MadFullInst_rrr<directive, VT, extBitwidth, IsSigned>;
    def opcode#rir : MadFullInst_rir<directive, VT, extBitwidth, IsSigned>;
    def opcode#rri : MadFullInst_rri<directive, VT, extBitwidth, IsSigned>;
    def opcode#rii : MadFullInst_rii<directive, VT, extBitwidth, IsSigned>;
  }
}

class RndmodeInst_rrr<string name, SDNode node, Intrinsic rndmode, pisaVT src0VT>:
  TernaryInst_rrr<name, node, src0VT, src0VT> {
  let Pattern = [(set dstRC:$dst, (rndmode (node src0RC:$src0, src1RC:$src1, src2RC:$src2)))];
  let RoundingMode = pISARoundingModeToXeCommon<rndmode>.RMValue;
}

multiclass FMadInst<string name> {
  foreach ftz = [0, 1] in {
    defvar ftzOpcode    = !if(ftz,"_ftz","");
    defvar ftzDirective = !if(ftz,".ftz","");
    foreach VT = [_f16, _f32, _f64] in {
      defvar opcode    = VT.opcode#ftzOpcode;
      defvar directive = name#ftzDirective#VT.directive;
      def opcode#_rrr : TernaryInst_rrr<directive, fma, VT, VT>, Requires<[!if(ftz,useFTZ,noFTZ)]>;
      def opcode#_rri : TernaryInst_rri<directive, fma, VT, VT>, Requires<[!if(ftz,useFTZ,noFTZ)]>;
      def opcode#_rir : TernaryInst_rir<directive, fma, VT, VT>, Requires<[!if(ftz,useFTZ,noFTZ)]>;
      def opcode#_rii : TernaryInst_rii<directive, fma, VT, VT>, Requires<[!if(ftz,useFTZ,noFTZ)]>;

      // Definitions of opcodes with rounding mode
      foreach rndmode = [int_pisa_assign_rndmode_re, int_pisa_assign_rndmode_rd,
                         int_pisa_assign_rndmode_ru, int_pisa_assign_rndmode_rz] in {
        defvar rndname = !subst("int_pisa_assign_rndmode_", "", ""#rndmode);
        def VT.opcode#_#rndname#ftzOpcode :
          RndmodeInst_rrr<name#"."#rndname#ftzDirective#VT.directive, fma, rndmode, VT>, Requires<[!if(ftz,useFTZ,noFTZ)]>;
      } // rounding mode
    } // types
  } // ftz
}

class MulInst_rr<string name, pisaVT bitwidth> : BinInst_rr<name, mul, bitwidth, bitwidth>;
class MulInst_ri<string name, pisaVT bitwidth> : BinInst_ri<name, mul, bitwidth, bitwidth>;

class MulFullInst2<string name, pisaVT bitwidth, pisaVT extBitwidth, bit IsSigned>
  : pISAInst2<name, extBitwidth, bitwidth, bitwidth> {
  SDNode extNode = !if(IsSigned, sext, zext);
}

class MulFullInst_rr<string name, pisaVT bitwidth, pisaVT extBitwidth, bit IsSigned>
  : MulFullInst2<name, bitwidth, extBitwidth, IsSigned> {
  let InOperandList = (ins src0RC:$src0, src1RC:$src1);
  let Pattern = [(set dstRC:$dst, (mul (extNode src0RC:$src0), (extNode src1RC:$src1)))];
}

class MulFullInst_ri<string name, pisaVT bitwidth, pisaVT extBitwidth, bit IsSigned>
  : MulFullInst2<name, bitwidth, extBitwidth, IsSigned> {
  ImmLeaf immCheck1 = immType<IsSigned, bitwidth.VT, extBitwidth.VT>.check;
  DAGOperand immOpd1 = extBitwidth.ImmOpnd;
  let InOperandList = (ins src0RC:$src0, immOpd1:$src1);
  let Pattern = [(set dstRC:$dst, (mul (extNode src0RC:$src0), (immCheck1:$src1)))];
}

multiclass MulInst<string name, bit IsSigned> {
  foreach VT = [_i16, _i32, _i64] in {
    defvar directive = VT.directive;
    defvar opcode = VT.opcode;
    def opcode#rr : MulInst_rr<name#directive, VT>;
    def opcode#ri : MulInst_ri<name#directive, VT>;
  }

  foreach VT = [_i16, _i32] in {
    defvar extBitwidth = !if(!eq(VT, _i16), _i32, _i64);
    defvar directive = name#".full"#VT.directive;
    defvar opcode = "full"#VT.opcode;
    def opcode#rr : MulFullInst_rr<directive, VT, extBitwidth, IsSigned>;
    def opcode#ri : MulFullInst_ri<directive, VT, extBitwidth, IsSigned>;
  }
}

class IavgBinaryInst_rr<string name, pisaVT src0VT>: pISAInst_rr<name, src0VT, src0VT, src0VT> {
  let Pattern = [(set dstRC:$dst, (int_pisa_iavg src0RC:$src0, src1RC:$src1))];
}

class IavgBinaryInst_ri<string name, pisaVT src0VT>: pISAInst_ri<name, src0VT, src0VT, src0VT> {
  let Pattern = [(set dstRC:$dst, (int_pisa_iavg src0RC:$src0, (src0VT.VT imm:$src1)))];
}

class IavgBinaryInst_ir<string name, pisaVT src0VT>: pISAInst_ir<name, src0VT, src0VT, src0VT> {
  let Pattern = [(set dstRC:$dst, (int_pisa_iavg (src0VT.VT imm:$src0), src1RC:$src1))];
}

multiclass IavgInst<string name> {
  foreach VT = [_i16, _i32, _i64] in {
    defvar opcode = VT.opcode;
    defvar directive = VT.directive;
    def opcode#rr : IavgBinaryInst_rr<name#directive, VT>;
    def opcode#ri : IavgBinaryInst_ri<name#directive, VT>;
    def opcode#ir : IavgBinaryInst_ir<name#directive, VT>;
  }
}

class DP4Info<string op, Intrinsic intri> {
  string opcode = op;
  Intrinsic intrinsic = intri;
}

def DP4Auu : DP4Info<"uu", int_pisa_dp4a_uu>;
def DP4Aus : DP4Info<"us", int_pisa_dp4a_us>;
def DP4Asu : DP4Info<"su", int_pisa_dp4a_su>;
def DP4Ass : DP4Info<"ss", int_pisa_dp4a_ss>;

multiclass DP4AInst<string name> {
  foreach tuple = [DP4Auu, DP4Aus, DP4Asu, DP4Ass] in {
    defvar directive = name#"."#tuple.opcode#".32b";
    def tuple.opcode#rrr:TernaryInst_rrr<directive, tuple.intrinsic, _i32, _i32>;
    def tuple.opcode#rri:TernaryInst_rri<directive, tuple.intrinsic, _i32, _i32>;
    def tuple.opcode#rir:TernaryInst_rir<directive, tuple.intrinsic, _i32, _i32>;
    def tuple.opcode#rii:TernaryInst_rii<directive, tuple.intrinsic, _i32, _i32>;
    def tuple.opcode#irr:TernaryInst_irr<directive, tuple.intrinsic, _i32, _i32>;
    def tuple.opcode#iri:TernaryInst_iri<directive, tuple.intrinsic, _i32, _i32>;
    def tuple.opcode#iir:TernaryInst_iir<directive, tuple.intrinsic, _i32, _i32>;
  }
}

multiclass ShiftInst<string name, SDNode node> {
  foreach VT = [_i16, _i32, _i64] in {
    defvar opcode = VT.opcode;
    defvar directive = VT.directive;
    def opcode#rr: BinInst_rr<name#directive, node, VT, _i32>;
    def opcode#ri: BinInst_ri<name#directive, node, VT, _i32>;
  }
}

multiclass FunnelShiftInst<string name, SDNode node> {
  defvar VT = _i32;
  defvar opcode = VT.opcode;
  defvar directive = VT.directive;
  def opcode#rrr : TernaryInst_rrr<name#directive, node, VT, VT>;
  def opcode#rri : TernaryInst_rri<name#directive, node, VT, VT>;
  def opcode#rir : TernaryInst_rir<name#directive, node, VT, VT>;
  def opcode#rii : TernaryInst_rii<name#directive, node, VT, VT>;
}

// Implement sin, cos, exp2, log2.
// These math operations are FP32 only, do not support rounding mode,
// but do support ftz.
multiclass UnaryFloatMathInst<string name, SDNode node> {
  def _r:     UnaryInst_r<name, node, _i32, _i32>, Requires<[noFTZ]>;
  def _ftzr : UnaryInst_r<name#".ftz", node, _i32, _i32>, Requires<[useFTZ]>;
}

// Float Instructions:
class RndmodeInst_rr<string name, SDNode node, Intrinsic rndmode, pisaVT srcVT>
  : BinInst_rr<name, node, srcVT, srcVT> {
  let Pattern = [(set dstRC:$dst, (rndmode (node src0RC:$src0, src1RC:$src1)))];
  let RoundingMode = pISARoundingModeToXeCommon<rndmode>.RMValue;
}

// The following generic multiclass produces definitions for FP instructions with
// the format: opcode_floattype_operandkind_rndmode_ftz
// E.g.        {fadd/...}_{hf,"",df}{rr,ri}_{re,rd,ru,rz}_{ftz}
multiclass FloatBinInst<string name, SDNode node> {
  foreach ftz = [0, 1] in {
    defvar ftzOpcode = !if(ftz,"_ftz","");
    defvar ftzDirective = !if(ftz,".ftz","");

    foreach VT = [_f16, _f32, _f64] in {
      defvar directive = name#ftzDirective#VT.directive;
      defvar opcode = ftzOpcode#VT.opcode;
      def opcode#rr : BinInst_rr<directive, node, VT, VT>, Requires<[!if(ftz,useFTZ,noFTZ)]>;
      def opcode#ri : BinInst_ri<directive, node, VT, VT>, Requires<[!if(ftz,useFTZ,noFTZ)]>;
    }

    // Definitions of opcodes with rounding mode
    foreach rndmode = [int_pisa_assign_rndmode_re, int_pisa_assign_rndmode_rd,
      int_pisa_assign_rndmode_ru, int_pisa_assign_rndmode_rz] in {
      defvar rndname = !subst("int_pisa_assign_rndmode_", "", ""#rndmode);

      foreach VT = [_f16, _f32, _f64] in {
        defvar directive = VT.directive;
        defvar opcode = VT.opcode;
        def rndname#ftzOpcode#opcode#rr:
          RndmodeInst_rr<name#"."#rndname#ftzDirective#directive, node, rndmode, VT>,
          Requires<[!if(ftz,useFTZ,noFTZ)]>;
      }
    } // rounding mode
  } // ftz
} // multiclass


// Float Instructions with Fast mode:
multiclass FloatBinInstFast<string name, SDNode node> {
  defm "" : FloatBinInst<name, node>;

  // fast mode is only supported for single precision FP.
  def _fastrr : BinInst_rr<name#".fast", node, _f32, _f32>, Requires<[noFTZ]>;
  def _ftz_fastrr : BinInst_rr<name#".ftz.fast", node, _f32, _f32>, Requires<[useFTZ]>;
}

// Fmin/fmax
// They do not support rounding mode but do support ftz.
multiclass FMinMaxInst<string name, SDNode node> {
  foreach ftz = [0, 1] in {
    defvar ftzOpcode = !if(ftz,"ftz","");
    defvar ftzDirective = !if(ftz,".ftz","");

    foreach VT = [_f16, _f32, _f64] in {
      defvar directive = name#ftzDirective#VT.directive;
      defvar opcode = "_"#ftzOpcode#VT.opcode;
      def opcode#rr : BinInst_rr<directive, node, VT, VT>, Requires<[!if(ftz,useFTZ,noFTZ)]>;
      def opcode#ri : BinInst_ri<directive, node, VT, VT>, Requires<[!if(ftz,useFTZ,noFTZ)]>;
    }
  } // ftz
} // multiclass

class CmpBinaryInstPred_rr<string name, CondCode setcnd, pisaVT srcVT>: pISAInst_rr<name, _i1, srcVT, srcVT> {
  let Pattern = [(set i1:$dst, (setcc srcVT.VT:$src0, srcVT.VT:$src1, setcnd))];
  let MVType = pISAVTToMVType<srcVT>.VTValue;
}

class CmpBinaryInstPred_ri<string name, CondCode setcnd, pisaVT srcVT>: pISAInst_ri<name, _i1, srcVT, srcVT> {
  let Pattern = [(set i1:$dst, (setcc srcVT.VT:$src0, immVal1:$src1, setcnd))];
  let MVType = pISAVTToMVType<srcVT>.VTValue;
}

def selextend : PatFrag<(ops node:$in), (select node:$in, -1, 0)>;

class CmpBinaryInstReg_rr<string name, CondCode setcnd, pisaVT srcVT>: pISAInst_rr<name, _i32, srcVT, srcVT> {
  let Pattern = [(set i32:$dst, (selextend (i1 (setcc srcVT.VT:$src0, srcVT.VT:$src1, setcnd))))];
  let MVType = pISAVTToMVType<srcVT>.VTValue;
}

class CmpBinaryInstReg_ri<string name, CondCode setcnd, pisaVT srcVT>: pISAInst_ri<name, _i32, srcVT, srcVT> {
  let Pattern = [(set i32:$dst, (selextend (i1 (setcc srcVT.VT:$src0, immVal1:$src1, setcnd))))];
  let MVType = pISAVTToMVType<srcVT>.VTValue;
}


class CondName<CondCode cnd> {
  string ret = !cond(!or(!eq(cnd, SETUGE), !eq(cnd, SETGE), !eq(cnd, SETOGE)) : "ge",
                     !or(!eq(cnd, SETUGT), !eq(cnd, SETGT), !eq(cnd, SETOGT)) : "gt",
                     !or(!eq(cnd, SETULE), !eq(cnd, SETLE), !eq(cnd, SETOLE)) : "le",
                     !or(!eq(cnd, SETULT), !eq(cnd, SETLT), !eq(cnd, SETOLT)) : "lt",
                     !or(!eq(cnd, SETUEQ), !eq(cnd, SETEQ), !eq(cnd, SETOEQ)) : "eq",
                     !or(!eq(cnd, SETUNE), !eq(cnd, SETNE)) : "ne",
                     true: "");
  assert !not(!eq(ret, "")), "unknown comparison condition!";
}

// Predefined comparison conditions
class CompConds {
  list<CondCode> Unsigned = [SETUGE, SETUGT, SETULE, SETULT, SETEQ, SETNE];
  list<CondCode> Signed = [SETGE, SETGT, SETLE, SETLT, SETEQ, SETNE];
  list<CondCode> Float = [SETOGE, SETOGT, SETOLE, SETOLT, SETOEQ, SETUNE];
}
def conds: CompConds;

// Integer comparison
multiclass CmpInst<string name, bits<1> isSigned> {
  defvar condList = !if(isSigned, conds.Signed, conds.Unsigned);
  foreach setcnd = condList in {
    defvar cond = CondName<setcnd>.ret;

    foreach VT = [_i16, _i32, _i64] in {
      defvar directive = name#"."#cond#VT.directive;

      // Defs for cmp opcodes where dst is a predicate
      // cmp is directly matched, e.g.
      //   %2 = icmp slt i32 %0, %1
      defvar p_opcode = "_"#cond#_p#VT.opcode;
      def p_opcode#rr : CmpBinaryInstPred_rr<directive, setcnd, VT>;
      def p_opcode#ri : CmpBinaryInstPred_ri<directive, setcnd, VT>;

      // Defs for cmp opcodes where dst is a 32b register
      // cmp+sext is matched, e.g.
      //  %2 = icmp slt i32 %0, %1
      //  %3 = sext i1 %2 to i32
      defvar r_opcode = "_"#cond#_r#VT.opcode;
      def r_opcode#rr : CmpBinaryInstReg_rr<directive, setcnd, VT>;
      def r_opcode#ri : CmpBinaryInstReg_ri<directive, setcnd, VT>;
    }
  }
}

// Float comparison
multiclass FCmpInst<string name> {
  foreach setcnd = conds.Float in {
    defvar cond = CondName<setcnd>.ret;
    foreach daz = [0, 1] in {
      defvar dazOpcode = !if(daz,"_daz","");
      defvar dazDirective = !if(daz,".daz","");

      foreach VT = [_f16, _f32, _f64] in {
        defvar directive = name#"."#cond#dazDirective#VT.directive;

        // Defs for fcmp opcodes where dst is a predicate
        // fcmp is directly matched, e.g.
        //   %2 = fcmp olt f32 %0, %1
        defvar p_opcode = "_"#cond#dazOpcode#_p#VT.opcode;
        def p_opcode#rr : CmpBinaryInstPred_rr<directive, setcnd, VT>, Requires<[!if(daz,useFTZ,noFTZ)]>;
        def p_opcode#ri : CmpBinaryInstPred_ri<directive, setcnd, VT>, Requires<[!if(daz,useFTZ,noFTZ)]>;

        // Defs for fcmp opcodes where dst is a 32b register
        // fcmp+sext is matched, e.g.
        //  %2 = fcmp olt i32 %0, %1
        //  %3 = sext i1 %2 to i32
        defvar r_opcode = "_"#cond#dazOpcode#_r#VT.opcode;
        def r_opcode#rr : CmpBinaryInstReg_rr<directive, setcnd, VT>, Requires<[!if(daz,useFTZ,noFTZ)]>;
        def r_opcode#ri : CmpBinaryInstReg_ri<directive, setcnd, VT>, Requires<[!if(daz,useFTZ,noFTZ)]>;
      }
    }
  }
}

class SelInst_prr<string name, pisaVT VT> : pISAInst_rr<name, VT, VT, VT> {
  let AsmString = "($p) "#name#" \t$dst, $src0, $src1;";
  let InOperandList = (ins Pred:$p, src0RC:$src0, src1RC:$src1);
  let Pattern = [(set VT.VT:$dst, (select i1:$p, VT.VT:$src0, src1RC:$src1))];
}

class SelInst_pri<string name, pisaVT VT> : pISAInst_ri<name, VT, VT, VT> {
  let AsmString = "($p) "#name#" \t$dst, $src0, $src1;";
  let InOperandList = (ins Pred:$p, src0RC:$src0, immOpd1:$src1);
  let Pattern = [(set VT.VT:$dst, (select i1:$p, VT.VT:$src0, immVal1:$src1))];
}

class SelInst_pir<string name, pisaVT VT> : pISAInst_ir<name, VT, VT, VT> {
  let AsmString = "($p) "#name#" \t$dst, $src0, $src1;";
  let InOperandList = (ins Pred:$p, immOpd0:$src0, src1RC:$src1);
  let Pattern = [(set VT.VT:$dst, (select i1:$p, immVal0:$src0, VT.VT:$src1))];
}

class SelInst_pii<string name, pisaVT VT> : pISAInst_ii<name, VT, VT, VT> {
  let AsmString = "($p) "#name#" \t$dst, $src0, $src1;";
  let InOperandList = (ins Pred:$p, immOpd0:$src0, immOpd1:$src1);
  let Pattern = [(set VT.VT:$dst, (select i1:$p, immVal0:$src0, immVal1:$src1))];
}

multiclass SelInst<string name> {
  foreach VT = [_i16, _i32, _i64, _f16, _f32, _f64] in {
    defvar directive = name#"."#VT.bitwidth;
    defvar opcode = VT.opcode;
    def opcode#prr : SelInst_prr<directive, VT>;
    def opcode#pir : SelInst_pir<directive, VT>;
    def opcode#pri : SelInst_pri<directive, VT>;
    def opcode#pii : SelInst_pii<directive, VT>;
  }
}

multiclass AddrOf {
    def _32b: pISAInst<(outs Reg32b:$var), (ins i32imm:$frameindex), "addrof.32b $var, $frameindex;">;
    def _64b: pISAInst<(outs Reg64b:$var), (ins i32imm:$frameindex), "addrof.64b $var, $frameindex;">;
}

class SimpleOp<string name, list<dag> pattern = []> : pISAInst<(outs), (ins), name, pattern>;

class MovInst_i<string name, pisaVT dstVT, pisaVT srcVT> : pISAInst_i<name, dstVT, srcVT> {
  let Pattern = [(set dstRC:$dst, immVal:$src)];
}

// Multiclass to represent immediates for G_CONSTANT and F_FCONSTANT
// Immediate values can be either int or FP. Destination is a virtual register.
multiclass Mov {
  foreach VT = [_i8, _i16, _i32, _i64,
                _f16, _f32, _f64] in {
    defvar opcode = "_imm"#VT;
    defvar directive = "mov."#VT.VT.Size#"b";
    def opcode: MovInst_i<directive, VT, VT>;
  }
  foreach VT = VTs.Fixed in {
    defvar opcode    = VT.opcode;
    defvar directive = "mov"#VT.directive;
    def opcode#r : pISAInst_r<directive, VT, VT>;
  }
}

// Template for integer add/sub with carry-in and carry-out.
// We rely on custom selection for them as GlobalISel doesn't seem to be able
// to match these with tablegen.
multiclass ADD_SUB_INT_CARRY<string OpcStr> {
  let hasSideEffects = 1 in {
    foreach VT = [_i32, _i64] in {
      defvar directive = VT.directive;
      defvar opcode = VT.opcode;
      def opcode#rr : pISAInst_rr<OpcStr#directive, VT, VT, VT>;
      def opcode#ri : pISAInst_ri<OpcStr#directive, VT, VT, VT>;
    }
  }
}

class SwizzleInst<string name, pisaVT outVT, pisaVT inVT> : _pISAInst {
  DAGOperand outRC = outVT.RegOpnd;
  DAGOperand inRC = inVT.RegOpnd;
  let AsmString = name#" \t$dst$swizzleDst, $src$swizzleSrc;";
  let OutOperandList = (outs RegNoSwizzle<outVT.VT>:$dst);
  let InOperandList = (ins RegNoSwizzle<inVT.VT>:$src, Swizzle:$swizzleDst, Swizzle:$swizzleSrc);
  let Pattern = [];
}

multiclass VectorScalar<string name, pisaIntVT VT1, pisaVectorVT VT2> {
  def VT1.name#_#VT2.name : SwizzleInst<name#"."#VT1.name, VT1, VT2>;
  def VT2.name#_#VT1.name : SwizzleInst<name#"."#VT1.name, VT2, VT1>;
}

multiclass VectorVector<string name, string bitwidth, pisaVectorVT VT1, pisaVectorVT VT2> {
  def VT1.name#_#VT2.name : SwizzleInst<name#"."#bitwidth, VT1, VT2>;
  def VT2.name#_#VT1.name : SwizzleInst<name#"."#bitwidth, VT2, VT1>;
}

multiclass BitcastInst<string name> {
  // Vector to scalar and vice-versa
  defm _: VectorScalar<name, _i16, _v2i8>;
  defm _: VectorScalar<name, _i32, _v4i8>;
  defm _: VectorScalar<name, _i32, _v2i16>;
  defm _: VectorScalar<name, _i64, _v4i16>;
  defm _: VectorScalar<name, _i64, _v2i32>;

  // Vector subregister to scalar and vice-versa
  defm _: VectorScalar<name, _i8, _v2i8>;
  defm _: VectorScalar<name, _i8, _v4i8>;
  defm _: VectorScalar<name, _i16, _v2i16>;
  defm _: VectorScalar<name, _i16, _v3i16>;
  defm _: VectorScalar<name, _i16, _v4i16>;
  defm _: VectorScalar<name, _i32, _v2i32>;
  defm _: VectorScalar<name, _i32, _v3i32>;
  defm _: VectorScalar<name, _i32, _v4i32>;
  defm _: VectorScalar<name, _i64, _v2i64>;
  defm _: VectorScalar<name, _i64, _v3i64>;
  defm _: VectorScalar<name, _i64, _v4i64>;

  // Vector to vector copy
  defm _: VectorVector<name, "32b", _v4i8, _v2i16>;
  defm _: VectorVector<name, "64b", _v4i16, _v2i32>;
}

def : GINodeEquiv<G_ADDRSPACE_CAST, addrspacecast>;

class AddrcastToGeneric<string name, AddressSpace Out, AddressSpace In> : _pISAInst {
  let AsmString = name#" \t$dst, $src;";
  let OutOperandList = (outs Out.RC:$dst);
  let InOperandList = (ins In.RC:$src);
  let Pattern = [(set (p4 Out.RC:$dst), (addrspacecast (In.PVT In.RC:$src)))];
}

class AddrcastFromGeneric<string name, AddressSpace Out, AddressSpace In>
  : AddrcastToGeneric<name, Out, In> {
  let Pattern = [(set (Out.PVT Out.RC:$dst), (addrspacecast (p4 In.RC:$src)))];
}

multiclass AddrcastInst<string name> {
  defvar Gen = generic;
  foreach NonGen = [private, shared, global, const] in {
    def _gen_#NonGen:  AddrcastToGeneric  <name#"."#Gen#"."#NonGen, Gen, NonGen>;
    def _#NonGen#_gen: AddrcastFromGeneric<name#"."#NonGen#"."#Gen, NonGen, Gen>;
  }
}

class IsaddrInst_r<string name, Intrinsic intrinsic, pisaVT dstVT, pisaVT srcVT>
  : UnaryInst_r<name, intrinsic, dstVT, srcVT>;

multiclass IsaddrInst<string name> {
  def _private: IsaddrInst_r<name#".private", int_pisa_isaddr_private, _i32, _i64>;
  def _shared : IsaddrInst_r<name#".shared",  int_pisa_isaddr_shared,  _i32, _i64>;
  def _global : IsaddrInst_r<name#".global",  int_pisa_isaddr_global,  _i32, _i64>;
  def _const  : IsaddrInst_r<name#".const",   int_pisa_isaddr_const,   _i32, _i64>;
}

class FP2IntRndmodeInst_r<string name, SDNode node, Intrinsic rndmode, pisaVT dstVT, pisaVT srcVT>
  : UnaryInst_r<name, node, dstVT, srcVT> {
  let Pattern = [(set dstRC:$dst, (node (srcVT.VT (rndmode srcRC:$src))))];
  let RoundingMode = pISARoundingModeToXeCommon<rndmode>.RMValue;
}

class Int2FPRndmodeInst_r<string name, SDNode node, Intrinsic rndmode, pisaVT dstVT, pisaVT srcVT>
  : UnaryInst_r<name, node, dstVT, srcVT> {
  let Pattern = [(set dstRC:$dst, (rndmode (dstVT.VT (node srcRC:$src))))];
  let RoundingMode = pISARoundingModeToXeCommon<rndmode>.RMValue;
}

multiclass FP2IntInst<string name> {
  foreach signed = [0, 1] in {
    defvar node = !if(signed, fp_to_sint, fp_to_uint);
    foreach daz = [0, 1] in {
      defvar dazOp = !if(daz,"_daz","");
      defvar dazDir = !if(daz,".daz","");
      foreach fromVT = [_f16, _f32, _f64] in {
        defvar fromDir = !if(!eq(fromVT.directive,""),".f",fromVT.directive);
        defvar fromOp = !if(!eq(fromVT.opcode,"_"),"_f",fromVT.opcode);
        foreach toVT = [_i8, _i16, _i32, _i64] in {
          defvar toDir = !if(signed,".s",".u")#toVT.VT.Size;
          defvar toOp = !if(signed,"_s","_u")#toVT.VT.Size;
          def toOp#fromOp#dazOp#_r :
            UnaryInst_r<name#toDir#fromDir#dazDir, node, toVT, fromVT>, Requires<[!if(daz,useFTZ,noFTZ)]>;
          def toOp#fromOp#dazOp#_i :
            UnaryInst_i<name#toDir#fromDir#dazDir, node, toVT, fromVT>;

          // Opcodes with rounding mode
          foreach rndmode = [int_pisa_assign_rndmode_re, int_pisa_assign_rndmode_rd,
                            int_pisa_assign_rndmode_ru, int_pisa_assign_rndmode_rz] in {
            defvar rndname = !subst("int_pisa_assign_rndmode_", "", ""#rndmode);
            def toOp#fromOp#dazOp#_#rndname# :
              FP2IntRndmodeInst_r<name#toDir#fromDir#dazDir#"."#rndname, node, rndmode, toVT, fromVT>,
              Requires<[!if(daz,useFTZ,noFTZ)]>;
          } // rounding mode
        } // to
      } // from
    } // daz
  } // sign
} // FP2Int multiclass

class convToF64<int signed, pisaVT fromVT> {
  int OC = !cond(!eq(fromVT, _f16) : OpcodeClass.ConvF64HF16,
                 !eq(fromVT, _f32) : OpcodeClass.ConvF64F32,
                 !eq(fromVT, _f64) : OpcodeClass.Mov,
                 !and(!eq(fromVT, _i8), !eq(signed, 0)) : OpcodeClass.ConvF64U8,
                 !and(!eq(fromVT, _i8), !eq(signed, 1)) : OpcodeClass.ConvF64S8,
                 !and(!eq(fromVT, _i16), !eq(signed, 0)) : OpcodeClass.ConvF64U16,
                 !and(!eq(fromVT, _i16), !eq(signed, 1)) : OpcodeClass.ConvF64S16,
                 !and(!eq(fromVT, _i32), !eq(signed, 0)) : OpcodeClass.ConvF64U32,
                 !and(!eq(fromVT, _i32), !eq(signed, 1)) : OpcodeClass.ConvF64S32,
                 !and(!eq(fromVT, _i64), !eq(signed, 0)) : OpcodeClass.ConvF64U64,
                 !and(!eq(fromVT, _i64), !eq(signed, 1)) : OpcodeClass.ConvF64S64,
                 true : -1);
}

class convToHF16<int signed, pisaVT fromVT> {
  int OC = !cond(!eq(fromVT, _f16) : OpcodeClass.Mov,
                 !eq(fromVT, _f32) : OpcodeClass.ConvHF16F32,
                 !eq(fromVT, _f64) : OpcodeClass.ConvHF16F64,
                 !and(!eq(fromVT, _i8), !eq(signed, 0)) : OpcodeClass.ConvHF16U8,
                 !and(!eq(fromVT, _i8), !eq(signed, 1)) : OpcodeClass.ConvHF16S8,
                 !and(!eq(fromVT, _i16), !eq(signed, 0)) : OpcodeClass.ConvHF16U16,
                 !and(!eq(fromVT, _i16), !eq(signed, 1)) : OpcodeClass.ConvHF16S16,
                 !and(!eq(fromVT, _i32), !eq(signed, 0)) : OpcodeClass.ConvHF16U32,
                 !and(!eq(fromVT, _i32), !eq(signed, 1)) : OpcodeClass.ConvHF16S32,
                 !and(!eq(fromVT, _i64), !eq(signed, 0)) : OpcodeClass.ConvHF16U64,
                 !and(!eq(fromVT, _i64), !eq(signed, 1)) : OpcodeClass.ConvHF16S64,
                 true : -1);
}

class convToF32<int signed, pisaVT fromVT> {
  int OC = !cond(!eq(fromVT, _f16) : OpcodeClass.ConvF32HF16,
                 !eq(fromVT, _f32) : OpcodeClass.Mov,
                 !eq(fromVT, _f64) : OpcodeClass.ConvF32F64,
                 !and(!eq(fromVT, _i8), !eq(signed, 0)) : OpcodeClass.ConvF32U8,
                 !and(!eq(fromVT, _i8), !eq(signed, 1)) : OpcodeClass.ConvF32S8,
                 !and(!eq(fromVT, _i16), !eq(signed, 0)) : OpcodeClass.ConvF32U16,
                 !and(!eq(fromVT, _i16), !eq(signed, 1)) : OpcodeClass.ConvF32S16,
                 !and(!eq(fromVT, _i32), !eq(signed, 0)) : OpcodeClass.ConvF32U32,
                 !and(!eq(fromVT, _i32), !eq(signed, 1)) : OpcodeClass.ConvF32S32,
                 !and(!eq(fromVT, _i64), !eq(signed, 0)) : OpcodeClass.ConvF32U64,
                 !and(!eq(fromVT, _i64), !eq(signed, 1)) : OpcodeClass.ConvF32S64,
                 true : -1);
}

class convToFloatOpcode<pisaVT fromVT, int signed, pisaVT toVT> {
  int OC = !cond(!eq(toVT, _f16) : convToHF16<signed, fromVT>.OC,
                 !eq(toVT, _f32) : convToF32<signed, fromVT>.OC,
                 !eq(toVT, _f64) : convToF64<signed, fromVT>.OC,
                 true : -1);
}

multiclass Int2FPInst<string name> {
  foreach signed = [0, 1] in {
    defvar node = !if(signed, sint_to_fp, uint_to_fp);
    foreach fromVT = [_i8, _i16, _i32, _i64] in {
      defvar fromDir = !if(signed,".s",".u")#fromVT.VT.Size;
      defvar fromOp = !if(signed,"_s","_u")#fromVT.VT.Size;
      foreach toVT = [_f16, _f32, _f64] in {
        defvar toDir = !if(!eq(toVT.directive,""),".f",toVT.directive);
        defvar toOp = !if(!eq(toVT.opcode,"_"),"_f",toVT.opcode);
        let OpcodeClass = convToFloatOpcode<fromVT, signed, toVT>.OC in {
          def toOp#fromOp#_r : UnaryInst_r<name#toDir#fromDir, node, toVT, fromVT>;
          def toOp#fromOp#_i : UnaryInst_i<name#toDir#fromDir, node, toVT, fromVT>;
        }

        // Opcodes with rounding mode
        foreach rndmode = [int_pisa_assign_rndmode_re, int_pisa_assign_rndmode_rd,
                          int_pisa_assign_rndmode_ru, int_pisa_assign_rndmode_rz] in {
          defvar rndname = !subst("int_pisa_assign_rndmode_", "", ""#rndmode);
          let OpcodeClass = convToFloatOpcode<fromVT, signed, toVT>.OC in {
            def toOp#fromOp#_#rndname# :
              Int2FPRndmodeInst_r<name#toDir#fromDir#"."#rndname, node, rndmode, toVT, fromVT>;
          }
        } // rounding mode
      } // from
    } // to
  } // sign
} // FP2Int multiclass

class ShflInfo<string op, Intrinsic intri> {
  string opcode = op;
  Intrinsic intrinsic = intri;
}

def ShflUp   : ShflInfo<"up", int_pisa_shfl_up>;
def ShflDown : ShflInfo<"dn", int_pisa_shfl_down>;
def ShflXor  : ShflInfo<"xor", int_pisa_shfl_xor>;
def ShflIdx  : ShflInfo<"idx", int_pisa_shfl_idx>;

multiclass ShflInst<string name> {
  foreach tuple = [ShflUp, ShflDown, ShflXor, ShflIdx] in {
    defvar directive = name#"."#tuple.opcode#".32b";
    def tuple.opcode#rrr:TernaryInst_rrr<directive, tuple.intrinsic, _i32, _i32>;
    def tuple.opcode#rri:TernaryInst_rri<directive, tuple.intrinsic, _i32, _i32>;
    def tuple.opcode#rir:TernaryInst_rir<directive, tuple.intrinsic, _i32, _i32>;
    def tuple.opcode#rii:TernaryInst_rii<directive, tuple.intrinsic, _i32, _i32>;
  }
}

multiclass SpecialRegPat<Intrinsic Intr, Register Reg> {
  defvar RetTy = Intr.RetTypes[0];
  defvar VT = !if(!isa<LLVMQualPointerType>(RetTy),
                !cast<pisaVT>("_p"#!cast<LLVMQualPointerType>(RetTy).AddrSpace),
                !cast<pisaVT>("_"#RetTy.VT));
  def : Pat<(Intr),
      (COPY_TO_REGCLASS (VT.VT Reg), VT.RC)>;
}

// 8 Special Registers

defm : SpecialRegPat<int_pisa_localid,           SpecialReg_LocalIdXYZ>;
defm : SpecialRegPat<int_pisa_localid_x,         SpecialReg_LocalIdX>;
defm : SpecialRegPat<int_pisa_localid_y,         SpecialReg_LocalIdY>;
defm : SpecialRegPat<int_pisa_localid_z,         SpecialReg_LocalIdZ>;
defm : SpecialRegPat<int_pisa_localsize,         SpecialReg_LocalSizeXYZ>;
defm : SpecialRegPat<int_pisa_localsize_x,       SpecialReg_LocalSizeX>;
defm : SpecialRegPat<int_pisa_localsize_y,       SpecialReg_LocalSizeY>;
defm : SpecialRegPat<int_pisa_localsize_z,       SpecialReg_LocalSizeZ>;
defm : SpecialRegPat<int_pisa_groupid,           SpecialReg_GroupIdXYZ>;
defm : SpecialRegPat<int_pisa_groupid_x,         SpecialReg_GroupIdX>;
defm : SpecialRegPat<int_pisa_groupid_y,         SpecialReg_GroupIdY>;
defm : SpecialRegPat<int_pisa_groupid_z,         SpecialReg_GroupIdZ>;
defm : SpecialRegPat<int_pisa_groupcount,        SpecialReg_GroupCountXYZ>;
defm : SpecialRegPat<int_pisa_groupcount_x,      SpecialReg_GroupCountX>;
defm : SpecialRegPat<int_pisa_groupcount_y,      SpecialReg_GroupCountY>;
defm : SpecialRegPat<int_pisa_groupcount_z,      SpecialReg_GroupCountZ>;
defm : SpecialRegPat<int_pisa_subgroupid,        SpecialReg_SubgroupId>;
defm : SpecialRegPat<int_pisa_laneid,            SpecialReg_LaneId>;
defm : SpecialRegPat<int_pisa_logicalsubsliceid, SpecialReg_LogicalSubSliceId>;
// Due to a tablegen issue, if your intrinsic returns a pointer, you'll need to
// implement it manually in instruction selection for now.

// 9.1 Arithmetic Instructions

let OpcodeClass = OpcodeClass.IAdd in {
  defm iadd: BinInst<"iadd", add>;
}
defm ptradd: BinInst<"iadd", ptradd>;
defm isub: BinInst<"isub", sub>;
defm iavg: IavgInst<"iavg">;
defm udiv_quo: BinInst<"udiv.quo", udiv>;
defm sdiv_quo: BinInst<"sdiv.quo", sdiv>;
defm udiv_rem: BinInst<"udiv.rem", urem>;
defm sdiv_rem: BinInst<"sdiv.rem", srem>;
defm umad: MadInst<"umad", 0>;
defm smad: MadInst<"smad", 1>;
defm umul: MulInst<"umul", 0>;
defm smul: MulInst<"smul", 1>;
defm dp4a: DP4AInst<"dp4a">;
defm iabs: UnaryInst<"iabs", abs>;
defm ineg: UnaryInst<"ineg", ineg>;

let isCommutable = 1 in {
  defm smax: BinInst<"smax", smax>;
  defm smin: BinInst<"smin", smin>;
  defm umax: BinInst<"umax", umax>;
  defm umin: BinInst<"umin", umin>;
}

// in32 and int64 addition and subtraction with carry-out.
defm iadd_co : ADD_SUB_INT_CARRY<"iadd.co">;
defm isub_co : ADD_SUB_INT_CARRY<"isub.co">;

// int32 and int64 addition and subtraction with carry-in and carry-out.
defm iadd_ci_co : ADD_SUB_INT_CARRY<"iadd.ci.co">;
defm isub_ci_co : ADD_SUB_INT_CARRY<"isub.ci.co">;

// 9.2 Floating-point Instructions

let OpcodeClass = OpcodeClass.FAdd in {
  defm fadd: FloatBinInst<"fadd", fadd>;
}
defm fsub: FloatBinInst<"fsub", fsub>;
defm fmul: FloatBinInst<"fmul", fmul>;
defm fdiv: FloatBinInstFast<"fdiv", fdiv>;
defm fmad: FMadInst<"fmad">;
defm fmin: FMinMaxInst<"fmin", fminnum>;
defm fmax: FMinMaxInst<"fmax", fmaxnum>;
defm fneg: UnaryFloatInst<"fneg", fneg>;
defm fsin: UnaryFloatMathInst<"fsin", fsin>;
defm fcos: UnaryFloatMathInst<"fcos", fcos>;
defm flog2: UnaryFloatMathInst<"flog2", flog2>;
defm fexp2: UnaryFloatMathInst<"fexp2", fexp2>;

// 9.3 Bitwise Instructions

let OpcodeClass = OpcodeClass.And in {
  defm and   : BinInst<"and", and>;
}
let OpcodeClass = OpcodeClass.Or in {
  defm or    : BinInst<"or", or>;
}
let OpcodeClass = OpcodeClass.Xor in {
  defm xor   : BinInst<"xor", xor>;
}
let OpcodeClass = OpcodeClass.Not in {
  defm not   : UnaryInst<"not", not>;
}

defm cbit  : UnaryInst<"cbit", ctpop>;
defm bfrev : UnaryInst<"bfrev", bitreverse>;
defm fbl   : UnaryInst<"fbl", cttz>;
defm fbl0  : UnaryInst<"fbl", cttz_zero_undef>;
defm ufbh  : UnaryInst<"ufbh", ctlz>;
defm ufbh0 : UnaryInst<"ufbh", ctlz_zero_undef>;

// 9.4 Shift Instructions

defm asr  : ShiftInst<"asr", sra>;
defm shr  : ShiftInst<"shr", srl>;
defm shl  : ShiftInst<"shl", shl>;
defm fshr : FunnelShiftInst<"shf.r", fshr>;
defm fshl : FunnelShiftInst<"shf.l", fshl>;

// 9.5 Data Movement Instructions

defm mov : Mov;
defm bitcast: BitcastInst<"bitcast">;
defm addrcast: AddrcastInst<"addrcast">;
defm isaddr: IsaddrInst<"isaddr">;
defm f2i: FP2IntInst<"f2i">;
defm i2f: Int2FPInst<"i2f">;
defm shfl: ShflInst<"shfl">;

// MIR for G_FRAME_INDEX
defm addrof : AddrOf<>;

// trunc
foreach FromVT = [_i16, _i32, _i64] in {
  def trunc_8b_#FromVT.name#rr : ConvInst<"trunc", _i8, FromVT, trunc>;
}
foreach FromVT = [_i32, _i64] in {
  def trunc_16b_#FromVT.name#rr : ConvInst<"trunc", _i16, FromVT, trunc>;
}
def trunc_32b_64brr : ConvInst<"trunc", _i32, _i64, trunc>;

// zext+anyext
foreach FromVT = [_i8, _i16, _i32] in {
  def anyext_64b_#FromVT.name#rr : ConvInst<"zext", _i64, FromVT, anyext>;
  def zext_64b_#FromVT.name#rr   : ConvInst<"zext", _i64, FromVT, zext>;
}
foreach FromVT = [_i8, _i16] in {
  def anyext_32b_#FromVT.name#rr : ConvInst<"zext", _i32, FromVT, anyext>;
  def zext_32b_#FromVT.name#rr   : ConvInst<"zext", _i32, FromVT, zext>;
}
def anyext_16b_8brr : ConvInst<"zext", _i16, _i8, anyext>;
def zext_16b_8brr   : ConvInst<"zext", _i16, _i8, zext>;

// sext
foreach FromVT = [_i8, _i16, _i32] in {
  def sext_64b_#FromVT.name#rr : ConvInst<"sext", _i64, FromVT, sext>;
}
foreach FromVT = [_i8, _i16] in {
  def sext_32b_#FromVT.name#rr : ConvInst<"sext", _i32, FromVT, sext>;
}
def sext_16b_8brr : ConvInst<"sext", _i16, _i8, sext>;

// fext
// We assume .ftz is not necessary when going from lower-precision to higher-precision float
def fext_f_hfr : FextInst<_f32, _f16>;
def fext_df_hfr : FextInst<_f64, _f16>;
def fext_df_fr : FextInst<_f64, _f32>;

// ftrunc
defm ftrunc_hf_f : FtruncInst<_f16, _f32>;
defm ftrunc_hf_df : FtruncInst<_f16, _f64>;
defm ftrunc_f_df : FtruncInst<_f32, _f64>;

// 9.6 Comparison Instructions

let OpcodeClass = OpcodeClass.UCmp in {
  defm ucmp: CmpInst<"ucmp", 0>;
}
let OpcodeClass = OpcodeClass.SCmp in {
  defm scmp: CmpInst<"scmp", 1>;
}
let OpcodeClass = OpcodeClass.FCmp in {
  defm fcmp: FCmpInst<"fcmp">;
}
defm sel: SelInst<"sel">;

// 9.7 Control Flow Instructions

def brtarget : Operand<OtherVT>;

def : GINodeEquiv<G_BRCOND, brcond>;

let isTerminator=1, isBranch=1 in {
  def gotolabel : pISAInst<(outs), (ins brtarget:$label),
                           "goto $label;",
                           [(br bb:$label)]> {
    let isBarrier = 1;
  }
  def predgoto : pISAInst<(outs), (ins Negate:$negate, Pred:$cond, brtarget:$label),
                          "($negate$cond) goto $label;",
                          [(brcond Pred:$cond, bb:$label)]>;
}

multiclass RetVal<string name> {
  // FIXME: Remove return predicate since it's not allowed in pISA
  def _p:  pISAInst<(outs), (ins Pred:$value),  name#" $value;">;

  foreach VT = VTs.Fixed in {
    def "_"#VT.name#"_r":  pISAInst<(outs), (ins VT.RegOpnd:$value),  name#" $value;">;
  }
}

let isReturn = 1, isBarrier = 1, isTerminator = 1, isNotDuplicable = 1 in {
  def ret: SimpleOp<"return void;">;
  defm retValue: RetVal<"return">;
}

// CallTargetFunc can be a register or globalVar
def CallTargetFunc : Operand<i64> {
  let PrintMethod="printCallTargetFunc";
}

let isCall = 1 in {
  def functionCall_void: pISAInst<(outs), (ins CallTargetFunc:$func, variable_ops),
                                           "call void, $func;">;

  foreach VT = VTs.Fixed in {
    def "functionCall_"#VT.name#"_r": pISAInst<(outs VT.RC:$ret),
            (ins CallTargetFunc:$func, variable_ops), "call $ret, $func;">;
  }
}

// 9.8 Memory Instructions

class IOTypes<list<pisaVT> ScalarTypes, list<pisaVT> Vector2Types,
              list<pisaVT> Vector3Types,
              list<pisaVT> Vector4Types> {
  list<pisaVT> VTs = !listconcat(ScalarTypes, Vector2Types, Vector3Types, Vector4Types);
}

def LoadStoreTypes : IOTypes<[ _i8, _i16, _i32, _i64 ],
                             [ _v2i8, _v2i16, _v2i32, _v2i64 ],
                             [ _v3i32 ],
                             [ _v4i8, _v4i16, _v4i32 ]>;

class MakeTypeName<pisaVT VT> {
  string name = !if(!isa<pisaPtrVT>(VT),
                    !cast<string>(VT),
                    VT.name);
}

multiclass LoadInstAllTypes<AddressSpace AS> {
  foreach pVT = LoadStoreTypes.VTs in {
    defvar VT = pVT.VT;
    defvar directive = "ld."#AS#pVT.directive;
    defvar name = MakeTypeName<pVT>.name;
    def "_"#name#"_r"  : pISALoad_r<directive, AS, pVT>;
    def "_"#name#"_ri" : pISALoad_ri<directive, AS, pVT>;
    def "_"#name#"_rr" : pISALoad_rr<directive, AS, pVT>;
  }
}

// Load Instructions:
// The following generic multiclass produces definitions for load instructions with
// the format: opcode_addrspace_memorder_cachecontrol_vec_bitwidth
// E.g.        load_{8b, 16b, 32b, 64b}_{private, shared, ...}_{relaxed, ...}_{ca, cg, ...}_{v2, v4}
multiclass LoadInst {
  foreach addrType = [global, const, generic, private, shared] in {
    defm "_"#addrType : LoadInstAllTypes<addrType>;
  }
}

defm load : LoadInst;

// Store Instructions:
// The following generic multiclass produces definitions for store instructions with
// the format: opcode_addrspace_memorder_cachecontrol_vec_bitwidth
// E.g.        store_{8b, 16b, 32b, 64b}_{private, shared, ...}_{relaxed, ...}_{ca, cg, ...}_{v2, v4}
multiclass StoreInstAllTypes<AddressSpace AS> {
  foreach VT = LoadStoreTypes.VTs in {
    defvar directive = "st."#AS#VT.directive;
    defvar name = MakeTypeName<VT>.name;
    def "_"#name#"_r"  : pISAStore_r <directive, AS, VT>;
    def "_"#name#"_ri" : pISAStore_ri<directive, AS, VT>;
    def "_"#name#"_rr" : pISAStore_rr<directive, AS, VT>;
  }
}

multiclass StoreInst {
  foreach as = [global, generic, private, shared] in {
    defm "_"#as : StoreInstAllTypes<as>;
  }
}

defm store : StoreInst;

let isMeta = 1 in {
multiclass FunctionParameter {
  foreach VT = VTs.Fixed in {
    def "_"#VT.name:  pISAInst<(outs VT.RC:$var),  (ins i32imm:$idx), " ; ">;
  }
}
defm functionParameter: FunctionParameter<>;
}

multiclass LoadParam {
  foreach VT = LoadStoreTypes.VTs in {
    defvar directive = VT.directive;
    defvar opcode = VT.opcode;
    def opcode#b: pISAInst<(outs VT.RC:$res), (ins i32imm:$idx, AddrOffImm<i32>:$offset),
                           "ld.param"#directive#" $res, [%arg$idx$offset];">;
  }
}
defm loadParam: LoadParam<>;

// 9.10 Synchronization Instructions

let isConvergent = 1, hasSideEffects = 1 in {
  def barrier_wg : pISAInst<(outs), (ins), "barrier.workgroup;", [(int_pisa_workgroup_barrier)]>;
  def barrier_sg : pISAInst<(outs), (ins), "barrier.subgroup;", [(int_pisa_subgroup_barrier)]>;
  def sbarrier_signal : pISAInst<(outs), (ins), "sbarrier.signal;", [(int_pisa_sbarrier_signal)]>;
  def sbarrier_wait : pISAInst<(outs), (ins), "sbarrier.wait;", [(int_pisa_sbarrier_wait)]>;
  def fence : pISAInst<(outs), (ins FenceScope:$scope), "fence.$scope;", [(int_pisa_fence i32:$scope)]>;
}

// 9.8.4 iatom

class IAtomOpName<string Op> {
  string name = !cond(!eq(Op, "min"): "smin",
                      !eq(Op, "max"): "smax",
                      true           : Op);
}

class IAtomMemOrder<string MO> {
  string name = !cond(!eq(MO, "monotonic"): "relaxed",
                      true                : MO);
}

// 1 source operand
defm atomic_load_incwrap : binary_atomic_op<atomic_load_uinc_wrap>;
defm atomic_load_decwrap : binary_atomic_op<atomic_load_udec_wrap>;

// 2 source operands
multiclass BinaryAtomicInst<string Op> {
  foreach AS = [global, shared, generic] in {
    foreach VT = [_i16, _i32, _i64] in {
      defvar VTStr = VT.directive;

      // iatom.shared.<non-cas-op> allowed only for i16, i32
      if !not(!and(!eq(AS, shared), !eq(VT, _i64))) then {
        foreach MO = ["monotonic", "release", "acquire", "acq_rel"] in {
          defvar typeName = MakeTypeName<VT>.name;
          defvar size = !subst(".", "", !subst("b", "", VT.directive));
          defvar fullName = !strconcat("atomic_load_" , Op,      "_", size, "_", MO);
          defvar node = !cast<PatFrag>(fullName);
          defvar prefix = "i";
          defvar MOStr = IAtomMemOrder<MO>.name;
          defvar OpStr = IAtomOpName<Op>.name;
          defvar directive = prefix#"atom."#AS.name#"."#MOStr#"."#OpStr#VTStr;

          def "_"#typeName#"_"#AS.name#"_"#MO#"_"#Op#"_r"  : pISABinaryAtom_r <directive, AS, VT, node>;
          def "_"#typeName#"_"#AS.name#"_"#MO#"_"#Op#"_ri" : pISABinaryAtom_ri<directive, AS, VT, node>;
          def "_"#typeName#"_"#AS.name#"_"#MO#"_"#Op#"_rr" : pISABinaryAtom_rr<directive, AS, VT, node>;
        }
      }
    }
  }
}

// 3 source operands
multiclass TernaryAtomicInst<string Op> {
  foreach AS = [global, shared, generic] in {
    foreach VT = [_i16, _i32, _i64] in {
      foreach MO = ["acq_rel"] in {
        defvar typeName = MakeTypeName<VT>.name;
        defvar size = !subst(".", "", !subst("b", "", VT.directive));
        defvar fullName = !strconcat("atomic_", Op, "_", size, "_", MO);
        defvar node = !cast<PatFrag>(fullName);
        defvar directive = "iatom."#AS.name#"."#IAtomMemOrder<MO>.name#"."#"cas"#VT.directive;

        def "_"#typeName#"_"#AS.name#"_"#MO#"_"#Op#"_r"  : pISATernaryAtom_r <directive, AS, VT, node>;
        def "_"#typeName#"_"#AS.name#"_"#MO#"_"#Op#"_ri" : pISATernaryAtom_ri<directive, AS, VT, node>;
        def "_"#typeName#"_"#AS.name#"_"#MO#"_"#Op#"_rr" : pISATernaryAtom_rr<directive, AS, VT, node>;
      }
    }
  }
}

defm atomic_load_incwrap_impl : BinaryAtomicInst<"incwrap">;
defm atomic_load_decwrap_impl : BinaryAtomicInst<"decwrap">;

defm atomic_load_add_impl : BinaryAtomicInst<"add">;
defm atomic_load_sub_impl : BinaryAtomicInst<"sub">;
defm atomic_load_and_impl : BinaryAtomicInst<"and">;
defm atomic_load_or_impl : BinaryAtomicInst<"or">;
defm atomic_load_xor_impl : BinaryAtomicInst<"xor">;
defm atomic_load_min_impl : BinaryAtomicInst<"min">;
defm atomic_load_max_impl : BinaryAtomicInst<"max">;
defm atomic_load_umin_impl : BinaryAtomicInst<"umin">;
defm atomic_load_umax_impl : BinaryAtomicInst<"umax">;
// TODO: llvm's cmpxchg instruction takes 2 memory orderings. Why?
defm atomic_cmp_swap   : TernaryAtomicInst<"cmp_swap">;

// 9.8.5 fatom

defm fatomic_load_add  : binary_atomic_op<atomic_load_fadd, 0>;
defm fatomic_load_sub  : binary_atomic_op<atomic_load_fsub, 0>;
defm fatomic_load_min  : binary_atomic_op<atomic_load_fmin, 0>;
defm fatomic_load_max  : binary_atomic_op<atomic_load_fmax, 0>;

// 2 source operands
multiclass BinaryFloatAtomicInst<string Op> {
  foreach AS = [global, shared, generic] in {
    foreach VT = [_f16, _f32, _f64] in {
      defvar VTStr = !if(!eq(VT, _f16), _i16.directive,
                         !if(!eq(VT, _f32), _i32.directive,
                         !if(!eq(VT, _f64), _i64.directive, "?")));

      assert !not(!eq(VTStr, "?")), "unknown VT!";

      // fatom.shared allowed only for f16, f32
      if !not(!and(!eq(AS, shared), !eq(VT, _f64))) then {
        foreach MO = ["monotonic", "release", "acquire", "acq_rel"] in {
          defvar typeName = MakeTypeName<VT>.name;
          defvar size = !if(!eq(VT, _f16), "16",
                            !if(!eq(VT, _f32), "32",
                            !if(!eq(VT, _f64), "64", "0")));
          defvar fullName = !strconcat("fatomic_load_", Op, "_", size, "_", MO);
          defvar node = !cast<PatFrag>(fullName);
          defvar MOStr = IAtomMemOrder<MO>.name;
          defvar prefix = "f";
          defvar directive = prefix#"atom."#AS.name#"."#MOStr#"."#Op#VTStr;

          def "_"#typeName#"_"#AS.name#"_"#MO#"_"#Op#"_r"  : pISABinaryAtom_r <directive, AS, VT, node>;
          def "_"#typeName#"_"#AS.name#"_"#MO#"_"#Op#"_ri" : pISABinaryAtom_ri<directive, AS, VT, node>;
          def "_"#typeName#"_"#AS.name#"_"#MO#"_"#Op#"_rr" : pISABinaryAtom_rr<directive, AS, VT, node>;
        }
      }
    }
  }
}

defm atomic_load_fadd_impl : BinaryFloatAtomicInst<"add">;
defm atomic_load_fsub_impl : BinaryFloatAtomicInst<"sub">;
defm atomic_load_fmin_impl : BinaryFloatAtomicInst<"min">;
defm atomic_load_fmax_impl : BinaryFloatAtomicInst<"max">;

// 3 src operands (cas)

multiclass TernaryFloatAtomicInst<string Op> {
  foreach AS = [global, shared, generic] in {
    foreach VT = [_f16, _f32, _f64] in {
      defvar VTStr = !if(!eq(VT, _f16), _i16.directive,
                         !if(!eq(VT, _f32), _i32.directive,
                         !if(!eq(VT, _f64), _i64.directive, "?")));

      assert !not(!eq(VTStr, "?")), "unknown VT!";

      // fatom.shared allowed only for f16, f32
      if !not(!and(!eq(AS, shared), !eq(VT, _f64))) then {
        foreach MO = ["acq_rel"] in {
          defvar typeName = MakeTypeName<VT>.name;
          defvar size = !if(!eq(VT, _f16), "16",
                            !if(!eq(VT, _f32), "32",
                            !if(!eq(VT, _f64), "64", "0")));
          defvar MOStr = IAtomMemOrder<MO>.name;

          defvar directive = "fatom."#AS.name#"."#MOStr#"."#Op#VTStr;

          def "_"#typeName#"_"#AS.name#"_"#MO#"_"#Op#"_r"  : pISAFAtomCas_r <directive, AS, VT>;
          def "_"#typeName#"_"#AS.name#"_"#MO#"_"#Op#"_ri" : pISAFAtomCas_ri<directive, AS, VT>;
          def "_"#typeName#"_"#AS.name#"_"#MO#"_"#Op#"_rr" : pISAFAtomCas_rr<directive, AS, VT>;
        }
      }
    }
  }
}

defm atomic_load_fcas_impl : TernaryFloatAtomicInst<"cas">;


// begin intel embargo
include "pISAInstrInfoRaytracing.td"
// end intel embargo
