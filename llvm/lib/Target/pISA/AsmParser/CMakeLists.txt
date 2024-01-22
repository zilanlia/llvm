add_llvm_component_library(LLVMpISAAsmParser
  pISAAsmParser.cpp

  LINK_COMPONENTS
  pISADesc
  pISAInfo
  Core
  MC
  MCParser
  Support

  ADD_TO_COMPONENT
  pISA
)
