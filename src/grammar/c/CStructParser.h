
// Generated from CStruct.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  CStructParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, Char = 2, Const = 3, Double = 4, Enum = 5, Float = 6, Int = 7, 
    Long = 8, TypeDef = 9, AlignAs = 10, SizeOf = 11, Short = 12, Signed = 13, 
    Struct = 14, Union = 15, Unsigned = 16, Void = 17, QuestionMark = 18, 
    LeftParen = 19, RightParen = 20, LeftBracket = 21, RightBracket = 22, 
    LeftBrace = 23, RightBrace = 24, LeftShift = 25, RightShift = 26, Plus = 27, 
    PlusPlus = 28, Minus = 29, MinusMinus = 30, Star = 31, Div = 32, Mod = 33, 
    And = 34, Or = 35, Caret = 36, Tilde = 37, Colon = 38, Semi = 39, Comma = 40, 
    Identifier = 41, IntegerConstant = 42, StringLiteral = 43, MultiLineMacroDefine = 44, 
    MultiLineMacro = 45, DirectiveInclude = 46, DirectiveDefine = 47, Directive = 48, 
    Whitespace = 49, Newline = 50, LINE_CONTINUATION = 51, BlockComment = 52, 
    LineComment = 53
  };

  enum {
    RuleInternalBasicTypes = 0, RuleSignOrUnsigned = 1, RulePrimaryExpression = 2, 
    RulePostfixExpression = 3, RuleUnaryExpression = 4, RuleUnaryOperator = 5, 
    RuleCastExpression = 6, RuleMultiplicativeExpression = 7, RuleAdditiveExpression = 8, 
    RuleShiftExpression = 9, RuleAndExpression = 10, RuleExclusiveOrExpression = 11, 
    RuleInclusiveOrExpression = 12, RuleAssignmentExpressionDef = 13, RuleAssignmentExpression = 14, 
    RuleDeclaration = 15, RuleDeclarationSpecifier = 16, RuleTypeSpecifier = 17, 
    RuleStructOrUnionSpecifier = 18, RuleStructOrUnion = 19, RuleAlignAsAttr = 20, 
    RuleStructDeclarationList = 21, RuleStructDeclaration = 22, RuleSpecifierQualifierList = 23, 
    RuleStructDeclaratorList = 24, RuleStructDeclarator = 25, RuleEnumSpecifier = 26, 
    RuleEnumeratorList = 27, RuleEnumerator = 28, RuleEnumerationConstant = 29, 
    RuleDeclarator = 30, RuleDirectDeclarator = 31, RulePointer = 32, RuleIdentifierList = 33, 
    RuleTypeName = 34, RuleAbstractDeclarator = 35, RuleDirectAbstractDeclarator = 36, 
    RuleCompilationUnit = 37, RuleTranslationUnit = 38, RuleExternalDeclaration = 39, 
    RuleDefineDecl = 40
  };

  explicit CStructParser(antlr4::TokenStream *input);

  CStructParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~CStructParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class InternalBasicTypesContext;
  class SignOrUnsignedContext;
  class PrimaryExpressionContext;
  class PostfixExpressionContext;
  class UnaryExpressionContext;
  class UnaryOperatorContext;
  class CastExpressionContext;
  class MultiplicativeExpressionContext;
  class AdditiveExpressionContext;
  class ShiftExpressionContext;
  class AndExpressionContext;
  class ExclusiveOrExpressionContext;
  class InclusiveOrExpressionContext;
  class AssignmentExpressionDefContext;
  class AssignmentExpressionContext;
  class DeclarationContext;
  class DeclarationSpecifierContext;
  class TypeSpecifierContext;
  class StructOrUnionSpecifierContext;
  class StructOrUnionContext;
  class AlignAsAttrContext;
  class StructDeclarationListContext;
  class StructDeclarationContext;
  class SpecifierQualifierListContext;
  class StructDeclaratorListContext;
  class StructDeclaratorContext;
  class EnumSpecifierContext;
  class EnumeratorListContext;
  class EnumeratorContext;
  class EnumerationConstantContext;
  class DeclaratorContext;
  class DirectDeclaratorContext;
  class PointerContext;
  class IdentifierListContext;
  class TypeNameContext;
  class AbstractDeclaratorContext;
  class DirectAbstractDeclaratorContext;
  class CompilationUnitContext;
  class TranslationUnitContext;
  class ExternalDeclarationContext;
  class DefineDeclContext; 

  class  InternalBasicTypesContext : public antlr4::ParserRuleContext {
  public:
    InternalBasicTypesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Char();
    antlr4::tree::TerminalNode *Short();
    antlr4::tree::TerminalNode *Int();
    std::vector<antlr4::tree::TerminalNode *> Long();
    antlr4::tree::TerminalNode* Long(size_t i);
    SignOrUnsignedContext *signOrUnsigned();
    antlr4::tree::TerminalNode *Const();
    antlr4::tree::TerminalNode *Float();
    antlr4::tree::TerminalNode *Double();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InternalBasicTypesContext* internalBasicTypes();

  class  SignOrUnsignedContext : public antlr4::ParserRuleContext {
  public:
    SignOrUnsignedContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Signed();
    antlr4::tree::TerminalNode *Unsigned();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SignOrUnsignedContext* signOrUnsigned();

  class  PrimaryExpressionContext : public antlr4::ParserRuleContext {
  public:
    PrimaryExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Identifier();
    antlr4::tree::TerminalNode *IntegerConstant();
    antlr4::tree::TerminalNode *LeftParen();
    AssignmentExpressionContext *assignmentExpression();
    antlr4::tree::TerminalNode *RightParen();
    antlr4::tree::TerminalNode *SizeOf();
    SpecifierQualifierListContext *specifierQualifierList();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrimaryExpressionContext* primaryExpression();

  class  PostfixExpressionContext : public antlr4::ParserRuleContext {
  public:
    PostfixExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PrimaryExpressionContext *primaryExpression();
    std::vector<antlr4::tree::TerminalNode *> PlusPlus();
    antlr4::tree::TerminalNode* PlusPlus(size_t i);
    std::vector<antlr4::tree::TerminalNode *> MinusMinus();
    antlr4::tree::TerminalNode* MinusMinus(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PostfixExpressionContext* postfixExpression();

  class  UnaryExpressionContext : public antlr4::ParserRuleContext {
  public:
    UnaryExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PostfixExpressionContext *postfixExpression();
    UnaryOperatorContext *unaryOperator();
    CastExpressionContext *castExpression();
    std::vector<antlr4::tree::TerminalNode *> PlusPlus();
    antlr4::tree::TerminalNode* PlusPlus(size_t i);
    std::vector<antlr4::tree::TerminalNode *> MinusMinus();
    antlr4::tree::TerminalNode* MinusMinus(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnaryExpressionContext* unaryExpression();

  class  UnaryOperatorContext : public antlr4::ParserRuleContext {
  public:
    UnaryOperatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Plus();
    antlr4::tree::TerminalNode *Minus();
    antlr4::tree::TerminalNode *Tilde();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnaryOperatorContext* unaryOperator();

  class  CastExpressionContext : public antlr4::ParserRuleContext {
  public:
    CastExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LeftParen();
    TypeNameContext *typeName();
    antlr4::tree::TerminalNode *RightParen();
    CastExpressionContext *castExpression();
    UnaryExpressionContext *unaryExpression();
    antlr4::tree::TerminalNode *IntegerConstant();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CastExpressionContext* castExpression();

  class  MultiplicativeExpressionContext : public antlr4::ParserRuleContext {
  public:
    MultiplicativeExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<CastExpressionContext *> castExpression();
    CastExpressionContext* castExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Star();
    antlr4::tree::TerminalNode* Star(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Div();
    antlr4::tree::TerminalNode* Div(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Mod();
    antlr4::tree::TerminalNode* Mod(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MultiplicativeExpressionContext* multiplicativeExpression();

  class  AdditiveExpressionContext : public antlr4::ParserRuleContext {
  public:
    AdditiveExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<MultiplicativeExpressionContext *> multiplicativeExpression();
    MultiplicativeExpressionContext* multiplicativeExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Plus();
    antlr4::tree::TerminalNode* Plus(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Minus();
    antlr4::tree::TerminalNode* Minus(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AdditiveExpressionContext* additiveExpression();

  class  ShiftExpressionContext : public antlr4::ParserRuleContext {
  public:
    ShiftExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<AdditiveExpressionContext *> additiveExpression();
    AdditiveExpressionContext* additiveExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> LeftShift();
    antlr4::tree::TerminalNode* LeftShift(size_t i);
    std::vector<antlr4::tree::TerminalNode *> RightShift();
    antlr4::tree::TerminalNode* RightShift(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ShiftExpressionContext* shiftExpression();

  class  AndExpressionContext : public antlr4::ParserRuleContext {
  public:
    AndExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ShiftExpressionContext *> shiftExpression();
    ShiftExpressionContext* shiftExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> And();
    antlr4::tree::TerminalNode* And(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AndExpressionContext* andExpression();

  class  ExclusiveOrExpressionContext : public antlr4::ParserRuleContext {
  public:
    ExclusiveOrExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<AndExpressionContext *> andExpression();
    AndExpressionContext* andExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Caret();
    antlr4::tree::TerminalNode* Caret(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExclusiveOrExpressionContext* exclusiveOrExpression();

  class  InclusiveOrExpressionContext : public antlr4::ParserRuleContext {
  public:
    InclusiveOrExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExclusiveOrExpressionContext *> exclusiveOrExpression();
    ExclusiveOrExpressionContext* exclusiveOrExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Or();
    antlr4::tree::TerminalNode* Or(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InclusiveOrExpressionContext* inclusiveOrExpression();

  class  AssignmentExpressionDefContext : public antlr4::ParserRuleContext {
  public:
    AssignmentExpressionDefContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AssignmentExpressionContext *assignmentExpression();
    antlr4::tree::TerminalNode *EOF();
    std::vector<antlr4::tree::TerminalNode *> Semi();
    antlr4::tree::TerminalNode* Semi(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AssignmentExpressionDefContext* assignmentExpressionDef();

  class  AssignmentExpressionContext : public antlr4::ParserRuleContext {
  public:
    AssignmentExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IntegerConstant();
    InclusiveOrExpressionContext *inclusiveOrExpression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AssignmentExpressionContext* assignmentExpression();

  class  DeclarationContext : public antlr4::ParserRuleContext {
  public:
    DeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    DeclarationSpecifierContext *declarationSpecifier();
    antlr4::tree::TerminalNode *Semi();
    antlr4::tree::TerminalNode *TypeDef();
    TypeSpecifierContext *typeSpecifier();
    antlr4::tree::TerminalNode *Identifier();
    PointerContext *pointer();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DeclarationContext* declaration();

  class  DeclarationSpecifierContext : public antlr4::ParserRuleContext {
  public:
    DeclarationSpecifierContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    StructOrUnionSpecifierContext *structOrUnionSpecifier();
    EnumSpecifierContext *enumSpecifier();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DeclarationSpecifierContext* declarationSpecifier();

  class  TypeSpecifierContext : public antlr4::ParserRuleContext {
  public:
    TypeSpecifierContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Void();
    InternalBasicTypesContext *internalBasicTypes();
    StructOrUnionSpecifierContext *structOrUnionSpecifier();
    EnumSpecifierContext *enumSpecifier();
    antlr4::tree::TerminalNode *Identifier();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeSpecifierContext* typeSpecifier();

  class  StructOrUnionSpecifierContext : public antlr4::ParserRuleContext {
  public:
    StructOrUnionSpecifierContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    StructOrUnionContext *structOrUnion();
    antlr4::tree::TerminalNode *LeftBrace();
    antlr4::tree::TerminalNode *RightBrace();
    AlignAsAttrContext *alignAsAttr();
    antlr4::tree::TerminalNode *Identifier();
    StructDeclarationListContext *structDeclarationList();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StructOrUnionSpecifierContext* structOrUnionSpecifier();

  class  StructOrUnionContext : public antlr4::ParserRuleContext {
  public:
    StructOrUnionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Struct();
    antlr4::tree::TerminalNode *Union();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StructOrUnionContext* structOrUnion();

  class  AlignAsAttrContext : public antlr4::ParserRuleContext {
  public:
    AlignAsAttrContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *AlignAs();
    antlr4::tree::TerminalNode *LeftParen();
    antlr4::tree::TerminalNode *IntegerConstant();
    antlr4::tree::TerminalNode *RightParen();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AlignAsAttrContext* alignAsAttr();

  class  StructDeclarationListContext : public antlr4::ParserRuleContext {
  public:
    StructDeclarationListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<StructDeclarationContext *> structDeclaration();
    StructDeclarationContext* structDeclaration(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StructDeclarationListContext* structDeclarationList();

  class  StructDeclarationContext : public antlr4::ParserRuleContext {
  public:
    StructDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    SpecifierQualifierListContext *specifierQualifierList();
    StructDeclaratorListContext *structDeclaratorList();
    antlr4::tree::TerminalNode *Semi();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StructDeclarationContext* structDeclaration();

  class  SpecifierQualifierListContext : public antlr4::ParserRuleContext {
  public:
    SpecifierQualifierListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TypeSpecifierContext *typeSpecifier();
    std::vector<antlr4::tree::TerminalNode *> Const();
    antlr4::tree::TerminalNode* Const(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SpecifierQualifierListContext* specifierQualifierList();

  class  StructDeclaratorListContext : public antlr4::ParserRuleContext {
  public:
    StructDeclaratorListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<StructDeclaratorContext *> structDeclarator();
    StructDeclaratorContext* structDeclarator(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StructDeclaratorListContext* structDeclaratorList();

  class  StructDeclaratorContext : public antlr4::ParserRuleContext {
  public:
    StructDeclaratorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    DeclaratorContext *declarator();
    antlr4::tree::TerminalNode *Colon();
    AssignmentExpressionContext *assignmentExpression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StructDeclaratorContext* structDeclarator();

  class  EnumSpecifierContext : public antlr4::ParserRuleContext {
  public:
    EnumSpecifierContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Enum();
    antlr4::tree::TerminalNode *LeftBrace();
    EnumeratorListContext *enumeratorList();
    antlr4::tree::TerminalNode *RightBrace();
    antlr4::tree::TerminalNode *Identifier();
    antlr4::tree::TerminalNode *Comma();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EnumSpecifierContext* enumSpecifier();

  class  EnumeratorListContext : public antlr4::ParserRuleContext {
  public:
    EnumeratorListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<EnumeratorContext *> enumerator();
    EnumeratorContext* enumerator(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EnumeratorListContext* enumeratorList();

  class  EnumeratorContext : public antlr4::ParserRuleContext {
  public:
    EnumeratorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    EnumerationConstantContext *enumerationConstant();
    AssignmentExpressionContext *assignmentExpression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EnumeratorContext* enumerator();

  class  EnumerationConstantContext : public antlr4::ParserRuleContext {
  public:
    EnumerationConstantContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Identifier();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EnumerationConstantContext* enumerationConstant();

  class  DeclaratorContext : public antlr4::ParserRuleContext {
  public:
    DeclaratorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    DirectDeclaratorContext *directDeclarator();
    PointerContext *pointer();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DeclaratorContext* declarator();

  class  DirectDeclaratorContext : public antlr4::ParserRuleContext {
  public:
    DirectDeclaratorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Identifier();
    antlr4::tree::TerminalNode *QuestionMark();
    antlr4::tree::TerminalNode *LeftParen();
    DeclaratorContext *declarator();
    antlr4::tree::TerminalNode *RightParen();
    DirectDeclaratorContext *directDeclarator();
    antlr4::tree::TerminalNode *LeftBracket();
    AssignmentExpressionContext *assignmentExpression();
    antlr4::tree::TerminalNode *RightBracket();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DirectDeclaratorContext* directDeclarator();
  DirectDeclaratorContext* directDeclarator(int precedence);
  class  PointerContext : public antlr4::ParserRuleContext {
  public:
    PointerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> Star();
    antlr4::tree::TerminalNode* Star(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Const();
    antlr4::tree::TerminalNode* Const(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PointerContext* pointer();

  class  IdentifierListContext : public antlr4::ParserRuleContext {
  public:
    IdentifierListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> Identifier();
    antlr4::tree::TerminalNode* Identifier(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IdentifierListContext* identifierList();

  class  TypeNameContext : public antlr4::ParserRuleContext {
  public:
    TypeNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    SpecifierQualifierListContext *specifierQualifierList();
    AbstractDeclaratorContext *abstractDeclarator();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeNameContext* typeName();

  class  AbstractDeclaratorContext : public antlr4::ParserRuleContext {
  public:
    AbstractDeclaratorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PointerContext *pointer();
    DirectAbstractDeclaratorContext *directAbstractDeclarator();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AbstractDeclaratorContext* abstractDeclarator();

  class  DirectAbstractDeclaratorContext : public antlr4::ParserRuleContext {
  public:
    DirectAbstractDeclaratorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LeftParen();
    AbstractDeclaratorContext *abstractDeclarator();
    antlr4::tree::TerminalNode *RightParen();
    antlr4::tree::TerminalNode *LeftBracket();
    AssignmentExpressionContext *assignmentExpression();
    antlr4::tree::TerminalNode *RightBracket();
    DirectAbstractDeclaratorContext *directAbstractDeclarator();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DirectAbstractDeclaratorContext* directAbstractDeclarator();
  DirectAbstractDeclaratorContext* directAbstractDeclarator(int precedence);
  class  CompilationUnitContext : public antlr4::ParserRuleContext {
  public:
    CompilationUnitContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    TranslationUnitContext *translationUnit();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CompilationUnitContext* compilationUnit();

  class  TranslationUnitContext : public antlr4::ParserRuleContext {
  public:
    TranslationUnitContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExternalDeclarationContext *> externalDeclaration();
    ExternalDeclarationContext* externalDeclaration(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TranslationUnitContext* translationUnit();

  class  ExternalDeclarationContext : public antlr4::ParserRuleContext {
  public:
    ExternalDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    DeclarationContext *declaration();
    DefineDeclContext *defineDecl();
    antlr4::tree::TerminalNode *Semi();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExternalDeclarationContext* externalDeclaration();

  class  DefineDeclContext : public antlr4::ParserRuleContext {
  public:
    DefineDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DirectiveDefine();
    antlr4::tree::TerminalNode *MultiLineMacroDefine();
    antlr4::tree::TerminalNode *DirectiveInclude();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DefineDeclContext* defineDecl();


  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  bool directDeclaratorSempred(DirectDeclaratorContext *_localctx, size_t predicateIndex);
  bool directAbstractDeclaratorSempred(DirectAbstractDeclaratorContext *_localctx, size_t predicateIndex);

  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

