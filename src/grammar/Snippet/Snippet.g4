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

grammar Snippet;

snippet: part* EOF;

part:
    TEXT_CONTENT          # Text
    | ESCAPED_CHAR        # EscapedChar
    | VARIABLE            # Variable
    | VARIABLE_WITH_DEFAULT # VariableWithDefault
    | PLACEHOLDER         # Placeholder
    | TABSTOP             # Tabstop
    | TABSTOP_WITH_DEFAULT # TabstopWithDefault
    | CHOICE              # Choice
    ;

TEXT_CONTENT: ~[$\r\n\\]+;

ESCAPED_CHAR: '\\' [${}[\]\\];

VARIABLE: '$' IDENTIFIER;

VARIABLE_WITH_DEFAULT: '${' IDENTIFIER ':' ~[}]* '}';

PLACEHOLDER: '$0';

TABSTOP: '$' [1-9] [0-9]*;

TABSTOP_WITH_DEFAULT: '${' [1-9] [0-9]* ':' ~[}]* '}';

CHOICE: '${' [1-9] [0-9]* '|' CHOICE_OPTIONS '}';

fragment CHOICE_OPTIONS: ~[|}\r\n]+ (',' ~[|}\r\n]+)*;

fragment IDENTIFIER: [a-zA-Z_] [a-zA-Z0-9_]*;

WS: [ \t\r\n]+ -> skip;

