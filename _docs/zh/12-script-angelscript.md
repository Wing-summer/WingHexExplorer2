---
title: 脚本开发 - AngelScript 简介
permalink: /docs/zh/script-angelscript
toc: true
---

## 简介

AngelScript 是由 Andreas Jönsson（AngelCode）开发的开源、跨平台、高性能脚本语言，专为嵌入 C++ 应用（尤其是游戏引擎）而设计，语法类似 C/C++，但去除了指针和多继承等复杂特性，以静态强类型、面向对象为核心，支持自动内存管理和自定义类型注册，能够无缝调用主程序中的 C/C++ 函数与类。如果你对这个语言很感兴趣，可以到这个 [官网](https://www.angelcode.com/angelscript/) 去了解。

在选择哪种脚本语言时，我（寂静的羽夏）当时有很多很多选项：Python、lua、js、ChaiScript 甚至是 C# ,在最初 WingHexExplorer 都选择过 Python ，为什么最终选择了这门语言？有下面几个考虑：

- 更新很勤快：我不需要为了 Bug 擦屁股；
- 轻量：我不需要奇奇怪怪的绑定才能舒服的使用；
- 跨平台且容易集成编译：当然这个是这个语言的副产物，这个是必须的，这不是它在这几个脚本语言选择中的优势，不过容易集成算是个不小的优势；
- 静态语言：实现一个功能稍微丰富代码编辑器，我只需要词法分析就基本完成代码填充工作，这就够了，代码风格我也很喜欢，当然作为对类型大小要求严格的十六进制这也是很需要的；
- 文档和周边比较多：我可以实现调试功能、各种可以借鉴学习的库和实现，这是一个巨大的优势；

## 引出下文

看到语法类似 C/C++ ，请不要害怕，它十分简单它不支持 C/C++ 一些奇奇怪怪嵌套的声明方法或者书写方式。如果你了解编译原理，它的完整的语法结构可以用 BNF 描述如下：

```bnf
SCRIPT        ::= {IMPORT | ENUM | TYPEDEF | CLASS | MIXIN | INTERFACE | FUNCDEF | VIRTPROP | VAR | FUNC | NAMESPACE | ';'}
NAMESPACE     ::= 'namespace' IDENTIFIER {'::' IDENTIFIER} '{' SCRIPT '}'
ENUM          ::= {'shared' | 'external'} 'enum' IDENTIFIER (';' | ('{' IDENTIFIER ['=' EXPR] {',' IDENTIFIER ['=' EXPR]} '}'))
CLASS         ::= {'shared' | 'abstract' | 'final' | 'external'} 'class' IDENTIFIER (';' | ([':' IDENTIFIER {',' IDENTIFIER}] '{' {VIRTPROP | FUNC | VAR | FUNCDEF} '}'))
TYPEDEF       ::= 'typedef' PRIMTYPE IDENTIFIER ';'
FUNC          ::= {'shared' | 'external'} ['private' | 'protected'] [((TYPE ['&']) | '~')] IDENTIFIER PARAMLIST ['const'] FUNCATTR (';' | STATBLOCK)
INTERFACE     ::= {'external' | 'shared'} 'interface' IDENTIFIER (';' | ([':' IDENTIFIER {',' IDENTIFIER}] '{' {VIRTPROP | INTFMTHD} '}'))
VAR           ::= ['private'|'protected'] TYPE IDENTIFIER [( '=' (INITLIST | EXPR)) | ARGLIST] {',' IDENTIFIER [( '=' (INITLIST | EXPR)) | ARGLIST]} ';'
IMPORT        ::= 'import' TYPE ['&'] IDENTIFIER PARAMLIST FUNCATTR 'from' STRING ';'
FUNCDEF       ::= {'external' | 'shared'} 'funcdef' TYPE ['&'] IDENTIFIER PARAMLIST ';'
VIRTPROP      ::= ['private' | 'protected'] TYPE ['&'] IDENTIFIER '{' {('get' | 'set') ['const'] FUNCATTR (STATBLOCK | ';')} '}'
MIXIN         ::= 'mixin' CLASS
INTFMTHD      ::= TYPE ['&'] IDENTIFIER PARAMLIST ['const'] ';'
STATBLOCK     ::= '{' {VAR | STATEMENT} '}'
PARAMLIST     ::= '(' ['void' | (TYPE TYPEMOD [IDENTIFIER] ['=' EXPR] {',' TYPE TYPEMOD [IDENTIFIER] ['=' EXPR]})] ')'
TYPEMOD       ::= ['&' ['in' | 'out' | 'inout']]
TYPE          ::= ['const'] SCOPE DATATYPE ['<' TYPE {',' TYPE} '>'] { ('[' ']') | ('@' ['const']) }
INITLIST      ::= '{' [ASSIGN | INITLIST] {',' [ASSIGN | INITLIST]} '}'
SCOPE         ::= ['::'] {IDENTIFIER '::'} [IDENTIFIER ['<' TYPE {',' TYPE} '>'] '::']
DATATYPE      ::= (IDENTIFIER | PRIMTYPE | '?' | 'auto')
PRIMTYPE      ::= 'void' | 'int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64' | 'float' | 'double' | 'bool'
FUNCATTR      ::= {'override' | 'final' | 'explicit' | 'property' | 'delete'}
STATEMENT     ::= (IF | FOR | WHILE | RETURN | STATBLOCK | BREAK | CONTINUE | DOWHILE | SWITCH | EXPRSTAT | TRY)
SWITCH        ::= 'switch' '(' ASSIGN ')' '{' {CASE} '}'
BREAK         ::= 'break' ';'
FOR           ::= 'for' '(' (VAR | EXPRSTAT) EXPRSTAT [ASSIGN {',' ASSIGN}] ')' STATEMENT
WHILE         ::= 'while' '(' ASSIGN ')' STATEMENT
DOWHILE       ::= 'do' STATEMENT 'while' '(' ASSIGN ')' ';'
IF            ::= 'if' '(' ASSIGN ')' STATEMENT ['else' STATEMENT]
CONTINUE      ::= 'continue' ';'
EXPRSTAT      ::= [ASSIGN] ';'
TRY           ::= 'try' STATBLOCK 'catch' STATBLOCK
RETURN        ::= 'return' [ASSIGN] ';'
CASE          ::= (('case' EXPR) | 'default') ':' {STATEMENT}
EXPR          ::= EXPRTERM {EXPROP EXPRTERM}
EXPRTERM      ::= ([TYPE '='] INITLIST) | ({EXPRPREOP} EXPRVALUE {EXPRPOSTOP})
EXPRVALUE     ::= 'void' | CONSTRUCTCALL | FUNCCALL | VARACCESS | CAST | LITERAL | '(' ASSIGN ')' | LAMBDA
CONSTRUCTCALL ::= TYPE ARGLIST
EXPRPREOP     ::= '-' | '+' | '!' | '++' | '--' | '~' | '@'
EXPRPOSTOP    ::= ('.' (FUNCCALL | IDENTIFIER)) | ('[' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':' ASSIGN} ']') | ARGLIST | '++' | '--'
CAST          ::= 'cast' '<' TYPE '>' '(' ASSIGN ')'
LAMBDA        ::= 'function' '(' [[TYPE TYPEMOD] [IDENTIFIER] {',' [TYPE TYPEMOD] [IDENTIFIER]}] ')' STATBLOCK
LITERAL       ::= NUMBER | STRING | BITS | 'true' | 'false' | 'null'
FUNCCALL      ::= SCOPE IDENTIFIER ARGLIST
VARACCESS     ::= SCOPE IDENTIFIER
ARGLIST       ::= '(' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ')'
ASSIGN        ::= CONDITION [ ASSIGNOP ASSIGN ]
CONDITION     ::= EXPR ['?' ASSIGN ':' ASSIGN]
EXPROP        ::= MATHOP | COMPOP | LOGICOP | BITOP
BITOP         ::= '&' | '|' | '^' | '<<' | '>>' | '>>>'
MATHOP        ::= '+' | '-' | '*' | '/' | '%' | '**'
COMPOP        ::= '==' | '!=' | '<' | '<=' | '>' | '>=' | 'is' | '!is'
LOGICOP       ::= '&&' | '||' | '^^' | 'and' | 'or' | 'xor'
ASSIGNOP      ::= '=' | '+=' | '-=' | '*=' | '/=' | '|=' | '&=' | '^=' | '%=' | '**=' | '<<=' | '>>=' | '>>>='
IDENTIFIER    ::= single token:  starts with letter or _, can include any letter and digit, same as in C++
NUMBER        ::= single token:  includes integers and real numbers, same as C++
STRING        ::= single token:  single quoted ', double quoted ", or heredoc multi-line string """
BITS          ::= single token:  binary 0b or 0B, octal 0o or 0O, decimal 0d or 0D, hexadecimal 0x or 0X
COMMENT       ::= single token:  starts with // and ends with new line or starts with /* and ends with */
WHITESPACE    ::= single token:  spaces, tab, carriage return, line feed, and UTF8 byte-order-mark
```

猛地一看确实挺长的，但是，在研究词法分析的时候，你就会知道这门语言设计的是多简单。请不要害怕，我会把 AngelScript 几乎所有的用法进行介绍，并且教你如何使用它来实现一个自动化分析程序中打开的二进制文件。

你可以把这个教程作为你使用 AngelScript 去开发其他软件的脚本，但是请注意。该软件使用的是 AngelScript 的子集，另外有自己的独特的 API 以及脚本代码预处理器。有些仅供参考，其他的脚本引擎环境未必拥有该软件实现的特性。
{: .notice--warning}

## 引擎配置说明

在`ScriptMachine`中，以下代码是对引擎进行配置处理，它影响了引擎的行为：

```cpp
_engine->SetEngineProperty(asEP_EXPAND_DEF_ARRAY_TO_TMPL, true);
_engine->SetEngineProperty(asEP_DISALLOW_EMPTY_LIST_ELEMENTS, true);
_engine->SetEngineProperty(asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE, false);
_engine->SetEngineProperty(asEP_ALLOW_MULTILINE_STRINGS, false);
_engine->SetEngineProperty(asEP_USE_CHARACTER_LITERALS, false);
_engine->SetEngineProperty(asEP_DISABLE_INTEGER_DIVISION, false);
_engine->SetEngineProperty(asEP_PRIVATE_PROP_AS_PROTECTED, false);
_engine->SetEngineProperty(asEP_ALTER_SYNTAX_NAMED_ARGS, 0);
_engine->SetEngineProperty(asEP_ALLOW_UNICODE_IDENTIFIERS, true);
_engine->SetEngineProperty(asEP_REQUIRE_ENUM_SCOPE, true); 
```

这些配置可以总结如下：

1. 数组你可以声明为`T[]`，而必须`array<T>`。该引擎默认不支持数组，需要注册额外的插件才能够使用。本软件已经注册并且支持`T[]`来更方便的进行声明。
2. 在初始化中不能有空对象，比如：`array<int> arr = {1,2,,4,5,};`这个就是非法的，因为这里面有的元素是空的。
3. 除法和字符串行为和 C/C++ 的一致（注：该脚本支持 heredoc 字符串）。
4. 枚举和 C++ 的枚举类行为一致。
5. 脚本代码和变量名、标识符支持中文。

有关其他的不同比如注册的插件或者注意事项将会在后续教程中来进行说明。

## 代码预处理器

