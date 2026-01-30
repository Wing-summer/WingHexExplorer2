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

options { tokenVocab = AngelscriptConsoleLexer; }

script
    : scriptItem* EOF
    ;

emptyDeclaration_
    : Semi
    ;

scriptItem
    : declSpecifierSeq initDeclaratorList Semi
    | emptyDeclaration_
    ;

declSpecifierSeq
    : declSpecifier+
    ;

declSpecifier
    : typeSpecifier
    ;

typeSpecifier
    : trailingTypeSpecifier
    ;

arraySpecifier
    : LeftBracket constantExpression? RightBracket
    ;

trailingTypeSpecifier
    : simpleTypeSpecifier arraySpecifier*
    | Const
    ;

simpleTypeSpecifier
    : Identifier
    | asGeneric
    ;

asGeneric
    : Identifier Less simpleTypeSpecifierList Greater
    ;

simpleTypeSpecifierList
    : simpleTypeSpecifier (Comma simpleTypeSpecifier)*
    ;

constantExpression
    : IntegerLiteral
    ;

initDeclaratorList
    : initDeclarator (Comma initDeclarator)*
    ;

initDeclarator
    : Identifier initializer?
    ;

expressionList
    : initializerList
    ;

initializer
    : braceOrEqualInitializer
    | LeftParen expressionList RightParen
    ;

/* Expressions */

primaryExpression
    : literal+
    | LeftParen expression RightParen
    | idExpression
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

typedefName
    : Identifier
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

pseudoDestructorName
    : nestedNameSpecifier? (theTypeName Doublecolon)? Tilde theTypeName
    | nestedNameSpecifier Doublecolon Tilde theTypeName
    | Tilde decltypeSpecifier
    ;

typeSpecifierSeq
    : typeSpecifier+
    ;

theTypeId
    : typeSpecifierSeq
    ;

postfixExpression
    : primaryExpression
    | postfixExpression LeftBracket (expression | bracedInitList) RightBracket
    | postfixExpression LeftParen expressionList? RightParen
    | simpleTypeSpecifier arraySpecifier* ( LeftParen expressionList? RightParen | bracedInitList)
    | postfixExpression Dot ( idExpression | pseudoDestructorName)
    | postfixExpression (PlusPlus | MinusMinus)
    | Cast Less theTypeId Greater LeftParen expression RightParen
    | LeftParen (expression | theTypeId) RightParen
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

expression
    : assignmentExpression (Comma assignmentExpression)*
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

literal
    : IntegerLiteral
    | CharacterLiteral
    | FloatingLiteral
    | StringLiteral
    | booleanLiteral
    | UserDefinedLiteral
    | Nullptr
    ;

booleanLiteral
    : False_
    | True_
    ;
