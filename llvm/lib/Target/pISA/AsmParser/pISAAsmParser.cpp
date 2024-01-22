//===- pISAAsmParser.cpp - Define TargetMachine for pISA --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/pISAMCTargetDesc.h"
#include "MCTargetDesc/pISARegEncoder.h"
#include "MCTargetDesc/pISATargetStreamer.h"
#include "TargetInfo/pISATargetInfo.h"
#include "pISADefines.h"
#include "pISAMCInstLower.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compiler.h"
#include <map>

using namespace llvm;

#define DEBUG_TYPE "pisa-asm-parser"

static cl::opt<bool>
    GuessRegisterIdAndType("pisa-guess-register-id-and-type",
                           cl::desc("Guess register id and type if there's no "
                                    "register declaration available"),
                           cl::init(false), cl::ReallyHidden);

namespace {

class pISAOperand : public MCParsedAsmOperand {
public:
  enum KindTy {
    k_Token,
    k_Imm1Opnd,
    k_Imm8Opnd,
    k_Imm16Opnd,
    k_Imm32Opnd,
    k_Imm64Opnd,
    k_FpImm16Opnd,
    k_FpImm32Opnd,
    k_FpImm64Opnd,
    k_PredOpnd,
    k_Reg8bOpnd,
    k_Reg16bOpnd,
    k_Reg32bOpnd,
    k_Reg64bOpnd,
    k_RegV2_8bOpnd,
    k_RegV4_8bOpnd,
    k_RegV2_16bOpnd,
    k_RegV3_16bOpnd,
    k_RegV4_16bOpnd,
    k_RegV2_32bOpnd,
    k_RegV3_32bOpnd,
    k_RegV4_32bOpnd,
    k_RegV2_64bOpnd,
    k_RegV3_64bOpnd,
    k_RegV4_64bOpnd,
  } Kind;

  struct RegOpnd {
    unsigned RegNo;
  };

  struct ImmOpnd {
    int64_t Val;
  };

  SMLoc StartLoc, EndLoc;
  union {
    StringRef Tok;
    RegOpnd Reg;
    ImmOpnd Imm;
  };

  pISAOperand(KindTy K) : MCParsedAsmOperand(), Kind(K) {}
  pISAOperand(const pISAOperand &Other) : MCParsedAsmOperand() {
    Kind = Other.Kind;
    StartLoc = Other.StartLoc;
    EndLoc = Other.EndLoc;
    switch (Kind) {
    case k_Token:
      Tok = Other.Tok;
      break;
    case k_PredOpnd:
    case k_Reg8bOpnd:
    case k_Reg16bOpnd:
    case k_Reg32bOpnd:
    case k_Reg64bOpnd:
    case k_RegV2_8bOpnd:
    case k_RegV4_8bOpnd:
    case k_RegV2_16bOpnd:
    case k_RegV3_16bOpnd:
    case k_RegV4_16bOpnd:
    case k_RegV2_32bOpnd:
    case k_RegV3_32bOpnd:
    case k_RegV4_32bOpnd:
    case k_RegV2_64bOpnd:
    case k_RegV3_64bOpnd:
    case k_RegV4_64bOpnd:
      Reg = Other.Reg;
      break;
    case k_Imm1Opnd:
    case k_Imm8Opnd:
    case k_Imm16Opnd:
    case k_Imm32Opnd:
    case k_Imm64Opnd:
    case k_FpImm16Opnd:
    case k_FpImm32Opnd:
    case k_FpImm64Opnd:
      Imm = Other.Imm;
      break;
    }
  }

  bool isToken() const override { return Kind == k_Token; }
  bool isReg() const override {
    switch (Kind) {
    case k_PredOpnd:
    case k_Reg8bOpnd:
    case k_Reg16bOpnd:
    case k_Reg32bOpnd:
    case k_Reg64bOpnd:
    case k_RegV2_8bOpnd:
    case k_RegV4_8bOpnd:
    case k_RegV2_16bOpnd:
    case k_RegV3_16bOpnd:
    case k_RegV4_16bOpnd:
    case k_RegV2_32bOpnd:
    case k_RegV3_32bOpnd:
    case k_RegV4_32bOpnd:
    case k_RegV2_64bOpnd:
    case k_RegV3_64bOpnd:
    case k_RegV4_64bOpnd:
      return true;
    default:
      break;
    }
    return false;
  }
  bool isImm() const override {
    switch (Kind) {
    case k_Imm1Opnd:
    case k_Imm8Opnd:
    case k_Imm16Opnd:
    case k_Imm32Opnd:
    case k_Imm64Opnd:
    case k_FpImm16Opnd:
    case k_FpImm32Opnd:
    case k_FpImm64Opnd:
      return true;
    default:
      break;
    }
    return false;
  }
  bool isMem() const override { return false; } // TODO:

  bool isImm1Opnd() const { return Kind == k_Imm1Opnd; }
  bool isImm8Opnd() const { return Kind == k_Imm8Opnd; }
  bool isImm16Opnd() const { return Kind == k_Imm16Opnd; }
  bool isImm32Opnd() const { return Kind == k_Imm32Opnd; }
  bool isImm64Opnd() const { return Kind == k_Imm64Opnd; }
  bool isFpImm16Opnd() const { return Kind == k_FpImm16Opnd; }
  bool isFpImm32Opnd() const { return Kind == k_FpImm32Opnd; }
  bool isFpImm64Opnd() const { return Kind == k_FpImm64Opnd; }

