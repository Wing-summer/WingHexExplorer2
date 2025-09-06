
// Generated from DefEvaluator.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "DefEvaluatorVisitor.h"


/**
 * This class provides an empty implementation of DefEvaluatorVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  DefEvaluatorBaseVisitor : public DefEvaluatorVisitor {
public:

  virtual std::any visitPrimaryExpression(DefEvaluatorParser::PrimaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryExpression(DefEvaluatorParser::UnaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryOperator(DefEvaluatorParser::UnaryOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCastExpression(DefEvaluatorParser::CastExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRelationalExpression(DefEvaluatorParser::RelationalExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEqualityExpression(DefEvaluatorParser::EqualityExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMultiplicativeExpression(DefEvaluatorParser::MultiplicativeExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAdditiveExpression(DefEvaluatorParser::AdditiveExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShiftExpression(DefEvaluatorParser::ShiftExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAndExpression(DefEvaluatorParser::AndExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExclusiveOrExpression(DefEvaluatorParser::ExclusiveOrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInclusiveOrExpression(DefEvaluatorParser::InclusiveOrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalAndExpression(DefEvaluatorParser::LogicalAndExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalOrExpression(DefEvaluatorParser::LogicalOrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConditionalExpression(DefEvaluatorParser::ConditionalExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignmentExpression(DefEvaluatorParser::AssignmentExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEntryExpression(DefEvaluatorParser::EntryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }


};

