
// Generated from NumCal.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "NumCalVisitor.h"


/**
 * This class provides an empty implementation of NumCalVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  NumCalBaseVisitor : public NumCalVisitor {
public:

  virtual std::any visitPrimaryExpression(NumCalParser::PrimaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryExpression(NumCalParser::UnaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryOperator(NumCalParser::UnaryOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCastExpression(NumCalParser::CastExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMultiplicativeExpression(NumCalParser::MultiplicativeExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAdditiveExpression(NumCalParser::AdditiveExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShiftExpression(NumCalParser::ShiftExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAndExpression(NumCalParser::AndExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExclusiveOrExpression(NumCalParser::ExclusiveOrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInclusiveOrExpression(NumCalParser::InclusiveOrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignmentExpression(NumCalParser::AssignmentExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEntryExpression(NumCalParser::EntryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrefixGoto(NumCalParser::PrefixGotoContext *ctx) override {
    return visitChildren(ctx);
  }


};

