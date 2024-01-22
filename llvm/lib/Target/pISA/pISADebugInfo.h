//=- pISAMCInstLower.h -- Convert pISA MachineInstr to MCInst --*- C++ -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_pISA_pISADEBUGINFO_H
#define LLVM_LIB_TARGET_pISA_pISADEBUGINFO_H

#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/raw_ostream.h"
#include "pISAMCInstLower.h"
#include <map>

namespace llvm {
class DebugInfoMetadata {
private:
  std::map<unsigned int, const MDNode *> IdToMD;
  DenseMap<const MDNode *, unsigned int> MDToId;

  void emit_MDTuple(const MDTuple *MD, unsigned int Id,
                    std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DIExpression(const DIExpression *MD, unsigned int Id,
                         std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DILabel(const DILabel *MD, unsigned int Id,
                    std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DIMacroFile(const DIMacroFile *MD, unsigned int Id,
                        std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DIMacro(const DIMacro *MD, unsigned int Id,
                    std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DIImportedEntity(const DIImportedEntity *MD, unsigned int Id,
                             std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DILocalVariable(const DILocalVariable *MD, unsigned int Id,
                            std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DILexicalBlockFile(const DILexicalBlockFile *MD, unsigned int Id,
                               std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DILexicalBlock(const DILexicalBlock *MD, unsigned int Id,
                           std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DIGlobalVariableExpr(const DIGlobalVariableExpression *MD,
                                 unsigned int Id,
                                 std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DIGlobalVariable(const DIGlobalVariable *MD, unsigned int Id,
                             std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DINamespace(const DINamespace *MD, unsigned int Id,
                        std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DITemplateValueParameter(const DITemplateValueParameter *MD,
                                     unsigned int Id,
                                     std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DITemplateTypeParameter(const DITemplateTypeParameter *MD,
                                    unsigned int Id,
                                    std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DIEnumerator(const DIEnumerator *MD, unsigned int Id,
                         std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DISubrange(const DISubrange *MD, unsigned int Id,
                       std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DICompositeType(const DICompositeType *MD, unsigned int Id,
                            std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DIDerivedType(const DIDerivedType *MD, unsigned int Id,
                          std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DISubroutineType(const DISubroutineType *MD, unsigned int Id,
                             std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DIBasicType(const DIBasicType *MD, unsigned int Id,
                        std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DICompileUnit(const DICompileUnit *MD, unsigned int Id,
                          std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DIFile(const DIFile *MD, unsigned int Id,
                   std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DISubprogram(const DISubprogram *MD, unsigned int Id,
                         std::unique_ptr<MCStreamer> &OutStreamer);
  void emit_DILocation(const DILocation *MD, unsigned int Id,
                       std::unique_ptr<MCStreamer> &OutStreamer);
  void emit(const MDNode *MD, unsigned int Id,
            std::unique_ptr<MCStreamer> &OutStreamer);

  void toString(const DIExpression *MD, raw_ostream &OS);

  void checkAndEmitField(StringRef Field, const MDNode *MD, raw_ostream &OS);

public:
  void addMD(const MDNode *MD, unsigned int Id) {
    IdToMD[Id] = MD;
    MDToId[MD] = Id;
  }

  const MDNode *getMD(unsigned int id) const {
    if (IdToMD.count(id) == 0)
      return nullptr;
    return IdToMD.at(id);
  }

  unsigned int getId(const MDNode *MD) const {
    if (MDToId.count(MD) == 0)
      return std::numeric_limits<unsigned int>::max();
    return MDToId.lookup(MD);
  }

  unsigned int getOrCreateId(const MDNode *MD);

  unsigned int getNextId() const { return IdToMD.size() + 1; }

  void emitDebugMetadata(std::unique_ptr<MCStreamer> &OutStreamer);

  void emitDbgDeclare(const MachineFunction &MF,
                      std::unique_ptr<MCStreamer> &OutStreamer);

  void emitDbgValue(const MachineInstr &MI, const pISAMCInst &Inst,
                    std::unique_ptr<MCStreamer> &OutStreamer);

  void attachDbgLoc(const MachineInstr &MI, pISAMCInst &Inst);
};
} // namespace llvm
#endif // LLVM_LIB_TARGET_pISA_pISADEBUGINFO_H
