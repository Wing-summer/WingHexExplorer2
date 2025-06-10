
// Generated from NumCal.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  NumCalLexer : public antlr4::Lexer {
public:
  enum {
    LeftParen = 1, RightParen = 2, LeftShift = 3, RightShift = 4, Plus = 5, 
    Minus = 6, Star = 7, Div = 8, Mod = 9, And = 10, Or = 11, Caret = 12, 
    Tilde = 13, LessThan = 14, Colon = 15, LeftBracket = 16, RightBracket = 17, 
    IntegerConstant = 18, Whitespace = 19
  };

  explicit NumCalLexer(antlr4::CharStream *input);

  ~NumCalLexer() override;


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

