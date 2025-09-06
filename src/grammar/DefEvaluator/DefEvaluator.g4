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

grammar DefEvaluator;

primaryExpression
    : Constant
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
    | '!'
    ;

castExpression
    : unaryExpression
    | Constant 
    ;

relationalExpression
    : shiftExpression (('<' | '>' | '<=' | '>=') shiftExpression)*
    ;

equalityExpression
    : relationalExpression (('==' | '!=') relationalExpression)*
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

logicalAndExpression
    : inclusiveOrExpression ('&&' inclusiveOrExpression)*
    ;

logicalOrExpression
    : logicalAndExpression ('||' logicalAndExpression)*
    ;

conditionalExpression
    : logicalOrExpression ('?' primaryExpression ':' conditionalExpression)?
    ;

assignmentExpression
    : conditionalExpression
    | inclusiveOrExpression
    ;

entryExpression
    : Constant EOF
    | assignmentExpression EOF
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

Not
    : '!'
    ;

Tilde
    : '~'
    ;

LessThan
    : '<'
    ;

fragment Digit
    : [0-9]
    ;

fragment HexQuad
    : HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit
    ;

Constant
    : IntegerConstant
    | FloatingConstant
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

fragment FloatingSuffix
: [flFL]
;

fragment HexadecimalFractionalConstant
    : HexadecimalDigitSequence? '.' HexadecimalDigitSequence
    | HexadecimalDigitSequence '.'
    ;

fragment HexadecimalDigit
    : [0-9a-fA-F]
    ;

fragment FloatingConstant
    : DecimalFloatingConstant
    ;

fragment DecimalFloatingConstant
    : FractionalConstant ExponentPart? FloatingSuffix?
    | DigitSequence ExponentPart FloatingSuffix?
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

fragment HexadecimalDigitSequence
    : HexadecimalDigit+
    ;

Whitespace
    : [ \t]+ -> skip
    ;
