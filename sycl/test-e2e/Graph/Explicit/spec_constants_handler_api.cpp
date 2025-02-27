// RUN: %{build} -o %t.out
// RUN: %{run} %t.out
// Extra run to check for leaks in Level Zero using UR_L0_LEAKS_DEBUG
// RUN: %if level_zero %{env UR_L0_LEAKS_DEBUG=1 %{run} %t.out 2>&1 | FileCheck %s %}
//
// CHECK-NOT: LEAK

// The following limitation is not restricted to Sycl-Graph
// but comes from the orignal test : `SpecConstants/2020/handler-api.cpp`
// FIXME: ACC devices use emulation path, which is not yet supported
// UNSUPPORTED: accelerator

#define GRAPH_E2E_EXPLICIT

#include "../Inputs/spec_constants_handler_api.cpp"