  bool isPredOpnd() const { return Kind == k_PredOpnd; }
  bool isReg8bOpnd() const { return Kind == k_Reg8bOpnd; }
  bool isReg16bOpnd() const { return Kind == k_Reg16bOpnd; }
  bool isReg32bOpnd() const { return Kind == k_Reg32bOpnd; }
  bool isReg64bOpnd() const { return Kind == k_Reg64bOpnd; }
  bool isRegV2_8bOpnd() const { return Kind == k_RegV2_8bOpnd; }
  bool isRegV4_8bOpnd() const { return Kind == k_RegV4_8bOpnd; }
  bool isRegV2_16bOpnd() const { return Kind == k_RegV2_16bOpnd; }
  bool isRegV3_16bOpnd() const { return Kind == k_RegV3_16bOpnd; }
  bool isRegV4_16bOpnd() const { return Kind == k_RegV4_16bOpnd; }
  bool isRegV2_32bOpnd() const { return Kind == k_RegV2_32bOpnd; }
  bool isRegV3_32bOpnd() const { return Kind == k_RegV3_32bOpnd; }
  bool isRegV4_32bOpnd() const { return Kind == k_RegV4_32bOpnd; }
  bool isRegV2_64bOpnd() const { return Kind == k_RegV2_64bOpnd; }
  bool isRegV3_64bOpnd() const { return Kind == k_RegV3_64bOpnd; }
  bool isRegV4_64bOpnd() const { return Kind == k_RegV4_64bOpnd; }


  SMLoc getStartLoc() const override { return StartLoc; }
  SMLoc getEndLoc() const override { return EndLoc; }

  StringRef getToken() const {
    assert(Kind == k_Token);
    return Tok;
  }
  unsigned getReg() const override {
    assert(isReg());
    return Reg.RegNo;
  }
  int64_t getImm() const {
    assert(isImm());
    return Imm.Val;
  }

  void print(raw_ostream &OS) const override {
    if (Kind == k_Token)
      OS << Tok;
    else if (isReg())
      OS << "<reg" << Reg.RegNo << ">";
    else if (isImm())
      OS << Imm.Val;
  }

  static std::unique_ptr<pISAOperand> createToken(StringRef Tok, SMLoc Loc) {
    auto Opnd = std::make_unique<pISAOperand>(k_Token);
    Opnd->Tok = Tok;
    Opnd->StartLoc = Loc;
    Opnd->EndLoc = Loc;
    return Opnd;
  }

  static std::unique_ptr<pISAOperand>
  createRegOpnd(KindTy K, unsigned RegNo, SMLoc StartLoc, SMLoc EndLoc) {
    auto Opnd = std::make_unique<pISAOperand>(K);
    Opnd->Reg.RegNo = RegNo;
    Opnd->StartLoc = StartLoc;
    Opnd->EndLoc = EndLoc;
    return Opnd;
  }

  static std::unique_ptr<pISAOperand>
  createImmOpnd(KindTy K, int64_t Val, SMLoc StartLoc, SMLoc EndLoc) {
    auto Opnd = std::make_unique<pISAOperand>(K);
    Opnd->Imm.Val = Val;
    Opnd->StartLoc = StartLoc;
    Opnd->EndLoc = EndLoc;
    return Opnd;
  }

  void setSwizzle(MCInst &MCI, unsigned OpNo, pISA::Swizzle Swz) const {
    unsigned CurFlags = MCI.getFlags();
    assert(pISAMCInstLower::BitEncoder{CurFlags}.Op[OpNo].Op.Value == 0 &&
           "already set?");

    pISAMCInstLower::BitEncoder BC{};
    BC.Op[OpNo].Op.fields.Swizzle = static_cast<unsigned>(Swz);

    CurFlags |= BC.Value;
    MCI.setFlags(CurFlags);
  }

  void addRegOperands(MCInst &MCI, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    MCI.addOperand(MCOperand::createReg(getReg()));
    // Set no swizzle.
    unsigned OpNo = MCI.getNumOperands() - 1;
    setSwizzle(MCI, OpNo, pISA::Swizzle::NONE);
  }

  void addImmOperands(MCInst &MCI, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    MCI.addOperand(MCOperand::createImm(getImm()));
  }

  void addImm1OpndOperands(MCInst &MCI, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    MCI.addOperand(MCOperand::createImm(getImm()));
  }
  void addImm8OpndOperands(MCInst &MCI, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    MCI.addOperand(MCOperand::createImm(getImm()));
  }
  void addImm16OpndOperands(MCInst &MCI, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    MCI.addOperand(MCOperand::createImm(getImm()));
  }
  void addImm32OpndOperands(MCInst &MCI, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    MCI.addOperand(MCOperand::createImm(getImm()));
  }
  void addImm64OpndOperands(MCInst &MCI, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    MCI.addOperand(MCOperand::createImm(getImm()));
  }
  void addFpImm16OpndOperands(MCInst &MCI, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    MCI.addOperand(MCOperand::createImm(getImm()));
  }
  void addFpImm32OpndOperands(MCInst &MCI, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    MCI.addOperand(MCOperand::createSFPImm(getImm()));
  }
  void addFpImm64OpndOperands(MCInst &MCI, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    MCI.addOperand(MCOperand::createDFPImm(getImm()));
  }

