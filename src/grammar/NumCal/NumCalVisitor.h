
// Generated from NumCal.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "NumCalParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by NumCalParser.
 */
class  NumCalVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by NumCalParser.
   */
    virtual std::any visitPrimaryExpression(NumCalParser::PrimaryExpressionContext *context) = 0;

    virtual std::any visitUnaryExpression(NumCalParser::UnaryExpressionContext *context) = 0;

    virtual std::any visitUnaryOperator(NumCalParser::UnaryOperatorContext *context) = 0;

    virtual std::any visitCastExpression(NumCalParser::CastExpressionContext *context) = 0;

    virtual std::any visitMultiplicativeExpression(NumCalParser::MultiplicativeExpressionContext *context) = 0;

    virtual std::any visitAdditiveExpression(NumCalParser::AdditiveExpressionContext *context) = 0;

    virtual std::any visitShiftExpression(NumCalParser::ShiftExpressionContext *context) = 0;

    virtual std::any visitAndExpression(NumCalParser::AndExpressionContext *context) = 0;

    virtual std::any visitExclusiveOrExpression(NumCalParser::ExclusiveOrExpressionContext *context) = 0;

    virtual std::any visitInclusiveOrExpression(NumCalParser::InclusiveOrExpressionContext *context) = 0;

    virtual std::any visitAssignmentExpression(NumCalParser::AssignmentExpressionContext *context) = 0;

    virtual std::any visitEntryExpression(NumCalParser::EntryExpressionContext *context) = 0;

    virtual std::any visitPrefixGoto(NumCalParser::PrefixGotoContext *context) = 0;


};

