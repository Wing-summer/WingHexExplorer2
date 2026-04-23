
// Generated from Snippet.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  SnippetLexer : public antlr4::Lexer {
public:
  enum {
    TEXT_CONTENT = 1, ESCAPED_CHAR = 2, VARIABLE = 3, VARIABLE_BRACED = 4, 
    VARIABLE_WITH_DEFAULT = 5, PLACEHOLDER = 6, TABSTOP = 7, TABSTOP_BRACED = 8, 
    TABSTOP_WITH_DEFAULT = 9, CHOICE = 10, WS = 11
  };

  explicit SnippetLexer(antlr4::CharStream *input);

  ~SnippetLexer() override;


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