  void addPredOpndOperands(MCInst &MCI, unsigned N) const {
    addRegOperands(MCI, N);
  }
  void addReg8bOpndOperands(MCInst &MCI, unsigned N) const {
    addRegOperands(MCI, N);
  }
  void addReg16bOpndOperands(MCInst &MCI, unsigned N) const {
    addRegOperands(MCI, N);
  }
  void addReg32bOpndOperands(MCInst &MCI, unsigned N) const {
    addRegOperands(MCI, N);
  }
  void addReg64bOpndOperands(MCInst &MCI, unsigned N) const {
    addRegOperands(MCI, N);
  }
  void addRegV2_8bOpndOperands(MCInst &MCI, unsigned N) const {
    addRegOperands(MCI, N);
  }
  void addRegV4_8bOpndOperands(MCInst &MCI, unsigned N) const {
    addRegOperands(MCI, N);
  }
  void addRegV2_16bOpndOperands(MCInst &MCI, unsigned N) const {
    addRegOperands(MCI, N);
  }
  void addRegV3_16bOpndOperands(MCInst &MCI, unsigned N) const {
    addRegOperands(MCI, N);
  }
  void addRegV4_16bOpndOperands(MCInst &MCI, unsigned N) const {
    addRegOperands(MCI, N);
  }
  void addRegV2_32bOpndOperands(MCInst &MCI, unsigned N) const {
    addRegOperands(MCI, N);
  }
  void addRegV3_32bOpndOperands(MCInst &MCI, unsigned N) const {
    addRegOperands(MCI, N);
  }
  void addRegV4_32bOpndOperands(MCInst &MCI, unsigned N) const {
    addRegOperands(MCI, N);
  }
  void addRegV2_64bOpndOperands(MCInst &MCI, unsigned N) const {
    addRegOperands(MCI, N);
  }
  void addRegV3_64bOpndOperands(MCInst &MCI, unsigned N) const {
    addRegOperands(MCI, N);
  }
  void addRegV4_64bOpndOperands(MCInst &MCI, unsigned N) const {
    addRegOperands(MCI, N);
  }

};

class pISAAsmParser : public MCTargetAsmParser {
  std::unique_ptr<pISA::FunctionSignature> CurrentFnSig;
  std::unique_ptr<pISA::RegDcls> CurrentRegDcls;
  std::unique_ptr<pISA::StackVariableDcls> CurrentStackVariableDcls;
  bool InFunctionBody = false;
  bool HasRegAndStackDcls = false;

public:
  enum pISAMatchResultTy {
    Match_
#define GET_OPERAND_DIAGNOSTIC_TYPES
#include "pISAGenAsmMatcher.inc"
  };

  pISAAsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                const MCInstrInfo &MII, const MCTargetOptions &Options)
      : MCTargetAsmParser(Options, STI, MII) {}

  bool parseRegister(MCRegister &Reg, SMLoc &StartLoc, SMLoc &EndLoc) override;

  ParseStatus tryParseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                        SMLoc &EndLoc) override;

  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;

  bool ParseDirective(AsmToken DirectiveID) override;

  bool MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;

  unsigned validateTargetOperandClass(MCParsedAsmOperand &Op,
                                      unsigned Kind) override;

  void doBeforeLabelEmit(MCSymbol *Symbol, SMLoc IDLoc) override;

  // Try to parse the optional linakge. As that linakge is optional, the
  // parsing should never fail.
  std::optional<bool> tryParseLinkage();
  std::optional<LLT> tryParseLLT();

  bool parseCallingConv(CallingConv::ID &CC);
  bool parseFunctionDirectiveAndName(pISA::FunctionDirectiveAndName &DN);
  bool parseFunctionParam(pISA::FunctionParameter &FuncParam);
  bool parseRegDcl(unsigned &NumElts, unsigned &BitWidth, unsigned &RegType,
                   std::vector<unsigned> &Ids);

protected:
#define GET_ASSEMBLER_HEADER
#include "pISAGenAsmMatcher.inc"

  pISATargetStreamer &getTargetStreamer() {
    return static_cast<pISATargetStreamer &>(
        *getParser().getStreamer().getTargetStreamer());
  }

  bool parseOperand(OperandVector &Operands, StringRef Mnemonic);
  OperandMatchResultTy parseRegister(pISAOperand::KindTy K,
                                     OperandVector &Operands);
  OperandMatchResultTy parseImmediate(pISAOperand::KindTy K,
                                      OperandVector &Operands);

  // 'parseRealValue` in AsmParser is not exposed yet. Duplicate here.
  bool parseRealValue(const fltSemantics &Semantics, APInt &Res);

  OperandMatchResultTy parseImm1Opnd(OperandVector &Operands);
  OperandMatchResultTy parseImm8Opnd(OperandVector &Operands);
  OperandMatchResultTy parseImm16Opnd(OperandVector &Operands);
  OperandMatchResultTy parseImm32Opnd(OperandVector &Operands);
  OperandMatchResultTy parseImm64Opnd(OperandVector &Operands);
  OperandMatchResultTy parseFpImm16Opnd(OperandVector &Operands);
  OperandMatchResultTy parseFpImm32Opnd(OperandVector &Operands);
  OperandMatchResultTy parseFpImm64Opnd(OperandVector &Operands);

  OperandMatchResultTy parsePredOpnd(OperandVector &Operands);
  OperandMatchResultTy parseReg8bOpnd(OperandVector &Operands);
  OperandMatchResultTy parseReg16bOpnd(OperandVector &Operands);
  OperandMatchResultTy parseReg32bOpnd(OperandVector &Operands);
  OperandMatchResultTy parseReg64bOpnd(OperandVector &Operands);
  OperandMatchResultTy parseRegV2_8bOpnd(OperandVector &Operands);
  OperandMatchResultTy parseRegV4_8bOpnd(OperandVector &Operands);
  OperandMatchResultTy parseRegV2_16bOpnd(OperandVector &Operands);
  OperandMatchResultTy parseRegV3_16bOpnd(OperandVector &Operands);
  OperandMatchResultTy parseRegV4_16bOpnd(OperandVector &Operands);
  OperandMatchResultTy parseRegV2_32bOpnd(OperandVector &Operands);
  OperandMatchResultTy parseRegV3_32bOpnd(OperandVector &Operands);
  OperandMatchResultTy parseRegV4_32bOpnd(OperandVector &Operands);
  OperandMatchResultTy parseRegV2_64bOpnd(OperandVector &Operands);
  OperandMatchResultTy parseRegV3_64bOpnd(OperandVector &Operands);
  OperandMatchResultTy parseRegV4_64bOpnd(OperandVector &Operands);

  // Consume EOL if any.
  void consumeEOL() {
    auto &Lexer = getLexer();
    // Eat EOL if any.
    if (Lexer.is(AsmToken::EndOfStatement) &&
        (getTok().getString().empty() || getTok().getString().front() == '\r' ||
         getTok().getString().front() == '\n'))
      Lex();
  }

