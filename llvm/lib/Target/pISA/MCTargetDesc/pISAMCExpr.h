
// pISA special MCTargetExpr class to model floating point immediates
// Modeled after ARMMCExpr

#ifndef LLVM_LIB_TARGET_pISA_pISAMCEXPR_H
#define LLVM_LIB_TARGET_pISA_pISAMCEXPR_H

#include "llvm/ADT/APFloat.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSymbol.h"
#include <utility>

namespace llvm {

/// MCExpr for Global Address.
/// The operand is the address to a global variable (.e.g. @foo).
/// It required a prefix "@" for the symbol name
class pISAGlobalAddressMCExpr : public MCTargetExpr {

private:
  const MCSymbol *Symbol;
  explicit pISAGlobalAddressMCExpr(const MCSymbol &Symbol);

public:
  static const pISAGlobalAddressMCExpr *create(const MCSymbol &Symbol,
                                               MCContext &Ctx);

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;

  bool evaluateAsRelocatableImpl(MCValue &Res, const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override {
    return false;
  }

  void visitUsedExpr(MCStreamer &Streamer) const override {}
  MCFragment *findAssociatedFragment() const override { return nullptr; }

  void fixELFSymbolsInTLSFixups(MCAssembler &) const override {}

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }
};

} // namespace llvm

#endif
