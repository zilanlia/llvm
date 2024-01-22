//===-- pISAInstrInfoRaytracing.td - Target Description for pISA Target ---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the pISA instructions for raytracing in TableGen format.
//
//===----------------------------------------------------------------------===//

// 10.1 Raytracing

defm : SpecialRegPat<int_pisa_rt_async_stackid, SpecialReg_RTAsyncStackId>;
defm : SpecialRegPat<int_pisa_rt_shader_type,   SpecialReg_RTShaderType>;

def rt_trace_async : pISAInst_rr<"rt.trace_async", _void, _p2, _i32> {
  let Pattern = [(int_pisa_rt_trace_async src0RC:$src0, src1RC:$src1)];
}

def rt_trace_sync : pISAInst_rr<"rt.trace_sync", _i32, _p2, _i32> {
  let Pattern = [(set dstRC:$dst, (int_pisa_rt_trace_sync src0RC:$src0, src1RC:$src1))];
}

def rt_sync : pISAInst_r<"rt.sync", _void, _i32> {
  let Pattern = [(int_pisa_rt_sync srcRC:$src)];
}

def rt_check   : SimpleOp<"rt.check;",   [(int_pisa_rt_check)]>;
def rt_release : SimpleOp<"rt.release;", [(int_pisa_rt_release)]>;

def btd : pISAInst_rrr<"btd", _void, _p2, _i16, _i64> {
  let Pattern = [(int_pisa_btd src0RC:$src0, src1RC:$src1, src2RC:$src2)];
}

def stackid_release : pISAInst_r<"stackid_release", _void, _i16> {
  let Pattern = [(int_pisa_stackid_release srcRC:$src)];
}

