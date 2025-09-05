
// Generated from CStruct.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  CStructLexer : public antlr4::Lexer {
public:
  enum {
    T__0 = 1, Char = 2, Const = 3, Double = 4, Enum = 5, Float = 6, Int = 7, 
    Long = 8, TypeDef = 9, AlignAs = 10, SizeOf = 11, Short = 12, Signed = 13, 
    Struct = 14, Union = 15, Unsigned = 16, Void = 17, QuestionMark = 18, 
    LeftParen = 19, RightParen = 20, LeftBracket = 21, RightBracket = 22, 
    LeftBrace = 23, RightBrace = 24, LeftShift = 25, RightShift = 26, Plus = 27, 
    PlusPlus = 28, Minus = 29, MinusMinus = 30, Star = 31, Div = 32, Mod = 33, 
    And = 34, Or = 35, Caret = 36, Tilde = 37, Colon = 38, Semi = 39, Comma = 40, 
    Identifier = 41, IntegerConstant = 42, StringLiteral = 43, MultiLineMacroDefine = 44, 
    MultiLineMacro = 45, DirectiveInclude = 46, DirectiveDefine = 47, Directive = 48, 
    Whitespace = 49, Newline = 50, LINE_CONTINUATION = 51, BlockComment = 52, 
    LineComment = 53
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

