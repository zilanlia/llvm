//===-- pISAInputTranslator.cpp - translate input to pISA -*- C++ -*-------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This is a temporary pass that will translate various input IR from
// clang/IGC into a form suitable for pISA consumption.
//
//===----------------------------------------------------------------------===//

#include "pISA.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicspISA.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

namespace llvm {
void initializepISAInputTranslatorPass(PassRegistry &);
} // namespace llvm

namespace {
class pISAInputTranslator
    : public ModulePass,
      public InstVisitor<pISAInputTranslator> {

  IRBuilder<> *IRB = nullptr;
  bool Changed = false;
public:
  static char ID;
  pISAInputTranslator() : ModulePass(ID) {
    initializepISAInputTranslatorPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
  }

  void visitCallInst(CallInst& CI);

private:
  void globalToAlloca(Module& M);
  void rewriteRaytracingShaders(Module& M);
  MDNode* getFuncMd(const Module& M) const;
  std::optional<CallingConv::ID> computeCallingConv(const Function& F) const;
  Value* createLocalId(Value* Idx, IRBuilder<> &IRB);
  Value* createLocalSize(Value* Idx, IRBuilder<> &IRB);
  Value* createGroupId(Value* Idx, IRBuilder<> &IRB);
  Value* createGroupCount(Value* Idx, IRBuilder<> &IRB);
};
} // namespace

char pISAInputTranslator::ID = 0;

INITIALIZE_PASS(pISAInputTranslator, "pisa-input-translator",
                                     "pISA translate input IR",
                false, false)

Value* pISAInputTranslator::createLocalId(Value* Idx, IRBuilder<> &IRB) {
  Value* NewVal = IRB.CreateIntrinsic(Intrinsic::pisa_localid, {}, {});
  return IRB.CreateExtractElement(NewVal, Idx);
}

Value* pISAInputTranslator::createLocalSize(Value* Idx, IRBuilder<>& IRB) {
  Value* NewVal = IRB.CreateIntrinsic(Intrinsic::pisa_localsize, {}, {});
  return IRB.CreateExtractElement(NewVal, Idx);
}

Value* pISAInputTranslator::createGroupId(Value* Idx, IRBuilder<>& IRB) {
  Value* NewVal = IRB.CreateIntrinsic(Intrinsic::pisa_groupid, {}, {});
  return IRB.CreateExtractElement(NewVal, Idx);
}

Value* pISAInputTranslator::createGroupCount(Value* Idx, IRBuilder<>& IRB) {
  Value* NewVal = IRB.CreateIntrinsic(Intrinsic::pisa_groupcount, {}, {});
  return IRB.CreateExtractElement(NewVal, Idx);
}

enum SGVUsage {
  // This is just copied from IGC. If any of these numbers get out of sync,
  // you'll need to update them here.
  THREAD_GROUP_ID_X             = 14,
  THREAD_GROUP_ID_Y             = 15,
  THREAD_GROUP_ID_Z             = 16,
  THREAD_ID_WITHIN_THREAD_GROUP = 51,
  SHADER_TYPE                   = 55,
};

enum LSC_SCOPE {
  // This is just copied from IGC. If any of these numbers get out of sync,
  // you'll need to update them here.
  LSC_SCOPE_GROUP,  // .group  (thread group)
  LSC_SCOPE_LOCAL,  // .local (dss?)
  LSC_SCOPE_TILE,   // .tile
  LSC_SCOPE_GPU,    // .gpu
  LSC_SCOPE_GPUS,   // .gpus
  LSC_SCOPE_SYSREL, // .sysrel
  LSC_SCOPE_SYSACQ, // .sysacq
};

