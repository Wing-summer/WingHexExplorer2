
// Generated from AngelscriptExprParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "AngelscriptExprParserParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by AngelscriptExprParserParser.
 */
class  AngelscriptExprParserVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by AngelscriptExprParserParser.
   */
    virtual std::any visitExpr(AngelscriptExprParserParser::ExprContext *context) = 0;

    virtual std::any visitLogicalOrExpr(AngelscriptExprParserParser::LogicalOrExprContext *context) = 0;

    virtual std::any visitLogicalAndExpr(AngelscriptExprParserParser::LogicalAndExprContext *context) = 0;

    virtual std::any visitBitOrExpr(AngelscriptExprParserParser::BitOrExprContext *context) = 0;

    virtual std::any visitBitXorExpr(AngelscriptExprParserParser::BitXorExprContext *context) = 0;

    virtual std::any visitBitAndExpr(AngelscriptExprParserParser::BitAndExprContext *context) = 0;

    virtual std::any visitEqualityExpr(AngelscriptExprParserParser::EqualityExprContext *context) = 0;

    virtual std::any visitRelationalExpr(AngelscriptExprParserParser::RelationalExprContext *context) = 0;

    virtual std::any visitShiftExpr(AngelscriptExprParserParser::ShiftExprContext *context) = 0;

    virtual std::any visitAdditiveExpr(AngelscriptExprParserParser::AdditiveExprContext *context) = 0;

    virtual std::any visitMultiplicativeExpr(AngelscriptExprParserParser::MultiplicativeExprContext *context) = 0;

    virtual std::any visitUnaryExpr(AngelscriptExprParserParser::UnaryExprContext *context) = 0;

    virtual std::any visitPrimaryExpr(AngelscriptExprParserParser::PrimaryExprContext *context) = 0;

    virtual std::any visitLiteral(AngelscriptExprParserParser::LiteralContext *context) = 0;


};