protected:
  void resetRegisterMap() {
    RegisterMap.clear();
    for (unsigned i = 0; i < pISA::RegEncoder::RegType::NUM_TYPE; ++i)
      RegTypeCount[i] = 0;
  }

  std::tuple</*RegId=*/unsigned, /*RegType=*/unsigned, /*DataType=*/LLT>
  getRegByName(std::string RegName) {
    auto I = RegisterMap.find(RegName);
    if (I != RegisterMap.end())
      return I->second;
    if (GuessRegisterIdAndType) {
      // FIXME: If there is no register map, e.g. an assembly fragment is being
      // parsed, we should be able to *guess* the register id from the name.
      // That is very convenient to construct simple tests for instructions. So
      // far, just return a random number.
      return std::make_tuple(1234, pISA::RegEncoder::REG, LLT::scalar(32));
    }
    llvm_unreachable("Undeclared register!");
  }

  unsigned getOrCreateReg(std::string RegName, unsigned RegType, LLT Ty) {
    auto I = RegisterMap.find(RegName);
    if (I != RegisterMap.end())
      return std::get<0>(I->second);
    unsigned RegId = RegTypeCount[RegType]++;
    RegisterMap.insert(
        std::make_pair(RegName, std::make_tuple(RegId, RegType, Ty)));
    return RegId;
  }

private:
  std::map<std::string,
           std::tuple</*Idx=*/unsigned, /*RegType=*/unsigned, /*DataType=*/LLT>>
      RegisterMap;
  unsigned RegTypeCount[pISA::RegEncoder::RegType::NUM_TYPE];
};

} // namespace

#define GET_REGISTER_MATCHER
#define GET_SUBTARGET_FEATURE_NAME
#define GET_MATCHER_IMPLEMENTATION
#define GET_MNEMONIC_SPELL_CHECKER
#define GET_MNEMONIC_CHECKER
#include "pISAGenAsmMatcher.inc"

bool pISAAsmParser::parseRealValue(const fltSemantics &Semantics, APInt &Res) {
  auto &Lexer = getLexer();

  // We don't truly support arithmetic on floating point expressions, so we
  // have to manually parse unary prefixes.
  bool IsNeg = false;
  if (getLexer().is(AsmToken::Minus)) {
    Lexer.Lex();
    IsNeg = true;
  } else if (getLexer().is(AsmToken::Plus))
    Lexer.Lex();

  if (Lexer.is(AsmToken::Error))
    return TokError(Lexer.getErr());
  if (Lexer.isNot(AsmToken::Integer) && Lexer.isNot(AsmToken::Real) &&
      Lexer.isNot(AsmToken::Identifier))
    return TokError("unexpected token in directive");

  // Convert to an APFloat.
  APFloat Value(Semantics);
  StringRef IDVal = getTok().getString();
  if (getLexer().is(AsmToken::Identifier)) {
    if (!IDVal.compare_insensitive("infinity") ||
        !IDVal.compare_insensitive("inf"))
      Value = APFloat::getInf(Semantics);
    else if (!IDVal.compare_insensitive("nan"))
      Value = APFloat::getNaN(Semantics, false, ~0);
    else
      return TokError("invalid floating point literal");
  } else if (errorToBool(
                 Value.convertFromString(IDVal, APFloat::rmNearestTiesToEven)
                     .takeError()))
    return TokError("invalid floating point literal");
  if (IsNeg)
    Value.changeSign();

  // Consume the numeric token.
  Lex();

  Res = Value.bitcastToAPInt();

  return false;
}

