add_llvm_component_library(LLVMpISADesc
  pISAMCAsmInfo.cpp
  pISAMCTargetDesc.cpp
  pISATargetStreamer.cpp
  pISAInstPrinter.cpp
  pISAMCExpr.cpp
  pISARegEncoder.cpp

  LINK_COMPONENTS
  Core
  MC
  pISAInfo
  Support
  TargetParser

  ADD_TO_COMPONENT
  pISA
  )
