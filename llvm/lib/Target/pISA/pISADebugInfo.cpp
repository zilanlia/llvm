//===-- pISADebugInfo.cpp - pISA LLVM assembly writer ------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains implementation to dump out debug info metadata nodes
// to pISA assembly.
//
//===----------------------------------------------------------------------===//

#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/IR/Metadata.h"
#include "MCTargetDesc/pISARegEncoder.h"
#include "pISAMCInstLower.h"
#include "pISADebugInfo.h"

using namespace llvm;

#define DEBUG_TYPE "pisa-debug-info-printer"

unsigned int DebugInfoMetadata::getOrCreateId(const llvm::MDNode* MD) {
  auto IdLookup = getId(MD);
  if (IdLookup != std::numeric_limits<unsigned int>::max())
    return IdLookup;

  auto NextId = getNextId();
  addMD(MD, NextId);

  return NextId;
}

void DebugInfoMetadata::checkAndEmitField(StringRef Field,
                                          const llvm::MDNode *MD,
                                          raw_ostream &OS) {
  if (MD) {
    auto ScopeId = getOrCreateId(MD);
    OS << Field << ScopeId;
  }
}

std::string toString_Flags(DINode::DIFlags Flags) {
  std::string Str;
  raw_string_ostream OS(Str);
  unsigned int PrevSize = 0;

  auto DelimNeeded = [&PrevSize, &Str]() {
    if (PrevSize == Str.size())
      return false;
    PrevSize = Str.size();
    return true;
  };

  const std::vector<std::pair<DINode::DIFlags, std::string>> SPFlagStr = {
      {DINode::DIFlags::FlagFwdDecl, "DIFlagFwdDecl"},
      {DINode::DIFlags::FlagVirtual, "DIFlagVirtual"},
      {DINode::DIFlags::FlagArtificial, "DIFlagArtificial"},
      {DINode::DIFlags::FlagExplicit, "DIFlagExplicit"},
      {DINode::DIFlags::FlagPrototyped, "DIFlagPrototyped"},
      {DINode::DIFlags::FlagObjectPointer, "DIFlagObjectPointer"},
      {DINode::DIFlags::FlagVector, "DIFlagVector"},
      {DINode::DIFlags::FlagStaticMember, "DIFlagStaticMember"},
      {DINode::DIFlags::FlagLValueReference, "DIFlagLValueReference"},
      {DINode::DIFlags::FlagRValueReference, "DIFlagRValueReference"},
      {DINode::DIFlags::FlagBitField, "DIFlagBitField"},
      {DINode::DIFlags::FlagNoReturn, "DIFlagNoReturn"},
      {DINode::DIFlags::FlagTypePassByValue, "DIFlagTypePassByValue"},
      {DINode::DIFlags::FlagTypePassByReference, "DIFlagTypePassByReference"},
      {DINode::DIFlags::FlagEnumClass, "DIFlagEnumClass"}};

  // Check these separately as they've bit-overlap
  if ((Flags & DINode::DIFlags::FlagPublic) == DINode::DIFlags::FlagPublic)
    OS << (DelimNeeded() ? "|" : "") << "DIFlagPublic";
  else if ((Flags & DINode::DIFlags::FlagProtected) ==
           DINode::DIFlags::FlagProtected)
    OS << (DelimNeeded() ? "|" : "") << "DIFlagProtected";
  else if ((Flags & DINode::DIFlags::FlagPrivate) ==
           DINode::DIFlags::FlagPrivate)
    OS << (DelimNeeded() ? "|" : "") << "DIFlagPrivate";

  for (const auto &P : SPFlagStr) {
    if ((Flags & P.first) == P.first)
      OS << (DelimNeeded() ? "|" : "") << P.second;
  }

  return Str;
}

std::string toString_SPFlags(DISubprogram::DISPFlags SPFlags) {
  std::string Str;
  raw_string_ostream OS(Str);
  unsigned int PrevSize = 0;

  auto DelimNeeded = [&PrevSize, &Str]() {
    if (PrevSize == Str.size())
      return false;
    PrevSize = Str.size();
    return true;
  };

  const std::vector<std::pair<DISubprogram::DISPFlags, std::string>> SPFlagStr =
      {{DISubprogram::DISPFlags::SPFlagVirtual, "DISPFlagVirtual"},
       {DISubprogram::DISPFlags::SPFlagPureVirtual, "DISPFlagPureVirtual"},
       {DISubprogram::DISPFlags::SPFlagLocalToUnit, "DISPFlagLocalToUnit"},
       {DISubprogram::DISPFlags::SPFlagDefinition, "DISPFlagDefinition"},
       {DISubprogram::DISPFlags::SPFlagOptimized, "DISPFlagOptimized"},
       {DISubprogram::DISPFlags::SPFlagPure, "DISPPure"},
       {DISubprogram::DISPFlags::SPFlagElemental, "DISPElemental"},
       {DISubprogram::DISPFlags::SPFlagRecursive, "DISPRecursive"},
       {DISubprogram::DISPFlags::SPFlagMainSubprogram, "DISPMainSubprogram"}};

  for (const auto &P : SPFlagStr) {
    if ((SPFlags & P.first) == P.first)
      OS << (DelimNeeded() ? "|" : "") << P.second;
  }

  return Str;
}