OperandMatchResultTy pISAAsmParser::parseImm1Opnd(OperandVector &Operands) {
  return parseImmediate(pISAOperand::k_Imm1Opnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseImm8Opnd(OperandVector &Operands) {
  return parseImmediate(pISAOperand::k_Imm8Opnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseImm16Opnd(OperandVector &Operands) {
  return parseImmediate(pISAOperand::k_Imm16Opnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseImm32Opnd(OperandVector &Operands) {
  return parseImmediate(pISAOperand::k_Imm32Opnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseImm64Opnd(OperandVector &Operands) {
  return parseImmediate(pISAOperand::k_Imm64Opnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseFpImm16Opnd(OperandVector &Operands) {
  auto &Lexer = getLexer();

  const auto &Tok = Lexer.getTok();
  if (Tok.isNot(AsmToken::Real))
    return MatchOperand_NoMatch;

  auto StartLoc = Tok.getLoc();
  auto EndLoc = Tok.getEndLoc();

  APInt Val;
  if (parseRealValue(APFloat::IEEEhalf(), Val))
    return MatchOperand_ParseFail;

  Operands.push_back(pISAOperand::createImmOpnd(
      pISAOperand::k_FpImm16Opnd, Val.getZExtValue(), StartLoc, EndLoc));

  return MatchOperand_Success;
}

OperandMatchResultTy pISAAsmParser::parseFpImm32Opnd(OperandVector &Operands) {
  auto &Lexer = getLexer();

  const auto &Tok = Lexer.getTok();
  if (Tok.isNot(AsmToken::Real))
    return MatchOperand_NoMatch;

  auto StartLoc = Tok.getLoc();
  auto EndLoc = Tok.getEndLoc();

  APInt Val;
  if (parseRealValue(APFloat::IEEEsingle(), Val))
    return MatchOperand_ParseFail;

  Operands.push_back(pISAOperand::createImmOpnd(
      pISAOperand::k_FpImm32Opnd, Val.getZExtValue(), StartLoc, EndLoc));

  return MatchOperand_Success;
}

OperandMatchResultTy pISAAsmParser::parseFpImm64Opnd(OperandVector &Operands) {
  auto &Lexer = getLexer();

  const auto &Tok = Lexer.getTok();
  if (Tok.isNot(AsmToken::Real))
    return MatchOperand_NoMatch;

  auto StartLoc = Tok.getLoc();
  auto EndLoc = Tok.getEndLoc();

  APInt Val;
  if (parseRealValue(APFloat::IEEEdouble(), Val))
    return MatchOperand_ParseFail;

  Operands.push_back(pISAOperand::createImmOpnd(
      pISAOperand::k_FpImm32Opnd, Val.getZExtValue(), StartLoc, EndLoc));

  return MatchOperand_Success;
}

OperandMatchResultTy pISAAsmParser::parsePredOpnd(OperandVector &Operands) {
  return parseRegister(pISAOperand::k_PredOpnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseReg8bOpnd(OperandVector &Operands) {
  return parseRegister(pISAOperand::k_Reg8bOpnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseReg16bOpnd(OperandVector &Operands) {
  return parseRegister(pISAOperand::k_Reg16bOpnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseReg32bOpnd(OperandVector &Operands) {
  return parseRegister(pISAOperand::k_Reg32bOpnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseReg64bOpnd(OperandVector &Operands) {
  return parseRegister(pISAOperand::k_Reg64bOpnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseRegV2_8bOpnd(OperandVector &Operands) {
  return parseRegister(pISAOperand::k_RegV2_8bOpnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseRegV4_8bOpnd(OperandVector &Operands) {
  return parseRegister(pISAOperand::k_RegV4_8bOpnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseRegV2_16bOpnd(OperandVector &Operands) {
  return parseRegister(pISAOperand::k_RegV2_16bOpnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseRegV3_16bOpnd(OperandVector &Operands) {
  return parseRegister(pISAOperand::k_RegV3_16bOpnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseRegV4_16bOpnd(OperandVector &Operands) {
  return parseRegister(pISAOperand::k_RegV4_16bOpnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseRegV2_32bOpnd(OperandVector &Operands) {
  return parseRegister(pISAOperand::k_RegV2_32bOpnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseRegV3_32bOpnd(OperandVector &Operands) {
  return parseRegister(pISAOperand::k_RegV3_32bOpnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseRegV4_32bOpnd(OperandVector &Operands) {
  return parseRegister(pISAOperand::k_RegV4_32bOpnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseRegV2_64bOpnd(OperandVector &Operands) {
  return parseRegister(pISAOperand::k_RegV2_64bOpnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseRegV3_64bOpnd(OperandVector &Operands) {
  return parseRegister(pISAOperand::k_RegV3_64bOpnd, Operands);
}

OperandMatchResultTy pISAAsmParser::parseRegV4_64bOpnd(OperandVector &Operands) {
  return parseRegister(pISAOperand::k_RegV4_64bOpnd, Operands);
}


OperandMatchResultTy pISAAsmParser::parseRegister(pISAOperand::KindTy K,
                                                  OperandVector &Operands) {
  auto &Lexer = getLexer();

  auto StartLoc = Lexer.getTok().getLoc();

  if (Lexer.getTok().isNot(AsmToken::Percent))
    return MatchOperand_NoMatch;

  // Don't eat '%' as we need to peek into the next token as that, if this
  // match fails, we could try other kinds.
  const auto &Tok = Lexer.peekTok();
  if (Tok.isNot(AsmToken::Identifier))
    return MatchOperand_NoMatch;

  std::string Name = ("%" + Tok.getIdentifier()).str();
  unsigned RegNo = MatchRegisterName(Name);
  if (RegNo) {
    auto EndLoc = Tok.getEndLoc();
    // FIXME: Add check on the kind of predefined registers.
    // Eat '%'.
    Lexer.Lex();
    // Eat identifier.
    Lexer.Lex();
    Operands.push_back(pISAOperand::createRegOpnd(K, RegNo, StartLoc, EndLoc));
    return MatchOperand_Success;
  }

  unsigned RegType;
  unsigned RegId;
  LLT Ty;
  std::tie(RegId, RegType, Ty) = getRegByName(Tok.getIdentifier().str());

  switch (K) {
  case pISAOperand::k_PredOpnd:
    if (Ty != LLT::scalar(1))
      return MatchOperand_NoMatch;
    break;
  case pISAOperand::k_Reg8bOpnd:
    if (Ty != LLT::scalar(8))
      return MatchOperand_NoMatch;
    break;
  case pISAOperand::k_Reg16bOpnd:
    if (Ty != LLT::scalar(16))
      return MatchOperand_NoMatch;
    break;
  case pISAOperand::k_Reg32bOpnd:
    if (Ty != LLT::scalar(32))
      return MatchOperand_NoMatch;
    break;
  case pISAOperand::k_Reg64bOpnd:
    if (Ty != LLT::scalar(64))
      return MatchOperand_NoMatch;
    break;
  case pISAOperand::k_RegV2_8bOpnd:
    if (Ty != LLT::vector(ElementCount::getFixed(2), 8))
      return MatchOperand_NoMatch;
    break;
  case pISAOperand::k_RegV4_8bOpnd:
    if (Ty != LLT::vector(ElementCount::getFixed(4), 8))
      return MatchOperand_NoMatch;
    break;
  case pISAOperand::k_RegV2_16bOpnd:
    if (Ty != LLT::vector(ElementCount::getFixed(2), 16))
      return MatchOperand_NoMatch;
    break;
  case pISAOperand::k_RegV3_16bOpnd:
    if (Ty != LLT::vector(ElementCount::getFixed(3), 16))
      return MatchOperand_NoMatch;
    break;
  case pISAOperand::k_RegV4_16bOpnd:
    if (Ty != LLT::vector(ElementCount::getFixed(4), 16))
      return MatchOperand_NoMatch;
    break;
  case pISAOperand::k_RegV2_32bOpnd:
    if (Ty != LLT::vector(ElementCount::getFixed(2), 32))
      return MatchOperand_NoMatch;
    break;
  case pISAOperand::k_RegV3_32bOpnd:
    if (Ty != LLT::vector(ElementCount::getFixed(3), 32))
      return MatchOperand_NoMatch;
    break;
  case pISAOperand::k_RegV4_32bOpnd:
    if (Ty != LLT::vector(ElementCount::getFixed(4), 32))
      return MatchOperand_NoMatch;
    break;
  case pISAOperand::k_RegV2_64bOpnd:
    if (Ty != LLT::vector(ElementCount::getFixed(2), 64))
      return MatchOperand_NoMatch;
    break;
  case pISAOperand::k_RegV3_64bOpnd:
    if (Ty != LLT::vector(ElementCount::getFixed(3), 64))
      return MatchOperand_NoMatch;
    break;
  case pISAOperand::k_RegV4_64bOpnd:
    if (Ty != LLT::vector(ElementCount::getFixed(4), 64))
      return MatchOperand_NoMatch;
    break;
  default:
    llvm_unreachable("Unexpected operand kind!");
  }

  auto EndLoc = Tok.getEndLoc();
  // Eat '%'.
  Lexer.Lex();
  // Eat identifier.
  Lexer.Lex();
  // FIXME: Need to parse the virtual register.
  RegNo = pISA::RegEncoder::encodeVirtualRegister(RegId, pISA::RegEncoder::REG);
  Operands.push_back(pISAOperand::createRegOpnd(K, RegNo, StartLoc, EndLoc));

  return MatchOperand_Success;
}

OperandMatchResultTy pISAAsmParser::parseImmediate(pISAOperand::KindTy K,
                                                   OperandVector &Operands) {
  auto &Lexer = getLexer();
  auto &Parser = getParser();

  const auto &Tok = Lexer.getTok();
  if (Tok.isNot(AsmToken::Integer))
    return MatchOperand_NoMatch;

  auto StartLoc = Tok.getLoc();
  auto EndLoc = Tok.getEndLoc();
  int64_t Val;
  if (Parser.parseIntToken(Val, "Unexpected integer"))
    return MatchOperand_ParseFail;

  Operands.push_back(pISAOperand::createImmOpnd(K, Val, StartLoc, EndLoc));

  return MatchOperand_Success;
}

bool pISAAsmParser::parseOperand(OperandVector &Operands, StringRef Mnemonic) {
  auto ResTy =
      MatchOperandParserImpl(Operands, Mnemonic, /*ParseForAllFeatures=*/true);
  if (ResTy == MatchOperand_Success)
    return false;
  llvm_unreachable("Not handled yet!");
  return true;
}

bool pISAAsmParser::parseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                  SMLoc &EndLoc) {
  llvm_unreachable("Not implemented yet!");
}

ParseStatus pISAAsmParser::tryParseRegister(MCRegister &Reg,
                                                     SMLoc &StartLoc,
                                                     SMLoc &EndLoc) {
  llvm_unreachable("Not implemented yet!");
}

void pISAAsmParser::doBeforeLabelEmit(MCSymbol *Symbol, SMLoc IDLoc) {
  if (HasRegAndStackDcls) {
    getTargetStreamer().emitRegDcls(*CurrentRegDcls);
    getTargetStreamer().emitStackVariableDcls(*CurrentStackVariableDcls);
    HasRegAndStackDcls = false;
  }
}

bool pISAAsmParser::ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                                     SMLoc NameLoc, OperandVector &Operands) {
  auto &Lexer = getLexer();

  LLVM_DEBUG(dbgs() << "Parse instruction starting with '" << Name << "'\n");

  if (HasRegAndStackDcls) {
    getTargetStreamer().emitRegDcls(*CurrentRegDcls);
    getTargetStreamer().emitStackVariableDcls(*CurrentStackVariableDcls);
    HasRegAndStackDcls = false;
  }

  if (Lexer.is(AsmToken::EndOfStatement)) {
    if (Name == "{" || Name == "}") {
      if (Name == "{") {
        getTargetStreamer().emitFuncBodyStart();
        InFunctionBody = !!CurrentFnSig;
        if (InFunctionBody)
          HasRegAndStackDcls = false;
      } else {
        getTargetStreamer().emitFuncBodyEnd();
        CurrentFnSig.release();
        InFunctionBody = false;
      }
      // Consume EOL to stop spurious blank lines added from the
      // target-independent asm parser.
      consumeEOL();
      // Fake an instruction without any operands, including the mnemonic.
      return false;
    }
  }

  StringRef Mnemonic = Name;

  Operands.push_back(pISAOperand::createToken(Mnemonic, NameLoc));

  if (Lexer.is(AsmToken::EndOfStatement))
    return false;

  if (parseOperand(Operands, Mnemonic))
    return true;

  while (Lexer.is(AsmToken::Comma)) {
    // Eat the comma.
    Lexer.Lex();
    if (parseOperand(Operands, Mnemonic))
      return true;
  }

  // FIXME: ';' is the end of the statement instead of a part of the
  // instruction. Fix the tablegen.
  if (Lexer.is(AsmToken::EndOfStatement)) {
    Operands.push_back(pISAOperand::createToken(";", Lexer.getTok().getLoc()));
    // Eat ';'.
    Lexer.Lex();
    // Consume EOL to stop spurious blank lines added from the
    // target-independent asm parser.
    consumeEOL();
  }

  LLVM_DEBUG(dbgs() << "Parse instruction ending with '";
             Lexer.getTok().dump(dbgs()); dbgs() << "'\n");
  return false;
}

std::optional<LLT> pISAAsmParser::tryParseLLT() {
  const auto &Tok = getLexer().getTok();
  if (!Tok.is(AsmToken::Identifier))
    return std::nullopt;
  StringRef IDVal = Tok.getIdentifier();
  std::optional<LLT> Ty = StringSwitch<std::optional<LLT>>(IDVal)
                              .Case(".pred", LLT::scalar(1))
                              .Case(".8b", LLT::scalar(8))
                              .Case(".16b", LLT::scalar(16))
                              .Case(".32b", LLT::scalar(32))
                              .Case(".64b", LLT::scalar(64))
                              .Case(".v2.8b", LLT::vector(ElementCount::getFixed(2), 8))
                              .Case(".v4.8b", LLT::vector(ElementCount::getFixed(4), 8))
                              .Case(".v2.16b", LLT::vector(ElementCount::getFixed(2), 16))
                              .Case(".v3.16b", LLT::vector(ElementCount::getFixed(3), 16))
                              .Case(".v4.16b", LLT::vector(ElementCount::getFixed(4), 16))
                              .Case(".v2.32b", LLT::vector(ElementCount::getFixed(2), 32))
                              .Case(".v3.32b", LLT::vector(ElementCount::getFixed(3), 32))
                              .Case(".v4.32b", LLT::vector(ElementCount::getFixed(4), 32))
                              .Case(".v2.64b", LLT::vector(ElementCount::getFixed(2), 64))
                              .Case(".v3.64b", LLT::vector(ElementCount::getFixed(3), 64))
                              .Case(".v4.64b", LLT::vector(ElementCount::getFixed(4), 64))
                              .Default(std::nullopt);
  if (Ty) {
    // Eat the return type token.
    getLexer().Lex();
  }
  return Ty;
}

std::optional<bool> pISAAsmParser::tryParseLinkage() {
  const auto &Tok = getLexer().getTok();
  if (!Tok.is(AsmToken::Identifier))
    return std::nullopt;
  StringRef IDVal = Tok.getIdentifier();
  if (IDVal != ".export" && IDVal != ".import")
    return std::nullopt;
  bool Linkage = (IDVal == ".export");
  // Eat the linkage token.
  getLexer().Lex();
  return Linkage;
}

bool pISAAsmParser::parseCallingConv(CallingConv::ID &CC) {
  const auto &Tok = getLexer().getTok();
  if (!Tok.is(AsmToken::Identifier))
    return Error(Tok.getLoc(), "calling conv identifier is expected!");
  StringRef IDVal = Tok.getIdentifier();
  std::optional<CallingConv::ID> Conv =
      StringSwitch<std::optional<CallingConv::ID>>(IDVal)
          .Case(".kernel", CallingConv::PISA_KERNEL)
          .Case(".function", CallingConv::PISA_FUNC)
          .Case(".rt.raygen", CallingConv::PISA_RT_RAYGEN)
          .Case(".rt.dispatch", CallingConv::PISA_RT_DISPATCH)
          .Default(std::nullopt);
  if (!Conv)
    return Error(Tok.getLoc(), "unexpected calling conv identifier!");
  CC = *Conv;
  // Eat the calling conv token.
  getLexer().Lex();
  return false;
}

bool pISAAsmParser::parseFunctionDirectiveAndName(
    pISA::FunctionDirectiveAndName &DN) {
  auto &Lexer = getLexer();

  // Parse the optional linkage.
  DN.Linkage = tryParseLinkage();
  // Parse the calling convention.
  if (parseCallingConv(DN.CC))
    return true;
  // Parese the optional return value.
  if (DN.CC == CallingConv::PISA_FUNC) {
    const auto &Tok = Lexer.getTok();
    if (!Tok.is(AsmToken::Identifier))
      return Error(Tok.getLoc(), "calling conv identifier is expected!");
    StringRef IDVal = Tok.getIdentifier();
    if (IDVal != "void") {
      DN.RetLLT = tryParseLLT();
      if (!DN.RetLLT)
        return Error(Tok.getLoc(), "unexpected return type!");
    } else {
      // Eat the type identifier.
      Lexer.Lex();
    }
  }
  // Parse the function name.
  if (!Lexer.getTok().is(AsmToken::At))
    return Error(Lexer.getLoc(), "function name is expected!");
  // Eat '@'.
  Lexer.Lex();
  const auto &Tok = Lexer.getTok();
  if (!Tok.is(AsmToken::Identifier))
    return Error(Tok.getLoc(), "function name is expected!");
  DN.Name = Tok.getIdentifier();
  // Eat the name identifer.
  Lexer.Lex();

  return false;
}

bool pISAAsmParser::parseFunctionParam(pISA::FunctionParameter &FuncParam) {
  auto &Lexer = getLexer();

  if (!Lexer.getTok().is(AsmToken::Identifier) ||
      Lexer.getTok().getIdentifier() != ".reg")
    return Error(Lexer.getLoc(), "parameter reg declaration is expected!");
  // Eat '.reg'.
  Lexer.Lex();
  // Parse the parameter type.
  std::optional<LLT> ParamLLT = tryParseLLT();
  if (!ParamLLT)
    return Error(Lexer.getLoc(), "unexpected parameter type!");
  FuncParam.Ty = *ParamLLT;
  // HACK: Need refactoring.
  FuncParam.Prefix = (FuncParam.Ty == LLT::scalar(1)) ? "%p" : "%r";
  unsigned RegType = (FuncParam.Ty == LLT::scalar(1)) ? pISA::RegEncoder::PRED
                                                      : pISA::RegEncoder::REG;

  if (!Lexer.getTok().is(AsmToken::Percent))
    return Error(Lexer.getLoc(), "parameter name is expected!");
  // Eat '%'.
  Lexer.Lex();

  if (!Lexer.getTok().is(AsmToken::Identifier))
    return Error(Lexer.getLoc(), "parameter reg name is expected!");
  StringRef RegName = Lexer.getTok().getIdentifier();
  FuncParam.Idx = getOrCreateReg(RegName.str(), RegType, *ParamLLT);
  // Eat the parameter name identifier.
  Lexer.Lex();

  return false;
}

bool pISAAsmParser::parseRegDcl(unsigned &NumElts, unsigned &BitWidth,
                                unsigned &RegType, std::vector<unsigned> &Ids) {
  auto &Lexer = getLexer();

  // Parse the declaration keyword.
  if (!Lexer.getTok().is(AsmToken::Identifier))
    return Error(Lexer.getLoc(), "register decl is expected!");
  std::optional<unsigned> RegT =
      StringSwitch<std::optional<unsigned>>(Lexer.getTok().getIdentifier())
          .Case(".pred", pISA::RegEncoder::PRED)
          .Case(".reg", pISA::RegEncoder::REG)
          .Default(std::nullopt);
  if (!RegT)
    return Error(Lexer.getLoc(), "register decl is expected!");
  RegType = *RegT;
  // Eat the decl keyword.
  Lexer.Lex();

  // Parse LLT.
  std::optional<LLT> RegLLT = tryParseLLT();
  if (!RegLLT)
    return Error(Lexer.getLoc(), "unexpected reg decl type!");
  NumElts = RegLLT->isScalar() ? 1 : RegLLT->getNumElements();
  BitWidth = RegLLT->getScalarSizeInBits();

  // Parse the list of registers.
  while (!Lexer.getTok().is(AsmToken::EndOfStatement)) {
    if (!Lexer.getTok().is(AsmToken::Percent))
      return Error(Lexer.getLoc(), "parameter name is expected!");
    // Eat '%'.
    Lexer.Lex();
    // Parse the register identifier.
    if (!Lexer.getTok().is(AsmToken::Identifier))
      return Error(Lexer.getLoc(), "parameter reg name is expected!");
    StringRef RegName = Lexer.getTok().getIdentifier();
    Ids.push_back(getOrCreateReg(RegName.str(), RegType, *RegLLT));
    // Eat the register identifier.
    Lexer.Lex();
    // Parse next register.
    if (Lexer.getTok().is(AsmToken::Comma)) {
      // Eat the comma.
      Lexer.Lex();
    }
  }

  // Eat ';'
  Lexer.Lex();

  if (Ids.empty())
    return Error(Lexer.getLoc(), "parameter name is expected!");

  return false;
}

bool pISAAsmParser::ParseDirective(AsmToken DirectiveID) {
  auto &Lexer = getLexer();

  // Push it back to simplify the following process.
  Lexer.UnLex(DirectiveID);

  if (InFunctionBody) {
    // In function body, parse reg/variable declarations.
    if (!Lexer.getTok().is(AsmToken::Identifier))
      return true;
    StringRef IDVal = Lexer.getTok().getIdentifier();
    if (IDVal == ".reg" || IDVal == ".pred") {
      // HACK: Need refactoring.
      unsigned NumElts, BitWidth, RegType;
      std::vector<unsigned> Ids;
      if (parseRegDcl(NumElts, BitWidth, RegType, Ids))
        return true;
      // Consume EOL to stop spurious blank lines added from the
      // target-independent asm parser.
      consumeEOL();
      // Update reg decls.
      assert(CurrentRegDcls && "Unexpected function body!");
      const char *Prefix = (RegType == pISA::RegEncoder::PRED) ? "%p" : "%r";
      for (auto Id : Ids) {
        CurrentRegDcls->Regs[std::make_tuple(NumElts, BitWidth, RegType)]
            .push_back(std::make_pair(Prefix, Id));
      }
      HasRegAndStackDcls = true;
      return false;
    }
  }

  std::unique_ptr<pISA::FunctionSignature> FnSig =
      std::make_unique<pISA::FunctionSignature>();

  if (parseFunctionDirectiveAndName(FnSig->DN))
    return true;

  if (!Lexer.getTok().is(AsmToken::LParen)) {
    // Import global variables or functions.
    assert(FnSig->DN.Linkage && !*FnSig->DN.Linkage &&
           "Expect external declaration only!");
    return false;
  }

  // Reset the register map.
  resetRegisterMap();

  // Eat that '('.
  Lexer.Lex();
  if (FnSig->DN.CC == CallingConv::PISA_FUNC) {
    while (!Lexer.getTok().is(AsmToken::RParen)) {
      pISA::FunctionParameter FuncParam;
      if (parseFunctionParam(FuncParam))
        return true;
      FnSig->FunctionParams.emplace_back(FuncParam);
      // Parse next parameter.
      if (Lexer.getTok().is(AsmToken::Comma)) {
        // Eat the comma.
        Lexer.Lex();
      }
    }
    // Eat ')'.
    Lexer.Lex();
  } else
    llvm_unreachable("Not implemented yet!");

  if (!Lexer.getTok().is(AsmToken::EndOfStatement))
    return Error(Lexer.getLoc(), "unexpected end of line!");

  getTargetStreamer().emitFunctionSignature(*FnSig);
  // Consume EOL to stop spurious blank lines added from the
  // target-independent asm parser.
  consumeEOL();

  // Record the current function signature.
  CurrentFnSig = std::move(FnSig);
  // Initialize local regs/stacks.
  CurrentRegDcls = std::make_unique<pISA::RegDcls>();
  CurrentStackVariableDcls = std::make_unique<pISA::StackVariableDcls>();

  return false;
}

bool pISAAsmParser::MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                            OperandVector &Operands,
                                            MCStreamer &Out,
                                            uint64_t &ErrorInfo,
                                            bool MatchingInlineAsm) {
  // Skip for the faked '{' and '}' instructions.
  if (Operands.empty())
    return false;

  // FIXME: This *definitely* breaks the MC layer.
  pISAMCInst MCI;
  FeatureBitset MissingFeatures;

  auto Result = MatchInstructionImpl(Operands, MCI, ErrorInfo, MissingFeatures,
                                     MatchingInlineAsm);
  switch (Result) {
  default:
    break;
  case Match_Success:
    Out.emitInstruction(MCI, getSTI());
    return false;
  }
  return true;
}

unsigned pISAAsmParser::validateTargetOperandClass(MCParsedAsmOperand &Op,
                                                   unsigned Kind) {
  switch (Kind) {
  case MCK_Reg32b:
    if (Op.isReg())
      return Match_Success;
    break;
  default:
    break;
  }
  return Match_InvalidOperand;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializepISAAsmParser() {
  RegisterMCAsmParser<pISAAsmParser> X(getThepISATarget());
}
