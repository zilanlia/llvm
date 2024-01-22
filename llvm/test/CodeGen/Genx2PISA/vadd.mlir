#loc = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":30:0)
module {
  tt.func public @add_kernel_0d1d2d3de(%arg0: !tt.ptr<f32, 1> {tt.divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":30:0), %arg1: !tt.ptr<f32, 1> {tt.divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":30:0), %arg2: !tt.ptr<f32, 1> {tt.divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":30:0), %arg3: i32 {tt.divisibility = 16 : i32, tt.max_divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":30:0)) attributes {noinline = false} {
    %c1024_i32 = arith.constant 1024 : i32 loc(#loc1)
    %0 = tt.get_program_id x : i32 loc(#loc2)
    %1 = arith.muli %0, %c1024_i32 : i32 loc(#loc3)
    %2 = tt.make_range {end = 1024 : i32, start = 0 : i32} : tensor<1024xi32> loc(#loc4)
    %3 = tt.splat %1 : (i32) -> tensor<1024xi32> loc(#loc5)
    %4 = arith.addi %3, %2 : tensor<1024xi32> loc(#loc5)
    %5 = tt.splat %arg3 : (i32) -> tensor<1024xi32> loc(#loc6)
    %6 = arith.cmpi slt, %4, %5 : tensor<1024xi32> loc(#loc6)
    %7 = tt.splat %arg0 : (!tt.ptr<f32, 1>) -> tensor<1024x!tt.ptr<f32, 1>> loc(#loc7)
    %8 = tt.addptr %7, %4 : tensor<1024x!tt.ptr<f32, 1>>, tensor<1024xi32> loc(#loc7)
    %9 = tt.load %8, %6 {cache = 1 : i32, evict = 1 : i32, isVolatile = false} : tensor<1024xf32> loc(#loc8)
    %10 = tt.splat %arg1 : (!tt.ptr<f32, 1>) -> tensor<1024x!tt.ptr<f32, 1>> loc(#loc9)
    %11 = tt.addptr %10, %4 : tensor<1024x!tt.ptr<f32, 1>>, tensor<1024xi32> loc(#loc9)
    %12 = tt.load %11, %6 {cache = 1 : i32, evict = 1 : i32, isVolatile = false} : tensor<1024xf32> loc(#loc10)
    %13 = arith.addf %9, %12 : tensor<1024xf32> loc(#loc11)
    %14 = tt.splat %arg2 : (!tt.ptr<f32, 1>) -> tensor<1024x!tt.ptr<f32, 1>> loc(#loc12)
    %15 = tt.addptr %14, %4 : tensor<1024x!tt.ptr<f32, 1>>, tensor<1024xi32> loc(#loc12)
    tt.store %15, %13, %6 {cache = 1 : i32, evict = 1 : i32} : tensor<1024xf32> loc(#loc13)
    tt.return loc(#loc14)
  } loc(#loc)
} loc(#loc)
#loc1 = loc(unknown)
#loc2 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":39:24)
#loc3 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":44:24)
#loc4 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":45:41)
#loc5 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":45:28)
#loc6 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":47:21)
#loc7 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":50:24)
#loc8 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":50:16)
#loc9 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":51:24)
#loc10 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":51:16)
#loc11 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":52:17)
#loc12 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":54:26)
#loc13 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":54:35)
#loc14 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":54:4)

------------------
#blocked = #triton_gpu.blocked<{sizePerThread = [4], threadsPerWarp = [32], warpsPerCTA = [4], order = [0], CTAsPerCGA = [1], CTASplitNum = [1], CTAOrder = [0]}>
#loc = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":30:0)
module attributes {"triton_gpu.compute-capability" = 0 : i32, "triton_gpu.num-ctas" = 1 : i32, "triton_gpu.num-warps" = 4 : i32, "triton_gpu.threads-per-warp" = 32 : i32} {
  tt.func public @add_kernel_0d1d2d3de(%arg0: !tt.ptr<f32, 1> {tt.divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":30:0), %arg1: !tt.ptr<f32, 1> {tt.divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":30:0), %arg2: !tt.ptr<f32, 1> {tt.divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":30:0), %arg3: i32 {tt.divisibility = 16 : i32, tt.max_divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":30:0)) attributes {noinline = false} {
    %c1024_i32 = arith.constant 1024 : i32 loc(#loc1)
    %0 = tt.get_program_id x : i32 loc(#loc2)
    %1 = arith.muli %0, %c1024_i32 : i32 loc(#loc3)
    %2 = tt.make_range {end = 1024 : i32, start = 0 : i32} : tensor<1024xi32, #blocked> loc(#loc4)
    %3 = tt.splat %1 : (i32) -> tensor<1024xi32, #blocked> loc(#loc5)
    %4 = arith.addi %3, %2 : tensor<1024xi32, #blocked> loc(#loc5)
    %5 = tt.splat %arg3 : (i32) -> tensor<1024xi32, #blocked> loc(#loc6)
    %6 = arith.cmpi slt, %4, %5 : tensor<1024xi32, #blocked> loc(#loc6)
    %7 = tt.splat %arg0 : (!tt.ptr<f32, 1>) -> tensor<1024x!tt.ptr<f32, 1>, #blocked> loc(#loc7)
    %8 = tt.addptr %7, %4 : tensor<1024x!tt.ptr<f32, 1>, #blocked>, tensor<1024xi32, #blocked> loc(#loc7)
    %9 = tt.load %8, %6 {cache = 1 : i32, evict = 1 : i32, isVolatile = false} : tensor<1024xf32, #blocked> loc(#loc8)
    %10 = tt.splat %arg1 : (!tt.ptr<f32, 1>) -> tensor<1024x!tt.ptr<f32, 1>, #blocked> loc(#loc9)
    %11 = tt.addptr %10, %4 : tensor<1024x!tt.ptr<f32, 1>, #blocked>, tensor<1024xi32, #blocked> loc(#loc9)
    %12 = tt.load %11, %6 {cache = 1 : i32, evict = 1 : i32, isVolatile = false} : tensor<1024xf32, #blocked> loc(#loc10)
    %13 = arith.addf %9, %12 : tensor<1024xf32, #blocked> loc(#loc11)
    %14 = tt.splat %arg2 : (!tt.ptr<f32, 1>) -> tensor<1024x!tt.ptr<f32, 1>, #blocked> loc(#loc12)
    %15 = tt.addptr %14, %4 : tensor<1024x!tt.ptr<f32, 1>, #blocked>, tensor<1024xi32, #blocked> loc(#loc12)
    tt.store %15, %13, %6 {cache = 1 : i32, evict = 1 : i32} : tensor<1024xf32, #blocked> loc(#loc13)
    tt.return loc(#loc14)
  } loc(#loc)
} loc(#loc)
#loc1 = loc(unknown)
#loc2 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":39:24)
#loc3 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":44:24)
#loc4 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":45:41)
#loc5 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":45:28)
#loc6 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":47:21)
#loc7 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":50:24)
#loc8 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":50:16)
#loc9 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":51:24)
#loc10 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":51:16)
#loc11 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":52:17)
#loc12 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":54:26)
#loc13 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":54:35)
#loc14 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":54:4)

---------------LLVM IR----------------
#loc = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":30:0)
#loc8 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":50:16)
#loc10 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":51:16)
module attributes {"triton_gpu.compute-capability" = 0 : i32, "triton_gpu.num-ctas" = 1 : i32, "triton_gpu.num-warps" = 4 : i32, triton_gpu.shared = 0 : i32, "triton_gpu.threads-per-warp" = 32 : i32} {
  llvm.func @add_kernel_0d1d2d3de(%arg0: !llvm.ptr<1> {tt.divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":30:0), %arg1: !llvm.ptr<1> {tt.divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":30:0), %arg2: !llvm.ptr<1> {tt.divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":30:0), %arg3: i32 {tt.divisibility = 16 : i32, tt.max_divisibility = 16 : i32} loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":30:0)) attributes {genx.intel_reqd_sub_group_size = [32 : i32], genx.kernel = 1 : i32, genx.max_work_group_size = [128 : i32, 1 : i32, 1 : i32], noinline = false, sym_visibility = "public", "triton_gpu.num-tma-load" = 0 : i32, "triton_gpu.num-tma-store" = 0 : i32} {
    %0 = llvm.mlir.constant(true) : i1 loc(#loc1)
    %1 = llvm.mlir.constant(0 : index) : i32 loc(#loc1)
    %2 = llvm.mlir.constant(512 : i32) : i32 loc(#loc1)
    %3 = llvm.mlir.constant(3 : i32) : i32 loc(#loc1)
    %4 = llvm.mlir.constant(2 : i32) : i32 loc(#loc1)
    %5 = llvm.mlir.constant(1024 : i32) : i32 loc(#loc1)
    %6 = llvm.mlir.constant(1 : i32) : i32 loc(#loc1)
    %7 = llvm.mlir.constant(0 : i32) : i32 loc(#loc1)
    %8 = llvm.mlir.constant(256 : i32) : i32 loc(#loc1)
    %9 = llvm.mlir.constant(8 : i32) : i32 loc(#loc1)
    %10 = llvm.mlir.constant(4 : i32) : i32 loc(#loc1)
    %11 = llvm.mlir.constant(32 : i32) : i32 loc(#loc1)
    %12 = genx.workitem.id.x : i32 loc(#loc2)
    %13 = llvm.urem %12, %11  : i32 loc(#loc2)
    %14 = llvm.udiv %12, %11  : i32 loc(#loc2)
    %15 = llvm.urem %14, %10  : i32 loc(#loc2)
    %16 = llvm.urem %13, %11  : i32 loc(#loc2)
    %17 = llvm.urem %15, %9  : i32 loc(#loc2)
    %18 = llvm.urem %16, %8  : i32 loc(#loc2)
    %19 = llvm.mul %17, %11  : i32 loc(#loc2)
    %20 = llvm.add %18, %19  : i32 loc(#loc2)
    %21 = llvm.mul %20, %10  : i32 loc(#loc2)
    %22 = llvm.urem %7, %6  : i32 loc(#loc2)
    %23 = llvm.mul %22, %5  : i32 loc(#loc2)
    %24 = llvm.add %21, %23  : i32 loc(#loc2)
    %25 = llvm.add %24, %7  : i32 loc(#loc2)
    %26 = llvm.add %24, %2  : i32 loc(#loc2)
    %27 = genx.workgroup.id.x : i32 loc(#loc3)
    %28 = llvm.mul %27, %5  : i32 loc(#loc4)
    %29 = llvm.add %25, %1  : i32 loc(#loc2)
    %30 = llvm.add %26, %1  : i32 loc(#loc2)
    %31 = llvm.add %28, %29  : i32 loc(#loc5)
    %32 = llvm.add %28, %30  : i32 loc(#loc5)
    %33 = llvm.icmp "slt" %31, %arg3 : i32 loc(#loc6)
    %34 = llvm.icmp "slt" %32, %arg3 : i32 loc(#loc6)
    %35 = llvm.getelementptr %arg0[%31] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc7)
    %36 = llvm.getelementptr %arg0[%32] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc7)
    %37 = llvm.mlir.undef : vector<4xi32> loc(#loc8)
    llvm.cond_br %33, ^bb1, ^bb2(%37 : vector<4xi32>) loc(#loc8)
  ^bb1:  // pred: ^bb0
    %38 = llvm.load %35 : !llvm.ptr<1> -> vector<4xi32> loc(#loc8)
    llvm.br ^bb2(%38 : vector<4xi32>) loc(#loc8)
  ^bb2(%39: vector<4xi32> loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":50:16)):  // 2 preds: ^bb0, ^bb1
    %40 = llvm.extractelement %39[%7 : i32] : vector<4xi32> loc(#loc8)
    %41 = llvm.bitcast %40 : i32 to vector<1xf32> loc(#loc8)
    %42 = llvm.extractelement %39[%6 : i32] : vector<4xi32> loc(#loc8)
    %43 = llvm.bitcast %42 : i32 to vector<1xf32> loc(#loc8)
    %44 = llvm.extractelement %39[%4 : i32] : vector<4xi32> loc(#loc8)
    %45 = llvm.bitcast %44 : i32 to vector<1xf32> loc(#loc8)
    %46 = llvm.extractelement %39[%3 : i32] : vector<4xi32> loc(#loc8)
    %47 = llvm.bitcast %46 : i32 to vector<1xf32> loc(#loc8)
    %48 = llvm.extractelement %41[%7 : i32] : vector<1xf32> loc(#loc8)
    %49 = llvm.extractelement %43[%7 : i32] : vector<1xf32> loc(#loc8)
    %50 = llvm.extractelement %45[%7 : i32] : vector<1xf32> loc(#loc8)
    %51 = llvm.extractelement %47[%7 : i32] : vector<1xf32> loc(#loc8)
    llvm.cond_br %34, ^bb3, ^bb4(%37 : vector<4xi32>) loc(#loc8)
  ^bb3:  // pred: ^bb2
    %52 = llvm.load %36 : !llvm.ptr<1> -> vector<4xi32> loc(#loc8)
    llvm.br ^bb4(%52 : vector<4xi32>) loc(#loc8)
  ^bb4(%53: vector<4xi32> loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":50:16)):  // 2 preds: ^bb2, ^bb3
    %54 = llvm.extractelement %53[%7 : i32] : vector<4xi32> loc(#loc8)
    %55 = llvm.bitcast %54 : i32 to vector<1xf32> loc(#loc8)
    %56 = llvm.extractelement %53[%6 : i32] : vector<4xi32> loc(#loc8)
    %57 = llvm.bitcast %56 : i32 to vector<1xf32> loc(#loc8)
    %58 = llvm.extractelement %53[%4 : i32] : vector<4xi32> loc(#loc8)
    %59 = llvm.bitcast %58 : i32 to vector<1xf32> loc(#loc8)
    %60 = llvm.extractelement %53[%3 : i32] : vector<4xi32> loc(#loc8)
    %61 = llvm.bitcast %60 : i32 to vector<1xf32> loc(#loc8)
    %62 = llvm.extractelement %55[%7 : i32] : vector<1xf32> loc(#loc8)
    %63 = llvm.extractelement %57[%7 : i32] : vector<1xf32> loc(#loc8)
    %64 = llvm.extractelement %59[%7 : i32] : vector<1xf32> loc(#loc8)
    %65 = llvm.extractelement %61[%7 : i32] : vector<1xf32> loc(#loc8)
    %66 = llvm.getelementptr %arg1[%31] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc9)
    %67 = llvm.getelementptr %arg1[%32] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc9)
    llvm.cond_br %33, ^bb5, ^bb6(%37 : vector<4xi32>) loc(#loc10)
  ^bb5:  // pred: ^bb4
    %68 = llvm.load %66 : !llvm.ptr<1> -> vector<4xi32> loc(#loc10)
    llvm.br ^bb6(%68 : vector<4xi32>) loc(#loc10)
  ^bb6(%69: vector<4xi32> loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":51:16)):  // 2 preds: ^bb4, ^bb5
    %70 = llvm.extractelement %69[%7 : i32] : vector<4xi32> loc(#loc10)
    %71 = llvm.bitcast %70 : i32 to vector<1xf32> loc(#loc10)
    %72 = llvm.extractelement %69[%6 : i32] : vector<4xi32> loc(#loc10)
    %73 = llvm.bitcast %72 : i32 to vector<1xf32> loc(#loc10)
    %74 = llvm.extractelement %69[%4 : i32] : vector<4xi32> loc(#loc10)
    %75 = llvm.bitcast %74 : i32 to vector<1xf32> loc(#loc10)
    %76 = llvm.extractelement %69[%3 : i32] : vector<4xi32> loc(#loc10)
    %77 = llvm.bitcast %76 : i32 to vector<1xf32> loc(#loc10)
    %78 = llvm.extractelement %71[%7 : i32] : vector<1xf32> loc(#loc10)
    %79 = llvm.extractelement %73[%7 : i32] : vector<1xf32> loc(#loc10)
    %80 = llvm.extractelement %75[%7 : i32] : vector<1xf32> loc(#loc10)
    %81 = llvm.extractelement %77[%7 : i32] : vector<1xf32> loc(#loc10)
    llvm.cond_br %34, ^bb7, ^bb8(%37 : vector<4xi32>) loc(#loc10)
  ^bb7:  // pred: ^bb6
    %82 = llvm.load %67 : !llvm.ptr<1> -> vector<4xi32> loc(#loc10)
    llvm.br ^bb8(%82 : vector<4xi32>) loc(#loc10)
  ^bb8(%83: vector<4xi32> loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":51:16)):  // 2 preds: ^bb6, ^bb7
    %84 = llvm.extractelement %83[%7 : i32] : vector<4xi32> loc(#loc10)
    %85 = llvm.bitcast %84 : i32 to vector<1xf32> loc(#loc10)
    %86 = llvm.extractelement %83[%6 : i32] : vector<4xi32> loc(#loc10)
    %87 = llvm.bitcast %86 : i32 to vector<1xf32> loc(#loc10)
    %88 = llvm.extractelement %83[%4 : i32] : vector<4xi32> loc(#loc10)
    %89 = llvm.bitcast %88 : i32 to vector<1xf32> loc(#loc10)
    %90 = llvm.extractelement %83[%3 : i32] : vector<4xi32> loc(#loc10)
    %91 = llvm.bitcast %90 : i32 to vector<1xf32> loc(#loc10)
    %92 = llvm.extractelement %85[%7 : i32] : vector<1xf32> loc(#loc10)
    %93 = llvm.extractelement %87[%7 : i32] : vector<1xf32> loc(#loc10)
    %94 = llvm.extractelement %89[%7 : i32] : vector<1xf32> loc(#loc10)
    %95 = llvm.extractelement %91[%7 : i32] : vector<1xf32> loc(#loc10)
    %96 = llvm.fadd %48, %78  : f32 loc(#loc11)
    %97 = llvm.fadd %49, %79  : f32 loc(#loc11)
    %98 = llvm.fadd %50, %80  : f32 loc(#loc11)
    %99 = llvm.fadd %51, %81  : f32 loc(#loc11)
    %100 = llvm.fadd %62, %92  : f32 loc(#loc11)
    %101 = llvm.fadd %63, %93  : f32 loc(#loc11)
    %102 = llvm.fadd %64, %94  : f32 loc(#loc11)
    %103 = llvm.fadd %65, %95  : f32 loc(#loc11)
    %104 = llvm.getelementptr %arg2[%31] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc12)
    %105 = llvm.getelementptr %arg2[%32] : (!llvm.ptr<1>, i32) -> !llvm.ptr<1>, f32 loc(#loc12)
    %106 = llvm.mlir.undef : vector<1xf32> loc(#loc13)
    %107 = llvm.insertelement %96, %106[%7 : i32] : vector<1xf32> loc(#loc13)
    %108 = llvm.bitcast %107 : vector<1xf32> to i32 loc(#loc13)
    %109 = llvm.insertelement %97, %106[%7 : i32] : vector<1xf32> loc(#loc13)
    %110 = llvm.bitcast %109 : vector<1xf32> to i32 loc(#loc13)
    %111 = llvm.insertelement %98, %106[%7 : i32] : vector<1xf32> loc(#loc13)
    %112 = llvm.bitcast %111 : vector<1xf32> to i32 loc(#loc13)
    %113 = llvm.insertelement %99, %106[%7 : i32] : vector<1xf32> loc(#loc13)
    %114 = llvm.bitcast %113 : vector<1xf32> to i32 loc(#loc13)
    %115 = llvm.and %0, %33  : i1 loc(#loc13)
    %116 = llvm.insertelement %108, %37[%7 : i32] : vector<4xi32> loc(#loc13)
    %117 = llvm.insertelement %110, %116[%6 : i32] : vector<4xi32> loc(#loc13)
    %118 = llvm.insertelement %112, %117[%4 : i32] : vector<4xi32> loc(#loc13)
    %119 = llvm.insertelement %114, %118[%3 : i32] : vector<4xi32> loc(#loc13)
    llvm.cond_br %115, ^bb9, ^bb10 loc(#loc13)
  ^bb9:  // pred: ^bb8
    llvm.store %119, %104 : vector<4xi32>, !llvm.ptr<1> loc(#loc13)
    llvm.br ^bb10 loc(#loc13)
  ^bb10:  // 2 preds: ^bb8, ^bb9
    %120 = llvm.insertelement %100, %106[%7 : i32] : vector<1xf32> loc(#loc13)
    %121 = llvm.bitcast %120 : vector<1xf32> to i32 loc(#loc13)
    %122 = llvm.insertelement %101, %106[%7 : i32] : vector<1xf32> loc(#loc13)
    %123 = llvm.bitcast %122 : vector<1xf32> to i32 loc(#loc13)
    %124 = llvm.insertelement %102, %106[%7 : i32] : vector<1xf32> loc(#loc13)
    %125 = llvm.bitcast %124 : vector<1xf32> to i32 loc(#loc13)
    %126 = llvm.insertelement %103, %106[%7 : i32] : vector<1xf32> loc(#loc13)
    %127 = llvm.bitcast %126 : vector<1xf32> to i32 loc(#loc13)
    %128 = llvm.and %0, %34  : i1 loc(#loc13)
    %129 = llvm.insertelement %121, %37[%7 : i32] : vector<4xi32> loc(#loc13)
    %130 = llvm.insertelement %123, %129[%6 : i32] : vector<4xi32> loc(#loc13)
    %131 = llvm.insertelement %125, %130[%4 : i32] : vector<4xi32> loc(#loc13)
    %132 = llvm.insertelement %127, %131[%3 : i32] : vector<4xi32> loc(#loc13)
    llvm.cond_br %128, ^bb11, ^bb12 loc(#loc13)
  ^bb11:  // pred: ^bb10
    llvm.store %132, %105 : vector<4xi32>, !llvm.ptr<1> loc(#loc13)
    llvm.br ^bb12 loc(#loc13)
  ^bb12:  // 2 preds: ^bb10, ^bb11
    llvm.return loc(#loc14)
  } loc(#loc)
} loc(#loc)
#loc1 = loc(unknown)
#loc2 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":45:41)
#loc3 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":39:24)
#loc4 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":44:24)
#loc5 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":45:28)
#loc6 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":47:21)
#loc7 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":50:24)
#loc9 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":51:24)
#loc11 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":52:17)
#loc12 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":54:26)
#loc13 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":54:35)
#loc14 = loc("/home/triton_llvm_target/intel-xpu-backend-for-triton/python/tutorials/01-vector-add.py":54:4)

------------------
; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; Function Attrs: nofree nosync nounwind memory(argmem: readwrite)
define spir_kernel void @add_kernel_0d1d2d3de(ptr addrspace(1) nocapture readonly %0, ptr addrspace(1) nocapture readonly %1, ptr addrspace(1) nocapture writeonly %2, i32 %3) local_unnamed_addr #0 !intel_reqd_sub_group_size !7 !max_work_group_size !8 {
  %5 = tail call i16 @llvm.pisa.localid.x()
  %6 = shl i16 %5, 2
  %7 = and i16 %6, 508
  %8 = zext nneg i16 %7 to i32
  %9 = tail call i32 @llvm.pisa.groupid.x()
  %10 = shl i32 %9, 10
  %11 = or disjoint i32 %10, %8
  %12 = or disjoint i32 %11, 512
  %13 = icmp slt i32 %11, %3
  %14 = icmp slt i32 %12, %3
  %15 = sext i32 %12 to i64
  %16 = getelementptr float, ptr addrspace(1) %0, i64 %15
  br i1 %13, label %17, label %21

17:                                               ; preds = %4
  %18 = sext i32 %11 to i64
  %19 = getelementptr float, ptr addrspace(1) %0, i64 %18
  %20 = load <4 x float>, ptr addrspace(1) %19, align 16
  br label %21

21:                                               ; preds = %17, %4
  %bc3 = phi <4 x float> [ %20, %17 ], [ undef, %4 ]
  br i1 %14, label %22, label %24

22:                                               ; preds = %21
  %23 = load <4 x float>, ptr addrspace(1) %16, align 16
  br label %24

24:                                               ; preds = %22, %21
  %bc6 = phi <4 x float> [ %23, %22 ], [ undef, %21 ]
  %25 = getelementptr float, ptr addrspace(1) %1, i64 %15
  br i1 %13, label %26, label %30

26:                                               ; preds = %24
  %27 = sext i32 %11 to i64
  %28 = getelementptr float, ptr addrspace(1) %1, i64 %27
  %29 = load <4 x float>, ptr addrspace(1) %28, align 16
  br label %30

30:                                               ; preds = %26, %24
  %bc9 = phi <4 x float> [ %29, %26 ], [ undef, %24 ]
  br i1 %14, label %31, label %33

31:                                               ; preds = %30
  %32 = load <4 x float>, ptr addrspace(1) %25, align 16
  br label %33

33:                                               ; preds = %31, %30
  %bc12 = phi <4 x float> [ %32, %31 ], [ undef, %30 ]
  %34 = fadd <4 x float> %bc6, %bc12
  %35 = fadd <4 x float> %bc6, %bc12
  %36 = fadd <4 x float> %bc6, %bc12
  %37 = fadd <4 x float> %bc6, %bc12
  %38 = getelementptr float, ptr addrspace(1) %2, i64 %15
  br i1 %13, label %39, label %49

39:                                               ; preds = %33
  %40 = fadd <4 x float> %bc3, %bc9
  %41 = fadd <4 x float> %bc3, %bc9
  %42 = fadd <4 x float> %bc3, %bc9
  %43 = shufflevector <4 x float> %41, <4 x float> %42, <4 x i32> <i32 0, i32 5, i32 poison, i32 poison>
  %44 = fadd <4 x float> %bc3, %bc9
  %45 = shufflevector <4 x float> %43, <4 x float> %44, <4 x i32> <i32 0, i32 1, i32 6, i32 poison>
  %46 = shufflevector <4 x float> %45, <4 x float> %40, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %47 = sext i32 %11 to i64
  %48 = getelementptr float, ptr addrspace(1) %2, i64 %47
  store <4 x float> %46, ptr addrspace(1) %48, align 16
  br label %49

49:                                               ; preds = %39, %33
  br i1 %14, label %50, label %54

50:                                               ; preds = %49
  %51 = shufflevector <4 x float> %34, <4 x float> %35, <4 x i32> <i32 0, i32 5, i32 poison, i32 poison>
  %52 = shufflevector <4 x float> %51, <4 x float> %36, <4 x i32> <i32 0, i32 1, i32 6, i32 poison>
  %53 = shufflevector <4 x float> %52, <4 x float> %37, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  store <4 x float> %53, ptr addrspace(1) %38, align 16
  br label %54

54:                                               ; preds = %50, %49
  ret void
}

; Function Attrs: nofree nosync nounwind memory(none)
declare spir_func i16 @llvm.pisa.localid.x() #1

; Function Attrs: nofree nosync nounwind memory(none)
declare spir_func i32 @llvm.pisa.groupid.x() #1

attributes #0 = { nofree nosync nounwind memory(argmem: readwrite) }
attributes #1 = { nofree nosync nounwind memory(none) }

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