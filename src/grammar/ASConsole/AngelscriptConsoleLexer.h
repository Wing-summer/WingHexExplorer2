
// Generated from AngelscriptConsoleLexer.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  AngelscriptConsoleLexer : public antlr4::Lexer {
public:
  enum {
    IntegerLiteral = 1, CharacterLiteral = 2, FloatingLiteral = 3, StringLiteral = 4, 
    UserDefinedLiteral = 5, Cast = 6, Auto = 7, Const = 8, False_ = 9, Nullptr = 10, 
    True_ = 11, LeftParen = 12, RightParen = 13, LeftBracket = 14, RightBracket = 15, 
    LeftBrace = 16, RightBrace = 17, Plus = 18, Minus = 19, Star = 20, Div = 21, 
    Mod = 22, Xor = 23, And = 24, Or = 25, Tilde = 26, Not = 27, Assign = 28, 
    Less = 29, Greater = 30, PlusAssign = 31, MinusAssign = 32, StarAssign = 33, 
    DivAssign = 34, ModAssign = 35, XorAssign = 36, AndAssign = 37, OrAssign = 38, 
    LeftShiftAssign = 39, RightShiftAssign = 40, Equal = 41, NotEqual = 42, 
    LessEqual = 43, GreaterEqual = 44, AndAnd = 45, OrOr = 46, PlusPlus = 47, 
    MinusMinus = 48, Comma = 49, Question = 50, Colon = 51, Doublecolon = 52, 
    Semi = 53, Dot = 54, Identifier = 55, DecimalLiteral = 56, OctalLiteral = 57, 
    HexadecimalLiteral = 58, BinaryLiteral = 59, Integersuffix = 60, UserDefinedIntegerLiteral = 61, 
    UserDefinedFloatingLiteral = 62, UserDefinedStringLiteral = 63, UserDefinedCharacterLiteral = 64, 
    Whitespace = 65, Newline = 66, BlockComment = 67, LineComment = 68
  };

  explicit AngelscriptConsoleLexer(antlr4::CharStream *input);

  ~AngelscriptConsoleLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

