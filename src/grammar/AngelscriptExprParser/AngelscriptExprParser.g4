grammar AngelscriptExprParser;

expr
    : logicalOrExpr EOF
    ;

logicalOrExpr
    : logicalAndExpr (OrOr logicalAndExpr)*
    ;

logicalAndExpr
    : bitOrExpr (AndAnd bitOrExpr)*
    ;

bitOrExpr
    : bitXorExpr (Or bitXorExpr)*
    ;

bitXorExpr
    : bitAndExpr (Xor bitAndExpr)*
    ;

bitAndExpr
    : equalityExpr (And equalityExpr)*
    ;

equalityExpr
    : relationalExpr ((Equal | NotEqual) relationalExpr)*
    ;

relationalExpr
    : shiftExpr ((Less | LessEqual | Greater | GreaterEqual) shiftExpr)*
    ;

shiftExpr
    : additiveExpr ((Greater Greater Greater | Greater Greater | Less Less) additiveExpr)*
    ;

additiveExpr
    : multiplicativeExpr ((Plus | Minus) multiplicativeExpr)*
    ;

multiplicativeExpr
    : unaryExpr ((Star | Div | Mod) unaryExpr)*
    ;

unaryExpr
    : (Plus | Minus | Not | Tilde)* primaryExpr
    ;

primaryExpr
    : LeftParen expr RightParen
    | literal
    ;

literal
    : IntegerLiteral
    | FloatingLiteral
    | CharacterLiteral
    | StringLiteral
    | True_
    | False_
    | Nullptr
    ;

// keywords
True_    : 'true';
False_   : 'false';
Nullptr  : 'null';

// operators and punctuation
LeftParen   : '(';
RightParen  : ')';
Plus        : '+';
Minus       : '-';
Star        : '*';
Div         : '/';
Mod         : '%';
Xor         : '^';
And         : '&';
Or          : '|';
Tilde       : '~';
Not         : '!';
Less        : '<';
Greater     : '>';
Equal       : '==';
NotEqual    : '!=';
LessEqual   : '<=';
GreaterEqual: '>=';
AndAnd      : '&&';
OrOr        : '||';

// literals
IntegerLiteral
    : DecimalLiteral
    | OctalLiteral
    | HexadecimalLiteral
    | BinaryLiteral
    ;

FloatingLiteral
    : Fractionalconstant Exponentpart?
    | Digitsequence Exponentpart
    ;

CharacterLiteral
    : '\'' (Cchar)+ '\''
    ;

StringLiteral
    : '"' (Schar)* '"'
    ;

// fragments
fragment NONDIGIT : [a-zA-Z_];
fragment DIGIT : [0-9];
fragment NONZERODIGIT : [1-9];
fragment OCTALDIGIT : [0-7];
fragment HEXADECIMALDIGIT : [0-9a-fA-F];
fragment BINARYDIGIT : [01];

fragment DecimalLiteral : NONZERODIGIT ('\''? DIGIT)*;
fragment OctalLiteral : '0' ('\''? OCTALDIGIT)*;
fragment HexadecimalLiteral : ('0x' | '0X') HEXADECIMALDIGIT ('\''? HEXADECIMALDIGIT)*;
fragment BinaryLiteral : ('0b' | '0B') BINARYDIGIT ('\''? BINARYDIGIT)*;

fragment Digitsequence : DIGIT ('\''? DIGIT)*;
fragment Fractionalconstant : Digitsequence? '.' Digitsequence | Digitsequence '.';
fragment Exponentpart : [eE] [+-]? Digitsequence;

fragment Cchar
    : ~['\\\r\n]
    | Escapesequence
    ;

fragment Schar
    : ~["\\\r\n]
    | Escapesequence
    ;

fragment Escapesequence
    : Simpleescapesequence
    | Octalescapesequence
    | Hexadecimalescapesequence
    | Unicodeescapesequence
    ;

fragment Simpleescapesequence
    : '\\\''
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

fragment Octalescapesequence
    : '\\' OCTALDIGIT
    | '\\' OCTALDIGIT OCTALDIGIT
    | '\\' OCTALDIGIT OCTALDIGIT OCTALDIGIT
    ;

fragment Hexadecimalescapesequence : '\\x' HEXADECIMALDIGIT+;

fragment Unicodeescapesequence
    : '\\u' HEXADECIMALDIGIT HEXADECIMALDIGIT HEXADECIMALDIGIT HEXADECIMALDIGIT
    | '\\U' HEXADECIMALDIGIT HEXADECIMALDIGIT HEXADECIMALDIGIT HEXADECIMALDIGIT HEXADECIMALDIGIT HEXADECIMALDIGIT HEXADECIMALDIGIT HEXADECIMALDIGIT
    ;

// whitespace
Whitespace   : [ \t]+ -> skip;
Newline      : ('\r' '\n'? | '\n') -> skip;
BlockComment : '/*' .*? '*/' -> skip;
LineComment  : '//' ~[\r\n]* -> skip;
