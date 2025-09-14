/*******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Camilo Sanchez (Camiloasc1) 2020 Martin Mirchev (Marti2203) 2022 Embark Studios AB.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * ****************************************************************************
 */

/*
	Adapted to Unreal Angelscript by Embark Studios AB (Fredrik Lindh [Temaran]).
	Based on the C++ grammar made by Camilo Sanchez (Camiloasc1) and Martin Mirchev (Marti2203). See the parser file.
 */

// $antlr-format alignTrailingComments true, columnLimit 150, minEmptyLines 1, maxEmptyLinesToKeep 1, reflowComments false, useTab false
// $antlr-format allowShortRulesOnASingleLine false, allowShortBlocksOnASingleLine true, alignSemicolons hanging, alignColons hanging

parser grammar AngelscriptConsoleParser;

options {
    tokenVocab = AngelscriptConsoleLexer;
}

/* Basic concepts */
script
    : scriptItem* EOF
    ;

scriptItem
    : declSpecifierSeq initDeclaratorList Semi  
    | statement                          
    ;

/* Angelscript */
asGeneric
    : Identifier Less simpleTypeSpecifierList Greater
    ;

simpleTypeSpecifierList
    : declSpecifierSeq (Comma declSpecifierSeq)*
    ;

booleanLiteral
    : False_
    | True_
    ;

/* Expressions */

primaryExpression
    : literal+
    | LeftParen expression RightParen
    | idExpression
    | lambdaExpression
    ;

idExpression
    : unqualifiedId
    | qualifiedId
    ;

unqualifiedId
    : Identifier
    ;

qualifiedId
    : nestedNameSpecifier unqualifiedId
    ;

nestedNameSpecifier
    : (theTypeName | namespaceName | decltypeSpecifier)? Doublecolon
    | nestedNameSpecifier Identifier Doublecolon
    ;

lambdaExpression
    : lambdaIntroducer lambdaDeclarator? compoundStatement
    ;

lambdaIntroducer
    : LeftBracket lambdaCapture? RightBracket
    ;

lambdaCapture
    : captureList
    | captureDefault (Comma captureList)?
    ;

captureDefault
    : And
    | Assign
    ;

captureList
    : capture (Comma capture)*
    ;

capture
    : simpleCapture
    | initcapture
    ;

simpleCapture
    : And? Identifier
    ;

initcapture
    : And? Identifier initializer
    ;

lambdaDeclarator
    : LeftParen parameterDeclarationClause? RightParen
    ;

postfixExpression
    : primaryExpression
    | postfixExpression LeftBracket (expression | bracedInitList) RightBracket
    | postfixExpression LeftParen expressionList? RightParen
    | simpleTypeSpecifier ( LeftParen expressionList? RightParen | bracedInitList)
    | postfixExpression Dot ( idExpression | pseudoDestructorName)
    | postfixExpression (PlusPlus | MinusMinus)
    | Cast Less theTypeId Greater LeftParen expression RightParen
    | LeftParen (expression | theTypeId) RightParen
    ;

/*
 add a middle layer to eliminate duplicated function declarations
 */

expressionList
    : initializerList
    ;

pseudoDestructorName
    : nestedNameSpecifier? (theTypeName Doublecolon)? Tilde theTypeName
    | nestedNameSpecifier Doublecolon Tilde theTypeName
    | Tilde decltypeSpecifier
    ;

unaryExpression
    : postfixExpression
    | (PlusPlus | MinusMinus | unaryOperator) unaryExpression
    | LeftParen theTypeId RightParen
    ;

unaryOperator
    : Or
    | Star
    | And
    | Plus
    | Tilde
    | Minus
    | Not
    ;

newPlacement
    : LeftParen expressionList RightParen
    ;

newInitializer_
    : LeftParen expressionList? RightParen
    | bracedInitList
    ;

castExpression
    : unaryExpression
    | Cast Less theTypeId Greater LeftParen castExpression RightParen
    ;

multiplicativeExpression
    : castExpression ((Star | Div | Mod) castExpression)*
    ;

additiveExpression
    : multiplicativeExpression ((Plus | Minus) multiplicativeExpression)*
    ;

shiftExpression
    : additiveExpression (shiftOperator additiveExpression)*
    ;

shiftOperator
    : Greater Greater
    | Less Less
    ;

relationalExpression
    : shiftExpression ((Less | Greater | LessEqual | GreaterEqual) shiftExpression)*
    ;

equalityExpression
    : relationalExpression ((Equal | NotEqual) relationalExpression)*
    ;

andExpression
    : equalityExpression (And equalityExpression)*
    ;

exclusiveOrExpression
    : andExpression (Xor andExpression)*
    ;

inclusiveOrExpression
    : exclusiveOrExpression (Or exclusiveOrExpression)*
    ;

logicalAndExpression
    : inclusiveOrExpression (AndAnd inclusiveOrExpression)*
    ;

logicalOrExpression
    : logicalAndExpression (OrOr logicalAndExpression)*
    ;

conditionalExpression
    : logicalOrExpression (Question expression Colon assignmentExpression)?
    ;

assignmentExpression
    : conditionalExpression
    | logicalOrExpression assignmentOperator initializerClause
    ;

assignmentOperator
    : Assign
    | StarAssign
    | DivAssign
    | ModAssign
    | PlusAssign
    | MinusAssign
    | RightShiftAssign
    | LeftShiftAssign
    | AndAssign
    | XorAssign
    | OrAssign
    ;

expression
    : assignmentExpression (Comma assignmentExpression)*
    ;

