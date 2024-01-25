add_llvm_component_group(pISA)

set(LLVM_TARGET_DEFINITIONS pISA.td)

tablegen(LLVM pISAGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM pISAGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM pISAGenGlobalISel.inc -gen-global-isel -warn-on-skipped-patterns)
tablegen(LLVM pISAGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM pISAGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM pISAGenRegisterBank.inc -gen-register-bank)
tablegen(LLVM pISAGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM pISAGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM pISAGenTables.inc -gen-searchable-tables)
#tablegen(LLVM pISAGenPreLegalizeGICombiner.inc -gen-global-isel-combiner
#              -combiners="pISAPreLegalizerCombinerHelper")
tablegen(LLVM pISAGenPostLegalizeGICombiner.inc -gen-global-isel-combiner
              -combiners="pISAPostLegalizerCombiner")

add_public_tablegen_target(pISACommonTableGen)

add_llvm_target(pISACodeGen
  pISAAsmPrinter.cpp
  pISACallLowering.cpp
  pISADebugInfo.cpp
  pISADetermineStackID.cpp
  pISAEmitIntrinsics.cpp
  pISAExpandIntrinsics.cpp
  pISAInputTranslator.cpp
  pISAInstrInfo.cpp
  pISAInstructionSelector.cpp
  pISAISelLowering.cpp
  pISALegalizerInfo.cpp
  pISALegalizeSubregAccess.cpp
  pISAMCInstLower.cpp
  pISAPostLegalizerCombiner.cpp
  #pISAPreLegalizerCombiner.cpp
  pISARegisterBankInfo.cpp
  pISARegisterInfo.cpp
  pISARegManager.cpp
  pISASubtarget.cpp
  pISATargetMachine.cpp
  pISAUtils.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  Core
  Demangle
  GlobalISel
  MC
  pISADesc
  pISAInfo
  SelectionDAG
  Support
  Target
  TargetParser
  TransformUtils

  ADD_TO_COMPONENT
  pISA
  )

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
add_subdirectory(AsmParser)
