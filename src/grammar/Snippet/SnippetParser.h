
// Generated from Snippet.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  SnippetParser : public antlr4::Parser {
public:
  enum {
    TEXT_CONTENT = 1, ESCAPED_CHAR = 2, VARIABLE = 3, VARIABLE_BRACED = 4, 
    VARIABLE_WITH_DEFAULT = 5, PLACEHOLDER = 6, TABSTOP = 7, TABSTOP_BRACED = 8, 
    TABSTOP_WITH_DEFAULT = 9, CHOICE = 10, WS = 11
  };

  enum {
    RuleSnippet = 0, RulePart = 1
  };

  explicit SnippetParser(antlr4::TokenStream *input);

  SnippetParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~SnippetParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class SnippetContext;
  class PartContext; 

  class  SnippetContext : public antlr4::ParserRuleContext {
  public:
    SnippetContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<PartContext *> part();
    PartContext* part(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SnippetContext* snippet();

  class  PartContext : public antlr4::ParserRuleContext {
  public:
    PartContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    PartContext() = default;
    void copyFrom(PartContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  PlaceholderContext : public PartContext {
  public:
    PlaceholderContext(PartContext *ctx);

    antlr4::tree::TerminalNode *PLACEHOLDER();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  VariableContext : public PartContext {
  public:
    VariableContext(PartContext *ctx);

    antlr4::tree::TerminalNode *VARIABLE();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ChoiceContext : public PartContext {
  public:
    ChoiceContext(PartContext *ctx);

    antlr4::tree::TerminalNode *CHOICE();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  BracedTabstopContext : public PartContext {
  public:
    BracedTabstopContext(PartContext *ctx);

    antlr4::tree::TerminalNode *TABSTOP_BRACED();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  EscapedCharContext : public PartContext {
  public:
    EscapedCharContext(PartContext *ctx);

    antlr4::tree::TerminalNode *ESCAPED_CHAR();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  BracedVariableContext : public PartContext {
  public:
    BracedVariableContext(PartContext *ctx);

    antlr4::tree::TerminalNode *VARIABLE_BRACED();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  TextContext : public PartContext {
  public:
    TextContext(PartContext *ctx);

    antlr4::tree::TerminalNode *TEXT_CONTENT();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  VariableWithDefaultContext : public PartContext {
  public:
    VariableWithDefaultContext(PartContext *ctx);

    antlr4::tree::TerminalNode *VARIABLE_WITH_DEFAULT();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  TabstopContext : public PartContext {
  public:
    TabstopContext(PartContext *ctx);

    antlr4::tree::TerminalNode *TABSTOP();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  WhiteSpaceContext : public PartContext {
  public:
    WhiteSpaceContext(PartContext *ctx);

    antlr4::tree::TerminalNode *WS();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  TabstopWithDefaultContext : public PartContext {
  public:
    TabstopWithDefaultContext(PartContext *ctx);

    antlr4::tree::TerminalNode *TABSTOP_WITH_DEFAULT();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  PartContext* part();


  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

