
// Generated from CStruct.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "CStructParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by CStructParser.
 */
class  CStructVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by CStructParser.
   */
    virtual std::any visitInternalBasicTypes(CStructParser::InternalBasicTypesContext *context) = 0;

    virtual std::any visitSignOrUnsigned(CStructParser::SignOrUnsignedContext *context) = 0;

    virtual std::any visitPrimaryExpression(CStructParser::PrimaryExpressionContext *context) = 0;

    virtual std::any visitPostfixExpression(CStructParser::PostfixExpressionContext *context) = 0;

    virtual std::any visitUnaryExpression(CStructParser::UnaryExpressionContext *context) = 0;

    virtual std::any visitUnaryOperator(CStructParser::UnaryOperatorContext *context) = 0;

    virtual std::any visitCastExpression(CStructParser::CastExpressionContext *context) = 0;

    virtual std::any visitMultiplicativeExpression(CStructParser::MultiplicativeExpressionContext *context) = 0;

    virtual std::any visitAdditiveExpression(CStructParser::AdditiveExpressionContext *context) = 0;

    virtual std::any visitShiftExpression(CStructParser::ShiftExpressionContext *context) = 0;

    virtual std::any visitAndExpression(CStructParser::AndExpressionContext *context) = 0;

    virtual std::any visitExclusiveOrExpression(CStructParser::ExclusiveOrExpressionContext *context) = 0;

    virtual std::any visitInclusiveOrExpression(CStructParser::InclusiveOrExpressionContext *context) = 0;

    virtual std::any visitAssignmentExpressionDef(CStructParser::AssignmentExpressionDefContext *context) = 0;

    virtual std::any visitAssignmentExpression(CStructParser::AssignmentExpressionContext *context) = 0;

    virtual std::any visitDeclaration(CStructParser::DeclarationContext *context) = 0;

    virtual std::any visitDeclarationSpecifier(CStructParser::DeclarationSpecifierContext *context) = 0;

    virtual std::any visitTypeSpecifier(CStructParser::TypeSpecifierContext *context) = 0;

    virtual std::any visitStructOrUnionSpecifier(CStructParser::StructOrUnionSpecifierContext *context) = 0;

    virtual std::any visitStructOrUnion(CStructParser::StructOrUnionContext *context) = 0;

    virtual std::any visitAlignAsAttr(CStructParser::AlignAsAttrContext *context) = 0;

    virtual std::any visitStructDeclarationList(CStructParser::StructDeclarationListContext *context) = 0;

    virtual std::any visitStructDeclaration(CStructParser::StructDeclarationContext *context) = 0;

    virtual std::any visitSpecifierQualifierList(CStructParser::SpecifierQualifierListContext *context) = 0;

    virtual std::any visitStructDeclaratorList(CStructParser::StructDeclaratorListContext *context) = 0;

    virtual std::any visitStructDeclarator(CStructParser::StructDeclaratorContext *context) = 0;

    virtual std::any visitEnumSpecifier(CStructParser::EnumSpecifierContext *context) = 0;

    virtual std::any visitEnumeratorList(CStructParser::EnumeratorListContext *context) = 0;

    virtual std::any visitEnumerator(CStructParser::EnumeratorContext *context) = 0;

    virtual std::any visitEnumerationConstant(CStructParser::EnumerationConstantContext *context) = 0;

    virtual std::any visitDeclarator(CStructParser::DeclaratorContext *context) = 0;

    virtual std::any visitDirectDeclarator(CStructParser::DirectDeclaratorContext *context) = 0;

    virtual std::any visitPointer(CStructParser::PointerContext *context) = 0;

    virtual std::any visitIdentifierList(CStructParser::IdentifierListContext *context) = 0;

    virtual std::any visitTypeName(CStructParser::TypeNameContext *context) = 0;

    virtual std::any visitAbstractDeclarator(CStructParser::AbstractDeclaratorContext *context) = 0;

    virtual std::any visitDirectAbstractDeclarator(CStructParser::DirectAbstractDeclaratorContext *context) = 0;

    virtual std::any visitCompilationUnit(CStructParser::CompilationUnitContext *context) = 0;

    virtual std::any visitTranslationUnit(CStructParser::TranslationUnitContext *context) = 0;

    virtual std::any visitExternalDeclaration(CStructParser::ExternalDeclarationContext *context) = 0;

    virtual std::any visitDefineDecl(CStructParser::DefineDeclContext *context) = 0;


};

