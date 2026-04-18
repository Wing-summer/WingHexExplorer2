
// Generated from AngelscriptExprParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  AngelscriptExprParserLexer : public antlr4::Lexer {
public:
  enum {
    True_ = 1, False_ = 2, Nullptr = 3, LeftParen = 4, RightParen = 5, Plus = 6, 
    Minus = 7, Star = 8, Div = 9, Mod = 10, Xor = 11, And = 12, Or = 13, 
    Tilde = 14, Not = 15, Less = 16, Greater = 17, Equal = 18, NotEqual = 19, 
    LessEqual = 20, GreaterEqual = 21, AndAnd = 22, OrOr = 23, IntegerLiteral = 24, 
    FloatingLiteral = 25, CharacterLiteral = 26, StringLiteral = 27, Whitespace = 28, 
    Newline = 29, BlockComment = 30, LineComment = 31
  };

  explicit AngelscriptExprParserLexer(antlr4::CharStream *input);

  ~AngelscriptExprParserLexer() override;


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

