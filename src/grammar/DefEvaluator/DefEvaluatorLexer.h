
// Generated from DefEvaluator.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  DefEvaluatorLexer : public antlr4::Lexer {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, LeftParen = 10, RightParen = 11, LeftShift = 12, 
    RightShift = 13, Plus = 14, Minus = 15, Star = 16, Div = 17, Mod = 18, 
    And = 19, Or = 20, Caret = 21, Not = 22, Tilde = 23, LessThan = 24, 
    Constant = 25, IntegerConstant = 26, Whitespace = 27
  };

  explicit DefEvaluatorLexer(antlr4::CharStream *input);

  ~DefEvaluatorLexer() override;


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

