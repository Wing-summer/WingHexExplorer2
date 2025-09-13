
// Generated from CStruct.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  CStructLexer : public antlr4::Lexer {
public:
  enum {
    T__0 = 1, Char = 2, Const = 3, Double = 4, Enum = 5, Float = 6, Int = 7, 
    Long = 8, TypeDef = 9, AlignAs = 10, SizeOf = 11, Short = 12, Signed = 13, 
    Struct = 14, Union = 15, Unsigned = 16, Void = 17, QuestionMark = 18, 
    LeftParen = 19, RightParen = 20, DoubleLeftBracket = 21, LeftBracket = 22, 
    DoubleRightBracket = 23, RightBracket = 24, LeftBrace = 25, RightBrace = 26, 
    LeftShift = 27, RightShift = 28, Plus = 29, PlusPlus = 30, Minus = 31, 
    MinusMinus = 32, Star = 33, Div = 34, Mod = 35, And = 36, Or = 37, Caret = 38, 
    Tilde = 39, Colon = 40, Semi = 41, Comma = 42, Identifier = 43, IntegerConstant = 44, 
    StringLiteral = 45, MultiLineMacroDefine = 46, MultiLineMacro = 47, 
    DirectiveInclude = 48, DirectiveDefine = 49, Directive = 50, Whitespace = 51, 
    Newline = 52, LINE_CONTINUATION = 53, BlockComment = 54, LineComment = 55
  };

  explicit CStructLexer(antlr4::CharStream *input);

  ~CStructLexer() override;


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

