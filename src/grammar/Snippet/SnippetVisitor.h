
// Generated from Snippet.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "SnippetParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by SnippetParser.
 */
class  SnippetVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by SnippetParser.
   */
    virtual std::any visitSnippet(SnippetParser::SnippetContext *context) = 0;

    virtual std::any visitText(SnippetParser::TextContext *context) = 0;

    virtual std::any visitEscapedChar(SnippetParser::EscapedCharContext *context) = 0;

    virtual std::any visitVariable(SnippetParser::VariableContext *context) = 0;

    virtual std::any visitVariableWithDefault(SnippetParser::VariableWithDefaultContext *context) = 0;

    virtual std::any visitPlaceholder(SnippetParser::PlaceholderContext *context) = 0;

    virtual std::any visitTabstop(SnippetParser::TabstopContext *context) = 0;

    virtual std::any visitTabstopWithDefault(SnippetParser::TabstopWithDefaultContext *context) = 0;

    virtual std::any visitChoice(SnippetParser::ChoiceContext *context) = 0;


};

