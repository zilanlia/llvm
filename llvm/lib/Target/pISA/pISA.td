//===-- pISA.td - Describe the pISA Target Machine ------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

include "pISARegisterInfo.td"
include "pISARegisterBanks.td"
include "pISAInstrInfo.td"
include "pISACombine.td"

def pISAInstrInfo : InstrInfo;

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

def pISA10 : SubtargetFeature<"pISA1.0", "pISAVersion", "10",
                              "Use pISA version 1.0">;

def pISAInstPrinter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  bit isMCAsmWriter = 1;
}

def pISAParser : AsmParser {
  let AsmParserClassName  = "AsmParser";
  let ShouldEmitMatchRegisterName = 1;
  bit HasMnemonicFirst = 0;
}

def pISAParserVariant : AsmParserVariant {
  int Variant = 0;
  string Name = "pISA";

  // TokenizingCharacters - Characters that are standalone tokens
  string TokenizingCharacters = ";()[]";

  // Discard comments in assembly strings.
  string CommentDelimiter = "#";

  // Recognize hard coded registers.
  string RegisterPrefix = "%";
}

def pISA : Target {
  let InstructionSet = pISAInstrInfo;
  let AssemblyWriters = [pISAInstPrinter];
  let AssemblyParsers = [pISAParser];
  let AssemblyParserVariants = [pISAParserVariant];
}
