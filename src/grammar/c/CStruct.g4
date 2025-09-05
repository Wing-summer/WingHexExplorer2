/*==============================================================================
** Copyright (C) 2025-2028 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

grammar CStruct;

// pure C types
internalBasicTypes
    : signOrUnsigned?  Const? ('char' | 'short' | 'int' | 'long')
    | ('char' | 'short' | 'int' | 'long')  Const? signOrUnsigned? 
    | signOrUnsigned?  Const? 'long' 'long'
    | 'long' 'long' Const? signOrUnsigned?
    | 'float'
    | 'double'
;

signOrUnsigned
    : Signed 
    | Unsigned
    ;

primaryExpression
    : Identifier
    | IntegerConstant
    | '(' assignmentExpression ')'
    | SizeOf '(' specifierQualifierList ')'
    ;

postfixExpression
    : primaryExpression (
        '++'
        | '--'
    )*
    ;

unaryExpression
    : ('++' | '--')* (
        postfixExpression
        | unaryOperator castExpression
    )
    ;

unaryOperator
    : '+'
    | '-'
    | '~'
    ;

castExpression
    : '(' typeName ')' castExpression
    | unaryExpression
    | IntegerConstant // for
    ;

multiplicativeExpression
    : castExpression (('*' | '/' | '%') castExpression)*
    ;

additiveExpression
    : multiplicativeExpression (('+' | '-') multiplicativeExpression)*
    ;

shiftExpression
    : additiveExpression (('<<' | '>>') additiveExpression)*
    ;

andExpression
    : shiftExpression ('&' shiftExpression)*
    ;

exclusiveOrExpression
    : andExpression ('^' andExpression)*
    ;

inclusiveOrExpression
    : exclusiveOrExpression ('|' exclusiveOrExpression)*
    ;

assignmentExpressionDef
    : assignmentExpression ';'* EOF
    ;

assignmentExpression
    : IntegerConstant
    | inclusiveOrExpression
    ;

declaration
    : declarationSpecifier ';'
    | 'typedef' typeSpecifier pointer? Identifier ';'
    ;

declarationSpecifier
    :  structOrUnionSpecifier
    | enumSpecifier
    ;

typeSpecifier
    : 'void' 
    | internalBasicTypes
    | structOrUnionSpecifier
    | enumSpecifier
    | Identifier
    ;

structOrUnionSpecifier
    : structOrUnion alignAsAttr? Identifier? '{' structDeclarationList? '}'
    | structOrUnion Identifier
    ;

structOrUnion
    : 'struct'
    | 'union'
    ;

alignAsAttr
    : 'alignas' '(' IntegerConstant ')'
    ;

structDeclarationList
    : structDeclaration+
    ;

structDeclaration // The first two rules have priority order and cannot be simplified to one expression.
    : specifierQualifierList structDeclaratorList ';'
    | specifierQualifierList ';'
    ;

specifierQualifierList
    : Const? typeSpecifier Const?
    ;

structDeclaratorList
    : structDeclarator (',' structDeclarator)*
    ;

structDeclarator
    : declarator
    | declarator? ':' assignmentExpression
    ;

enumSpecifier
    : 'enum' Identifier? '{' enumeratorList ','? '}'
    | 'enum' Identifier
    ;

enumeratorList
    : enumerator (',' enumerator)*
    ;

enumerator
    : enumerationConstant ('=' assignmentExpression)?
    ;

enumerationConstant
    : Identifier
    ;

declarator
    : pointer? directDeclarator 
    ;

directDeclarator
    : Identifier
    | '?'
    | '(' declarator ')'
    | directDeclarator '[' assignmentExpression ']'
    ;

pointer
    : ('*' Const?)+ 
    ;

identifierList
    : Identifier (',' Identifier)*
    ;

typeName
    : specifierQualifierList abstractDeclarator?
    ;

abstractDeclarator
    : pointer
    | pointer? directAbstractDeclarator 
    ;

directAbstractDeclarator
    : '(' abstractDeclarator ')'
    | '[' assignmentExpression ']'
    | directAbstractDeclarator '[' assignmentExpression ']'
    ;

compilationUnit
    : translationUnit? EOF
    ;

translationUnit
    : externalDeclaration+
    ;

externalDeclaration
    : declaration
    | defineDecl
    | ';' // stray ;
    ;

defineDecl
    : DirectiveDefine
    | MultiLineMacroDefine
    | DirectiveInclude
    ;

Char
    : 'char'
    ;

Const
    : 'const'
    ;

Double
    : 'double'
    ;

Enum
    : 'enum'
    ;

Float
    : 'float'
    ;

Int
    : 'int'
    ;

Long
    : 'long'
    ;

TypeDef
    : 'typedef'
    ;

AlignAs
    :'alignas'
    ;

SizeOf
    : 'sizeof'
    ;

Short
    : 'short'
    ;

Signed
    : 'signed'
    ;

Struct
    : 'struct'
    ;

Union
    : 'union'
    ;

Unsigned
    : 'unsigned'
    ;

Void
    : 'void'
    ;

QuestionMark
    : '?'
    ;

LeftParen
    : '('
    ;

RightParen
    : ')'
    ;

LeftBracket
    : '['
    ;

RightBracket
    : ']'
    ;

LeftBrace
    : '{'
    ;

RightBrace
    : '}'
    ;

LeftShift
    : '<<'
    ;

RightShift
    : '>>'
    ;

Plus
    : '+'
    ;

PlusPlus
    : '++'
    ;

Minus
    : '-'
    ;

MinusMinus
    : '--'
    ;

Star
    : '*'
    ;

Div
    : '/'
    ;

Mod
    : '%'
    ;

And
    : '&'
    ;

Or
    : '|'
    ;

Caret
    : '^'
    ;

Tilde
    : '~'
    ;

Colon
    : ':'
    ;

Semi
    : ';'
    ;

Comma
    : ','
    ;
    
Identifier
    : IdentifierNondigit (IdentifierNondigit | Digit)*
    ;

fragment IdentifierNondigit
    : Nondigit
    ;

fragment Nondigit
    : [a-zA-Z_]
    ;

fragment Digit
    : [0-9]
    ;

fragment HexQuad
    : HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit
    ;

IntegerConstant
    : DecimalConstant IntegerSuffix? 
    | OctalConstant IntegerSuffix? 
    | HexadecimalConstant IntegerSuffix? 
    | BinaryConstant
    | '\'' CChar '\'' // character constant
    ;

fragment BinaryConstant
    : '0' [bB] [0-1]+
    ;

fragment DecimalConstant
    : NonzeroDigit Digit*
    ;

fragment OctalConstant
    : '0' OctalDigit*
    ;

fragment HexadecimalConstant
    : HexadecimalPrefix HexadecimalDigit+
    ;

fragment HexadecimalPrefix
    : '0' [xX]
    ;

fragment NonzeroDigit
    : [1-9]
    ;

fragment OctalDigit
    : [0-7]
    ;

fragment HexadecimalDigit
    : [0-9a-fA-F]
    ;

fragment IntegerSuffix
    : UnsignedSuffix LongSuffix?
    | UnsignedSuffix LongLongSuffix
    | LongSuffix UnsignedSuffix?
    | LongLongSuffix UnsignedSuffix?
    ;

fragment UnsignedSuffix
    : [uU]
    ;

fragment LongSuffix
    : [lL]
    ;

fragment LongLongSuffix
    : 'll'
    | 'LL'
    ;

fragment FractionalConstant
    : DigitSequence? '.' DigitSequence
    | DigitSequence '.'
    ;

fragment ExponentPart
    : [eE] Sign? DigitSequence
    ;

fragment Sign
    : [+-]
    ;

fragment DigitSequence
    : Digit+
    ;

fragment BinaryExponentPart
    : [pP] Sign? DigitSequence
    ;

fragment HexadecimalDigitSequence
    : HexadecimalDigit+
    ;

StringLiteral
    : '"' SCharSequence? '"'
    ;

fragment SCharSequence
    : CChar+
    ;

fragment CChar
    : ~['\\\r\n]
    | EscapeSequence
    ;

fragment EscapeSequence
    : SimpleEscapeSequence
    | OctalEscapeSequence
    | HexadecimalEscapeSequence
    ;

fragment SimpleEscapeSequence
    : '\\' ['"?abfnrtv\\]
    ;

fragment OctalEscapeSequence
    : '\\' OctalDigit OctalDigit? OctalDigit?
    ;

fragment HexadecimalEscapeSequence
    : '\\x' HexadecimalDigit+
    ;

MultiLineMacroDefine
    :'#' 'define' (~[\n]*? '\\' '\r'? '\n')+ ~ [\n]+ 
    ;

MultiLineMacro
    : '#' Identifier (~[\n]*? '\\' '\r'? '\n')+ ~ [\n]+ -> skip
    ;

DirectiveInclude
    : '#' 'include' ~ [\n]* 
    ;


DirectiveDefine
    : '#' 'define' ~ [\n]* 
    ;

Directive
    : '#' Identifier ~ [\n]* ->skip
    ;

Whitespace
    : [ \t]+ -> skip
    ;

Newline
    : ('\r' '\n'? | '\n') -> skip
    ;

LINE_CONTINUATION
    :   '\\' [\r]? '\n' -> skip
    ;

BlockComment
    : '/*' .*? '*/' -> skip
    ;

LineComment
    : '//' ~[\r\n]* -> skip
    ;