void DebugInfoMetadata::emit_MDTuple(
    const llvm::MDTuple *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto isDistinct = MD->isDistinct() ? "distinct " : "";
  OS << "!" << Id << " = " << isDistinct << "!{";

  bool NonEmpty = false;
  auto NumOpnds = MD->getNumOperands();
  for (unsigned int i = 0; i != NumOpnds; ++i) {
    const auto &Opnd = MD->getOperand(i);
    if (!Opnd) {
      if (NonEmpty)
        OS << ", ";
      OS << "null";
      NonEmpty = true;
      continue;
    }
    if (isa<const MDNode>(Opnd.get())) {
      const auto *M = cast<const MDNode>(Opnd.get());
      auto OpndId = getOrCreateId(M);
      if (NonEmpty)
        OS << ", ";
      OS << "!" << OpndId;
      NonEmpty = true;
    }
  }

  OS << "}";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::toString(const llvm::DIExpression *MD,
                                 llvm::raw_ostream &OS) {
  if (!MD->isValid())
    return;

  std::string ExprStr;
  raw_string_ostream ExprStrOS(ExprStr);
  MD->print(ExprStrOS);

  // replace any occurences of _LLVM_ with _PISA_
  size_t pos = 0;
  std::string SearchFor = "_LLVM_";
  std::string ReplaceWith = "_PISA_";
  while ((pos = ExprStr.find(SearchFor, pos)) != std::string::npos) {
    ExprStr.replace(pos, SearchFor.length(), ReplaceWith);
    pos += ReplaceWith.length();
  }
  
  OS << ExprStr;
}

void DebugInfoMetadata::emitDbgDeclare(
    const MachineFunction &MF,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  const auto &DbgDcls = MF.getVariableDbgInfo();
  if (DbgDcls.empty())
    return;

  std::string Str;
  raw_string_ostream OS(Str);

  OS << "\n";
  for (const auto &DbgDcl : DbgDcls) {
    const auto *VarMD = DbgDcl.Var;
    const auto *Expr = DbgDcl.Expr;
    const auto *DbgLoc = DbgDcl.Loc;
    // const auto Slot = DbgDcl.Slot;

    OS << "\tcall void, pisa.dbg.declare(";
    // OS << "@R" << Slot;
    unsigned int Id = getOrCreateId(VarMD);
    OS << ", !" << Id;

    OS << ",";
    toString(cast<const llvm::DIExpression>(Expr), OS);

    OS << ")";

    if (DbgLoc) {
      unsigned int Id = getOrCreateId(DbgLoc);
      OS << " !dbg !" << std::to_string(Id);
    }
    OS << "\n";
  }

  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emitDbgValue(
    const MachineInstr &MI, const pISAMCInst &Inst,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string InstStr;
  raw_string_ostream OS(InstStr);

  OS << "\tcall void, pisa.dbg.value(";

  // TODO: Handle DIArglist
  auto &Opnd0 = Inst.getOperand(0);
  if (Opnd0.isReg()) {
    if (Opnd0.getReg() == MCRegister::NoRegister)
      OS << "undef";
    else {
      auto [Prefix, Idx] =
          pISA::RegEncoder::decodeVirtualRegister(Opnd0.getReg());
      OS << Prefix << Idx;
    }
  } else if (Opnd0.isImm()) {
    OS << (int64_t)Opnd0.getImm();
  } else if (Opnd0.isExpr()) {
    // TODO: Handle zeroinitializer
    OS << "undef";
  } else {
    llvm_unreachable("unexpected opnd type");
  }

  auto &MD = MI.getOperand(2);
  unsigned int Id = getOrCreateId(MD.getMetadata());
  OS << ", !" << Id;

  auto &Exp = MI.getOperand(3);
  OS << ", ";
  toString(cast<const llvm::DIExpression>(Exp.getMetadata()), OS);

  OS << ");";

  const auto &DbgLoc = MI.getDebugLoc();
  if (DbgLoc) {
    unsigned int Id = getOrCreateId(DbgLoc.getAsMDNode());
    OS << " !dbg !" << std::to_string(Id);
  }

  OutStreamer->emitRawText(InstStr);
}

void DebugInfoMetadata::emit_DIExpression(
    const llvm::DIExpression *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DIExpression(";

  toString(MD, OS);

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DILabel(
    const llvm::DILabel *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer>& OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Scope = MD->getScope();
  auto Name = MD->getName();
  auto File = MD->getFile();
  auto Line = MD->getLine();

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DILabel(";

  OS << "name: \"" << Name << "\"";
  checkAndEmitField(", scope: ", Scope, OS);
  checkAndEmitField(", file: ", File, OS);
  if (Line)
    OS << ", line: " << Line;

  OS << ")";
  OutStreamer->emitRawText(Str);

}

void DebugInfoMetadata::emit_DIMacroFile(
    const llvm::DIMacroFile *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto MacInfo = MD->getMacinfoType();
  auto Line = MD->getLine();
  auto File = MD->getFile();
  auto Elements = MD->getElements();

  std::string MacInfoStr;
  if (MacInfo == dwarf::DW_MACINFO_start_file)
    MacInfoStr = "DW_MACINFO_start_file";
  else if (MacInfo == dwarf::DW_MACINFO_end_file)
    MacInfoStr = "DW_MACINFO_end_file";

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DIMacroFile(";

  OS << "macinfo: " << MacInfoStr;
  if (Line)
    OS << ", line: " << Line;
  checkAndEmitField(", file: ", File, OS);
  checkAndEmitField(", nodes: ", Elements.get(), OS);

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DIMacro(
    const llvm::DIMacro *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto MacInfo = MD->getMacinfoType();
  auto Line = MD->getLine();
  auto Name = MD->getName();
  auto Value = MD->getValue();

  std::string MacInfoStr;
  if (MacInfo == (unsigned int)dwarf::DW_MACINFO_define)
    MacInfoStr = "DW_MACINFO_define";
  else if (MacInfo == (unsigned int)dwarf::DW_MACINFO_undef)
    MacInfoStr = "DW_MACINFO_undef";

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DIMacro(";

  OS << "macinfo: " << MacInfoStr;
  if (Line)
    OS << ", line: " << Line;
  OS << ", name: \"" << Name << "\"";
  OS << ", value: \"" << Value << "\"";

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DIImportedEntity(
    const llvm::DIImportedEntity *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Tag = MD->getTag();
  auto Name = MD->getName();
  auto Scope = MD->getScope();
  auto Entity = MD->getEntity();
  auto Line = MD->getLine();
  auto Elements = MD->getElements();

  std::string TagStr;
  if (Tag == dwarf::DW_TAG_imported_declaration)
    TagStr = "DW_TAG_imported_declaration";
  else if (Tag == dwarf::DW_TAG_imported_module)
    TagStr = "DW_TAG_imported_module";

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DIImportedEntity(";

  OS << "tag: " << TagStr;
  OS << ", name: \"" << Name << "\"";
  checkAndEmitField(", scope: !", Scope, OS);
  checkAndEmitField(", entity: !", Entity, OS);
  if (Line)
    OS << ", line: " << Line;
  checkAndEmitField(", elements: !", Elements.get(), OS);

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DILocalVariable(
    const llvm::DILocalVariable *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Name = MD->getName();
  auto Scope = MD->getScope();
  auto File = MD->getFile();
  auto Line = MD->getLine();
  auto Type = MD->getType();
  auto Flags = MD->getFlags();
  auto Arg = MD->getArg();

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DILocalVariable(";

  OS << "name: \"" << Name << "\"";
  checkAndEmitField(", scope: !", Scope, OS);
  checkAndEmitField(", file: !", File, OS);
  if (Line)
    OS << ", line: " << Line;
  checkAndEmitField(", type: !", Type, OS);

  std::string FlagStr = toString_Flags(Flags);
  if (!FlagStr.empty())
    OS << ", flags: " << FlagStr;

  if (Arg)
    OS << ", arg: " << Arg;

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DILexicalBlockFile(
    const llvm::DILexicalBlockFile *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Scope = MD->getScope();
  auto File = MD->getFile();
  auto Disc = MD->getDiscriminator();

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DILexicalBlock(";

  checkAndEmitField("scope: !", Scope, OS);
  checkAndEmitField(", file: !", File, OS);
  if (Disc)
    OS << ", discriminator: " << Disc;

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DILexicalBlock(
    const llvm::DILexicalBlock *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Scope = MD->getScope();
  auto File = MD->getFile();
  auto Line = MD->getLine();
  auto Col = MD->getColumn();

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DILexicalBlock(";

  checkAndEmitField("scope: !", Scope, OS);
  checkAndEmitField(", file: !", File, OS);
  if (Line)
    OS << ", line: " << Line;
  if (Col)
    OS << ", column: " << Col;

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DIGlobalVariableExpr(
    const llvm::DIGlobalVariableExpression *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Var = MD->getVariable();
  auto Expr = MD->getExpression();

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DIGlobalVariableExpression(";

  checkAndEmitField("var: !", Var, OS);
  checkAndEmitField(", expr: !", Expr, OS);


  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DIGlobalVariable(
    const llvm::DIGlobalVariable *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Name = MD->getName();
  auto LinkageName = MD->getLinkageName();
  auto Scope = MD->getScope();
  auto Type = MD->getType();
  auto File = MD->getFile();
  auto Line = MD->getLine();
  auto IsLocal = MD->isLocalToUnit();
  auto IsDef = MD->isDefinition();
  auto Decl = MD->getStaticDataMemberDeclaration();
  auto TemplateParams = MD->getTemplateParams();

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DIGlobalVariable(";

  OS << "name: \"" << Name << "\"";
  OS << "linkageName: " << LinkageName;
  checkAndEmitField(", scope: !", Scope, OS);
  checkAndEmitField(", type: !", Type, OS);
  checkAndEmitField(", file: !", File, OS);
  if (Line)
    OS << ", line: " << Line;
  OS << ", isLocal: " << (IsLocal ? "true" : "false");
  OS << ", isDefinition: " << (IsDef ? "true" : "false");
  checkAndEmitField(", decl: !", Decl, OS);
  checkAndEmitField(", templateParams: !", TemplateParams, OS);

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DINamespace(
    const llvm::DINamespace *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Name = MD->getName();
  auto File = MD->getFile();
  auto Scope = MD->getScope();

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DINamespace(";

  OS << "name: \"" << Name << "\"";
  checkAndEmitField(", file: !", File, OS);
  checkAndEmitField(", scope: !", Scope, OS);

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DITemplateValueParameter(
    const llvm::DITemplateValueParameter *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Name = MD->getName();
  auto Type = MD->getType();
  auto Value = MD->getValue();
  auto Tag = MD->getTag();

  std::string TagStr = "DW_TAG_template_value_parameter";
  bool DumpTag = true;
  if (Tag == dwarf::DW_TAG_GNU_template_template_param)
    TagStr = "DW_TAG_GNU_template_template_param";
  else if (Tag == dwarf::DW_TAG_GNU_template_parameter_pack)
    TagStr = "DW_TAG_GNU_template_parameter_pack";
  else
    DumpTag = false;

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DITemplateValueParameter(";
  OS << "name: \"" << Name << "\"";
  checkAndEmitField(", type: !", Type, OS);
  if (isa<ConstantAsMetadata>(Value)) {
    const auto &IntValue =
        dyn_cast<ConstantAsMetadata>(Value)->getValue()->getUniqueInteger();
    OS << ", value: "
       << (IntValue.isSignBitSet() ? IntValue.getSExtValue()
                                   : IntValue.getZExtValue());
  }
  if (DumpTag)
    OS << ", tag: " << TagStr;

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DITemplateTypeParameter(
    const llvm::DITemplateTypeParameter *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Name = MD->getName();
  auto Type = MD->getType();

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DITemplateTypeParameter(";
  OS << "name: \"" << Name << "\"";
  checkAndEmitField(", type: !", Type, OS);

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DIEnumerator(
    const llvm::DIEnumerator *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Name = MD->getName();
  auto Value = MD->getValue();
  auto isUnsigned = MD->isUnsigned();

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DIEnumerator(";
  OS << "name: \"" << Name << "\"";
  if (isUnsigned)
    OS << ", value: " << Value.getZExtValue();
  else
    OS << ", value: " << Value.getSExtValue();

  if (isUnsigned)
    OS << ", isUnsigned: true";

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DISubrange(
    const llvm::DISubrange *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Count = MD->getCount();
  auto LowerBound = MD->getLowerBound();

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DISubrange(";
  if (Count.is<ConstantInt *>()) {
    const auto &UniqueInt = Count.get<ConstantInt *>()->getUniqueInteger();
    OS << "count: " << UniqueInt.getSExtValue();
  } else if (Count.is<DIVariable *>()) {
    checkAndEmitField("count: !", Count.get<DIVariable *>(), OS);
  } else if (Count.is<DIExpression *>()) {
    checkAndEmitField("count: !", Count.get<DIExpression *>(), OS);
  }

  if (LowerBound && LowerBound.is<ConstantInt *>()) {
    const auto &UniqueInt = LowerBound.get<ConstantInt *>()->getUniqueInteger();
    if (UniqueInt.getZExtValue() > 0)
      OS << ", lowerBound: !" << UniqueInt.getZExtValue();
  }

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DISubroutineType(
    const llvm::DISubroutineType *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto TypeTuple = MD->getTypeArray();

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DISubroutineType(";
  checkAndEmitField("types: !", TypeTuple.get(), OS);

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DICompositeType(
    const llvm::DICompositeType *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Tag = MD->getTag();
  auto Name = MD->getName();
  auto Elements = MD->getElements();
  auto File = MD->getFile();
  auto Line = MD->getLine();
  auto BaseType = MD->getBaseType();
  auto SizeInBits = MD->getSizeInBits();
  auto Flags = MD->getFlags();
  auto AlignInBits = MD->getAlignInBits();
  auto Identifier = MD->getIdentifier();
  auto DataLocation = MD->getDataLocation();
  auto VTableHolder = MD->getVTableHolder();

  std::string TagStr;
  if (Tag == dwarf::DW_TAG_array_type)
    TagStr = "DW_TAG_array_type";
  else if (Tag == dwarf::DW_TAG_class_type)
    TagStr = "DW_TAG_class_type";
  else if (Tag == dwarf::DW_TAG_enumeration_type)
    TagStr = "DW_TAG_enumeration_type";
  else if (Tag == dwarf::DW_TAG_structure_type)
    TagStr = "DW_TAG_structure_type";
  else if (Tag == dwarf::DW_TAG_union_type)
    TagStr = "DW_TAG_union_type";

  auto isDistinct = MD->isDistinct() ? "distinct " : "";
  OS << "!" << Id << " = " << isDistinct << "!DICompositeType(";
  OS << "tag: " << TagStr;
  OS << ", name: \"" << Name << "\"";
  checkAndEmitField(", elements: !", Elements.get(), OS);
  checkAndEmitField(", file: !", File, OS);
  if (Line)
    OS << ", line: " << Line;
  checkAndEmitField(", baseType: !", BaseType, OS);
  if (SizeInBits)
    OS << ", size: " << SizeInBits;
  auto FlagStr = toString_Flags(Flags);
  if (!FlagStr.empty())
    OS << ", flags: " << FlagStr;
  if (AlignInBits)
    OS << ", align: " << AlignInBits;
  OS << ", identifier: \"" << Identifier << "\"";
  checkAndEmitField(", dataLocation: !", DataLocation, OS);
  checkAndEmitField(", vtableHolder: !", VTableHolder, OS);

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DIDerivedType(
    const llvm::DIDerivedType *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto isDistinct = MD->isDistinct() ? "distinct " : "";
  OS << "!" << Id << " = " << isDistinct << "!DIDerivedType(";

  auto Tag = MD->getTag();
  auto BaseType = MD->getBaseType();
  auto Scope = MD->getScope();
  auto Name = MD->getName();
  auto File = MD->getFile();
  auto Line = MD->getLine();
  auto SizeInBits = MD->getSizeInBits();
  auto AlignInBits = MD->getAlignInBits();
  auto OffsetInBits = MD->getOffsetInBits();
  auto AS = MD->getDWARFAddressSpace();
  auto Flags = MD->getFlags();
  auto ExtraData = MD->getExtraData();

  std::string TagStr;
  if (Tag == dwarf::Tag::DW_TAG_member)
    TagStr = "DW_TAG_member";
  else if (Tag == dwarf::Tag::DW_TAG_pointer_type)
    TagStr = "DW_TAG_pointer_type";
  else if (Tag == dwarf::Tag::DW_TAG_reference_type)
    TagStr = "DW_TAG_reference_type";
  else if (Tag == dwarf::Tag::DW_TAG_typedef)
    TagStr = "DW_TAG_typedef";
  else if (Tag == dwarf::Tag::DW_TAG_inheritance)
    TagStr = "DW_TAG_inheritance";
  else if (Tag == dwarf::Tag::DW_TAG_ptr_to_member_type)
    TagStr = "DW_TAG_ptr_to_member_type";
  else if (Tag == dwarf::Tag::DW_TAG_const_type)
    TagStr = "DW_TAG_const_type";
  else if (Tag == dwarf::Tag::DW_TAG_friend)
    TagStr = "DW_TAG_friend";
  else if (Tag == dwarf::Tag::DW_TAG_volatile_type)
    TagStr = "DW_TAG_volatile_type";
  else if (Tag == dwarf::Tag::DW_TAG_restrict_type)
    TagStr = "DW_TAG_restrict_type";
  else if (Tag == dwarf::Tag::DW_TAG_atomic_type)
    TagStr = "DW_TAG_atomic_type";
  else if (Tag == dwarf::Tag::DW_TAG_immutable_type)
    TagStr = "DW_TAG_immutable_type";

  OS << "tag: " << TagStr;
  checkAndEmitField(", baseType: !", BaseType, OS);
  checkAndEmitField(", scope: !", Scope, OS);
  if (!Name.empty())
    OS << ", name: \"" << Name << "\"";
  checkAndEmitField(", file: !", File, OS);
  if (Line)
    OS << ", line: " << Line;
  if (SizeInBits)
    OS << ", size: " << SizeInBits;
  if (AlignInBits)
    OS << ", align: " << AlignInBits;
  if (OffsetInBits)
    OS << ", offset: " << OffsetInBits;
  if (AS.has_value() && *AS)
    OS << ", addressSpace: " << AS;
  if (ExtraData && isa<const MDNode>(ExtraData)) {
    checkAndEmitField(", extraData: !", cast<const MDNode>(ExtraData), OS);
  }
  auto FlagStr = toString_Flags(Flags);
  if (!FlagStr.empty())
    OS << ", flags: " << FlagStr;

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DIBasicType(
    const llvm::DIBasicType *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Name = MD->getName();
  auto SizeInBits = MD->getSizeInBits();
  auto Tag = MD->getTag();
  auto AlignInBits = MD->getAlignInBits();
  auto Encoding = MD->getEncoding();

  std::string TagStr = "DW_TAG_base_type";
  if (Tag == dwarf::Tag::DW_TAG_unspecified_type)
    TagStr = "DW_TAG_unspecified_type";

  std::string EncodingStr;
  bool HasEncoding = true;
  if (Encoding == dwarf::DW_ATE_boolean)
    EncodingStr = "DW_ATE_boolean";
  else if (Encoding == dwarf::DW_ATE_complex_float)
    EncodingStr = "DW_ATE_complex_float";
  else if (Encoding == dwarf::DW_ATE_float)
    EncodingStr = "DW_ATE_float";
  else if (Encoding == dwarf::DW_ATE_imaginary_float)
    EncodingStr = "DW_ATE_imaginary_float";
  else if (Encoding == dwarf::DW_ATE_signed)
    EncodingStr = "DW_ATE_signed";
  else if (Encoding == dwarf::DW_ATE_signed_char)
    EncodingStr = "DW_ATE_signed_char";
  else if (Encoding == dwarf::DW_ATE_unsigned)
    EncodingStr = "DW_ATE_unsigned";
  else if (Encoding == dwarf::DW_ATE_unsigned_char)
    EncodingStr = "DW_ATE_unsigned_char";
  else
    HasEncoding = false;

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DIBasicType(";
  OS << "name: \"" << Name << "\"";
  if (Tag == dwarf::Tag::DW_TAG_unspecified_type) {
  } else {
    OS << ", size: " << SizeInBits;
    if (AlignInBits)
      OS << ", align: " << AlignInBits;
    if (HasEncoding)
      OS << ", encoding: " << EncodingStr;
  }

  OS << ")";
  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DICompileUnit(
    const llvm::DICompileUnit *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Language = MD->getSourceLanguage();
  auto File = MD->getFile();
  auto Producer = MD->getProducer();
  auto IsOpt = MD->isOptimized();
  auto Flags = MD->getFlags();
  auto RTVer = MD->getRuntimeVersion();
  auto SplitDbgFN = MD->getSplitDebugFilename();
  auto SplitDbgInlining = MD->getSplitDebugInlining();
  auto NameTableKind = MD->getNameTableKind();
  auto EmissionKind = MD->getEmissionKind();
  auto Enums = MD->getEnumTypes();
  auto RetainedTypes = MD->getRetainedTypes();
  auto Globals = MD->getGlobalVariables();
  auto Imports = MD->getImportedEntities();
  auto Macros = MD->getMacros();
  auto DWOId = MD->getDWOId();
  auto DbgInfoForProf = MD->getDebugInfoForProfiling();

  std::string LanguageStr = "Unknown";
  if (Language == dwarf::SourceLanguage::DW_LANG_C99)
    LanguageStr = "DW_LANG_C99";
  else if (Language == dwarf::SourceLanguage::DW_LANG_C_plus_plus)
    LanguageStr = "DW_LANG_C_plus_plus";
  else if (Language == dwarf::SourceLanguage::DW_LANG_Fortran95)
    LanguageStr = "DW_LANG_Fortran95";

  std::string EmissionKindStr;
  if (EmissionKind == DICompileUnit::DebugEmissionKind::NoDebug)
    EmissionKindStr = "NoDebug";
  else if (EmissionKind == DICompileUnit::DebugEmissionKind::LineTablesOnly)
    EmissionKindStr = "LineTablesOnly";
  else if (EmissionKind == DICompileUnit::DebugEmissionKind::FullDebug)
    EmissionKindStr = "FullDebug";

  std::string NameTableStr;
  if (NameTableKind == DICompileUnit::DebugNameTableKind::Default)
    NameTableStr = "Default";
  else if (NameTableKind == DICompileUnit::DebugNameTableKind::GNU)
    NameTableStr = "GNU";

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DICompileUnit(";
  OS << "language: " << LanguageStr;
  checkAndEmitField(", file: !", File, OS);
  OS << ", producer: \"" << Producer << "\"";
  OS << ", isOptimized: " << (IsOpt ? "true" : "false");
  OS << ", flags: \"" << Flags << "\"";
  OS << ", runtimeVersion: " << RTVer;
  if (!SplitDbgFN.empty())
    OS << ", splitDebugFilename: \"" << SplitDbgFN << "\"";
  if (SplitDbgInlining)
    OS << ", splitDebugInlining: true";
  if (!NameTableStr.empty())
    OS << ", nameTableKind: " << NameTableStr;
  if (!EmissionKindStr.empty())
    OS << ", emissionKind: " << EmissionKindStr;
  if (!Enums.empty())
    checkAndEmitField(", enums: !", Enums.get(), OS);
  if (!RetainedTypes.empty())
    checkAndEmitField(", retainedTypes: !", RetainedTypes.get(), OS);
  if (!Globals.empty())
    checkAndEmitField(", globals: !", Globals.get(), OS);
  if (!Imports.empty())
    checkAndEmitField(", imports: !", Imports.get(), OS);
  if (!Macros.empty())
    checkAndEmitField(", macros: !", Macros.get(), OS);
  if (DWOId)
    OS << ", dwoId: " << DWOId;
  if (DbgInfoForProf)
    OS << ", debugInfoForProfiling: true";

  OS << ")";

  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DIFile(
    const llvm::DIFile *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Filename = MD->getFilename();
  auto Dir = MD->getDirectory();
  auto CK = MD->getChecksum();

  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DIFile(";
  OS << "filename: \"" << Filename << "\"";
  OS << ", directory: \"" << Dir << "\"";
  if (CK.has_value()) {
    auto CKKind = MD->getChecksum().value().getKindAsString();
    auto CKVal = MD->getChecksum().value().Value;
    OS << ", checksumkind: " << CKKind;
    OS << ", checksum: \"" << CKVal << "\"";
  }

  OS << ")";

  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DISubprogram(
    const llvm::DISubprogram *MD, unsigned int Id,
    std::unique_ptr<llvm::MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Name = MD->getName();
  auto LinkageName = MD->getLinkageName();
  auto Scope = MD->getScope();
  auto File = MD->getFile();
  auto Line = MD->getLine();
  auto Type = MD->getType();
  auto Flags = MD->getFlags();
  auto ScopeLine = MD->getScopeLine();
  auto ContainingType = MD->getContainingType();
  auto VirtualIndex = MD->getVirtualIndex();
  auto SPFlags = MD->getSPFlags();
  auto Unit = MD->getUnit();
  auto TemplateParams = MD->getTemplateParams();
  auto Decl = MD->getDeclaration();
  auto RetainedNodes = MD->getRetainedNodes();
  auto ThrownTypes = MD->getThrownTypes();
  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DISubprogram(";
  OS << "name : \"" << Name << "\"";
  if (!LinkageName.empty())
    OS << ", linkageName: \"" << LinkageName << "\"";
  checkAndEmitField(", scope: !", Scope, OS);
  checkAndEmitField(", file: !", File, OS);
  OS << ", line: " << Line;
  checkAndEmitField(", type: !", Type, OS);
  auto SPFlagStr = toString_SPFlags(SPFlags);
  if (!SPFlagStr.empty())
    OS << ", spFlags: " << SPFlagStr;
  OS << ", scopeLine: " << ScopeLine;
  checkAndEmitField(", containingType: !", ContainingType, OS);
  if (MD->getVirtuality())
    OS << ", virtualIndex: " << VirtualIndex;
  auto FlagStr = toString_Flags(Flags);
  if (!FlagStr.empty())
    OS << ", flags: " << FlagStr;
  checkAndEmitField(", unit: !", Unit, OS);
  if (!TemplateParams.empty())
    checkAndEmitField(", templateParams: !", TemplateParams.get(), OS);
  checkAndEmitField(", declaration: !", Decl, OS);
  if (!RetainedNodes.empty())
    checkAndEmitField(", retainedNodes: !", RetainedNodes.get(), OS);
  if (!ThrownTypes.empty())
    checkAndEmitField(", thrownTypes: !", ThrownTypes.get(), OS);

  OS << ")";

  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit_DILocation(
    const DILocation *MD, unsigned int Id,
    std::unique_ptr<MCStreamer> &OutStreamer) {
  std::string Str;
  raw_string_ostream OS(Str);

  auto Line = MD->getLine();
  auto Col = MD->getColumn();
  const auto *Scope = MD->getScope();
  const auto *IAT = MD->getInlinedAt();
  auto isDistinct = MD->isDistinct() ? "distinct " : "";

  OS << "!" << Id << " = " << isDistinct << "!DILocation(line: " << Line << ", column: " << Col;
  checkAndEmitField(", scope: !", Scope, OS);
  checkAndEmitField(", inlinedAt: !", IAT, OS);

  OS << ")";

  OutStreamer->emitRawText(Str);
}

void DebugInfoMetadata::emit(const MDNode *MD, unsigned int Id,
                             std::unique_ptr<MCStreamer> &OutStreamer) {

  if (isa<DILocation>(MD))
    emit_DILocation(cast<const DILocation>(MD), Id, OutStreamer);
  else if (isa<DISubprogram>(MD))
    emit_DISubprogram(cast<const DISubprogram>(MD), Id, OutStreamer);
  else if (isa<DIFile>(MD))
    emit_DIFile(cast<const DIFile>(MD), Id, OutStreamer);
  else if (isa<DICompileUnit>(MD))
    emit_DICompileUnit(cast<const DICompileUnit>(MD), Id, OutStreamer);
  else if (isa<DIBasicType>(MD))
    emit_DIBasicType(cast<const DIBasicType>(MD), Id, OutStreamer);
  else if (isa<DISubroutineType>(MD))
    emit_DISubroutineType(cast<const DISubroutineType>(MD), Id, OutStreamer);
  else if (isa<DIDerivedType>(MD))
    emit_DIDerivedType(cast<const DIDerivedType>(MD), Id, OutStreamer);
  else if (isa<DICompositeType>(MD))
    emit_DICompositeType(cast<const DICompositeType>(MD), Id, OutStreamer);
  else if (isa<DISubrange>(MD))
    emit_DISubrange(cast<const DISubrange>(MD), Id, OutStreamer);
  else if (isa<DIEnumerator>(MD))
    emit_DIEnumerator(cast<const DIEnumerator>(MD), Id, OutStreamer);
  else if (isa<DITemplateTypeParameter>(MD))
    emit_DITemplateTypeParameter(cast<const DITemplateTypeParameter>(MD), Id,
                                 OutStreamer);
  else if (isa<DITemplateValueParameter>(MD))
    emit_DITemplateValueParameter(cast<const DITemplateValueParameter>(MD), Id,
                                  OutStreamer);
  else if (isa<DINamespace>(MD))
    emit_DINamespace(cast<const DINamespace>(MD), Id, OutStreamer);
  else if (isa<DIGlobalVariable>(MD))
    emit_DIGlobalVariable(cast<const DIGlobalVariable>(MD), Id, OutStreamer);
  else if (isa<DIGlobalVariableExpression>(MD))
    emit_DIGlobalVariableExpr(cast<const DIGlobalVariableExpression>(MD), Id,
                              OutStreamer);
  else if (isa<DILexicalBlock>(MD))
    emit_DILexicalBlock(cast<const DILexicalBlock>(MD), Id, OutStreamer);
  else if (isa<DILexicalBlockFile>(MD))
    emit_DILexicalBlockFile(cast<const DILexicalBlockFile>(MD), Id,
                            OutStreamer);
  else if (isa<DILocalVariable>(MD))
    emit_DILocalVariable(cast<const DILocalVariable>(MD), Id, OutStreamer);
  else if (isa<DIImportedEntity>(MD))
    emit_DIImportedEntity(cast<const DIImportedEntity>(MD), Id, OutStreamer);
  else if (isa<DIMacro>(MD))
    emit_DIMacro(cast<const DIMacro>(MD), Id, OutStreamer);
  else if (isa<DIMacroFile>(MD))
    emit_DIMacroFile(cast<const DIMacroFile>(MD), Id, OutStreamer);
  else if (isa<DILabel>(MD))
    emit_DILabel(cast<const DILabel>(MD), Id, OutStreamer);
  else if (isa<DIExpression>(MD))
    emit_DIExpression(cast<const DIExpression>(MD), Id, OutStreamer);
  else if (isa<MDTuple>(MD))
    emit_MDTuple(cast<const MDTuple>(MD), Id, OutStreamer);
}

void DebugInfoMetadata::emitDebugMetadata(
    std::unique_ptr<MCStreamer> &OutStreamer) {
  for(const auto& MetadataNode : IdToMD) {
    unsigned int Id = MetadataNode.first;
    const auto *MD = MetadataNode.second;

    emit(MD, Id, OutStreamer);
  }
}

void DebugInfoMetadata::attachDbgLoc(const MachineInstr& MI, pISAMCInst& Inst) {
  const auto &DbgLoc = MI.getDebugLoc();
  if (!DbgLoc)
    return;
  auto *MCWithDbgLoc = static_cast<pISAMCInst *>(&Inst);
  MCWithDbgLoc->setDebugLoc(DbgLoc);
  unsigned int Id = getOrCreateId(DbgLoc.getAsMDNode());
  MCWithDbgLoc->setId(Id);
}


