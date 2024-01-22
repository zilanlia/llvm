//===-- pISAInstrFormats.td - pISA Instruction Formats --*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
include "pISAXeCommon.td"

class pISAAsmOperand<string am> : AsmOperandClass {
  let Name = am;
  let ParserMethod = "parse"#am;
}
class pISAOperand<ValueType ty, string pm, string am, string enc, string dec>
  : Operand<ty> {
  let PrintMethod = pm;
  let ParserMatchClass = pISAAsmOperand<am>;
  let EncoderMethod = enc;
  let DecoderMethod = dec;
}
class pISARegisterOperand<RegisterClass rc,
                          string pm, string am, string enc, string dec>
  : RegisterOperand<rc, pm> {
  let PrintMethod = pm;
  let ParserMatchClass = pISAAsmOperand<am>;
  let EncoderMethod = enc;
  let DecoderMethod = dec;
}
class pISAImmediateOperand<ValueType ty,
                           string pm, string am, string enc, string dec>
  : pISAOperand<ty, pm, am, enc, dec> {
  let OperandType = "OPERAND_IMMEDIATE";
}

// Immediate address offset operand for the load/store
class AddrOffImm<ValueType VT> : Operand<VT>{
  let OperandType = "OPERAND_IMMEDIATE";
  let PrintMethod = "printAddrOffsetImm";
}

class OpcodeInfo {
  int OpcodeClass = -1;
  int MVType = -1;
  int RoundingMode = -1;
}

class _pISAInst : Instruction, OpcodeInfo {
  field bits<16> Inst;

  let Namespace = "pISA";
  let DecoderNamespace = "pISA";

  let UseNamedOperandTable = 1;
}

class pISAInst<dag outs, dag ins, string asmstr, list<dag> pattern = []>
  : _pISAInst {
  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;
}

// pisaVT should be a container class only instead of a ValueType.
class pisaVT<ValueType vt, string d, RegisterClass rc, bit hasRC = 1> {
  ValueType VT = vt;
  string directive = d;
  string _d = !if(!eq(d, ""), ".", d);
  string opcode = !subst("b", "",
                  !subst(".", "_", _d));
  string __d = !if(!eq(d, ""), ".f", d);
  // remove leading '_' as the name
  string name = !substr(!subst(".", "_", __d), 1);

  string bitwidth = vt.Size#b;
  RegisterClass RC = rc;
  DAGOperand ImmOpnd;
  DAGOperand RegOpnd = !if(hasRC, pISARegisterOperand<rc,
                                                      "print"#rc#"Opnd",
                                                      rc#"Opnd",
                                                      "encode"#rc#"Opnd",
                                                      "decode"#rc#"Opnd">,
                                  ?);
}

// Subclasses of pisaIntVT, pisaFloatVT, pisaPtrVT, pisaVectorVT, and pisaVoidVT
// are inherited from pisaVT.
// The main goal is to add the functionailty of !isa<subclass>,
// e.g., !isa<pisaIntVT>(_i16) return 1 to tell that it is an int type.
class pisaIntVT<ValueType vt, string d, RegisterClass rc> :
  pisaVT<vt, d, rc> {
  string immName = "Imm"#vt.Size#"Opnd";
  let ImmOpnd = pISAImmediateOperand <vt,
                                      "print"#immName,
                                      immName,
                                      "encode"#immName,
                                      "decode"#immName>;
}

class pisaFloatVT<ValueType vt, string d, RegisterClass rc> :
  pisaVT<vt, d, rc> {
  string immName = "FpImm"#vt.Size#"Opnd";
  let ImmOpnd = pISAImmediateOperand <vt,
                                      "print"#immName,
                                      immName,
                                      "encode"#immName,
                                      "decode"#immName>;
}

class pisaPtrVT<ValueType vt, string d, RegisterClass rc> :
  pisaVT<vt, d, rc>;

class pisaVectorVT<ValueType vt, string d, RegisterClass rc> :
  pisaVT<vt, d, rc>;

