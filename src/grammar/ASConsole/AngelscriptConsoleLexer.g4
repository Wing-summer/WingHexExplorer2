/*
	Adapted to Unreal Angelscript by Embark Studios AB (Fredrik Lindh [Temaran]).
	Based on the C++ grammar made by Camilo Sanchez (Camiloasc1) and Martin Mirchev (Marti2203). See the parser file.
 */

// $antlr-format alignTrailingComments true, columnLimit 150, maxEmptyLinesToKeep 1, reflowComments false, useTab false
// $antlr-format allowShortRulesOnASingleLine true, allowShortBlocksOnASingleLine true, minEmptyLines 0, alignSemicolons ownLine
// $antlr-format alignColons trailing, singleLineOverrulesHangingColon true, alignLexerCommands true, alignLabels true, alignTrailers true

lexer grammar AngelscriptConsoleLexer;

IntegerLiteral:
    DecimalLiteral Integersuffix?
    | OctalLiteral Integersuffix?
    | HexadecimalLiteral Integersuffix?
    | BinaryLiteral Integersuffix?
;

CharacterLiteral: '\'' Cchar+ '\'';

FloatingLiteral:
    Fractionalconstant Exponentpart? Floatingsuffix?
    | Digitsequence Exponentpart Floatingsuffix?
;

StringLiteral:
    '"""' .*? '"""'
    | '"' ( ~["\\\u0085\u2028\u2029] | Escapesequence)* '"'
    | '\'' ( ~['\\\u0085\u2028\u2029] | Escapesequence)* '\''
;

UserDefinedLiteral:
    UserDefinedIntegerLiteral
    | UserDefinedFloatingLiteral
    | UserDefinedStringLiteral
    | UserDefinedCharacterLiteral
;

/*Angelscript*/

Cast: 'cast';

Int    : 'int';
Int8   : 'int8';
Int16  : 'int16';
Int32  : 'int32';
Int64  : 'int64';
UInt   : 'uint';
UInt8  : 'uint8';
UInt16 : 'uint16';
UInt32 : 'uint32';
UInt64 : 'uint64';
Float  : 'float';
Double : 'double';
Bool   : 'bool';

/*Keywords*/

Auto: 'auto';

Break: 'break';

Case: 'case';

Catch: 'catch';

Char: 'char';

Const: 'const';

Continue: 'continue';

Default: 'default';

Do: 'do';

Else: 'else';

False_: 'false';

For: 'for';

If: 'if';

Nullptr: 'null';

Return: 'return';

Short: 'short';

Switch: 'switch';

True_: 'true';

Void: 'void';

While: 'while';

/*Operators*/

LeftParen: '(';

RightParen: ')';

LeftBracket: '[';

RightBracket: ']';

LeftBrace: '{';

RightBrace: '}';

Plus: '+';

Minus: '-';

Star: '*';

Div: '/';

Mod: '%';

Xor: '^';

And: '&';

Or: '|';

Tilde: '~';

Not: '!';

Assign: '=';

Less: '<';

Greater: '>';

PlusAssign: '+=';

MinusAssign: '-=';

StarAssign: '*=';

DivAssign: '/=';

ModAssign: '%=';

XorAssign: '^=';

AndAssign: '&=';

OrAssign: '|=';

LeftShiftAssign: '<<=';

RightShiftAssign: '>>=';

Equal: '==';

NotEqual: '!=';

LessEqual: '<=';

GreaterEqual: '>=';

AndAnd: '&&';

OrOr: '||';

PlusPlus: '++';

MinusMinus: '--';

Comma: ',';

Question: '?';

Colon: ':';

Doublecolon: '::';

Semi: ';';

Dot: '.';

Identifier:
    /*
	 Identifiernondigit | Identifier Identifiernondigit | Identifier DIGIT
	 */ Identifiernondigit (Identifiernondigit | DIGIT)*
;

fragment Identifiernondigit: NONDIGIT;

fragment NONDIGIT: [a-zA-Z_];

fragment DIGIT: [0-9];

DecimalLiteral: NONZERODIGIT ('\''? DIGIT)*;

OctalLiteral: '0' ('\''? OCTALDIGIT)*;

HexadecimalLiteral: ('0x' | '0X') HEXADECIMALDIGIT ( '\''? HEXADECIMALDIGIT)*;

BinaryLiteral: ('0b' | '0B') BINARYDIGIT ('\''? BINARYDIGIT)*;

fragment NONZERODIGIT: [1-9];

fragment OCTALDIGIT: [0-7];

fragment HEXADECIMALDIGIT: [0-9a-fA-F];

fragment BINARYDIGIT: [01];

Integersuffix:
    Unsignedsuffix Longsuffix?
    | Unsignedsuffix Longlongsuffix?
    | Longsuffix Unsignedsuffix?
    | Longlongsuffix Unsignedsuffix?
;

fragment Unsignedsuffix: [uU];

fragment Longsuffix: [lL];

fragment Longlongsuffix: 'll' | 'LL';

fragment Cchar: ~ ['\\\r\n] | Escapesequence;

fragment Escapesequence: Simpleescapesequence | Octalescapesequence | Hexadecimalescapesequence;

fragment Simpleescapesequence:
    '\\\''
    | '\\"'
    | '\\?'
    | '\\\\'
    | '\\a'
    | '\\b'
    | '\\f'
    | '\\n'
    | '\\r'
    | ('\\' ('\r' '\n'? | '\n'))
    | '\\t'
    | '\\v'
;

fragment Octalescapesequence:
    '\\' OCTALDIGIT
    | '\\' OCTALDIGIT OCTALDIGIT
    | '\\' OCTALDIGIT OCTALDIGIT OCTALDIGIT
;

fragment Hexadecimalescapesequence: '\\x' HEXADECIMALDIGIT+;

fragment Fractionalconstant: Digitsequence? '.' Digitsequence | Digitsequence '.';

fragment Exponentpart: 'e' SIGN? Digitsequence | 'E' SIGN? Digitsequence;

fragment SIGN: [+-];

fragment Digitsequence: DIGIT ('\''? DIGIT)*;

fragment Floatingsuffix: [flFL];

fragment Encodingprefix: 'u8' | 'u' | 'U' | 'L';

UserDefinedIntegerLiteral:
    DecimalLiteral Udsuffix
    | OctalLiteral Udsuffix
    | HexadecimalLiteral Udsuffix
    | BinaryLiteral Udsuffix
;

UserDefinedFloatingLiteral:
    Fractionalconstant Exponentpart? Udsuffix
    | Digitsequence Exponentpart Udsuffix
;

UserDefinedStringLiteral: StringLiteral Udsuffix;

UserDefinedCharacterLiteral: CharacterLiteral Udsuffix;

fragment Udsuffix: Identifier;

Whitespace: [ \t]+ -> channel(HIDDEN);

Newline                   : ('\r' '\n'? | '\n')                               -> skip;
BlockComment              : '/*' .*? '*/'                                     -> skip;
LineComment               : '//' ~ [\r\n]*                                    -> skip;