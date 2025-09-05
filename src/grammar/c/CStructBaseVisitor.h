
// Generated from CStruct.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "CStructVisitor.h"


/**
 * This class provides an empty implementation of CStructVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  CStructBaseVisitor : public CStructVisitor {
public:

  virtual std::any visitInternalBasicTypes(CStructParser::InternalBasicTypesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSignOrUnsigned(CStructParser::SignOrUnsignedContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimaryExpression(CStructParser::PrimaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPostfixExpression(CStructParser::PostfixExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryExpression(CStructParser::UnaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryOperator(CStructParser::UnaryOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCastExpression(CStructParser::CastExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMultiplicativeExpression(CStructParser::MultiplicativeExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAdditiveExpression(CStructParser::AdditiveExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShiftExpression(CStructParser::ShiftExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAndExpression(CStructParser::AndExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExclusiveOrExpression(CStructParser::ExclusiveOrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInclusiveOrExpression(CStructParser::InclusiveOrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignmentExpressionDef(CStructParser::AssignmentExpressionDefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignmentExpression(CStructParser::AssignmentExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeclaration(CStructParser::DeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeclarationSpecifier(CStructParser::DeclarationSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeSpecifier(CStructParser::TypeSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStructOrUnionSpecifier(CStructParser::StructOrUnionSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStructOrUnion(CStructParser::StructOrUnionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlignAsAttr(CStructParser::AlignAsAttrContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStructDeclarationList(CStructParser::StructDeclarationListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStructDeclaration(CStructParser::StructDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSpecifierQualifierList(CStructParser::SpecifierQualifierListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStructDeclaratorList(CStructParser::StructDeclaratorListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStructDeclarator(CStructParser::StructDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumSpecifier(CStructParser::EnumSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumeratorList(CStructParser::EnumeratorListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumerator(CStructParser::EnumeratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumerationConstant(CStructParser::EnumerationConstantContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeclarator(CStructParser::DeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDirectDeclarator(CStructParser::DirectDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPointer(CStructParser::PointerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIdentifierList(CStructParser::IdentifierListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeName(CStructParser::TypeNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAbstractDeclarator(CStructParser::AbstractDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDirectAbstractDeclarator(CStructParser::DirectAbstractDeclaratorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCompilationUnit(CStructParser::CompilationUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTranslationUnit(CStructParser::TranslationUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExternalDeclaration(CStructParser::ExternalDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDefineDecl(CStructParser::DefineDeclContext *ctx) override {
    return visitChildren(ctx);
  }


};