class pisaVoidVT<ValueType vt, string d, RegisterClass rc> :
  pisaVT<vt, d, rc, 0>;

def p0 : PtrValueType<i32, 0>;
def p1 : PtrValueType<i64, 1>;
def p2 : PtrValueType<i64, 2>;
def p3 : PtrValueType<i32, 3>;
def p4 : PtrValueType<i64, 4>;

class AddressSpace<int n, pisaVT vt>{
  int num = n;
  pisaVT VT = vt;
  PtrValueType PVT = !cast<PtrValueType>("p"#num);
  DAGOperand RC = vt.RegOpnd;
  string name = NAME;
}

//                       <ValueType, directive, RegisterClass>;
def _void  :   pisaVoidVT<isVoid,    "",        ?            >;
def _i1    :    pisaIntVT<i1,        ".1b",     Pred         >;
def _i8    :    pisaIntVT<i8,        ".8b",     Reg8b        >;
def _i16   :    pisaIntVT<i16,       ".16b",    Reg16b       >;
def _i32   :    pisaIntVT<i32,       ".32b",    Reg32b       >;
def _i64   :    pisaIntVT<i64,       ".64b",    Reg64b       >;
def _f16   :  pisaFloatVT<f16,       ".hf",     Reg16b       >;
def _f32   :  pisaFloatVT<f32,       "",        Reg32b       >;
def _f64   :  pisaFloatVT<f64,       ".df",     Reg64b       >;
def _p0    :    pisaPtrVT<p0,        ".32b",    Reg32b       >;
def _p1    :    pisaPtrVT<p1,        ".64b",    Reg64b       >;
def _p2    :    pisaPtrVT<p2,        ".64b",    Reg64b       >;
def _p3    :    pisaPtrVT<p3,        ".32b",    Reg32b       >;
def _p4    :    pisaPtrVT<p4,        ".64b",    Reg64b       >;
def _v2i8  : pisaVectorVT<v2i8,      ".v2.8b",  RegV2_8b     >;
def _v4i8  : pisaVectorVT<v4i8,      ".v4.8b",  RegV4_8b     >;
def _v2i16 : pisaVectorVT<v2i16,     ".v2.16b", RegV2_16b    >;
def _v3i16 : pisaVectorVT<v3i16,     ".v3.16b", RegV3_16b    >;
def _v4i16 : pisaVectorVT<v4i16,     ".v4.16b", RegV4_16b    >;
def _v2i32 : pisaVectorVT<v2i32,     ".v2.32b", RegV2_32b    >;
def _v3i32 : pisaVectorVT<v3i32,     ".v3.32b", RegV3_32b    >;
def _v4i32 : pisaVectorVT<v4i32,     ".v4.32b", RegV4_32b    >;
def _v2i64 : pisaVectorVT<v2i64,     ".v2.64b", RegV2_64b    >;
def _v3i64 : pisaVectorVT<v3i64,     ".v3.64b", RegV3_64b    >;
def _v4i64 : pisaVectorVT<v4i64,     ".v4.64b", RegV4_64b    >;

class pISAValueTypes {
  list<pisaVT> Integer = [_i8, _i16, _i32, _i64];
  list<pisaVT> Float = [_f16, _f32, _f64];
  list<pisaVT> Ptr = [_p0, _p1, _p2, _p3, _p4];
  list<pisaVT> Vector = [_v2i8,          _v4i8,
                         _v2i16, _v3i16, _v4i16,
                         _v2i32, _v3i32, _v4i32,
                         _v2i64, _v3i64, _v4i64];
  list<pisaVT> Fixed = !listconcat(Integer, Vector);
}

class pISAVTToMVType<pisaVT VT> {
  int VTValue = VT.VT.Value;
}

class pISARoundingModeToXeCommon<Intrinsic Func> {
  string rndname = !subst("int_pisa_assign_rndmode_", "", ""#Func);
  int RMValue = !if(!eq(rndname, "re"), RoundingModes.RE,
                    !if(!eq(rndname, "rd"), RoundingModes.RD,
                    !if(!eq(rndname, "ru"), RoundingModes.RU,
                    !if(!eq(rndname, "rz"), RoundingModes.RZ,
                    -1))));
}

def private : AddressSpace<0, _i32>;
def global  : AddressSpace<1, _i64>;
def const   : AddressSpace<2, _i64>;
def shared  : AddressSpace<3, _i32>;
def generic : AddressSpace<4, _i64>;

class StoreAddrspacePat<AddressSpace AS> : PatFrag<(ops node:$data, node:$ptr), (store node:$data, node:$ptr)> {
  let IsStore = true;
  let IsUnindexed = true;
  let IsNonExtLoad = true;
  let AddressSpaces = [ AS.num ];
}

class LoadAddrspacePat<AddressSpace AS> : PatFrag<(ops node:$ptr), (load node:$ptr)> {
  let IsLoad = true;
  let IsUnindexed = true;
  let IsNonExtLoad = true;
  let AddressSpaces = [ AS.num ];
}

class AtomAddrspacePat<AddressSpace Addrspace, PatFrag pat> :
      PatFrag<(ops node:$ptr, node:$other), (pat node:$ptr, node:$other)> {
  let AddressSpaces = [ Addrspace.num ];
  let IsAtomic = 1;
}

class TernaryAtomAddrspacePat<AddressSpace Addrspace, PatFrag pat> :
      PatFrag<(ops node:$ptr, node:$opnd1, node:$opnd2), (pat node:$ptr, node:$opnd1, node:$opnd2)> {
  let AddressSpaces = [ Addrspace.num ];
  let IsAtomic = 1;
}

class FAtomCasGlobal :
      PatFrag<(ops node:$ptr, node:$opnd1, node:$opnd2), (int_pisa_cas_fatom_global node:$ptr, node:$opnd1, node:$opnd2)> {
  let AddressSpaces = [ global.num ];
}

class FAtomCasShared :
      PatFrag<(ops node:$ptr, node:$opnd1, node:$opnd2), (int_pisa_cas_fatom_shared node:$ptr, node:$opnd1, node:$opnd2)> {
  let AddressSpaces = [ shared.num ];
}

class FAtomCasGeneric:
      PatFrag<(ops node:$ptr, node:$opnd1, node:$opnd2), (int_pisa_cas_fatom_generic node:$ptr, node:$opnd1, node:$opnd2)> {
  let AddressSpaces = [ generic.num ];
}

/********************************* pISAInst class hierachy *********************************/
//                          +--------------------------------+
//                          | _pISAInst:                     |
//                          |   defines Inst, Namespace,     |
//             (Layer 1)    |           DecoderNamespace,    |
//                          |           UseNamedOperandTable |
//                          +--------------------------------+
//                             |             |            |
//             +---------------+             |            +------------------+
//             v                             v                               v
// +--------------------------+  +--------------------------+  +----------------------------+
// | pISAInst1:               |  | pISAInst2:               |  | pISAInst3:                 |
// |   defines srcRC,         |  |   defines src0RC, src1RC,|  |   defines src0RC, src1RC,  |
// |           dstRC,         |  |           dstRC,         |  |           src2RC, dstRC,   |
// |           AsmString,     |  |           AsmString,     |  |           AsmString,       |
// |           OutOperandList |  |           OutOperandList |  |           OutOperandList   |
// +--------------------------+  +--------------------------+  +----------------------------+
//             |                             |                               |
//             v                             v                               v
// +--------------------------+  +--------------------------+  +----------------------------+
// | pISAInst_r,              |  | pISAInst_rr, pISAInst_ri,|  | pISAInst_rrr, pISAInst_irr,|
// | pISAInst_i:              |  | pISAInst_ir, pISAInst_ii:|  | pISAInst_rri, pISAInst_iri,|
// |   defines InOperandList, |  |   defines InOperandList, |  | pISAInst_rir, pISAInst_iir,|
// |           Imm opd        |  |           Imm opd        |  | pISAInst_rii, pISAInst_irr,|
// +--------------------------+  +--------------------------+  | pISAInst_iii:              |
//             |                             |                 |    defines InOperandList,  |
//             |                             |                 |            Imm opd         |
//             |                             |                 +----------------------------+
//             |                             |                               |
//             | (Classes below are Layer 4) |                               |
//             | (x is either r or i)        |                               |
//             v                             v                               v
// +--------------------------+  +--------------------------+  +----------------------------+
// | UnaryInst_x:             |  | BinInst_xx:              |  | TernaryInst_xxx            |
// |   defines Pattern        |  |   defines Pattern        |  |   defines Pattern          |
// |                          |  |                          |  |                            |
// | See also -               |  | See also -               |  | See also -                 |
// |   FP2IntRndmodeInst_r    |  |   IavgBinaryInst_xx      |  |   RndmodeInst_rrr          |
// |   Int2FPRndmodeInst_r    |  |   RndmodeInst_rx         |  |   MadInst_rxx              |
// |                          |  |   CmpBinaryInstPred_rx   |  |                            |
// |                          |  |   CmpBinaryInstReg_rx    |  |                            |
// +--------------------------+  +--------------------------+  +----------------------------+
//
// Description: This is the class hierachy for pISAInst. Each layer defines different fields
//              to share common code. The normal usage is to inherit a class from Layer 3
//              with desired patterns like rr, ri, etc, and to define its own 'Pattern' for
//              DAG selection. All these classes are for non-memory operations. We need to
//              define another formats for memop.
//

class pISAInst1<string name, pisaVT dstVT, pisaVT srcVT> : _pISAInst {
  DAGOperand srcRC = srcVT.RegOpnd;
  DAGOperand dstRC = dstVT.RegOpnd;
  let AsmString = !if(!isa<pisaVoidVT>(dstVT),
    name#" \t$src;",
    name#" \t$dst, $src;");
  let OutOperandList = !if(!isa<pisaVoidVT>(dstVT),
    (outs),
    (outs dstRC:$dst));
  let MVType = pISAVTToMVType<dstVT>.VTValue;
}

class pISAInst_r<string name, pisaVT dstVT, pisaVT srcVT> : pISAInst1<name, dstVT, srcVT> {
  let InOperandList = (ins srcRC:$src);
}

class pISAInst_i<string name, pisaVT dstVT, pisaVT srcVT> : pISAInst1<name, dstVT, srcVT> {
  SDNode immVal = !if(!isa<pisaFloatVT>(srcVT), fpimm, imm);
  DAGOperand immOpd = srcVT.ImmOpnd;
  let InOperandList = (ins immOpd:$src);
}

class pISAInst2<string name, pisaVT dstVT, pisaVT src0VT, pisaVT src1VT> : _pISAInst {
  DAGOperand src0RC = src0VT.RegOpnd;
  DAGOperand src1RC = src1VT.RegOpnd;
  DAGOperand dstRC = dstVT.RegOpnd;
  let AsmString = !if(!isa<pisaVoidVT>(dstVT),
    name#" \t$src0, $src1;",
    name#" \t$dst, $src0, $src1;");
  let OutOperandList = !if(!isa<pisaVoidVT>(dstVT),
    (outs),
    (outs dstRC:$dst));
  let MVType = pISAVTToMVType<dstVT>.VTValue;
}

class pISAInst_rr<string name, pisaVT dstVT, pisaVT src0VT, pisaVT src1VT> : pISAInst2<name, dstVT, src0VT, src1VT> {
  let InOperandList = (ins src0RC:$src0, src1RC:$src1);
}

class pISAInst_ri<string name, pisaVT dstVT, pisaVT src0VT, pisaVT src1VT> : pISAInst2<name, dstVT, src0VT, src1VT> {
  SDNode immVal1 = !if(!isa<pisaFloatVT>(src1VT), fpimm, imm);
  DAGOperand immOpd1 = src1VT.ImmOpnd;
  let InOperandList = (ins src0RC:$src0, immOpd1:$src1);
}

class pISAInst_ir<string name, pisaVT dstVT, pisaVT src0VT, pisaVT src1VT> : pISAInst2<name, dstVT, src0VT, src1VT> {
  SDNode immVal0 = !if(!isa<pisaFloatVT>(src0VT), fpimm, imm);
  DAGOperand immOpd0 = src0VT.ImmOpnd;
  let InOperandList = (ins immOpd0:$src0, src1RC:$src1);
}

class pISAInst_ii<string name, pisaVT dstVT, pisaVT src0VT, pisaVT src1VT> : pISAInst2<name, dstVT, src0VT, src1VT> {
  SDNode immVal0 = !if(!isa<pisaFloatVT>(src0VT), fpimm, imm);
  SDNode immVal1 = !if(!isa<pisaFloatVT>(src1VT), fpimm, imm);
  DAGOperand immOpd0 = src0VT.ImmOpnd;
  DAGOperand immOpd1 = src1VT.ImmOpnd;
  let InOperandList = (ins immOpd0:$src0, immOpd1:$src1);
}

class pISAInst3<string name, pisaVT dstVT, pisaVT src0VT, pisaVT src1VT, pisaVT src2VT> : _pISAInst {
  DAGOperand src0RC = src0VT.RegOpnd;
  DAGOperand src1RC = src1VT.RegOpnd;
  DAGOperand src2RC = src2VT.RegOpnd;
  DAGOperand dstRC = dstVT.RegOpnd;
  let AsmString = !if(!isa<pisaVoidVT>(dstVT),
    name#" \t$src0, $src1, $src2;",
    name#" \t$dst, $src0, $src1, $src2;");
  let OutOperandList = !if(!isa<pisaVoidVT>(dstVT),
    (outs),
    (outs dstRC:$dst));
}

class pISAInst_rrr<string name, pisaVT dstVT, pisaVT src0VT, pisaVT src1VT, pisaVT src2VT> : pISAInst3<name, dstVT, src0VT, src1VT, src2VT> {
  let InOperandList = (ins src0RC:$src0, src1RC:$src1, src2RC:$src2);
}

class pISAInst_rri<string name, pisaVT dstVT, pisaVT src0VT, pisaVT src1VT, pisaVT src2VT> : pISAInst3<name, dstVT, src0VT, src1VT, src2VT> {
  SDNode immVal2 = !if(!isa<pisaFloatVT>(src2VT), fpimm, imm);
  DAGOperand immOpd2 = src2VT.ImmOpnd;
  let InOperandList = (ins src0RC:$src0, src1RC:$src1, immOpd2:$src2);
}

class pISAInst_rir<string name, pisaVT dstVT, pisaVT src0VT, pisaVT src1VT, pisaVT src2VT> : pISAInst3<name, dstVT, src0VT, src1VT, src2VT> {
  SDNode immVal1 = !if(!isa<pisaFloatVT>(src1VT), fpimm, imm);
  DAGOperand immOpd1 = src1VT.ImmOpnd;
  let InOperandList = (ins src0RC:$src0, immOpd1:$src1, src2RC:$src2);
}

class pISAInst_rii<string name, pisaVT dstVT, pisaVT src0VT, pisaVT src1VT, pisaVT src2VT> : pISAInst3<name, dstVT, src0VT, src1VT, src2VT> {
  SDNode immVal1 = !if(!isa<pisaFloatVT>(src1VT), fpimm, imm);
  SDNode immVal2 = !if(!isa<pisaFloatVT>(src2VT), fpimm, imm);
  DAGOperand immOpd1 = src1VT.ImmOpnd;
  DAGOperand immOpd2 = src2VT.ImmOpnd;
  let InOperandList = (ins src0RC:$src0, immOpd1:$src1, immOpd2:$src2);
}

class pISAInst_irr<string name, pisaVT dstVT, pisaVT src0VT, pisaVT src1VT, pisaVT src2VT> : pISAInst3<name, dstVT, src0VT, src1VT, src2VT> {
  SDNode immVal0 = !if(!isa<pisaFloatVT>(src0VT), fpimm, imm);
  DAGOperand immOpd0 = src0VT.ImmOpnd;
  let InOperandList = (ins immOpd0:$src0, src1RC:$src1, src2RC:$src2);
}

class pISAInst_iri<string name, pisaVT dstVT, pisaVT src0VT, pisaVT src1VT, pisaVT src2VT> : pISAInst3<name, dstVT, src0VT, src1VT, src2VT> {
  SDNode immVal0 = !if(!isa<pisaFloatVT>(src0VT), fpimm, imm);
  SDNode immVal2 = !if(!isa<pisaFloatVT>(src2VT), fpimm, imm);
  DAGOperand immOpd0 = src0VT.ImmOpnd;
  DAGOperand immOpd2 = src2VT.ImmOpnd;
  let InOperandList = (ins immOpd0:$src0, src1RC:$src1, immOpd2:$src2);
}

class pISAInst_iir<string name, pisaVT dstVT, pisaVT src0VT, pisaVT src1VT, pisaVT src2VT> : pISAInst3<name, dstVT, src0VT, src1VT, src2VT> {
  SDNode immVal0 = !if(!isa<pisaFloatVT>(src0VT), fpimm, imm);
  SDNode immVal1 = !if(!isa<pisaFloatVT>(src1VT), fpimm, imm);
  DAGOperand immOpd0 = src0VT.ImmOpnd;
  DAGOperand immOpd1 = src1VT.ImmOpnd;
  let InOperandList = (ins immOpd0:$src0, immOpd1:$src1, src2RC:$src2);
}

class pISAInst_iii<string name, pisaVT dstVT, pisaVT src0VT, pisaVT src1VT, pisaVT src2VT> : pISAInst3<name, dstVT, src0VT, src1VT, src2VT> {
  SDNode immVal0 = !if(!isa<pisaFloatVT>(src0VT), fpimm, imm);
  SDNode immVal1 = !if(!isa<pisaFloatVT>(src1VT), fpimm, imm);
  SDNode immVal2 = !if(!isa<pisaFloatVT>(src2VT), fpimm, imm);
  DAGOperand immOpd0 = src0VT.ImmOpnd;
  DAGOperand immOpd1 = src1VT.ImmOpnd;
  DAGOperand immOpd2 = src2VT.ImmOpnd;
  let InOperandList = (ins immOpd0:$src0, immOpd1:$src1, immOpd2:$src2);
}

/****************************** End of pISAInst class hierachy *****************************/

/******************************* pISAMemInst class hierachy ********************************/
//                          +--------------------------------+
//                          | _pISAInst:                     |
//                          |   defines Inst, Namespace,     |
//             (Layer 1)    |           DecoderNamespace,    |
//                          |           UseNamedOperandTable |
//                          +--------------------------------+
//                             |             |            |
//             +---------------+             |            +------------------+
//             v                             v                               v
// +--------------------------+  +--------------------------+  +--------------------------------+
// | pISAStore:               |  | pISALoad:                |  | pISABinaryAtom:                |
// |   defines InOperandList, |  |   defines InOperandList, |  | pISATernaryAtom:               |
// |           OutOperandList,|  |           OutOperandList,|  | pISAFAtomCas:                  |
// |           AsmString,     |  |           AsmString,     |  |    defines InOperandList,      |
// |           Pattern        |  |           Pattern        |  |            OutOperandList,     |
// |                          |  |                          |  |            AsmString,          |
// +--------------------------+  +--------------------------+  |            Pattern             |
//                                                             +--------------------------------+
//
// Description: This is the class hierachy for pISAMemInst. The classes shown above define basic
//              structures of pISAMemInst for load, store and atomic operations, like inputs,
//              outputs, printing formats, patterns, etc. Each memory operation can have different
//              operand types for the addressing mode. We use MemProperty class to define
//              variants of formats for the operand types in a form of pISAMem_xx, where x
//              can be either v(Var), r(Reg), or i(Imm) for the corresponding operand.
//

def ADDR_r  : ComplexPattern<iPTR, 2, "SelectAddr_r",  [], []>;
def ADDR_rr : ComplexPattern<iPTR, 2, "SelectAddr_rr", [], []>;
def ADDR_ri : ComplexPattern<iPTR, 2, "SelectAddr_ri", [], []>;

class MEM_r<AddressSpace AS> : Operand<i64> {
  // For MEM_r, we insert 0 as the imm val for a canonical form: [opnd1 + opnd2]
  // When printing it, only [opnd1] will be printed because it is the same as [opnd1 + 0]
  let MIOperandInfo = (ops AS.RC, i64imm);
  let PrintMethod   = "printMemOperand";
}

class MEM_rr<AddressSpace AS> : Operand<i64> {
  let MIOperandInfo = (ops AS.RC, AS.RC);
  let PrintMethod   = "printMemOperand";
  //let ParserMatchClass = MemRegAsmOperand;
}

class MEM_ri<AddressSpace AS> : Operand<i64> {
  let MIOperandInfo = (ops AS.RC, i64imm);
  let PrintMethod   = "printMemOperand";
}

def gi_ADDR_r  : GIComplexOperandMatcher<s64, "SelectAddr_r">,  GIComplexPatternEquiv<ADDR_r>;
def gi_ADDR_rr : GIComplexOperandMatcher<s64, "SelectAddr_rr">, GIComplexPatternEquiv<ADDR_rr>;
def gi_ADDR_ri : GIComplexOperandMatcher<s64, "SelectAddr_ri">, GIComplexPatternEquiv<ADDR_ri>;

class MemProperty<AddressSpace as, dag i, ComplexPattern pattern>{
  dag Ins = i;
  ComplexPattern Pattern = pattern;
  AddressSpace AS = as;
}

class pISAMem_r <AddressSpace AS> : MemProperty<AS, (ins MEM_r <AS>:$addr), ADDR_r>;
class pISAMem_rr<AddressSpace AS> : MemProperty<AS, (ins MEM_rr<AS>:$addr), ADDR_rr>;
class pISAMem_ri<AddressSpace AS> : MemProperty<AS, (ins MEM_ri<AS>:$addr), ADDR_ri>;

class pISAStore<string directive, AddressSpace AS, pisaVT VT, MemProperty M> : _pISAInst {
  dag OutOperandList = (outs);
  dag InOperandList = !con(M.Ins, (ins VT.RC:$data));
  string AsmString = directive#"\t $addr, $data;";
  list<dag> Pattern = [(set (StoreAddrspacePat<AS> VT.VT:$data, M.Pattern:$addr))];
}

class pISALoad<string directive, AddressSpace AS, pisaVT VT, MemProperty M> : _pISAInst {
  dag OutOperandList = (outs VT.RC:$dst);
  dag InOperandList = M.Ins;
  string AsmString = directive#"\t $dst, $addr;";
  list<dag> Pattern = [(set VT.VT:$dst, (LoadAddrspacePat<AS> M.Pattern:$addr))];
}

// The definitions of $dst, $src0 and $src1 are defined in the pISA SPEC
// https://github.com/intel-sandbox/drivers.gpu.compiler.pisa/blob/main/docs/source/instructions_memory.rst#iatom
class pISABinaryAtom<string directive, AddressSpace AS, pisaVT VT, MemProperty M, PatFrag node> : _pISAInst {
  dag OutOperandList = (outs VT.RC:$dst);
  dag InOperandList = !con(M.Ins, (ins VT.RC:$src0));
  string AsmString = directive#"\t $dst, $addr, $src0;";
  list<dag> Pattern = [(set VT.VT:$dst, (AtomAddrspacePat<AS, node> M.Pattern:$addr, VT.VT:$src0))];
}

class pISATernaryAtom<string directive, AddressSpace AS, pisaVT VT, MemProperty M, PatFrag node> : _pISAInst {
  dag OutOperandList = (outs VT.RC:$dst);
  dag InOperandList = !con(M.Ins, (ins VT.RC:$src0, VT.RC:$src1));
  string AsmString = directive#"\t $dst, $addr, $src0, $src1;";
  list<dag> Pattern = [(set VT.VT:$dst, (TernaryAtomAddrspacePat<AS, node> M.Pattern:$addr, VT.VT:$src0, VT.VT:$src1))];
}

class pISAFAtomCas<string directive, AddressSpace AS, pisaVT VT, MemProperty M> : _pISAInst {
  dag OutOperandList = (outs VT.RC:$dst);
  dag InOperandList = !con(M.Ins, (ins VT.RC:$src0, VT.RC:$src1));
  string AsmString = directive#"\t $dst, $addr, $src0, $src1;";
  list<dag> Pattern = !if(!eq(AS, global), [(set VT.VT:$dst,
                                            (FAtomCasGlobal<> M.Pattern:$addr, VT.VT:$src0, VT.VT:$src1))],
                          !if(!eq(AS, shared), [(set VT.VT:$dst,
                                               (FAtomCasShared<> M.Pattern:$addr, VT.VT:$src0, VT.VT:$src1))],
                          !if(!eq(AS, generic), [(set VT.VT:$dst,
                                                 (FAtomCasGeneric<> M.Pattern:$addr, VT.VT:$src0, VT.VT:$src1))],
                                                 [])));
}

// Remap to pISA[MemOp]_xx for readability, where x represents either r(Reg), or i(Imm) for the corresponding operand
class pISALoad_r        <string d, AddressSpace AS, pisaVT VT>               : pISALoad       <d, AS, VT, pISAMem_r<AS>>;
class pISAStore_r       <string d, AddressSpace AS, pisaVT VT>               : pISAStore      <d, AS, VT, pISAMem_r<AS>>;
class pISABinaryAtom_r  <string d, AddressSpace AS, pisaVT VT, PatFrag node> : pISABinaryAtom <d, AS, VT, pISAMem_r<AS>, node>;
class pISATernaryAtom_r <string d, AddressSpace AS, pisaVT VT, PatFrag node> : pISATernaryAtom<d, AS, VT, pISAMem_r<AS>, node>;
class pISAFAtomCas_r    <string d, AddressSpace AS, pisaVT VT>               : pISAFAtomCas   <d, AS, VT, pISAMem_r<AS>>;

class pISALoad_rr       <string d, AddressSpace AS, pisaVT VT>               : pISALoad       <d, AS, VT, pISAMem_rr<AS>>;
class pISAStore_rr      <string d, AddressSpace AS, pisaVT VT>               : pISAStore      <d, AS, VT, pISAMem_rr<AS>>;
class pISABinaryAtom_rr <string d, AddressSpace AS, pisaVT VT, PatFrag node> : pISABinaryAtom <d, AS, VT, pISAMem_rr<AS>, node>;
class pISATernaryAtom_rr<string d, AddressSpace AS, pisaVT VT, PatFrag node> : pISATernaryAtom<d, AS, VT, pISAMem_rr<AS>, node>;
class pISAFAtomCas_rr   <string d, AddressSpace AS, pisaVT VT>               : pISAFAtomCas   <d, AS, VT, pISAMem_rr<AS>>;

class pISALoad_ri       <string d, AddressSpace AS, pisaVT VT>               : pISALoad       <d, AS, VT, pISAMem_ri<AS>>;
class pISAStore_ri      <string d, AddressSpace AS, pisaVT VT>               : pISAStore      <d, AS, VT, pISAMem_ri<AS>>;
class pISABinaryAtom_ri <string d, AddressSpace AS, pisaVT VT, PatFrag node> : pISABinaryAtom <d, AS, VT, pISAMem_ri<AS>, node>;
class pISATernaryAtom_ri<string d, AddressSpace AS, pisaVT VT, PatFrag node> : pISATernaryAtom<d, AS, VT, pISAMem_ri<AS>, node>;
class pISAFAtomCas_ri   <string d, AddressSpace AS, pisaVT VT>               : pISAFAtomCas   <d, AS, VT, pISAMem_ri<AS>>;

/**************************** End of pISAMemInst class hierachy ****************************/

