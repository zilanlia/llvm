add_llvm_component_library(LLVMpISAInfo
  pISATargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  pISA
  )