void pISAInputTranslator::visitCallInst(CallInst& CI) {
  auto* F = CI.getCalledFunction();
  if (!F)
    return;

  IRBuilder<> IRB(&CI);
  Value* NewVal = nullptr;

  if (F->getName() == "_Z12get_local_idj") {
    NewVal = IRB.CreateZExt(
      createLocalId(CI.getArgOperand(0), IRB),
      CI.getType());
  }
  else if (F->getName() == "_Z14get_local_sizej") {
    NewVal = createLocalSize(CI.getArgOperand(0), IRB);
    NewVal = IRB.CreateZExt(NewVal, CI.getType());
  }
  else if (F->getName() == "_Z12get_group_idj") {
    NewVal = createGroupId(CI.getArgOperand(0), IRB);
    NewVal = IRB.CreateZExt(NewVal, CI.getType());
  }
  else if (F->getName() == "_Z14get_num_groupsj") {
    NewVal = createGroupCount(CI.getArgOperand(0), IRB);
    NewVal = IRB.CreateZExt(NewVal, CI.getType());
  }
  else if (F->getName() == "_Z13get_global_idj") {
    // groupid * localsize + localid
    Value* Idx = CI.getArgOperand(0);
    NewVal = IRB.CreateAdd(
      IRB.CreateMul(
        createGroupId(Idx, IRB),
        IRB.CreateZExt(createLocalSize(Idx, IRB), IRB.getInt32Ty())),
      IRB.CreateZExt(createLocalId(Idx, IRB), IRB.getInt32Ty()));
    NewVal = IRB.CreateZExt(NewVal, CI.getType());
  }
  else if (F->getName() == "_Z7barrierj") {
    // TODO: also add a fence according to flags
    NewVal = IRB.CreateIntrinsic(Intrinsic::pisa_workgroup_barrier, {}, {});
  }
  else if (F->getName() == "_Z3madfff") {
    NewVal = IRB.CreateIntrinsic(Intrinsic::fma, CI.getType(),
      { CI.getArgOperand(0), CI.getArgOperand(1), CI.getArgOperand(2) });
  }
  else if (F->getName().startswith("__imf_umulhi")) {
    Value* lhs = IRB.CreateSExt(CI.getArgOperand(0), IRB.getInt64Ty());
    Value* rhs = IRB.CreateSExt(CI.getArgOperand(1), IRB.getInt64Ty());
    Value* product64b = IRB.CreateMul(lhs, rhs);
    Value* productHigh32b = IRB.CreateLShr(product64b, IRB.getInt64(32));
    NewVal = IRB.CreateTrunc(productHigh32b, IRB.getInt32Ty());
  }
  else if (F->getName().startswith("__imf_rsqrtf")) {
    NewVal = IRB.CreateIntrinsic(Intrinsic::sqrt, CI.getType(),
      { CI.getArgOperand(0)});
    Type *FltTy = IRB.getFloatTy();
    Value *Val = ConstantFP::get(FltTy, 1.0);
    NewVal = IRB.CreateFDiv(Val, NewVal);
  }
  else if (F->getName() == "llvm.genx.GenISA.PreemptionDisable") {
    // The idea is that the finalizer should be taking care of preemption
    // controls. So we don't currently expose it in pISA.
    CI.eraseFromParent();
    Changed = true;
  }
  else if (F->getName().startswith("llvm.genx.GenISA.GlobalBufferPointer.")) {
    NewVal = IRB.CreateIntrinsic(Intrinsic::pisa_rt_globalpointer, {}, {});
  }
  else if (F->getName().startswith("llvm.genx.GenISA.ptr.to.pair.")) {
    auto* P2I = IRB.CreatePtrToInt(CI.getArgOperand(0), IRB.getInt64Ty());
    auto* Lo = IRB.CreateTrunc(P2I, IRB.getInt32Ty());
    auto* Shft = IRB.CreateLShr(P2I, 32);
    auto* Hi = IRB.CreateTrunc(Shft, IRB.getInt32Ty());
    NewVal = IRB.CreateInsertValue(PoisonValue::get(CI.getType()), Lo, 0);
    NewVal = IRB.CreateInsertValue(NewVal, Hi, 1);
  }
  else if (F->getName() == "llvm.genx.GenISA.AsyncStackID") {
    NewVal = IRB.CreateIntrinsic(Intrinsic::pisa_rt_async_stackid, {}, {});
  }
  else if (F->getName() == "llvm.genx.GenISA.StackIDRelease") {
    // The second arg should always be true. It is for predication on DG2 only.
    assert(cast<ConstantInt>(CI.getArgOperand(1))->getZExtValue());
    NewVal = IRB.CreateIntrinsic(Intrinsic::pisa_stackid_release, {},
      CI.getArgOperand(0));
  }
  else if (F->getName().startswith("llvm.genx.GenISA.TraceRayAsync.")) {
    NewVal = IRB.CreateIntrinsic(Intrinsic::pisa_rt_trace_async, {},
      { CI.getArgOperand(0), CI.getArgOperand(1) });
  }
  else if (F->getName() == "llvm.genx.GenISA.TileXOffset") {
    // These were for a subtile experiment that we don't currently use.
    assert(cast<ConstantInt>(CI.getArgOperand(2))->getZExtValue() == 0);
    assert(cast<ConstantInt>(CI.getArgOperand(3))->getZExtValue() == 0);
    auto* TID = CI.getArgOperand(0);
    uint64_t XDim = cast<ConstantInt>(CI.getArgOperand(1))->getZExtValue();
    constexpr uint32_t NumLanes = pISA::SIMD_SIZE;
    auto* LaneId = IRB.CreateIntrinsic(Intrinsic::pisa_laneid, {}, {});
    if (XDim >= NumLanes) {
      auto* XCnt = IRB.CreateAnd(TID, IRB.getInt16(XDim / NumLanes - 1));
      NewVal = IRB.CreateAdd(
        LaneId, IRB.CreateMul(XCnt, IRB.getInt16(NumLanes)));
    }
    else {
      NewVal = IRB.CreateAnd(LaneId, IRB.getInt16(XDim - 1));
    }
  }
  else if (F->getName() == "llvm.genx.GenISA.TileYOffset") {
    // These were for a subtile experiment that we don't currently use.
    assert(cast<ConstantInt>(CI.getArgOperand(2))->getZExtValue() == 0);
    assert(cast<ConstantInt>(CI.getArgOperand(3))->getZExtValue() == 0);
    auto* TID = CI.getArgOperand(0);
    uint64_t XDim = cast<ConstantInt>(CI.getArgOperand(1))->getZExtValue();
    constexpr uint32_t NumLanes = pISA::SIMD_SIZE;
    if (XDim >= NumLanes) {
      NewVal = IRB.CreateUDiv(TID, IRB.getInt16(XDim / NumLanes));
    }
    else {
      auto* LaneId = IRB.CreateIntrinsic(Intrinsic::pisa_laneid, {}, {});
      auto* YVals = IRB.CreateUDiv(LaneId, IRB.getInt16(XDim));
      NewVal = IRB.CreateAdd(
        YVals, IRB.CreateMul(TID, IRB.getInt16(NumLanes / XDim)));
    }
  }
  else if (F->getName() == "llvm.genx.GenISA.logical.subslice.id") {
    NewVal = IRB.CreateZExt(
      IRB.CreateIntrinsic(Intrinsic::pisa_logicalsubsliceid, {}, {}),
      IRB.getInt32Ty());
  }
  else if (F->getName() == "llvm.genx.GenISA.LSCFence") {
    // TODO: how to model sfid?
    assert(cast<ConstantInt>(CI.getArgOperand(2))->getZExtValue() == 0 &&
      "fence op not yet supported!");
    auto LSCScope = static_cast<LSC_SCOPE>(
      cast<ConstantInt>(CI.getArgOperand(1))->getZExtValue());
    pISA::FenceScope Scope{};
    switch (LSCScope) {
    case LSC_SCOPE_GROUP:
      Scope = pISA::FenceScope::workgroup;
      break;
    case LSC_SCOPE_LOCAL:
      Scope = pISA::FenceScope::local;
      break;
    case LSC_SCOPE_TILE:
      Scope = pISA::FenceScope::tile;
      break;
    case LSC_SCOPE_GPU:
      Scope = pISA::FenceScope::gpu;
      break;
    case LSC_SCOPE_SYSREL:
    case LSC_SCOPE_SYSACQ:
      Scope = pISA::FenceScope::system;
      break;
    case LSC_SCOPE_GPUS:
    default:
      assert(0 && "unknown lsc scope!");
      break;
    }
    NewVal = IRB.CreateIntrinsic(Intrinsic::pisa_fence, {},
      { IRB.getInt32(static_cast<unsigned>(Scope)) });
  }
  else if (F->getName().startswith("llvm.genx.GenISA.DCL.SystemValue.")) {
    auto SGV = static_cast<SGVUsage>(
      cast<ConstantInt>(CI.getArgOperand(0))->getZExtValue());
    switch (SGV) {
    case SHADER_TYPE:
      NewVal = IRB.CreateZExt(
        IRB.CreateIntrinsic(Intrinsic::pisa_rt_shader_type, {}, {}),
        IRB.getInt32Ty());
      break;
    case THREAD_GROUP_ID_X:
      NewVal = IRB.CreateBitCast(
        IRB.CreateZExt(IRB.CreateIntrinsic(Intrinsic::pisa_localid_x, {}, {}),
          IRB.getInt32Ty()),
        CI.getType());
      break;
    case THREAD_GROUP_ID_Y:
      NewVal = IRB.CreateBitCast(
        IRB.CreateZExt(IRB.CreateIntrinsic(Intrinsic::pisa_localid_y, {}, {}),
          IRB.getInt32Ty()),
        CI.getType());
      break;
    case THREAD_GROUP_ID_Z:
      NewVal = IRB.CreateBitCast(
        IRB.CreateZExt(IRB.CreateIntrinsic(Intrinsic::pisa_localid_z, {}, {}),
          IRB.getInt32Ty()),
        CI.getType());
      break;
    case THREAD_ID_WITHIN_THREAD_GROUP:
      NewVal = IRB.CreateTrunc(
        IRB.CreateIntrinsic(Intrinsic::pisa_subgroupid, {}, {}),
        IRB.getInt16Ty());
      break;
    default:
      assert(0 && "unhandled SGV value!");
      break;
    }
  }

  if (NewVal) {
    CI.replaceAllUsesWith(NewVal);
    CI.eraseFromParent();
    Changed = true;
  }
}

