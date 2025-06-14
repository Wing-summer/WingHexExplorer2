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

grammar NumCal;

primaryExpression
    : IntegerConstant
    | '(' assignmentExpression ')'
    ;

unaryExpression
    : primaryExpression
    | unaryOperator castExpression
    ;

unaryOperator
    : '+'
    | '-'
    | '~'
    ;

castExpression
    : unaryExpression
    | IntegerConstant 
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

assignmentExpression
    : IntegerConstant
    | inclusiveOrExpression
    ;

entryExpression
    : prefixGoto? IntegerConstant EOF
    | (prefixGoto Colon)? assignmentExpression EOF
    | prefixGoto LeftBracket assignmentExpression RightBracket EOF
    | prefixGoto LeftParen assignmentExpression RightParen EOF
    ;

prefixGoto
    : (Plus | Minus | LessThan)
    ;


LeftParen
    : '('
    ;

RightParen
    : ')'
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

Minus
    : '-'
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

LessThan
    : '<'
    ;

Colon
    : ':'
    ;

LeftBracket
    : '['
    ;

RightBracket
    : ']'
    ;

fragment Digit
    : [0-9]
    ;

fragment HexQuad
    : HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit
    ;

IntegerConstant
    : DecimalConstant 
    | OctalConstant
    | HexadecimalConstant
    | BinaryConstant
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

Whitespace
    : [ \t]+ -> skip
    ;
