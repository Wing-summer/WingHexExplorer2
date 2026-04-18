
// Generated from AngelscriptExprParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "AngelscriptExprParserVisitor.h"


/**
 * This class provides an empty implementation of AngelscriptExprParserVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  AngelscriptExprParserBaseVisitor : public AngelscriptExprParserVisitor {
public:

  virtual std::any visitExpr(AngelscriptExprParserParser::ExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalOrExpr(AngelscriptExprParserParser::LogicalOrExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalAndExpr(AngelscriptExprParserParser::LogicalAndExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBitOrExpr(AngelscriptExprParserParser::BitOrExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBitXorExpr(AngelscriptExprParserParser::BitXorExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBitAndExpr(AngelscriptExprParserParser::BitAndExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEqualityExpr(AngelscriptExprParserParser::EqualityExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRelationalExpr(AngelscriptExprParserParser::RelationalExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShiftExpr(AngelscriptExprParserParser::ShiftExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAdditiveExpr(AngelscriptExprParserParser::AdditiveExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMultiplicativeExpr(AngelscriptExprParserParser::MultiplicativeExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryExpr(AngelscriptExprParserParser::UnaryExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimaryExpr(AngelscriptExprParserParser::PrimaryExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLiteral(AngelscriptExprParserParser::LiteralContext *ctx) override {
    return visitChildren(ctx);
  }


};

