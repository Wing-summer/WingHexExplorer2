
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
    LeftShiftAssign = 39, RightShiftAssign = 40, ArithRightShiftAssign = 41, 
    Equal = 42, NotEqual = 43, LessEqual = 44, GreaterEqual = 45, AndAnd = 46, 
    OrOr = 47, PlusPlus = 48, MinusMinus = 49, Comma = 50, Question = 51, 
    Colon = 52, Doublecolon = 53, Semi = 54, Dot = 55, Identifier = 56, 
    DecimalLiteral = 57, OctalLiteral = 58, HexadecimalLiteral = 59, BinaryLiteral = 60, 
    Integersuffix = 61, UserDefinedIntegerLiteral = 62, UserDefinedFloatingLiteral = 63, 
    UserDefinedStringLiteral = 64, UserDefinedCharacterLiteral = 65, Whitespace = 66, 
    Newline = 67, BlockComment = 68, LineComment = 69
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