static MDNode* findNode(StringRef Name, const MDNode* Parent) {
  if (!Parent)
    return nullptr;
  for (uint32_t i = 0; i < Parent->getNumOperands(); i++) {
    auto* CurNode = dyn_cast<MDNode>(Parent->getOperand(i));
    if (!CurNode)
      continue;
    if (CurNode->getNumOperands() == 0)
      continue;
    auto* NodeName = dyn_cast<MDString>(CurNode->getOperand(0));
    if (!NodeName)
      continue;
    if (NodeName->getString() == Name)
      return CurNode;
  }
  return nullptr;
}

MDNode* pISAInputTranslator::getFuncMd(const Module& M) const {
  auto* MD = M.getNamedMetadata("IGCMetadata");
  if (!MD)
    return nullptr;
  auto* ModuleMD = MD->getOperand(0);
  MDNode* FuncMd = findNode("FuncMD", ModuleMD);
  assert(FuncMd && "No FuncMD?");
  return FuncMd;
}

std::optional<CallingConv::ID>
pISAInputTranslator::computeCallingConv(const Function& F) const {
  // Trace through the IGC metadata to extract the shader type. The path
  // looks something like (there maybe other shaders and fields within any
  // piece of metdata):
  //
  // !IGCMetadata = !{!1}
  // !1 = !{!"ModuleMD", !2}
  // !2 = !{!"FuncMD", !3, !4}
  // !3 = !{!"FuncMDMap[0]", void (<8 x i32>)* @"\01?MyRaygenShader@@YAXXZ"}
  // !4 = !{!"FuncMDValue[0]", !5}
  // !5 = !{!"rtInfo", !6}
  // !6 = !{!"callableShaderType", !"RayGen"}
  auto* MD = getFuncMd(*F.getParent());
  if (!MD)
    return std::nullopt;
  unsigned NumFuncs = (MD->getNumOperands() - 1) / 2;
  MDNode* ValueMD = nullptr;
  for (uint32_t i = 0; i < NumFuncs; i++) {
    std::string Key = "FuncMDMap[" + std::to_string(i) + "]";
    auto* CurNode = findNode(Key, MD);
    assert(CurNode && "missing entry?");
    if (cast<ValueAsMetadata>(CurNode->getOperand(1))->getValue() == &F) {
      ValueMD = cast<MDNode>(MD->getOperand(2 * (i + 1)));
      break;
    }
  }
  if (!ValueMD)
    return std::nullopt;

  auto* RtInfoMD = findNode("rtInfo", ValueMD);
  assert(RtInfoMD && "missing rtInfo?");

  auto* CSTMD = findNode("callableShaderType", RtInfoMD);
  assert(CSTMD && "missing rtInfo?");

  StringRef ShaderType = cast<MDString>(CSTMD->getOperand(1))->getString();

  if (ShaderType == "RayGen")
    return CallingConv::PISA_RT_RAYGEN;
  if (ShaderType == "AnyHit")
    return CallingConv::PISA_RT_DISPATCH;
  if (ShaderType == "ClosestHit")
    return CallingConv::PISA_RT_DISPATCH;
  if (ShaderType == "Intersection")
    return CallingConv::PISA_RT_DISPATCH;
  if (ShaderType == "Miss")
    return CallingConv::PISA_RT_DISPATCH;
  if (ShaderType == "Callable")
    return CallingConv::PISA_RT_DISPATCH;
  if (ShaderType == "CallStackHandler")
    return CallingConv::PISA_RT_DISPATCH;

  return std::nullopt;
}

