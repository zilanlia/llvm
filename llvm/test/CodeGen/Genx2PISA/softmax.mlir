#loc = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0)
#loc1 = loc(unknown)
module {
  tt.func public @softmax_kernel_0d1d234(%arg0: !tt.ptr<f32, 1> {tt.divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0), %arg1: !tt.ptr<f32, 1> {tt.divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0), %arg2: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0), %arg3: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0), %arg4: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0)) attributes {noinline = false} {
    %cst = arith.constant dense<0xFF800000> : tensor<1024xf32> loc(#loc1)
    %0 = tt.get_program_id x : i32 loc(#loc2)
    %1 = arith.muli %0, %arg2 : i32 loc(#loc3)
    %2 = tt.addptr %arg1, %1 : !tt.ptr<f32, 1>, i32 loc(#loc4)
    %3 = tt.make_range {end = 1024 : i32, start = 0 : i32} : tensor<1024xi32> loc(#loc5)
    %4 = tt.splat %2 : (!tt.ptr<f32, 1>) -> tensor<1024x!tt.ptr<f32, 1>> loc(#loc6)
    %5 = tt.addptr %4, %3 : tensor<1024x!tt.ptr<f32, 1>>, tensor<1024xi32> loc(#loc6)
    %6 = tt.splat %arg4 : (i32) -> tensor<1024xi32> loc(#loc7)
    %7 = arith.cmpi slt, %3, %6 : tensor<1024xi32> loc(#loc7)
    %8 = tt.load %5, %7, %cst {cache = 1 : i32, evict = 1 : i32, isVolatile = false} : tensor<1024xf32> loc(#loc8)
    %9 = "tt.reduce"(%8) <{axis = 0 : i32}> ({
    ^bb0(%arg5: f32 loc(unknown), %arg6: f32 loc(unknown)):
      %20 = arith.maxnumf %arg5, %arg6 : f32 loc(#loc27)
      tt.reduce.return %20 : f32 loc(#loc23)
    }) : (tensor<1024xf32>) -> f32 loc(#loc23)
    %10 = tt.splat %9 : (f32) -> tensor<1024xf32> loc(#loc12)
    %11 = arith.subf %8, %10 : tensor<1024xf32> loc(#loc12)
    %12 = math.exp %11 : tensor<1024xf32> loc(#loc13)
    %13 = "tt.reduce"(%12) <{axis = 0 : i32}> ({
    ^bb0(%arg5: f32 loc(unknown), %arg6: f32 loc(unknown)):
      %20 = arith.addf %arg5, %arg6 : f32 loc(#loc28)
      tt.reduce.return %20 : f32 loc(#loc25)
    }) : (tensor<1024xf32>) -> f32 loc(#loc25)
    %14 = tt.splat %13 : (f32) -> tensor<1024xf32> loc(#loc17)
    %15 = arith.divf %12, %14 : tensor<1024xf32> loc(#loc17)
    %16 = arith.muli %0, %arg3 : i32 loc(#loc18)
    %17 = tt.addptr %arg0, %16 : !tt.ptr<f32, 1>, i32 loc(#loc19)
    %18 = tt.splat %17 : (!tt.ptr<f32, 1>) -> tensor<1024x!tt.ptr<f32, 1>> loc(#loc20)
    %19 = tt.addptr %18, %3 : tensor<1024x!tt.ptr<f32, 1>>, tensor<1024xi32> loc(#loc20)
    tt.store %19, %15, %7 {cache = 1 : i32, evict = 1 : i32} : tensor<1024xf32> loc(#loc21)
    tt.return loc(#loc22)
  } loc(#loc)
} loc(#loc)
#loc2 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":78:28)
#loc3 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":80:42)
#loc4 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":80:32)
#loc5 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":83:31)
#loc6 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":84:33)
#loc7 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":86:49)
#loc8 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":86:18)
#loc9 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/triton/language/standard.py":177:40)
#loc10 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":88:33)
#loc11 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/triton/language/standard.py":132:26)
#loc12 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":88:26)
#loc13 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":90:23)
#loc14 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/triton/language/standard.py":251:36)
#loc15 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":91:25)
#loc16 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/triton/language/standard.py":241:15)
#loc17 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":92:33)
#loc18 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":94:50)
#loc19 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":94:40)
#loc20 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":95:41)
#loc21 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":96:26)
#loc22 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":96:4)
#loc23 = loc(callsite(#loc9 at #loc10))
#loc24 = loc(callsite(#loc11 at #loc9))
#loc25 = loc(callsite(#loc14 at #loc15))
#loc26 = loc(callsite(#loc16 at #loc14))
#loc27 = loc(callsite(#loc24 at #loc10))
#loc28 = loc(callsite(#loc26 at #loc15))

------------------
#blocked = #triton_gpu.blocked<{sizePerThread = [1], threadsPerWarp = [32], warpsPerCTA = [4], order = [0], CTAsPerCGA = [1], CTASplitNum = [1], CTAOrder = [0]}>
#loc = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0)
#loc1 = loc(unknown)
module attributes {"triton_gpu.compute-capability" = 0 : i32, "triton_gpu.num-ctas" = 1 : i32, "triton_gpu.num-warps" = 4 : i32, "triton_gpu.threads-per-warp" = 32 : i32} {
  tt.func public @softmax_kernel_0d1d234(%arg0: !tt.ptr<f32, 1> {tt.divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0), %arg1: !tt.ptr<f32, 1> {tt.divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0), %arg2: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0), %arg3: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0), %arg4: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0)) attributes {noinline = false} {
    %cst = arith.constant dense<0xFF800000> : tensor<1024xf32, #blocked> loc(#loc1)
    %0 = tt.get_program_id x : i32 loc(#loc2)
    %1 = arith.muli %0, %arg2 : i32 loc(#loc3)
    %2 = tt.addptr %arg1, %1 : !tt.ptr<f32, 1>, i32 loc(#loc4)
    %3 = tt.make_range {end = 1024 : i32, start = 0 : i32} : tensor<1024xi32, #blocked> loc(#loc5)
    %4 = tt.splat %2 : (!tt.ptr<f32, 1>) -> tensor<1024x!tt.ptr<f32, 1>, #blocked> loc(#loc6)
    %5 = tt.addptr %4, %3 : tensor<1024x!tt.ptr<f32, 1>, #blocked>, tensor<1024xi32, #blocked> loc(#loc6)
    %6 = tt.splat %arg4 : (i32) -> tensor<1024xi32, #blocked> loc(#loc7)
    %7 = arith.cmpi slt, %3, %6 : tensor<1024xi32, #blocked> loc(#loc7)
    %8 = tt.load %5, %7, %cst {cache = 1 : i32, evict = 1 : i32, isVolatile = false} : tensor<1024xf32, #blocked> loc(#loc8)
    %9 = "tt.reduce"(%8) <{axis = 0 : i32}> ({
    ^bb0(%arg5: f32 loc(unknown), %arg6: f32 loc(unknown)):
      %20 = arith.maxnumf %arg5, %arg6 : f32 loc(#loc27)
      tt.reduce.return %20 : f32 loc(#loc23)
    }) : (tensor<1024xf32, #blocked>) -> f32 loc(#loc23)
    %10 = tt.splat %9 : (f32) -> tensor<1024xf32, #blocked> loc(#loc12)
    %11 = arith.subf %8, %10 : tensor<1024xf32, #blocked> loc(#loc12)
    %12 = math.exp %11 : tensor<1024xf32, #blocked> loc(#loc13)
    %13 = "tt.reduce"(%12) <{axis = 0 : i32}> ({
    ^bb0(%arg5: f32 loc(unknown), %arg6: f32 loc(unknown)):
      %20 = arith.addf %arg5, %arg6 : f32 loc(#loc28)
      tt.reduce.return %20 : f32 loc(#loc25)
    }) : (tensor<1024xf32, #blocked>) -> f32 loc(#loc25)
    %14 = tt.splat %13 : (f32) -> tensor<1024xf32, #blocked> loc(#loc17)
    %15 = arith.divf %12, %14 : tensor<1024xf32, #blocked> loc(#loc17)
    %16 = arith.muli %0, %arg3 : i32 loc(#loc18)
    %17 = tt.addptr %arg0, %16 : !tt.ptr<f32, 1>, i32 loc(#loc19)
    %18 = tt.splat %17 : (!tt.ptr<f32, 1>) -> tensor<1024x!tt.ptr<f32, 1>, #blocked> loc(#loc20)
    %19 = tt.addptr %18, %3 : tensor<1024x!tt.ptr<f32, 1>, #blocked>, tensor<1024xi32, #blocked> loc(#loc20)
    tt.store %19, %15, %7 {cache = 1 : i32, evict = 1 : i32} : tensor<1024xf32, #blocked> loc(#loc21)
    tt.return loc(#loc22)
  } loc(#loc)
} loc(#loc)
#loc2 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":78:28)
#loc3 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":80:42)
#loc4 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":80:32)
#loc5 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":83:31)
#loc6 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":84:33)
#loc7 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":86:49)
#loc8 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":86:18)
#loc9 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/triton/language/standard.py":177:40)
#loc10 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":88:33)
#loc11 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/triton/language/standard.py":132:26)
#loc12 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":88:26)
#loc13 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":90:23)
#loc14 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/triton/language/standard.py":251:36)
#loc15 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":91:25)
#loc16 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/triton/language/standard.py":241:15)
#loc17 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":92:33)
#loc18 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":94:50)
#loc19 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":94:40)
#loc20 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":95:41)
#loc21 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":96:26)
#loc22 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":96:4)
#loc23 = loc(callsite(#loc9 at #loc10))
#loc24 = loc(callsite(#loc11 at #loc9))
#loc25 = loc(callsite(#loc14 at #loc15))
#loc26 = loc(callsite(#loc16 at #loc14))
#loc27 = loc(callsite(#loc24 at #loc10))
#loc28 = loc(callsite(#loc26 at #loc15))

---------------LLVM IR----------------
#loc = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0)
#loc8 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":86:18)
#loc10 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/triton/language/standard.py":177:40)
#loc11 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":88:33)
#loc14 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/triton/language/standard.py":251:36)
#loc15 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":91:25)
#loc24 = loc(callsite(#loc10 at #loc11))
#loc25 = loc(callsite(#loc14 at #loc15))
module attributes {"triton_gpu.compute-capability" = 0 : i32, "triton_gpu.num-ctas" = 1 : i32, "triton_gpu.num-warps" = 4 : i32, triton_gpu.shared = 16 : i32, "triton_gpu.threads-per-warp" = 32 : i32} {
  llvm.func @softmax_kernel_0d1d234(%arg0: !llvm.ptr<1> {tt.divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0), %arg1: !llvm.ptr<1> {tt.divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0), %arg2: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0), %arg3: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0), %arg4: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0), %arg5: !llvm.ptr<3> loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":76:0)) attributes {genx.intel_reqd_sub_group_size = [32 : i32], genx.kernel = 1 : i32, genx.max_work_group_size = [128 : i32, 1 : i32, 1 : i32], noinline = false, sym_visibility = "public", "triton_gpu.num-tma-load" = 0 : i32, "triton_gpu.num-tma-store" = 0 : i32} {
    %0 = llvm.mlir.constant(true) : i1 loc(#loc1)
    %1 = llvm.mlir.constant(1.44269502 : f32) : f32 loc(#loc1)
    %2 = llvm.mlir.constant(2 : i32) : i32 loc(#loc1)
    %3 = llvm.mlir.constant(8 : i32) : i32 loc(#loc1)
    %4 = llvm.mlir.constant(16 : i32) : i32 loc(#loc1)
    %5 = llvm.mlir.constant(0 : index) : i32 loc(#loc1)
    %6 = llvm.mlir.constant(0xFF800000 : f32) : f32 loc(#loc1)
    %7 = llvm.mlir.constant(896 : i32) : i32 loc(#loc1)
    %8 = llvm.mlir.constant(768 : i32) : i32 loc(#loc1)
    %9 = llvm.mlir.constant(640 : i32) : i32 loc(#loc1)
    %10 = llvm.mlir.constant(512 : i32) : i32 loc(#loc1)
    %11 = llvm.mlir.constant(384 : i32) : i32 loc(#loc1)
    %12 = llvm.mlir.constant(256 : i32) : i32 loc(#loc1)
    %13 = llvm.mlir.constant(128 : i32) : i32 loc(#loc1)
    %14 = llvm.mlir.constant(0 : i32) : i32 loc(#loc1)
    %15 = llvm.mlir.constant(1 : i32) : i32 loc(#loc1)
    %16 = llvm.mlir.constant(1024 : i32) : i32 loc(#loc1)
    %17 = llvm.mlir.constant(4 : i32) : i32 loc(#loc1)
    %18 = llvm.mlir.constant(32 : i32) : i32 loc(#loc1)
    %19 = genx.workitem.id.x : i32 loc(#loc2)
    %20 = llvm.urem %19, %18  : i32 loc(#loc2)
    %21 = llvm.udiv %19, %18  : i32 loc(#loc2)
    %22 = llvm.urem %21, %17  : i32 loc(#loc2)
    %23 = llvm.urem %20, %18  : i32 loc(#loc2)
    %24 = llvm.urem %22, %18  : i32 loc(#loc2)
    %25 = llvm.urem %23, %16  : i32 loc(#loc2)
    %26 = llvm.mul %24, %18  : i32 loc(#loc2)
    %27 = llvm.add %25, %26  : i32 loc(#loc2)
    %28 = llvm.mul %27, %15  : i32 loc(#loc2)
    %29 = llvm.urem %14, %15  : i32 loc(#loc2)
    %30 = llvm.mul %29, %16  : i32 loc(#loc2)
    %31 = llvm.add %28, %30  : i32 loc(#loc2)
    %32 = llvm.add %31, %14  : i32 loc(#loc2)
    %33 = llvm.add %31, %13  : i32 loc(#loc2)
    %34 = llvm.add %31, %12  : i32 loc(#loc2)
    %35 = llvm.add %31, %11  : i32 loc(#loc2)
    %36 = llvm.add %31, %10  : i32 loc(#loc2)
    %37 = llvm.add %31, %9  : i32 loc(#loc2)
    %38 = llvm.add %31, %8  : i32 loc(#loc2)
    %39 = llvm.add %31, %7  : i32 loc(#loc2)
    %40 = genx.workgroup.id.x : i32 loc(#loc3)
    %41 = llvm.mul %40, %arg2  : i32 loc(#loc4)
    %42 = llvm.getelementptr %arg1[%41] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc5)
    %43 = llvm.add %32, %5  : i32 loc(#loc2)
    %44 = llvm.add %33, %5  : i32 loc(#loc2)
    %45 = llvm.add %34, %5  : i32 loc(#loc2)
    %46 = llvm.add %35, %5  : i32 loc(#loc2)
    %47 = llvm.add %36, %5  : i32 loc(#loc2)
    %48 = llvm.add %37, %5  : i32 loc(#loc2)
    %49 = llvm.add %38, %5  : i32 loc(#loc2)
    %50 = llvm.add %39, %5  : i32 loc(#loc2)
    %51 = llvm.getelementptr %42[%43] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc6)
    %52 = llvm.getelementptr %42[%44] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc6)
    %53 = llvm.getelementptr %42[%45] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc6)
    %54 = llvm.getelementptr %42[%46] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc6)
    %55 = llvm.getelementptr %42[%47] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc6)
    %56 = llvm.getelementptr %42[%48] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc6)
    %57 = llvm.getelementptr %42[%49] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc6)
    %58 = llvm.getelementptr %42[%50] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc6)
    %59 = llvm.icmp "slt" %43, %arg4 : i32 loc(#loc7)
    %60 = llvm.icmp "slt" %44, %arg4 : i32 loc(#loc7)
    %61 = llvm.icmp "slt" %45, %arg4 : i32 loc(#loc7)
    %62 = llvm.icmp "slt" %46, %arg4 : i32 loc(#loc7)
    %63 = llvm.icmp "slt" %47, %arg4 : i32 loc(#loc7)
    %64 = llvm.icmp "slt" %48, %arg4 : i32 loc(#loc7)
    %65 = llvm.icmp "slt" %49, %arg4 : i32 loc(#loc7)
    %66 = llvm.icmp "slt" %50, %arg4 : i32 loc(#loc7)
    %67 = llvm.mlir.undef : vector<1xf32> loc(#loc8)
    %68 = llvm.insertelement %6, %67[%5 : i32] : vector<1xf32> loc(#loc8)
    %69 = llvm.bitcast %68 : vector<1xf32> to i32 loc(#loc8)
    llvm.cond_br %59, ^bb1, ^bb2(%69 : i32) loc(#loc8)
  ^bb1:  // pred: ^bb0
    %70 = llvm.load %51 : !llvm.ptr<1> -> i32 loc(#loc8)
    llvm.br ^bb2(%70 : i32) loc(#loc8)
  ^bb2(%71: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":86:18)):  // 2 preds: ^bb0, ^bb1
    %72 = llvm.bitcast %71 : i32 to vector<1xf32> loc(#loc8)
    %73 = llvm.extractelement %72[%14 : i32] : vector<1xf32> loc(#loc8)
    llvm.cond_br %60, ^bb3, ^bb4(%69 : i32) loc(#loc8)
  ^bb3:  // pred: ^bb2
    %74 = llvm.load %52 : !llvm.ptr<1> -> i32 loc(#loc8)
    llvm.br ^bb4(%74 : i32) loc(#loc8)
  ^bb4(%75: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":86:18)):  // 2 preds: ^bb2, ^bb3
    %76 = llvm.bitcast %75 : i32 to vector<1xf32> loc(#loc8)
    %77 = llvm.extractelement %76[%14 : i32] : vector<1xf32> loc(#loc8)
    llvm.cond_br %61, ^bb5, ^bb6(%69 : i32) loc(#loc8)
  ^bb5:  // pred: ^bb4
    %78 = llvm.load %53 : !llvm.ptr<1> -> i32 loc(#loc8)
    llvm.br ^bb6(%78 : i32) loc(#loc8)
  ^bb6(%79: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":86:18)):  // 2 preds: ^bb4, ^bb5
    %80 = llvm.bitcast %79 : i32 to vector<1xf32> loc(#loc8)
    %81 = llvm.extractelement %80[%14 : i32] : vector<1xf32> loc(#loc8)
    llvm.cond_br %62, ^bb7, ^bb8(%69 : i32) loc(#loc8)
  ^bb7:  // pred: ^bb6
    %82 = llvm.load %54 : !llvm.ptr<1> -> i32 loc(#loc8)
    llvm.br ^bb8(%82 : i32) loc(#loc8)
  ^bb8(%83: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":86:18)):  // 2 preds: ^bb6, ^bb7
    %84 = llvm.bitcast %83 : i32 to vector<1xf32> loc(#loc8)
    %85 = llvm.extractelement %84[%14 : i32] : vector<1xf32> loc(#loc8)
    llvm.cond_br %63, ^bb9, ^bb10(%69 : i32) loc(#loc8)
  ^bb9:  // pred: ^bb8
    %86 = llvm.load %55 : !llvm.ptr<1> -> i32 loc(#loc8)
    llvm.br ^bb10(%86 : i32) loc(#loc8)
  ^bb10(%87: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":86:18)):  // 2 preds: ^bb8, ^bb9
    %88 = llvm.bitcast %87 : i32 to vector<1xf32> loc(#loc8)
    %89 = llvm.extractelement %88[%14 : i32] : vector<1xf32> loc(#loc8)
    llvm.cond_br %64, ^bb11, ^bb12(%69 : i32) loc(#loc8)
  ^bb11:  // pred: ^bb10
    %90 = llvm.load %56 : !llvm.ptr<1> -> i32 loc(#loc8)
    llvm.br ^bb12(%90 : i32) loc(#loc8)
  ^bb12(%91: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":86:18)):  // 2 preds: ^bb10, ^bb11
    %92 = llvm.bitcast %91 : i32 to vector<1xf32> loc(#loc8)
    %93 = llvm.extractelement %92[%14 : i32] : vector<1xf32> loc(#loc8)
    llvm.cond_br %65, ^bb13, ^bb14(%69 : i32) loc(#loc8)
  ^bb13:  // pred: ^bb12
    %94 = llvm.load %57 : !llvm.ptr<1> -> i32 loc(#loc8)
    llvm.br ^bb14(%94 : i32) loc(#loc8)
  ^bb14(%95: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":86:18)):  // 2 preds: ^bb12, ^bb13
    %96 = llvm.bitcast %95 : i32 to vector<1xf32> loc(#loc8)
    %97 = llvm.extractelement %96[%14 : i32] : vector<1xf32> loc(#loc8)
    llvm.cond_br %66, ^bb15, ^bb16(%69 : i32) loc(#loc8)
  ^bb15:  // pred: ^bb14
    %98 = llvm.load %58 : !llvm.ptr<1> -> i32 loc(#loc8)
    llvm.br ^bb16(%98 : i32) loc(#loc8)
  ^bb16(%99: i32 loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":86:18)):  // 2 preds: ^bb14, ^bb15
    %100 = llvm.bitcast %99 : i32 to vector<1xf32> loc(#loc8)
    %101 = llvm.extractelement %100[%14 : i32] : vector<1xf32> loc(#loc8)
    %102 = llvm.intr.maxnum(%73, %77)  : (f32, f32) -> f32 loc(#loc27)
    %103 = llvm.intr.maxnum(%102, %81)  : (f32, f32) -> f32 loc(#loc27)
    %104 = llvm.intr.maxnum(%103, %85)  : (f32, f32) -> f32 loc(#loc27)
    %105 = llvm.intr.maxnum(%104, %89)  : (f32, f32) -> f32 loc(#loc27)
    %106 = llvm.intr.maxnum(%105, %93)  : (f32, f32) -> f32 loc(#loc27)
    %107 = llvm.intr.maxnum(%106, %97)  : (f32, f32) -> f32 loc(#loc27)
    %108 = llvm.intr.maxnum(%107, %101)  : (f32, f32) -> f32 loc(#loc27)
    %109 = genx.sub_group_shuffle  XOR %108, %4 : f32 -> f32 loc(#loc24)
    %110 = llvm.intr.maxnum(%108, %109)  : (f32, f32) -> f32 loc(#loc27)
    %111 = genx.sub_group_shuffle  XOR %110, %3 : f32 -> f32 loc(#loc24)
    %112 = llvm.intr.maxnum(%110, %111)  : (f32, f32) -> f32 loc(#loc27)
    %113 = genx.sub_group_shuffle  XOR %112, %17 : f32 -> f32 loc(#loc24)
    %114 = llvm.intr.maxnum(%112, %113)  : (f32, f32) -> f32 loc(#loc27)
    %115 = genx.sub_group_shuffle  XOR %114, %2 : f32 -> f32 loc(#loc24)
    %116 = llvm.intr.maxnum(%114, %115)  : (f32, f32) -> f32 loc(#loc27)
    %117 = genx.sub_group_shuffle  XOR %116, %15 : f32 -> f32 loc(#loc24)
    %118 = llvm.intr.maxnum(%116, %117)  : (f32, f32) -> f32 loc(#loc27)
    %119 = llvm.icmp "eq" %23, %14 : i32 loc(#loc24)
    %120 = llvm.getelementptr %arg5[%22] : (!llvm.ptr<3>, i32) -> !llvm.ptr<3>, f32 loc(#loc24)
    llvm.cond_br %119, ^bb17, ^bb18 loc(#loc24)
  ^bb17:  // pred: ^bb16
    llvm.store %118, %120 : f32, !llvm.ptr<3> loc(#loc24)
    llvm.br ^bb18 loc(#loc24)
  ^bb18:  // 2 preds: ^bb16, ^bb17
    genx.barrier loc(#loc24)
    %121 = llvm.icmp "slt" %19, %17 : i32 loc(#loc24)
    %122 = llvm.getelementptr %arg5[%19] : (!llvm.ptr<3>, i32) -> !llvm.ptr<3>, f32 loc(#loc24)
    %123 = llvm.mlir.undef : f32 loc(#loc24)
    llvm.cond_br %121, ^bb19, ^bb20(%123 : f32) loc(#loc24)
  ^bb19:  // pred: ^bb18
    %124 = llvm.load %122 : !llvm.ptr<3> -> f32 loc(#loc24)
    llvm.br ^bb20(%124 : f32) loc(#loc24)
  ^bb20(%125: f32 loc(callsite(#loc10 at #loc11))):  // 2 preds: ^bb18, ^bb19
    %126 = genx.sub_group_shuffle  XOR %125, %2 : f32 -> f32 loc(#loc24)
    %127 = llvm.intr.maxnum(%125, %126)  : (f32, f32) -> f32 loc(#loc27)
    %128 = genx.sub_group_shuffle  XOR %127, %15 : f32 -> f32 loc(#loc24)
    %129 = llvm.intr.maxnum(%127, %128)  : (f32, f32) -> f32 loc(#loc27)
    %130 = llvm.urem %20, %17  : i32 loc(#loc24)
    %131 = llvm.icmp "eq" %130, %14 : i32 loc(#loc24)
    %132 = llvm.and %121, %131  : i1 loc(#loc24)
    llvm.cond_br %132, ^bb21, ^bb22 loc(#loc24)
  ^bb21:  // pred: ^bb20
    llvm.store %129, %122 : f32, !llvm.ptr<3> loc(#loc24)
    llvm.br ^bb22 loc(#loc24)
  ^bb22:  // 2 preds: ^bb20, ^bb21
    genx.barrier loc(#loc24)
    %133 = llvm.load %arg5 : !llvm.ptr<3> -> f32 loc(#loc24)
    %134 = llvm.fsub %73, %133  : f32 loc(#loc12)
    %135 = llvm.fsub %77, %133  : f32 loc(#loc12)
    %136 = llvm.fsub %81, %133  : f32 loc(#loc12)
    %137 = llvm.fsub %85, %133  : f32 loc(#loc12)
    %138 = llvm.fsub %89, %133  : f32 loc(#loc12)
    %139 = llvm.fsub %93, %133  : f32 loc(#loc12)
    %140 = llvm.fsub %97, %133  : f32 loc(#loc12)
    %141 = llvm.fsub %101, %133  : f32 loc(#loc12)
    %142 = llvm.fmul %134, %1  : f32 loc(#loc13)
    %143 = llvm.intr.exp2(%142)  : (f32) -> f32 loc(#loc13)
    %144 = llvm.fmul %135, %1  : f32 loc(#loc13)
    %145 = llvm.intr.exp2(%144)  : (f32) -> f32 loc(#loc13)
    %146 = llvm.fmul %136, %1  : f32 loc(#loc13)
    %147 = llvm.intr.exp2(%146)  : (f32) -> f32 loc(#loc13)
    %148 = llvm.fmul %137, %1  : f32 loc(#loc13)
    %149 = llvm.intr.exp2(%148)  : (f32) -> f32 loc(#loc13)
    %150 = llvm.fmul %138, %1  : f32 loc(#loc13)
    %151 = llvm.intr.exp2(%150)  : (f32) -> f32 loc(#loc13)
    %152 = llvm.fmul %139, %1  : f32 loc(#loc13)
    %153 = llvm.intr.exp2(%152)  : (f32) -> f32 loc(#loc13)
    %154 = llvm.fmul %140, %1  : f32 loc(#loc13)
    %155 = llvm.intr.exp2(%154)  : (f32) -> f32 loc(#loc13)
    %156 = llvm.fmul %141, %1  : f32 loc(#loc13)
    %157 = llvm.intr.exp2(%156)  : (f32) -> f32 loc(#loc13)
    genx.barrier loc(#loc25)
    %158 = llvm.fadd %143, %145  : f32 loc(#loc28)
    %159 = llvm.fadd %158, %147  : f32 loc(#loc28)
    %160 = llvm.fadd %159, %149  : f32 loc(#loc28)
    %161 = llvm.fadd %160, %151  : f32 loc(#loc28)
    %162 = llvm.fadd %161, %153  : f32 loc(#loc28)
    %163 = llvm.fadd %162, %155  : f32 loc(#loc28)
    %164 = llvm.fadd %163, %157  : f32 loc(#loc28)
    %165 = genx.sub_group_shuffle  XOR %164, %4 : f32 -> f32 loc(#loc25)
    %166 = llvm.fadd %164, %165  : f32 loc(#loc28)
    %167 = genx.sub_group_shuffle  XOR %166, %3 : f32 -> f32 loc(#loc25)
    %168 = llvm.fadd %166, %167  : f32 loc(#loc28)
    %169 = genx.sub_group_shuffle  XOR %168, %17 : f32 -> f32 loc(#loc25)
    %170 = llvm.fadd %168, %169  : f32 loc(#loc28)
    %171 = genx.sub_group_shuffle  XOR %170, %2 : f32 -> f32 loc(#loc25)
    %172 = llvm.fadd %170, %171  : f32 loc(#loc28)
    %173 = genx.sub_group_shuffle  XOR %172, %15 : f32 -> f32 loc(#loc25)
    %174 = llvm.fadd %172, %173  : f32 loc(#loc28)
    llvm.cond_br %119, ^bb23, ^bb24 loc(#loc25)
  ^bb23:  // pred: ^bb22
    llvm.store %174, %120 : f32, !llvm.ptr<3> loc(#loc25)
    llvm.br ^bb24 loc(#loc25)
  ^bb24:  // 2 preds: ^bb22, ^bb23
    genx.barrier loc(#loc25)
    llvm.cond_br %121, ^bb25, ^bb26(%123 : f32) loc(#loc25)
  ^bb25:  // pred: ^bb24
    %175 = llvm.load %122 : !llvm.ptr<3> -> f32 loc(#loc25)
    llvm.br ^bb26(%175 : f32) loc(#loc25)
  ^bb26(%176: f32 loc(callsite(#loc14 at #loc15))):  // 2 preds: ^bb24, ^bb25
    %177 = genx.sub_group_shuffle  XOR %176, %2 : f32 -> f32 loc(#loc25)
    %178 = llvm.fadd %176, %177  : f32 loc(#loc28)
    %179 = genx.sub_group_shuffle  XOR %178, %15 : f32 -> f32 loc(#loc25)
    %180 = llvm.fadd %178, %179  : f32 loc(#loc28)
    llvm.cond_br %132, ^bb27, ^bb28 loc(#loc25)
  ^bb27:  // pred: ^bb26
    llvm.store %180, %122 : f32, !llvm.ptr<3> loc(#loc25)
    llvm.br ^bb28 loc(#loc25)
  ^bb28:  // 2 preds: ^bb26, ^bb27
    genx.barrier loc(#loc25)
    %181 = llvm.load %arg5 : !llvm.ptr<3> -> f32 loc(#loc25)
    %182 = llvm.fdiv %143, %181  : f32 loc(#loc17)
    %183 = llvm.fdiv %145, %181  : f32 loc(#loc17)
    %184 = llvm.fdiv %147, %181  : f32 loc(#loc17)
    %185 = llvm.fdiv %149, %181  : f32 loc(#loc17)
    %186 = llvm.fdiv %151, %181  : f32 loc(#loc17)
    %187 = llvm.fdiv %153, %181  : f32 loc(#loc17)
    %188 = llvm.fdiv %155, %181  : f32 loc(#loc17)
    %189 = llvm.fdiv %157, %181  : f32 loc(#loc17)
    %190 = llvm.mul %40, %arg3  : i32 loc(#loc18)
    %191 = llvm.getelementptr %arg0[%190] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc19)
    %192 = llvm.getelementptr %191[%43] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc20)
    %193 = llvm.getelementptr %191[%44] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc20)
    %194 = llvm.getelementptr %191[%45] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc20)
    %195 = llvm.getelementptr %191[%46] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc20)
    %196 = llvm.getelementptr %191[%47] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc20)
    %197 = llvm.getelementptr %191[%48] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc20)
    %198 = llvm.getelementptr %191[%49] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc20)
    %199 = llvm.getelementptr %191[%50] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc20)
    %200 = llvm.insertelement %182, %67[%14 : i32] : vector<1xf32> loc(#loc21)
    %201 = llvm.bitcast %200 : vector<1xf32> to i32 loc(#loc21)
    %202 = llvm.and %0, %59  : i1 loc(#loc21)
    %203 = llvm.mlir.undef : vector<1xi32> loc(#loc21)
    %204 = llvm.insertelement %201, %203[%14 : i32] : vector<1xi32> loc(#loc21)
    llvm.cond_br %202, ^bb29, ^bb30 loc(#loc21)
  ^bb29:  // pred: ^bb28
    llvm.store %204, %192 : vector<1xi32>, !llvm.ptr<1> loc(#loc21)
    llvm.br ^bb30 loc(#loc21)
  ^bb30:  // 2 preds: ^bb28, ^bb29
    %205 = llvm.insertelement %183, %67[%14 : i32] : vector<1xf32> loc(#loc21)
    %206 = llvm.bitcast %205 : vector<1xf32> to i32 loc(#loc21)
    %207 = llvm.and %0, %60  : i1 loc(#loc21)
    %208 = llvm.insertelement %206, %203[%14 : i32] : vector<1xi32> loc(#loc21)
    llvm.cond_br %207, ^bb31, ^bb32 loc(#loc21)
  ^bb31:  // pred: ^bb30
    llvm.store %208, %193 : vector<1xi32>, !llvm.ptr<1> loc(#loc21)
    llvm.br ^bb32 loc(#loc21)
  ^bb32:  // 2 preds: ^bb30, ^bb31
    %209 = llvm.insertelement %184, %67[%14 : i32] : vector<1xf32> loc(#loc21)
    %210 = llvm.bitcast %209 : vector<1xf32> to i32 loc(#loc21)
    %211 = llvm.and %0, %61  : i1 loc(#loc21)
    %212 = llvm.insertelement %210, %203[%14 : i32] : vector<1xi32> loc(#loc21)
    llvm.cond_br %211, ^bb33, ^bb34 loc(#loc21)
  ^bb33:  // pred: ^bb32
    llvm.store %212, %194 : vector<1xi32>, !llvm.ptr<1> loc(#loc21)
    llvm.br ^bb34 loc(#loc21)
  ^bb34:  // 2 preds: ^bb32, ^bb33
    %213 = llvm.insertelement %185, %67[%14 : i32] : vector<1xf32> loc(#loc21)
    %214 = llvm.bitcast %213 : vector<1xf32> to i32 loc(#loc21)
    %215 = llvm.and %0, %62  : i1 loc(#loc21)
    %216 = llvm.insertelement %214, %203[%14 : i32] : vector<1xi32> loc(#loc21)
    llvm.cond_br %215, ^bb35, ^bb36 loc(#loc21)
  ^bb35:  // pred: ^bb34
    llvm.store %216, %195 : vector<1xi32>, !llvm.ptr<1> loc(#loc21)
    llvm.br ^bb36 loc(#loc21)
  ^bb36:  // 2 preds: ^bb34, ^bb35
    %217 = llvm.insertelement %186, %67[%14 : i32] : vector<1xf32> loc(#loc21)
    %218 = llvm.bitcast %217 : vector<1xf32> to i32 loc(#loc21)
    %219 = llvm.and %0, %63  : i1 loc(#loc21)
    %220 = llvm.insertelement %218, %203[%14 : i32] : vector<1xi32> loc(#loc21)
    llvm.cond_br %219, ^bb37, ^bb38 loc(#loc21)
  ^bb37:  // pred: ^bb36
    llvm.store %220, %196 : vector<1xi32>, !llvm.ptr<1> loc(#loc21)
    llvm.br ^bb38 loc(#loc21)
  ^bb38:  // 2 preds: ^bb36, ^bb37
    %221 = llvm.insertelement %187, %67[%14 : i32] : vector<1xf32> loc(#loc21)
    %222 = llvm.bitcast %221 : vector<1xf32> to i32 loc(#loc21)
    %223 = llvm.and %0, %64  : i1 loc(#loc21)
    %224 = llvm.insertelement %222, %203[%14 : i32] : vector<1xi32> loc(#loc21)
    llvm.cond_br %223, ^bb39, ^bb40 loc(#loc21)
  ^bb39:  // pred: ^bb38
    llvm.store %224, %197 : vector<1xi32>, !llvm.ptr<1> loc(#loc21)
    llvm.br ^bb40 loc(#loc21)
  ^bb40:  // 2 preds: ^bb38, ^bb39
    %225 = llvm.insertelement %188, %67[%14 : i32] : vector<1xf32> loc(#loc21)
    %226 = llvm.bitcast %225 : vector<1xf32> to i32 loc(#loc21)
    %227 = llvm.and %0, %65  : i1 loc(#loc21)
    %228 = llvm.insertelement %226, %203[%14 : i32] : vector<1xi32> loc(#loc21)
    llvm.cond_br %227, ^bb41, ^bb42 loc(#loc21)
  ^bb41:  // pred: ^bb40
    llvm.store %228, %198 : vector<1xi32>, !llvm.ptr<1> loc(#loc21)
    llvm.br ^bb42 loc(#loc21)
  ^bb42:  // 2 preds: ^bb40, ^bb41
    %229 = llvm.insertelement %189, %67[%14 : i32] : vector<1xf32> loc(#loc21)
    %230 = llvm.bitcast %229 : vector<1xf32> to i32 loc(#loc21)
    %231 = llvm.and %0, %66  : i1 loc(#loc21)
    %232 = llvm.insertelement %230, %203[%14 : i32] : vector<1xi32> loc(#loc21)
    llvm.cond_br %231, ^bb43, ^bb44 loc(#loc21)
  ^bb43:  // pred: ^bb42
    llvm.store %232, %199 : vector<1xi32>, !llvm.ptr<1> loc(#loc21)
    llvm.br ^bb44 loc(#loc21)
  ^bb44:  // 2 preds: ^bb42, ^bb43
    llvm.return loc(#loc22)
  } loc(#loc)
} loc(#loc)
#loc1 = loc(unknown)
#loc2 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":83:31)
#loc3 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":78:28)
#loc4 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":80:42)
#loc5 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":80:32)
#loc6 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":84:33)
#loc7 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":86:49)
#loc9 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/triton/language/standard.py":132:26)
#loc12 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":88:26)
#loc13 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":90:23)
#loc16 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/triton/language/standard.py":241:15)
#loc17 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":92:33)
#loc18 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":94:50)
#loc19 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":94:40)
#loc20 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":95:41)
#loc21 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":96:26)
#loc22 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/02-fused-softmax.py":96:4)
#loc23 = loc(callsite(#loc9 at #loc10))
#loc26 = loc(callsite(#loc16 at #loc14))
#loc27 = loc(callsite(#loc23 at #loc11))
#loc28 = loc(callsite(#loc26 at #loc15))

------------------
; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

define spir_kernel void @softmax_kernel_0d1d234(ptr addrspace(1) nocapture writeonly %0, ptr addrspace(1) nocapture readonly %1, i32 %2, i32 %3, i32 %4, ptr addrspace(3) nocapture %5) local_unnamed_addr !intel_reqd_sub_group_size !7 !max_work_group_size !8 {
  %7 = tail call i16 @llvm.pisa.localid.x()
  %8 = zext i16 %7 to i32
  %9 = and i32 %8, 31
  %10 = lshr i32 %8, 5
  %11 = and i32 %10, 3
  %urem = and i32 %8, 127
  %12 = or disjoint i32 %urem, 128
  %13 = or disjoint i32 %urem, 256
  %14 = or disjoint i32 %urem, 384
  %15 = or disjoint i32 %urem, 512
  %16 = or disjoint i32 %urem, 640
  %17 = or disjoint i32 %urem, 768
  %18 = or disjoint i32 %urem, 896
  %19 = tail call i32 @llvm.pisa.groupid.x()
  %20 = mul i32 %19, %2
  %21 = sext i32 %20 to i64
  %22 = getelementptr float, ptr addrspace(1) %1, i64 %21
  %23 = zext nneg i32 %12 to i64
  %24 = getelementptr float, ptr addrspace(1) %22, i64 %23
  %25 = zext nneg i32 %13 to i64
  %26 = getelementptr float, ptr addrspace(1) %22, i64 %25
  %27 = zext nneg i32 %14 to i64
  %28 = getelementptr float, ptr addrspace(1) %22, i64 %27
  %29 = zext nneg i32 %15 to i64
  %30 = getelementptr float, ptr addrspace(1) %22, i64 %29
  %31 = zext nneg i32 %16 to i64
  %32 = getelementptr float, ptr addrspace(1) %22, i64 %31
  %33 = zext nneg i32 %17 to i64
  %34 = getelementptr float, ptr addrspace(1) %22, i64 %33
  %35 = zext nneg i32 %18 to i64
  %36 = getelementptr float, ptr addrspace(1) %22, i64 %35
  %37 = icmp slt i32 %urem, %4
  %38 = icmp slt i32 %12, %4
  %39 = icmp slt i32 %13, %4
  %40 = icmp slt i32 %14, %4
  %41 = icmp slt i32 %15, %4
  %42 = icmp slt i32 %16, %4
  %43 = icmp slt i32 %17, %4
  %44 = icmp slt i32 %18, %4
  br i1 %37, label %45, label %49

45:                                               ; preds = %6
  %46 = zext nneg i32 %urem to i64
  %47 = getelementptr float, ptr addrspace(1) %22, i64 %46
  %48 = load <1 x float>, ptr addrspace(1) %47, align 4
  br label %49

49:                                               ; preds = %45, %6
  %50 = phi <1 x float> [ %48, %45 ], [ <float 0xFFF0000000000000>, %6 ]
  %51 = extractelement <1 x float> %50, i64 0
  br i1 %38, label %52, label %54

52:                                               ; preds = %49
  %53 = load <1 x float>, ptr addrspace(1) %24, align 4
  br label %54

54:                                               ; preds = %52, %49
  %55 = phi <1 x float> [ %53, %52 ], [ <float 0xFFF0000000000000>, %49 ]
  %56 = extractelement <1 x float> %55, i64 0
  br i1 %39, label %57, label %59

57:                                               ; preds = %54
  %58 = load <1 x float>, ptr addrspace(1) %26, align 4
  br label %59

59:                                               ; preds = %57, %54
  %60 = phi <1 x float> [ %58, %57 ], [ <float 0xFFF0000000000000>, %54 ]
  %61 = extractelement <1 x float> %60, i64 0
  br i1 %40, label %62, label %64

62:                                               ; preds = %59
  %63 = load <1 x float>, ptr addrspace(1) %28, align 4
  br label %64

64:                                               ; preds = %62, %59
  %65 = phi <1 x float> [ %63, %62 ], [ <float 0xFFF0000000000000>, %59 ]
  %66 = extractelement <1 x float> %65, i64 0
  br i1 %41, label %67, label %69

67:                                               ; preds = %64
  %68 = load <1 x float>, ptr addrspace(1) %30, align 4
  br label %69

69:                                               ; preds = %67, %64
  %70 = phi <1 x float> [ %68, %67 ], [ <float 0xFFF0000000000000>, %64 ]
  %71 = extractelement <1 x float> %70, i64 0
  br i1 %42, label %72, label %74

72:                                               ; preds = %69
  %73 = load <1 x float>, ptr addrspace(1) %32, align 4
  br label %74

74:                                               ; preds = %72, %69
  %75 = phi <1 x float> [ %73, %72 ], [ <float 0xFFF0000000000000>, %69 ]
  %76 = extractelement <1 x float> %75, i64 0
  br i1 %43, label %77, label %79

77:                                               ; preds = %74
  %78 = load <1 x float>, ptr addrspace(1) %34, align 4
  br label %79

79:                                               ; preds = %77, %74
  %80 = phi <1 x float> [ %78, %77 ], [ <float 0xFFF0000000000000>, %74 ]
  %81 = extractelement <1 x float> %80, i64 0
  br i1 %44, label %82, label %84

82:                                               ; preds = %79
  %83 = load <1 x float>, ptr addrspace(1) %36, align 4
  br label %84

84:                                               ; preds = %82, %79
  %85 = phi <1 x float> [ %83, %82 ], [ <float 0xFFF0000000000000>, %79 ]
  %86 = extractelement <1 x float> %85, i64 0
  %87 = tail call float @llvm.maxnum.f32(float %51, float %56)
  %88 = tail call float @llvm.maxnum.f32(float %87, float %61)
  %89 = tail call float @llvm.maxnum.f32(float %88, float %66)
  %90 = tail call float @llvm.maxnum.f32(float %89, float %71)
  %91 = tail call float @llvm.maxnum.f32(float %90, float %76)
  %92 = tail call float @llvm.maxnum.f32(float %91, float %81)
  %93 = tail call float @llvm.maxnum.f32(float %92, float %86)
  %94 = tail call float @llvm.pisa.shfl.xorfj(float %93, i32 16, i32 0) #2
  %95 = tail call float @llvm.maxnum.f32(float %93, float %94)
  %96 = tail call float @llvm.pisa.shfl.xorfj(float %95, i32 8, i32 0) #2
  %97 = tail call float @llvm.maxnum.f32(float %95, float %96)
  %98 = tail call float @llvm.pisa.shfl.xorfj(float %97, i32 4, i32 0) #2
  %99 = tail call float @llvm.maxnum.f32(float %97, float %98)
  %100 = tail call float @llvm.pisa.shfl.xorfj(float %99, i32 2, i32 0) #2
  %101 = tail call float @llvm.maxnum.f32(float %99, float %100)
  %102 = tail call float @llvm.pisa.shfl.xorfj(float %101, i32 1, i32 0) #2
  %103 = icmp eq i32 %9, 0
  %104 = zext nneg i32 %11 to i64
  %105 = getelementptr float, ptr addrspace(3) %5, i64 %104
  br i1 %103, label %106, label %108

106:                                              ; preds = %84
  %107 = tail call float @llvm.maxnum.f32(float %101, float %102)
  store float %107, ptr addrspace(3) %105, align 4
  br label %108

108:                                              ; preds = %106, %84
  tail call void @llvm.pisa.workgroup.barrier() #2
  %109 = icmp ult i16 %7, 4
  %110 = zext i16 %7 to i64
  %111 = getelementptr float, ptr addrspace(3) %5, i64 %110
  br i1 %109, label %112, label %114

112:                                              ; preds = %108
  %113 = load float, ptr addrspace(3) %111, align 4
  br label %114

114:                                              ; preds = %112, %108
  %115 = phi float [ %113, %112 ], [ undef, %108 ]
  %116 = tail call float @llvm.pisa.shfl.xorfj(float %115, i32 2, i32 0) #2
  %117 = tail call float @llvm.maxnum.f32(float %115, float %116)
  %118 = tail call float @llvm.pisa.shfl.xorfj(float %117, i32 1, i32 0) #2
  %119 = and i32 %8, 3
  %120 = icmp eq i32 %119, 0
  %121 = and i1 %109, %120
  br i1 %121, label %122, label %124

122:                                              ; preds = %114
  %123 = tail call float @llvm.maxnum.f32(float %117, float %118)
  store float %123, ptr addrspace(3) %111, align 4
  br label %124

124:                                              ; preds = %122, %114
  tail call void @llvm.pisa.workgroup.barrier() #2
  %125 = load float, ptr addrspace(3) %5, align 4
  %126 = fsub float %51, %125
  %127 = fsub float %56, %125
  %128 = fsub float %61, %125
  %129 = fsub float %66, %125
  %130 = fsub float %71, %125
  %131 = fsub float %76, %125
  %132 = fsub float %81, %125
  %133 = fsub float %86, %125
  %134 = fmul float %126, 0x3FF7154760000000
  %135 = tail call float @llvm.exp2.f32(float %134)
  %136 = fmul float %127, 0x3FF7154760000000
  %137 = tail call float @llvm.exp2.f32(float %136)
  %138 = fmul float %128, 0x3FF7154760000000
  %139 = tail call float @llvm.exp2.f32(float %138)
  %140 = fmul float %129, 0x3FF7154760000000
  %141 = tail call float @llvm.exp2.f32(float %140)
  %142 = fmul float %130, 0x3FF7154760000000
  %143 = tail call float @llvm.exp2.f32(float %142)
  %144 = fmul float %131, 0x3FF7154760000000
  %145 = tail call float @llvm.exp2.f32(float %144)
  %146 = fmul float %132, 0x3FF7154760000000
  %147 = tail call float @llvm.exp2.f32(float %146)
  %148 = fmul float %133, 0x3FF7154760000000
  %149 = tail call float @llvm.exp2.f32(float %148)
  tail call void @llvm.pisa.workgroup.barrier() #2
  %150 = fadd float %135, %137
  %151 = fadd float %139, %150
  %152 = fadd float %141, %151
  %153 = fadd float %143, %152
  %154 = fadd float %145, %153
  %155 = fadd float %147, %154
  %156 = fadd float %149, %155
  %157 = tail call float @llvm.pisa.shfl.xorfj(float %156, i32 16, i32 0) #2
  %158 = fadd float %157, %156
  %159 = tail call float @llvm.pisa.shfl.xorfj(float %158, i32 8, i32 0) #2
  %160 = fadd float %159, %158
  %161 = tail call float @llvm.pisa.shfl.xorfj(float %160, i32 4, i32 0) #2
  %162 = fadd float %161, %160
  %163 = tail call float @llvm.pisa.shfl.xorfj(float %162, i32 2, i32 0) #2
  %164 = fadd float %163, %162
  %165 = tail call float @llvm.pisa.shfl.xorfj(float %164, i32 1, i32 0) #2
  br i1 %103, label %166, label %168

166:                                              ; preds = %124
  %167 = fadd float %165, %164
  store float %167, ptr addrspace(3) %105, align 4
  br label %168

168:                                              ; preds = %166, %124
  tail call void @llvm.pisa.workgroup.barrier() #2
  br i1 %109, label %169, label %171

169:                                              ; preds = %168
  %170 = load float, ptr addrspace(3) %111, align 4
  br label %171

171:                                              ; preds = %169, %168
  %172 = phi float [ %170, %169 ], [ undef, %168 ]
  %173 = tail call float @llvm.pisa.shfl.xorfj(float %172, i32 2, i32 0) #2
  %174 = fadd float %172, %173
  %175 = tail call float @llvm.pisa.shfl.xorfj(float %174, i32 1, i32 0) #2
  br i1 %121, label %176, label %178

176:                                              ; preds = %171
  %177 = fadd float %174, %175
  store float %177, ptr addrspace(3) %111, align 4
  br label %178

178:                                              ; preds = %176, %171
  tail call void @llvm.pisa.workgroup.barrier() #2
  %179 = load float, ptr addrspace(3) %5, align 4
  %180 = fdiv float %137, %179
  %181 = fdiv float %139, %179
  %182 = fdiv float %141, %179
  %183 = fdiv float %143, %179
  %184 = fdiv float %145, %179
  %185 = fdiv float %147, %179
  %186 = fdiv float %149, %179
  %187 = mul i32 %19, %3
  %188 = sext i32 %187 to i64
  %189 = getelementptr float, ptr addrspace(1) %0, i64 %188
  %190 = getelementptr float, ptr addrspace(1) %189, i64 %23
  %191 = getelementptr float, ptr addrspace(1) %189, i64 %25
  %192 = getelementptr float, ptr addrspace(1) %189, i64 %27
  %193 = getelementptr float, ptr addrspace(1) %189, i64 %29
  %194 = getelementptr float, ptr addrspace(1) %189, i64 %31
  %195 = getelementptr float, ptr addrspace(1) %189, i64 %33
  %196 = getelementptr float, ptr addrspace(1) %189, i64 %35
  br i1 %37, label %197, label %201

197:                                              ; preds = %178
  %198 = fdiv float %135, %179
  %199 = zext nneg i32 %urem to i64
  %200 = getelementptr float, ptr addrspace(1) %189, i64 %199
  store float %198, ptr addrspace(1) %200, align 4
  br label %201

201:                                              ; preds = %197, %178
  br i1 %38, label %202, label %203

202:                                              ; preds = %201
  store float %180, ptr addrspace(1) %190, align 4
  br label %203

203:                                              ; preds = %202, %201
  br i1 %39, label %204, label %205

204:                                              ; preds = %203
  store float %181, ptr addrspace(1) %191, align 4
  br label %205

205:                                              ; preds = %204, %203
  br i1 %40, label %206, label %207

206:                                              ; preds = %205
  store float %182, ptr addrspace(1) %192, align 4
  br label %207

207:                                              ; preds = %206, %205
  br i1 %41, label %208, label %209

208:                                              ; preds = %207
  store float %183, ptr addrspace(1) %193, align 4
  br label %209

209:                                              ; preds = %208, %207
  br i1 %42, label %210, label %211

210:                                              ; preds = %209
  store float %184, ptr addrspace(1) %194, align 4
  br label %211

211:                                              ; preds = %210, %209
  br i1 %43, label %212, label %213

212:                                              ; preds = %211
  store float %185, ptr addrspace(1) %195, align 4
  br label %213

213:                                              ; preds = %212, %211
  br i1 %44, label %214, label %215

214:                                              ; preds = %213
  store float %186, ptr addrspace(1) %196, align 4
  br label %215

215:                                              ; preds = %214, %213
  ret void
}

; Function Attrs: nofree nosync nounwind memory(none)
declare spir_func i16 @llvm.pisa.localid.x() #0

; Function Attrs: nofree nosync nounwind memory(none)
declare spir_func i32 @llvm.pisa.groupid.x() #0

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.maxnum.f32(float, float) #1

; Function Attrs: convergent
declare spir_func float @llvm.pisa.shfl.xorfj(float, i32, i32) #2

; Function Attrs: convergent nounwind
declare spir_func void @llvm.pisa.workgroup.barrier() #3

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.exp2.f32(float) #1

attributes #0 = { nofree nosync nounwind memory(none) }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { convergent }
attributes #3 = { convergent nounwind }

!llvm.module.flags = !{!0, !1, !2}
!opencl.spir.version = !{!3, !3, !3, !3, !3, !3, !3, !3, !3, !3, !3, !3, !3, !3, !3, !3, !3}
!spirv.Source = !{!4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4, !4}
!opencl.compiler.options = !{!5, !5, !5, !5, !5, !5, !5, !5, !5, !5, !5, !5, !5, !5, !5, !5, !5}
!llvm.ident = !{!6, !6, !6, !6, !6, !6, !6, !6, !6, !6, !6, !6, !6, !6, !6, !6, !6}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{i32 1, i32 2}
!4 = !{i32 4, i32 100000}
!5 = !{}
!6 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.2.0.20230622)"}
!7 = !{i64 32}
!8 = !{i64 128, i64 1, i64 1}