constantExpression
    : conditionalExpression
    ;

/* Statements */
statement
    : labeledStatement
    | declaration
    | expressionStatement
    | compoundStatement
    | selectionStatement
    | iterationStatement
    | jumpStatement
    ;

labeledStatement
    : (Identifier | Case constantExpression | Default) Colon statement
    ;

expressionStatement
    : expression? Semi
    ;

compoundStatement
    : LeftBrace statementSeq? RightBrace
    ;

statementSeq
    : statement+
    ;

selectionStatement
    : If LeftParen condition RightParen statement (Else statement)?
    | Switch LeftParen condition RightParen statement
    ;

condition
    : expression
    | declSpecifierSeq declarator ( Assign initializerClause | bracedInitList)
    ;

iterationStatement
    : While LeftParen condition RightParen statement
    | Do statement While LeftParen expression RightParen Semi
    | For LeftParen (
        forInitStatement condition? Semi expression?
        | forRangeDeclaration Colon forRangeInitializer
    ) RightParen statement
    ;

forInitStatement
    : expressionStatement
    | simpleDeclaration
    ;

forRangeDeclaration
    : declSpecifierSeq Identifier
    ;

forRangeInitializer
    : expression
    | bracedInitList
    ;

jumpStatement
    : (Break | Continue | Return) Semi
    ;

/* Declarations */

declarationseq
    : declaration+
    ;

declaration
    : simpleDeclaration
    | emptyDeclaration_
    ;

aliasDeclaration
    : Identifier Assign theTypeId Semi
    ;

simpleDeclaration
    : declSpecifierSeq (initDeclaratorList | assignmentExpression)? Semi
    ;

emptyDeclaration_
    : Semi
    ;

declSpecifier
    : typeSpecifier
    ;

declSpecifierSeq
    : declSpecifier+
    ;

typedefName
    : Identifier
    ;

typeSpecifier
    : trailingTypeSpecifier
    ;

trailingTypeSpecifier
    : simpleTypeSpecifier
    | Const
    ;

typeSpecifierSeq
    : typeSpecifier+
    ;

trailingTypeSpecifierSeq
    : trailingTypeSpecifier+
    ;

simpleTypeSpecifier
    : nestedNameSpecifier? theTypeName
    | asGeneric
    | Int
    | Int8
    | Int16
    | Int32
    | Int64
    | UInt
    | UInt8
    | UInt16
    | UInt32
    | UInt64
    | Float
    | Double
    | Bool
    | Void
    | Auto
    | decltypeSpecifier
    ;

theTypeName
    : typedefName
    ;

decltypeSpecifier
    : LeftParen (expression | Auto) RightParen
    ;

namespaceName
    : Identifier
    ;

qualifiednamespacespecifier
    : nestedNameSpecifier? namespaceName
    ;

balancedTokenSeq
    : balancedtoken+
    ;

balancedtoken
    : LeftParen balancedTokenSeq RightParen
    | LeftBracket balancedTokenSeq RightBracket
    | LeftBrace balancedTokenSeq RightBrace
    | ~(LeftParen | RightParen | LeftBrace | RightBrace | LeftBracket | RightBracket)+
    ;

/* Declarators */

initDeclaratorList
    : initDeclarator (Comma initDeclarator)*
    ;

initDeclarator
    : Identifier initializer?
    ;

declarator
    : declaratorDef parametersAndQualifiers
    ;

declaratorDef
    : declaratorid
    | declaratorDef (parametersAndQualifiers | LeftBracket constantExpression? RightBracket)
    ;

parametersAndQualifiers
    : LeftParen parameterDeclarationClause? RightParen Const? refqualifier?
    ;

refqualifier
    : And
    | AndAnd
    ;

declaratorid
    : idExpression
    ;

theTypeId
    : typeSpecifierSeq
    ;

parameterDeclarationClause
    : parameterDeclarationList
    ;

parameterDeclarationList
    : parameterDeclaration (Comma parameterDeclaration)*
    ;

parameterDeclaration
    : declSpecifierSeq Identifier? (Assign initializerClause)?
    ;

initializer
    : braceOrEqualInitializer
    | LeftParen expressionList RightParen
    ;

braceOrEqualInitializer
    : Assign initializerClause
    | bracedInitList
    ;

initializerClause
    : assignmentExpression
    | bracedInitList
    ;

initializerList
    : initializerClause (Comma initializerClause)* Comma?
    ; // I *really* don't like that trailing commas are a thing in AS...

bracedInitList
    : (LeftBrace | LeftBracket) (initializerList Comma?)? (RightBrace | RightBracket)
    ;

/* Lexer token groupings referenced in parser (kept as parser rules here) */

theOperator
    :
    | Plus
    | Minus
    | Star
    | Div
    | Mod
    | Xor
    | And
    | Or
    | Tilde
    | Not
    | Assign
    | Greater
    | Less
    | GreaterEqual
    | PlusAssign
    | MinusAssign
    | StarAssign
    | ModAssign
    | XorAssign
    | AndAssign
    | OrAssign
    | Less Less
    | Greater Greater
    | RightShiftAssign
    | LeftShiftAssign
    | Equal
    | NotEqual
    | LessEqual
    | AndAnd
    | OrOr
    | PlusPlus
    | MinusMinus
    | Comma
    | LeftParen RightParen
    | LeftBracket RightBracket
    ;

literal
    : IntegerLiteral
    | CharacterLiteral
    | FloatingLiteral
    | StringLiteral
    | booleanLiteral
    | UserDefinedLiteral
    | Nullptr
    ;