void pISAInputTranslator::rewriteRaytracingShaders(Module& M) {
  // IGC will often add implicit args to the shaders such as:
  //
  // define void @Miss(%struct.RayPayload addrspace(1)*, <8 x i32> %r0, i8* %privateBase)
  //
  // The first payload argument is rewritten to read from the SWStack, so it
  // isn't used. `%r0` and `%privateBase` are implicit args that get added that
  // we don't end up using. Here, we removes those arguments from the shader.
  // In addition, for the raygen shader case, it will make references to
  // inline data via:
  //
  // %inlineData = call align 256 i8 addrspace(2)* @llvm.genx.GenISA.InlinedData.p2i8(i32 1)
  //
  // pISA has no special means of identifying what is to be loaded by inline,
  // data. Instead, These intrinsic references are lifted into function
  // arguments that will ultimately be read via `ld.param`.

  SmallVector<Function*> RTShaders;

  for (auto& F : M) {
    auto CC = computeCallingConv(F);
    if (CC == CallingConv::PISA_RT_DISPATCH ||
        CC == CallingConv::PISA_RT_RAYGEN) {
      F.setCallingConv(*CC);
      RTShaders.push_back(&F);
    }
  }

  for (auto *F : RTShaders) {
    assert(llvm::all_of(F->args(), [](auto& Arg) { return Arg.use_empty(); }));
    std::map<unsigned, SmallVector<Instruction*>> InlineArgs;

    if (F->getCallingConv() == CallingConv::PISA_RT_RAYGEN) {
      for (auto& I : instructions(F)) {
        if (auto* II = dyn_cast<IntrinsicInst>(&I)) {
          StringRef FnName = II->getCalledFunction()->getName();
          if (FnName.startswith("llvm.genx.GenISA.InlinedData.")) {
            uint64_t Offset =
              cast<ConstantInt>(II->getArgOperand(0))->getZExtValue();
            InlineArgs[Offset].push_back(II);
          }
        }
      }
    }

    Changed = true;

    SmallVector<Type*> ArgTys;
    for (auto &[_, Insts] : InlineArgs)
      ArgTys.push_back(Insts[0]->getType());

    FunctionType* FTy = FunctionType::get(
      F->getFunctionType()->getReturnType(),
      ArgTys,
      false);

    Function* NewF = Function::Create(
      FTy, F->getLinkage(), F->getAddressSpace(), F->getName(), F->getParent());

    for (auto [Idx, Arg] : llvm::enumerate(NewF->args()))
      Arg.setName("Arg" + std::to_string(Idx));

    ValueToValueMapTy VMap;

    auto* CurArg = NewF->arg_begin();
    for (auto& [_, Insts] : InlineArgs) {
      for (auto* I : Insts)
        VMap[I] = CurArg;
      CurArg++;
    }

    NewF->splice(NewF->end(), F);
    RemapFunction(
      *NewF, VMap, RF_IgnoreMissingLocals | RF_ReuseAndMutateDistinctMDs);

    for (auto& [_, Insts] : InlineArgs) {
      for (auto* I : Insts) {
        assert(I->use_empty());
        I->eraseFromParent();
      }
    }

    NewF->takeName(F);
    NewF->copyMetadata(F, 0);
    NewF->copyAttributesFrom(F);
    // After copying the attributes, we've also copied the param attributes.
    // Given that we've removed all the original params, we need to remove the
    // parameter attributes to satisfy the verifier.
    auto AL = NewF->getAttributes();
    // for (uint32_t i = 0; i < F->getFunctionType()->getNumParams(); i++)
    //   NewF->removeParamAttrs(i, AL.getParamAttrs(i));
    NewF->setSubprogram(F->getSubprogram());
    NewF->setCallingConv(F->getCallingConv());
    F->replaceAllUsesWith(NewF);
    F->eraseFromParent();
  }
}

