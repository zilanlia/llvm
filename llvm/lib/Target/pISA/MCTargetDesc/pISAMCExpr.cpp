#include "pISAMCExpr.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/Support/Format.h"
using namespace llvm;

const pISAGlobalAddressMCExpr *
pISAGlobalAddressMCExpr::create(const MCSymbol &Symbol, MCContext &Ctx) {
  return new (Ctx) pISAGlobalAddressMCExpr(Symbol);
}

pISAGlobalAddressMCExpr::pISAGlobalAddressMCExpr(const MCSymbol &Sym)
    : Symbol(&Sym) {
  assert(Symbol);
}

void pISAGlobalAddressMCExpr::printImpl(raw_ostream &OS,
                                        const MCAsmInfo *MAI) const {
  OS << "@";
  Symbol->print(OS, MAI);
}
