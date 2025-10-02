
// Generated from Snippet.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "SnippetVisitor.h"


/**
 * This class provides an empty implementation of SnippetVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  SnippetBaseVisitor : public SnippetVisitor {
public:

  virtual std::any visitSnippet(SnippetParser::SnippetContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitText(SnippetParser::TextContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEscapedChar(SnippetParser::EscapedCharContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVariable(SnippetParser::VariableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVariableWithDefault(SnippetParser::VariableWithDefaultContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPlaceholder(SnippetParser::PlaceholderContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTabstop(SnippetParser::TabstopContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTabstopWithDefault(SnippetParser::TabstopWithDefaultContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitChoice(SnippetParser::ChoiceContext *ctx) override {
    return visitChildren(ctx);
  }


};

