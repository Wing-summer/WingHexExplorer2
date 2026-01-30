
// Generated from AngelscriptConsoleParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "AngelscriptConsoleParserVisitor.h"


/**
 * This class provides an empty implementation of AngelscriptConsoleParserVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  AngelscriptConsoleParserBaseVisitor : public AngelscriptConsoleParserVisitor {
public:

  virtual std::any visitScript(AngelscriptConsoleParser::ScriptContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEmptyDeclaration_(AngelscriptConsoleParser::EmptyDeclaration_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitScriptItem(AngelscriptConsoleParser::ScriptItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeclSpecifierSeq(AngelscriptConsoleParser::DeclSpecifierSeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeclSpecifier(AngelscriptConsoleParser::DeclSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeSpecifier(AngelscriptConsoleParser::TypeSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArraySpecifier(AngelscriptConsoleParser::ArraySpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTrailingTypeSpecifier(AngelscriptConsoleParser::TrailingTypeSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimpleTypeSpecifier(AngelscriptConsoleParser::SimpleTypeSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAsGeneric(AngelscriptConsoleParser::AsGenericContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimpleTypeSpecifierList(AngelscriptConsoleParser::SimpleTypeSpecifierListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConstantExpression(AngelscriptConsoleParser::ConstantExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInitDeclaratorList(AngelscriptConsoleParser::InitDeclaratorListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInitDeclarator(AngelscriptConsoleParser::InitDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpressionList(AngelscriptConsoleParser::ExpressionListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInitializer(AngelscriptConsoleParser::InitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimaryExpression(AngelscriptConsoleParser::PrimaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIdExpression(AngelscriptConsoleParser::IdExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnqualifiedId(AngelscriptConsoleParser::UnqualifiedIdContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitQualifiedId(AngelscriptConsoleParser::QualifiedIdContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNestedNameSpecifier(AngelscriptConsoleParser::NestedNameSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypedefName(AngelscriptConsoleParser::TypedefNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTheTypeName(AngelscriptConsoleParser::TheTypeNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDecltypeSpecifier(AngelscriptConsoleParser::DecltypeSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNamespaceName(AngelscriptConsoleParser::NamespaceNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitQualifiednamespacespecifier(AngelscriptConsoleParser::QualifiednamespacespecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPseudoDestructorName(AngelscriptConsoleParser::PseudoDestructorNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeSpecifierSeq(AngelscriptConsoleParser::TypeSpecifierSeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTheTypeId(AngelscriptConsoleParser::TheTypeIdContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPostfixExpression(AngelscriptConsoleParser::PostfixExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryExpression(AngelscriptConsoleParser::UnaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryOperator(AngelscriptConsoleParser::UnaryOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCastExpression(AngelscriptConsoleParser::CastExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMultiplicativeExpression(AngelscriptConsoleParser::MultiplicativeExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAdditiveExpression(AngelscriptConsoleParser::AdditiveExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShiftExpression(AngelscriptConsoleParser::ShiftExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShiftOperator(AngelscriptConsoleParser::ShiftOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRelationalExpression(AngelscriptConsoleParser::RelationalExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEqualityExpression(AngelscriptConsoleParser::EqualityExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAndExpression(AngelscriptConsoleParser::AndExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExclusiveOrExpression(AngelscriptConsoleParser::ExclusiveOrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInclusiveOrExpression(AngelscriptConsoleParser::InclusiveOrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalAndExpression(AngelscriptConsoleParser::LogicalAndExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalOrExpression(AngelscriptConsoleParser::LogicalOrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConditionalExpression(AngelscriptConsoleParser::ConditionalExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpression(AngelscriptConsoleParser::ExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignmentExpression(AngelscriptConsoleParser::AssignmentExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignmentOperator(AngelscriptConsoleParser::AssignmentOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBraceOrEqualInitializer(AngelscriptConsoleParser::BraceOrEqualInitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInitializerClause(AngelscriptConsoleParser::InitializerClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInitializerList(AngelscriptConsoleParser::InitializerListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBracedInitList(AngelscriptConsoleParser::BracedInitListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLiteral(AngelscriptConsoleParser::LiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBooleanLiteral(AngelscriptConsoleParser::BooleanLiteralContext *ctx) override {
    return visitChildren(ctx);
  }


};

