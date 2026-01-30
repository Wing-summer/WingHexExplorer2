
// Generated from AngelscriptConsoleParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "AngelscriptConsoleParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by AngelscriptConsoleParser.
 */
class  AngelscriptConsoleParserVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by AngelscriptConsoleParser.
   */
    virtual std::any visitScript(AngelscriptConsoleParser::ScriptContext *context) = 0;

    virtual std::any visitEmptyDeclaration_(AngelscriptConsoleParser::EmptyDeclaration_Context *context) = 0;

    virtual std::any visitScriptItem(AngelscriptConsoleParser::ScriptItemContext *context) = 0;

    virtual std::any visitDeclSpecifierSeq(AngelscriptConsoleParser::DeclSpecifierSeqContext *context) = 0;

    virtual std::any visitDeclSpecifier(AngelscriptConsoleParser::DeclSpecifierContext *context) = 0;

    virtual std::any visitTypeSpecifier(AngelscriptConsoleParser::TypeSpecifierContext *context) = 0;

    virtual std::any visitArraySpecifier(AngelscriptConsoleParser::ArraySpecifierContext *context) = 0;

    virtual std::any visitTrailingTypeSpecifier(AngelscriptConsoleParser::TrailingTypeSpecifierContext *context) = 0;

    virtual std::any visitSimpleTypeSpecifier(AngelscriptConsoleParser::SimpleTypeSpecifierContext *context) = 0;

    virtual std::any visitAsGeneric(AngelscriptConsoleParser::AsGenericContext *context) = 0;

    virtual std::any visitSimpleTypeSpecifierList(AngelscriptConsoleParser::SimpleTypeSpecifierListContext *context) = 0;

    virtual std::any visitConstantExpression(AngelscriptConsoleParser::ConstantExpressionContext *context) = 0;

    virtual std::any visitInitDeclaratorList(AngelscriptConsoleParser::InitDeclaratorListContext *context) = 0;

    virtual std::any visitInitDeclarator(AngelscriptConsoleParser::InitDeclaratorContext *context) = 0;

    virtual std::any visitExpressionList(AngelscriptConsoleParser::ExpressionListContext *context) = 0;

    virtual std::any visitInitializer(AngelscriptConsoleParser::InitializerContext *context) = 0;

    virtual std::any visitPrimaryExpression(AngelscriptConsoleParser::PrimaryExpressionContext *context) = 0;

    virtual std::any visitIdExpression(AngelscriptConsoleParser::IdExpressionContext *context) = 0;

    virtual std::any visitUnqualifiedId(AngelscriptConsoleParser::UnqualifiedIdContext *context) = 0;

    virtual std::any visitQualifiedId(AngelscriptConsoleParser::QualifiedIdContext *context) = 0;

    virtual std::any visitNestedNameSpecifier(AngelscriptConsoleParser::NestedNameSpecifierContext *context) = 0;

    virtual std::any visitTypedefName(AngelscriptConsoleParser::TypedefNameContext *context) = 0;

    virtual std::any visitTheTypeName(AngelscriptConsoleParser::TheTypeNameContext *context) = 0;

    virtual std::any visitDecltypeSpecifier(AngelscriptConsoleParser::DecltypeSpecifierContext *context) = 0;

    virtual std::any visitNamespaceName(AngelscriptConsoleParser::NamespaceNameContext *context) = 0;

    virtual std::any visitQualifiednamespacespecifier(AngelscriptConsoleParser::QualifiednamespacespecifierContext *context) = 0;

    virtual std::any visitPseudoDestructorName(AngelscriptConsoleParser::PseudoDestructorNameContext *context) = 0;

    virtual std::any visitTypeSpecifierSeq(AngelscriptConsoleParser::TypeSpecifierSeqContext *context) = 0;

    virtual std::any visitTheTypeId(AngelscriptConsoleParser::TheTypeIdContext *context) = 0;

    virtual std::any visitPostfixExpression(AngelscriptConsoleParser::PostfixExpressionContext *context) = 0;

    virtual std::any visitUnaryExpression(AngelscriptConsoleParser::UnaryExpressionContext *context) = 0;

    virtual std::any visitUnaryOperator(AngelscriptConsoleParser::UnaryOperatorContext *context) = 0;

    virtual std::any visitCastExpression(AngelscriptConsoleParser::CastExpressionContext *context) = 0;

    virtual std::any visitMultiplicativeExpression(AngelscriptConsoleParser::MultiplicativeExpressionContext *context) = 0;

    virtual std::any visitAdditiveExpression(AngelscriptConsoleParser::AdditiveExpressionContext *context) = 0;

    virtual std::any visitShiftExpression(AngelscriptConsoleParser::ShiftExpressionContext *context) = 0;

    virtual std::any visitShiftOperator(AngelscriptConsoleParser::ShiftOperatorContext *context) = 0;

    virtual std::any visitRelationalExpression(AngelscriptConsoleParser::RelationalExpressionContext *context) = 0;

    virtual std::any visitEqualityExpression(AngelscriptConsoleParser::EqualityExpressionContext *context) = 0;

    virtual std::any visitAndExpression(AngelscriptConsoleParser::AndExpressionContext *context) = 0;

    virtual std::any visitExclusiveOrExpression(AngelscriptConsoleParser::ExclusiveOrExpressionContext *context) = 0;

    virtual std::any visitInclusiveOrExpression(AngelscriptConsoleParser::InclusiveOrExpressionContext *context) = 0;

    virtual std::any visitLogicalAndExpression(AngelscriptConsoleParser::LogicalAndExpressionContext *context) = 0;

    virtual std::any visitLogicalOrExpression(AngelscriptConsoleParser::LogicalOrExpressionContext *context) = 0;

    virtual std::any visitConditionalExpression(AngelscriptConsoleParser::ConditionalExpressionContext *context) = 0;

    virtual std::any visitExpression(AngelscriptConsoleParser::ExpressionContext *context) = 0;

    virtual std::any visitAssignmentExpression(AngelscriptConsoleParser::AssignmentExpressionContext *context) = 0;

    virtual std::any visitAssignmentOperator(AngelscriptConsoleParser::AssignmentOperatorContext *context) = 0;

    virtual std::any visitBraceOrEqualInitializer(AngelscriptConsoleParser::BraceOrEqualInitializerContext *context) = 0;

    virtual std::any visitInitializerClause(AngelscriptConsoleParser::InitializerClauseContext *context) = 0;

    virtual std::any visitInitializerList(AngelscriptConsoleParser::InitializerListContext *context) = 0;

    virtual std::any visitBracedInitList(AngelscriptConsoleParser::BracedInitListContext *context) = 0;

    virtual std::any visitLiteral(AngelscriptConsoleParser::LiteralContext *context) = 0;

    virtual std::any visitBooleanLiteral(AngelscriptConsoleParser::BooleanLiteralContext *context) = 0;


};