void pISAInputTranslator::globalToAlloca(Module& M) {
  // clang will typically generate shared variable declarations as global
  // variables. For example, an OpenCL kernel that does this:
  //
  // __local unsigned char sub[SUB_SIZE][SUB_SIZE];
  //
  // Will generate:
  //
  // @filterImageWithCache.sub = internal unnamed_addr addrspace(3) global [16 x [16 x i8]] undef, align 1
  //
  // Since we expect locals to be allocated using `alloca`, we set these here
  IRBuilder<> IRB(M.getContext());
  for (auto& G : make_early_inc_range(M.globals())) {
    if (G.getAddressSpace() != (unsigned)pISA::AddressSpace::SHARED)
      continue;

    DenseMap<Function*, AllocaInst*> Map;
    SmallVector<User*> Users{ G.user_begin(), G.user_end() };

    for (auto* U : Users) {
      auto* I = dyn_cast<Instruction>(U);
      if (!I)
        continue;

      Changed = true;

      auto* CurF = I->getFunction();

      auto& AI = Map[CurF];
      if (!AI) {
        IRB.SetInsertPoint(&CurF->getEntryBlock().front());
        AI = IRB.CreateAlloca(
          G.getValueType(), (unsigned)pISA::AddressSpace::SHARED);
        if (auto Align = G.getAlign())
          AI->setAlignment(*Align);
        AI->setName(G.getName());
      }

      U->replaceUsesOfWith(&G, AI);
    }

    if (G.use_empty())
      G.eraseFromParent();
  }
}

bool pISAInputTranslator::runOnModule(Module &M) {
  Changed = false;

  globalToAlloca(M);
  rewriteRaytracingShaders(M);
  visit(M);

  if (auto* MD = M.getNamedMetadata("IGCMetadata"))
    MD->eraseFromParent();

  if (auto* MD = M.getNamedMetadata("igc.functions"))
    MD->eraseFromParent();

  return Changed;
}

ModulePass *llvm::createpISAInputTranslatorPass() {
  return new pISAInputTranslator();
}
