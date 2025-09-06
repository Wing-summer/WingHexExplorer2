
// Generated from DefEvaluator.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "DefEvaluatorParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by DefEvaluatorParser.
 */
class  DefEvaluatorVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by DefEvaluatorParser.
   */
    virtual std::any visitPrimaryExpression(DefEvaluatorParser::PrimaryExpressionContext *context) = 0;

    virtual std::any visitUnaryExpression(DefEvaluatorParser::UnaryExpressionContext *context) = 0;

    virtual std::any visitUnaryOperator(DefEvaluatorParser::UnaryOperatorContext *context) = 0;

    virtual std::any visitCastExpression(DefEvaluatorParser::CastExpressionContext *context) = 0;

    virtual std::any visitRelationalExpression(DefEvaluatorParser::RelationalExpressionContext *context) = 0;

    virtual std::any visitEqualityExpression(DefEvaluatorParser::EqualityExpressionContext *context) = 0;

    virtual std::any visitMultiplicativeExpression(DefEvaluatorParser::MultiplicativeExpressionContext *context) = 0;

    virtual std::any visitAdditiveExpression(DefEvaluatorParser::AdditiveExpressionContext *context) = 0;

    virtual std::any visitShiftExpression(DefEvaluatorParser::ShiftExpressionContext *context) = 0;

    virtual std::any visitAndExpression(DefEvaluatorParser::AndExpressionContext *context) = 0;

    virtual std::any visitExclusiveOrExpression(DefEvaluatorParser::ExclusiveOrExpressionContext *context) = 0;

    virtual std::any visitInclusiveOrExpression(DefEvaluatorParser::InclusiveOrExpressionContext *context) = 0;

    virtual std::any visitLogicalAndExpression(DefEvaluatorParser::LogicalAndExpressionContext *context) = 0;

    virtual std::any visitLogicalOrExpression(DefEvaluatorParser::LogicalOrExpressionContext *context) = 0;

    virtual std::any visitConditionalExpression(DefEvaluatorParser::ConditionalExpressionContext *context) = 0;

    virtual std::any visitAssignmentExpression(DefEvaluatorParser::AssignmentExpressionContext *context) = 0;

    virtual std::any visitEntryExpression(DefEvaluatorParser::EntryExpressionContext *context) = 0;


};

