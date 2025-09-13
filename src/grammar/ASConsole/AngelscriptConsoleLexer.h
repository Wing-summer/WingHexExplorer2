
// Generated from AngelscriptConsoleLexer.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  AngelscriptConsoleLexer : public antlr4::Lexer {
public:
  enum {
    IntegerLiteral = 1, CharacterLiteral = 2, FloatingLiteral = 3, StringLiteral = 4, 
    UserDefinedLiteral = 5, Cast = 6, Int = 7, Int8 = 8, Int16 = 9, Int32 = 10, 
    Int64 = 11, UInt = 12, UInt8 = 13, UInt16 = 14, UInt32 = 15, UInt64 = 16, 
    Float = 17, Double = 18, Bool = 19, Auto = 20, Break = 21, Case = 22, 
    Catch = 23, Char = 24, Const = 25, Continue = 26, Default = 27, Do = 28, 
    Else = 29, False_ = 30, For = 31, If = 32, Nullptr = 33, Return = 34, 
    Short = 35, Switch = 36, True_ = 37, Void = 38, While = 39, LeftParen = 40, 
    RightParen = 41, LeftBracket = 42, RightBracket = 43, LeftBrace = 44, 
    RightBrace = 45, Plus = 46, Minus = 47, Star = 48, Div = 49, Mod = 50, 
    Xor = 51, And = 52, Or = 53, Tilde = 54, Not = 55, Assign = 56, Less = 57, 
    Greater = 58, PlusAssign = 59, MinusAssign = 60, StarAssign = 61, DivAssign = 62, 
    ModAssign = 63, XorAssign = 64, AndAssign = 65, OrAssign = 66, LeftShiftAssign = 67, 
    RightShiftAssign = 68, Equal = 69, NotEqual = 70, LessEqual = 71, GreaterEqual = 72, 
    AndAnd = 73, OrOr = 74, PlusPlus = 75, MinusMinus = 76, Comma = 77, 
    Question = 78, Colon = 79, Doublecolon = 80, Semi = 81, Dot = 82, Identifier = 83, 
    DecimalLiteral = 84, OctalLiteral = 85, HexadecimalLiteral = 86, BinaryLiteral = 87, 
    Integersuffix = 88, UserDefinedIntegerLiteral = 89, UserDefinedFloatingLiteral = 90, 
    UserDefinedStringLiteral = 91, UserDefinedCharacterLiteral = 92, Whitespace = 93, 
    Newline = 94, BlockComment = 95, LineComment = 96
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

