/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
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

#include "qascodeparser.h"

#include <QStack>

#include "AngelScript/sdk/angelscript/source/as_scriptengine.h"
#include "AngelScript/sdk/angelscript/source/as_tokenizer.h"

/* Some information about AngelScript and this class:
 * * This class is designed to get variables and functions symbol.
 * * Strings are also included.
 * * AngelScript's class and enum can not be nested unlike C++.
 * * We only support class / mixin class / interface / function / enum.
 *
 * * auto type can not be derived by this class, PRs are welcomed
 */

QAsCodeParser::QAsCodeParser(asCScriptEngine *engine) : engine(engine) {
    Q_ASSERT(engine);
    _checkValidTypes = false;
    _isParsingAppInterface = false;
}

QAsCodeParser::QAsCodeParser(asIScriptEngine *engine) {
    this->engine = dynamic_cast<asCScriptEngine *>(engine);
    Q_ASSERT(engine);
    _checkValidTypes = false;
    _isParsingAppInterface = false;
}

QAsCodeParser::~QAsCodeParser() {}

QList<QAsCodeParser::CodeSegment>
QAsCodeParser::parse(const QByteArray &codes) {
    reset();
    _code = codes;
    return parseScript(false);
}

QList<QAsCodeParser::Symbol>
QAsCodeParser::parseAndIntell(qsizetype offset, const QByteArray &codes) {
    return parseIntell(offset, parse(codes));
}

QList<QAsCodeParser::Symbol>
QAsCodeParser::parseIntell(qsizetype offset,
                           const QList<QAsCodeParser::CodeSegment> &segs) {
    reset();

    QList<Symbol> ret;

    // first: global entries
    for (auto &seg : segs) {
        if (!seg.isValid()) {
            continue;
        }

        Symbol sym;
        sym.symtype = seg.type;
        sym.scope = seg.scope;
        sym.offset = seg.offset;
        sym.name = seg.name;

        switch (seg.type) {
        case SymbolType::Function: {
            sym.type = seg.additonalInfos.at(0);
            sym.additonalInfo = seg.additonalInfos.at(1);
            if (offset > seg.offset && offset < seg.end()) {
                sym.children = parseStatementBlock(sym.scope, seg.codes,
                                                   offset - seg.offset);
            }
        } break;
        case SymbolType::Enum:
            if (offset < seg.offset || offset >= seg.end()) {
                sym.children =
                    parseEnumerationContent(sym.scope, sym.name, seg.codes);
            }
            break;
        case SymbolType::TypeDef:
            if (offset < seg.offset || offset >= seg.end()) {
                sym = parseTypedefContent(sym.scope, seg.codes);
            }
            break;
        case SymbolType::Variable:
            if (offset < seg.offset || offset >= seg.end()) {
                ret << parseGlobalVarDecls(sym.scope, seg.codes);
                continue;
            }
            break;
        case SymbolType::FnDef:
            if (offset < seg.offset || offset >= seg.end()) {
                sym = parseFuncDefContent(sym.scope, seg.codes);
                QByteArrayList argBuffer;
                for (auto &c : sym.children) {
                    if (c.name.isEmpty()) {
                        argBuffer.append(c.type);
                    } else {
                        argBuffer.append(c.type + ' ' + c.name);
                    }
                }
                sym.additonalInfo = argBuffer.join(", ");
                ret.append(sym);
                continue;
            }
            break;
        case SymbolType::Class: {
            auto syms =
                parseClassContent(offset - seg.offset, sym.scope, seg.codes);
            // TODO: PRS, 'cause i have no need to code-complete a class
            sym.inherit = syms.first;
            sym.children = syms.second;
            if (offset > seg.offset && offset < seg.end()) {
                ret.append(syms.second);
            }
        } break;
        case SymbolType::Interface: {
            auto syms = parseInterfaceContent(offset - seg.offset, sym.scope,
                                              seg.codes);
            // TODO: PRS, 'cause i have no need to code-complete an interface
            sym.inherit = syms.first;
            sym.children = syms.second;
            if (offset > seg.offset && offset < seg.end()) {
                ret.append(syms.second);
            }
        } break;
        case SymbolType::Invalid:
        case SymbolType::Import:
            continue;
        }

        ret.append(sym);
    }

    return ret;
}

void QAsCodeParser::reset() {
    _errorWhileParsing = false;
    _isSyntaxError = false;
    _checkValidTypes = false;
    _isParsingAppInterface = false;

    _sourcePos = 0;
    _code.clear();

    _lastToken.length = 0;
    _lastToken.type = ttUnrecognizedToken;
    _lastToken.pos = size_t(-1);
}

void QAsCodeParser::getToken(sToken *token) {
    // Check if the token has already been parsed
    if (_lastToken.pos == _sourcePos) {
        *token = _lastToken;
        _sourcePos += token->length;

        if (token->type == ttWhiteSpace || token->type == ttOnelineComment ||
            token->type == ttMultilineComment)
            getToken(token);

        return;
    }

    // Parse new token
    size_t sourceLength = _code.length();
    do {
        if (_sourcePos >= sourceLength) {
            token->type = ttEnd;
            token->length = 0;
        } else
            token->type =
                engine->tok.GetToken(_code.data() + _sourcePos,
                                     sourceLength - _sourcePos, &token->length);

        token->pos = _sourcePos;

        // Update state
        _sourcePos += token->length;
    }
    // Filter out whitespace and comments
    while (token->type == ttWhiteSpace || token->type == ttOnelineComment ||
           token->type == ttMultilineComment);
}

void QAsCodeParser::rewindTo(const sToken *token) {
    // TODO: optimize: Perhaps we can optimize this further by having the parser
    //                 set an explicit return point, after which each token will
    //                 be stored. That way not just one token will be reused but
    //                 no token will have to be tokenized more than once.

    // Store the token so it doesn't have to be tokenized again
    _lastToken = *token;

    _sourcePos = token->pos;
}

void QAsCodeParser::setPos(size_t pos) {
    _lastToken.pos = size_t(-1);
    _sourcePos = pos;
}

void QAsCodeParser::rewindErrorTo(sToken *token) {
    rewindTo(token);

    _isSyntaxError = true;
    _errorWhileParsing = true;
}

QList<QAsCodeParser::Symbol> QAsCodeParser::parseParameterListContent() {
    QList<QAsCodeParser::Symbol> ret;

    sToken t1;
    getToken(&t1);
    if (t1.type != ttOpenParenthesis) {
        rewindErrorTo(&t1);
        return ret;
    }

    getToken(&t1);
    if (t1.type == ttCloseParenthesis) {
        // Statement block is finished
        return ret;
    } else {
        // If the parameter list is just (void) then the void token should be
        // ignored
        if (t1.type == ttVoid) {
            sToken t2;
            getToken(&t2);
            if (t2.type == ttCloseParenthesis) {
                return ret;
            }
        }

        rewindTo(&t1);

        for (;;) {
            // Parse data type
            auto t = parseType(true, _isParsingAppInterface);
            if (_isSyntaxError)
                return ret;

            parseTypeMod(true);
            if (_isSyntaxError)
                return ret;

            Symbol se;
            se.symtype = SymbolType::Variable;
            se.type = t;

            // Parse optional identifier
            getToken(&t1);
            if (t1.type == ttIdentifier) {
                rewindTo(&t1);

                auto iden = parseIdentifier();
                se.name = getSymbolString(iden);
                se.offset = iden.pos;

                if (_isSyntaxError)
                    return ret;

                getToken(&t1);
            }

            ret.append(se);

            // Parse optional expression for the default arg
            if (t1.type == ttAssignment) {
                // Do a superficial parsing of the default argument
                // The actual parsing will be done when the argument is compiled
                // for a function call
                superficiallyParseExpression();
                if (_isSyntaxError)
                    return ret;

                getToken(&t1);
            }

            // Check if list continues
            if (t1.type == ttCloseParenthesis) {
                return ret;
            } else if (t1.type == ttListSeparator)
                continue;
            else {
                rewindErrorTo(&t1);
                return ret;
            }
        }
    }
    UNREACHABLE_RETURN;
}

void QAsCodeParser::superficiallyParseExpression() {
    // Simply parse everything until the first , or ), whichever comes first.
    // Keeping in mind that () and {} can group expressions.
    sToken start;
    getToken(&start);
    rewindTo(&start);

    asCString stack;
    sToken t;
    for (;;) {
        getToken(&t);

        if (t.type == ttOpenParenthesis)
            stack += "(";
        else if (t.type == ttCloseParenthesis) {
            if (stack == "") {
                // Expression has ended. This token is not part of expression
                rewindTo(&t);
                break;
            } else if (stack[stack.GetLength() - 1] == '(') {
                // Group has ended
                stack.SetLength(stack.GetLength() - 1);
            } else {
                // Wrong syntax
                rewindTo(&t);
                rewindErrorTo(&t);
                return;
            }
        } else if (t.type == ttListSeparator) {
            if (stack == "") {
                // Expression has ended. This token is not part of expression
                rewindTo(&t);
                break;
            }
        } else if (t.type == ttStartStatementBlock)
            stack += "{";
        else if (t.type == ttEndStatementBlock) {
            if (stack == "" || stack[stack.GetLength() - 1] != '{') {
                // Wrong syntax
                rewindErrorTo(&t);
                return;
            } else {
                // Group has ended
                stack.SetLength(stack.GetLength() - 1);
            }
        } else if (t.type == ttEndStatement) {
            // Wrong syntax (since we're parsing a default arg expression)
            rewindErrorTo(&t);
            return;
        } else if (t.type == ttNonTerminatedStringConstant) {
            rewindErrorTo(&t);
            return;
        } else if (t.type == ttEnd) {
            // Wrong syntax
            rewindErrorTo(&start);
            return;
        }

        // Include the token in the node
    }
}

void QAsCodeParser::superficiallyParseTemplateList() {
    sToken t;

    // Starts with '<'
    getToken(&t);
    if (t.type != ttLessThan) {
        return;
    }

    // End with '>'
    // Accept >> and >>> tokens too. But then force the tokenizer to move
    // only 1 character ahead (thus splitting the token in two).
    while (true) {
        getToken(&t);
        if (_code.length() <= t.pos) {
            break;
        }
        if (_code[QString::size_type(t.pos)] == '>') {
            setPos(t.pos + 1);
            break;
        }
    }
}

QByteArray QAsCodeParser::parseType(bool allowConst, bool allowVariableType,
                                    bool allowAuto) {
    sToken t;

    if (allowConst) {
        getToken(&t);
        rewindTo(&t);
        if (t.type == ttConst) {
            parseToken(ttConst);
            if (_isSyntaxError)
                return {};
        }
    }

    // Parse scope prefix
    parseOptionalScope();

    // Parse the actual type
    auto dt = parseDataType(allowVariableType, allowAuto);
    if (_isSyntaxError)
        return {};

    // If the datatype is a template type, then parse the subtype within the < >
    getToken(&t);
    rewindTo(&t);

    auto dtType = getSymbolString(dt);
    if (engine->IsTemplateType(dtType) && t.type == ttLessThan) {
        superficiallyParseTemplateList();
    }

    // Parse [] and @
    getToken(&t);
    rewindTo(&t);
    while (t.type == ttOpenBracket || t.type == ttHandle) {
        if (t.type == ttOpenBracket) {
            parseToken(ttOpenBracket);
            if (_isSyntaxError)
                return {};

            getToken(&t);
            if (t.type != ttCloseBracket) {
                rewindErrorTo(&t);
                return {};
            }
            dtType.prepend("array<").append(">");
        } else {
            parseToken(ttHandle);
            if (_isSyntaxError)
                return {};

            getToken(&t);
            rewindTo(&t);
            if (t.type == ttConst) {
                parseToken(ttConst);
                if (_isSyntaxError)
                    return {};
            }
        }

        getToken(&t);
        rewindTo(&t);
    }

    return dtType;
}

QAsCodeParser::Symbol
QAsCodeParser::parseFuncDefContent(const QByteArrayList &ns) {
    Symbol sym;

    sToken t1;
    getToken(&t1);

    if (t1.type != ttFuncDef) {
        rewindErrorTo(&t1);
        return sym;
    }

    sym.type = parseType(true);
    if (_isSyntaxError)
        return sym;

    parseTypeMod(false);
    if (_isSyntaxError)
        return sym;

    auto iden = parseIdentifier();
    if (_isSyntaxError)
        return sym;

    sym.scope = ns;
    sym.name = getSymbolString(iden);
    sym.offset = iden.pos;

    auto args = parseParameterListContent();
    if (_isSyntaxError)
        return sym;
    sym.children = args;

    getToken(&t1);
    if (t1.type != ttEndStatement) {
        rewindErrorTo(&t1);
        return sym;
    }

    sym.symtype = SymbolType::FnDef;
    return sym;
}

QList<QAsCodeParser::Symbol>
QAsCodeParser::parseStatementBlock(const QByteArrayList &ns,
                                   const QByteArray &code, qsizetype end) {
    reset();
    _code = code;

    bool startBlockSkip = false;

    sToken t1;
    getToken(&t1);
    if (t1.type != ttStartStatementBlock) {
        rewindErrorTo(&t1);
        return {};
    }

    QStack<QList<QAsCodeParser::Symbol>> syms;

    auto getSymbols =
        [](const QStack<QList<QAsCodeParser::Symbol>> &syms) -> auto {
        QList<QAsCodeParser::Symbol> ret;
        for (auto &symlist : syms) {
            for (auto &sym : symlist) {
                if (sym.symtype == SymbolType::Variable &&
                    !sym.type.isEmpty()) {
                    auto var = sym.name;
                    auto n = std::find_if(
                        ret.begin(), ret.end(),
                        [var](const QAsCodeParser::Symbol &sym) {
                            return sym.symtype == SymbolType::Variable &&
                                   sym.name == var;
                        });
                    if (n != ret.end()) {
                        *n = sym;
                    } else {
                        ret.append(sym);
                    }
                } else {
                    ret.append(sym);
                }
            }
        }
        return ret;
    };

    syms.push({});

    for (;;) {
        while (!_isSyntaxError) {
            getToken(&t1);
            if (t1.type == ttStartStatementBlock) {
                if (startBlockSkip) {
                    startBlockSkip = false;
                } else {
                    syms.push({});
                }
            } else if (t1.type == ttEndStatementBlock) {
                syms.pop();
            } else if (t1.type == ttFor) {
                // this block contains symbols
                getToken(&t1);
                if (t1.type != ttOpenParenthesis) {
                    _isSyntaxError = true;
                }
                syms.push({});
                startBlockSkip = true;
                continue;
            } else {
                rewindTo(&t1);

                if (isVarDecl()) {
                    syms.top().append(parseDeclaration(ns));
                } else {
                    _isSyntaxError = true;
                }
            }
        }

        if (_isSyntaxError) {
            // Search for either ';', '{', '}', or end
            getToken(&t1);
            while (t1.type != ttEndStatement && t1.type != ttEnd &&
                   t1.type != ttStartStatementBlock &&
                   t1.type != ttEndStatementBlock) {
                getToken(&t1);
                if (t1.pos + t1.length > end) {
                    return getSymbols(syms);
                }
            }

            if (t1.type == ttStartStatementBlock ||
                t1.type == ttEndStatementBlock) {
                rewindTo(&t1);
            } else if (t1.type == ttEnd) {
                return getSymbols(syms);
            }

            _isSyntaxError = false;
        }

        if (t1.pos + t1.length > end) {
            return getSymbols(syms);
        }
    }

    UNREACHABLE_RETURN;
}

void QAsCodeParser::parseTypeMod(bool isParam) {
    sToken t;

    // Parse possible & token
    getToken(&t);
    rewindTo(&t);
    if (t.type == ttAmp) {
        parseToken(ttAmp);
        if (_isSyntaxError)
            return;

        if (isParam) {
            getToken(&t);
            rewindTo(&t);

            if (t.type == ttIn || t.type == ttOut || t.type == ttInOut) {
                int tokens[3] = {ttIn, ttOut, ttInOut};
                parseOneOf(tokens, 3);
            }
        }
    }

    // Parse possible + token
    getToken(&t);
    rewindTo(&t);
    if (t.type == ttPlus) {
        parseToken(ttPlus);
        if (_isSyntaxError)
            return;
    }

    // Parse possible if_handle_then_const token
    getToken(&t);
    rewindTo(&t);
    if (identifierIs(t, IF_HANDLE_TOKEN)) {
        parseToken(ttIdentifier);
        if (_isSyntaxError)
            return;
    }
}

QByteArrayList QAsCodeParser::parseOptionalScope() {
    QByteArrayList scopes;

    sToken t1, t2;
    getToken(&t1);
    getToken(&t2);
    if (t1.type == ttScope) {
        rewindTo(&t1);
        parseToken(ttScope);
        getToken(&t1);
        getToken(&t2);
    }
    while (t1.type == ttIdentifier && t2.type == ttScope) {
        rewindTo(&t1);
        auto id = parseIdentifier();
        scopes.append(getSymbolString(id));
        parseToken(ttScope);
        getToken(&t1);
        getToken(&t2);
    }

    // The innermost scope may be a template type
    if (t1.type == ttIdentifier && t2.type == ttLessThan) {
        auto type = getSymbolString(t1);
        if (engine->IsTemplateType(type)) {
            rewindTo(&t1);
            parseIdentifier();
            superficiallyParseTemplateList();
            getToken(&t2);
            if (t2.type == ttScope) {
                // Template type is part of the scope
                // Nothing more needs to be done

                return scopes;
            } else {
                // The template type is not part of the scope
                // Rewind to the template type and end the scope
                rewindTo(&t1);
                return scopes;
            }
        }
    }

    // The identifier is not part of the scope
    rewindTo(&t1);

    return scopes;
}

sToken QAsCodeParser::parseRealType() {
    sToken t1;

    getToken(&t1);
    if (!isPrimType(t1.type)) {
        rewindErrorTo(&t1);
    }

    return t1;
}

sToken QAsCodeParser::parseDataType(bool allowVariableType, bool allowAuto) {
    sToken t1;

    getToken(&t1);
    if (!isDataType(t1) && !(allowVariableType && t1.type == ttQuestion) &&
        !(allowAuto && t1.type == ttAuto)) {
        rewindErrorTo(&t1);
        return t1;
    }

    return t1;
}

sToken QAsCodeParser::parseIdentifier() {
    sToken t1;

    getToken(&t1);
    if (t1.type != ttIdentifier) {
        rewindErrorTo(&t1);
    }

    return t1;
}

void QAsCodeParser::parseMethodAttributes() {
    sToken t1;

    for (;;) {
        getToken(&t1);
        rewindTo(&t1);

        if (identifierIs(t1, FINAL_TOKEN) || identifierIs(t1, OVERRIDE_TOKEN) ||
            identifierIs(t1, EXPLICIT_TOKEN) ||
            identifierIs(t1, PROPERTY_TOKEN) || identifierIs(t1, DELETE_TOKEN))
            ;
        else
            break;
    }
}

bool QAsCodeParser::isPrimType(int tokenType) {
    if (tokenType == ttVoid || tokenType == ttInt || tokenType == ttInt8 ||
        tokenType == ttInt16 || tokenType == ttInt64 || tokenType == ttUInt ||
        tokenType == ttUInt8 || tokenType == ttUInt16 ||
        tokenType == ttUInt64 || tokenType == ttFloat || tokenType == ttBool ||
        tokenType == ttDouble)
        return true;

    return false;
}

bool QAsCodeParser::isDataType(const sToken &token) {
    if (token.type == ttIdentifier) {
#ifndef AS_NO_COMPILER
        if (_checkValidTypes) {
            // Check if this is an existing type, regardless of namespace
            auto type = getSymbolString(token);
            if (!typeExist(type))
                return false;
        }
#endif
        return true;
    }

    if (isPrimType(token.type))
        return true;

    return false;
}

bool QAsCodeParser::identifierIs(const sToken &t, const char *str) {
    if (t.type != ttIdentifier)
        return false;

    return _code.sliced(t.pos, t.length) == QByteArray(str);
}

sToken QAsCodeParser::superficiallyParseStatementBlock() {
    // This function will only superficially parse the statement block in order
    // to find the end of it
    sToken t1;

    getToken(&t1);
    if (t1.type != ttStartStatementBlock) {
        rewindErrorTo(&t1);
        return t1;
    }

    sToken start = t1;

    int level = 1;
    while (level > 0 && !_isSyntaxError) {
        getToken(&t1);
        if (t1.type == ttEndStatementBlock)
            level--;
        else if (t1.type == ttStartStatementBlock)
            level++;
        else if (t1.type == ttEnd) {
            rewindErrorTo(&start);
            return start;
        }
    }

    return t1;
}

QAsCodeParser::CodeSegment QAsCodeParser::parseFunction() {
    CodeSegment seg;

    sToken t1;
    getToken(&t1);

    // A global function can be marked as shared and external
    while (t1.type == ttIdentifier) {
        if (identifierIs(t1, SHARED_TOKEN) ||
            identifierIs(t1, EXTERNAL_TOKEN)) {
            rewindTo(&t1);
            parseIdentifier();
            if (_isSyntaxError)
                return seg;
        } else
            break;

        getToken(&t1);
    }

    if (_isSyntaxError)
        return seg;

    // If it is a global function, or a method, except constructor and
    // destructor, then the return type is parsed
    sToken t2;
    getToken(&t2);
    rewindTo(&t1);

    QByteArray rettype;

    if (t1.type != ttBitNot && t2.type != ttOpenParenthesis) {
        auto id = parseType(true);
        if (_isSyntaxError)
            return seg;
        rettype = id;

        parseTypeMod(false);
        if (_isSyntaxError)
            return seg;
    } else {
        rewindErrorTo(&t1);
        return seg;
    }

    // the first is returning value type
    seg.additonalInfos.append(rettype);

    auto iden = parseIdentifier();
    if (_isSyntaxError)
        return seg;
    seg.name = getSymbolString(iden);

    getToken(&t1);
    if (t1.type != ttOpenParenthesis) {
        rewindErrorTo(&t1);
        return seg;
    }

    getToken(&t1);
    // the second is params raw string
    if (t1.type == ttCloseParenthesis) {
        // Statement block is finished
        seg.additonalInfos.append(QByteArray());
    } else {
        auto begin = t1.pos;
        while (true) {
            getToken(&t1);
            if (t1.type == ttCloseParenthesis) {
                break;
            }
            // just some easier way to dectect errors
            switch (t1.type) {
            case ttStartStatementBlock:
            case ttEndStatementBlock:
            case ttEndStatement:
            case ttEnd:
                rewindErrorTo(&t1);
                return seg;
            default:
                break;
            }
        }

        auto end = t1.pos;
        seg.additonalInfos.append(_code.sliced(begin, end - begin));
    }

    // TODO: Should support abstract methods, in which case no statement block
    // should be provided
    parseMethodAttributes();
    if (_isSyntaxError)
        return seg;

    // External shared functions must be ended with ';'
    getToken(&t1);
    rewindTo(&t1);

    seg.scope = currentNs;
    seg.type = SymbolType::Function;
    seg.offset = t1.pos;

    if (t1.type == ttEndStatement) {
        parseToken(ttEndStatement);
        return seg;
    }

    auto begin = t1.pos;

    // We should just find the end of the statement block here.
    t1 = superficiallyParseStatementBlock();
    auto end = t1.pos;
    seg.codes = _code.sliced(begin, end - begin + 1);
    return seg;
}

QAsCodeParser::CodeSegment QAsCodeParser::parseFuncDef() {
    CodeSegment seg;
    sToken t1;

    // Allow keywords 'external' and 'shared' before 'interface'
    getToken(&t1);
    while (identifierIs(t1, SHARED_TOKEN) || identifierIs(t1, EXTERNAL_TOKEN)) {
        rewindTo(&t1);

        if (_isSyntaxError)
            return seg;

        getToken(&t1);
    }

    if (t1.type != ttFuncDef) {
        rewindErrorTo(&t1);
        return seg;
    }

    auto begin = t1.pos;
    skipCodeBlock();
    getToken(&t1);
    rewindTo(&t1);
    auto end = t1.pos;

    // seg.name is empty
    seg.scope = currentNs;
    seg.offset = begin;
    seg.type = SymbolType::FnDef;
    seg.codes = _code.sliced(begin, end - begin);
    return seg;
}

QAsCodeParser::CodeSegment QAsCodeParser::parseInterface() {
    CodeSegment seg;
    sToken t;

    // Allow keywords 'external' and 'shared' before 'interface'
    getToken(&t);
    while (identifierIs(t, SHARED_TOKEN) || identifierIs(t, EXTERNAL_TOKEN)) {
        rewindTo(&t);
        parseIdentifier();

        if (_isSyntaxError)
            return seg;

        getToken(&t);
    }

    if (t.type != ttInterface) {
        rewindErrorTo(&t);
        return seg;
    }

    auto id = parseIdentifier();
    seg.name = getSymbolString(id);

    seg.scope = currentNs;
    seg.type = SymbolType::Interface;

    // External shared declarations are ended with ';'
    getToken(&t);
    seg.offset = t.pos;
    if (t.type == ttEndStatement) {
        rewindTo(&t);
        parseToken(ttEndStatement);
        return seg;
    }

    auto begin = t.pos;
    rewindTo(&t);
    t = superficiallyParseStatementBlock();
    auto end = t.pos;
    seg.codes = _code.sliced(begin, end - begin + 1);

    return seg;
}

void QAsCodeParser::superficiallyParseVarInit() {
    sToken t;
    getToken(&t);

    if (t.type == ttAssignment) {
        getToken(&t);
        sToken start = t;
        rewindTo(&t);

        // Find the end of the expression
        int indentParan = 0;
        int indentBrace = 0;
        do {
            getToken(&t);
            if (t.type == ttOpenParenthesis) {
                indentParan++;
            } else if (t.type == ttCloseParenthesis) {
                indentParan--;
            } else if (t.type == ttStartStatementBlock) {
                indentBrace++;
            } else if (t.type == ttEndStatementBlock) {
                indentBrace--;
            } else if (t.type == ttNonTerminatedStringConstant) {
                rewindErrorTo(&t);
                break;
            } else if (t.type == ttEnd) {
                rewindErrorTo(&t);
                break;
            }
        } while (indentParan || indentBrace ||
                 (t.type != ttListSeparator && t.type != ttEndStatement &&
                  t.type != ttEndStatementBlock));
        rewindTo(&t);
    } else if (t.type == ttOpenParenthesis) {
        sToken start = t;

        // Find the end of the argument list
        int indent = 1;
        while (indent) {
            getToken(&t);
            if (t.type == ttOpenParenthesis)
                indent++;
            else if (t.type == ttCloseParenthesis)
                indent--;
            else if (t.type == ttNonTerminatedStringConstant) {
                rewindErrorTo(&t);
                break;
            } else if (t.type == ttEnd) {
                rewindErrorTo(&start);
                break;
            }
        }
    } else {
        rewindErrorTo(&t);
    }
}

QList<QAsCodeParser::Symbol>
QAsCodeParser::parseDeclaration(const QByteArrayList &ns, bool isClassProp,
                                bool isGlobalVar) {
    QList<QAsCodeParser::Symbol> ret;
    Symbol sym;

    sToken t;
    getToken(&t);
    rewindTo(&t);

    // A class property can be preceeded by private
    if (t.type == ttPrivate && isClassProp) {
        parseToken(ttPrivate);
        sym.vis = Visiblity::Private;
    } else if (t.type == ttProtected && isClassProp) {
        parseToken(ttProtected);
        sym.vis = Visiblity::Protected;
    }

    // Parse data type
    auto type = parseType(true, false, !isClassProp);
    if (_isSyntaxError)
        return ret;

    sym.type = type;

    for (;;) {
        // Parse identifier
        auto id = parseIdentifier();
        if (_isSyntaxError)
            return ret;

        sym.name = getSymbolString(id);
        sym.offset = id.pos;
        sym.symtype = QAsCodeParser::SymbolType::Variable;

        if (isClassProp || isGlobalVar) {
            // Only superficially parse the initialization RewindErrorTo for the
            // class property
            getToken(&t);
            rewindTo(&t);
            if (t.type == ttAssignment || t.type == ttOpenParenthesis) {
                superficiallyParseVarInit();
                if (_isSyntaxError)
                    return ret;
            }
        } else {
            // If next token is assignment, parse expression
            getToken(&t);
            if (t.type == ttOpenParenthesis || t.type == ttAssignment) {
                rewindTo(&t);
                superficiallyParseVarInit();
                if (_isSyntaxError)
                    return ret;
            } else {
                rewindTo(&t);
            }
        }

        // continue if list separator, else terminate with end statement
        getToken(&t);
        if (t.type == ttListSeparator) {
            ret.append(sym);
            continue;
        } else {
            ret.append(sym);
            return ret;
        }
    }
    UNREACHABLE_RETURN;
}

QList<QAsCodeParser::CodeSegment> QAsCodeParser::parseScript(bool inBlock) {
    QList<CodeSegment> segs;

    for (;;) {
        while (!_isSyntaxError) {
            sToken tStart;
            getToken(&tStart);

            // Optimize by skipping tokens 'shared', 'external', 'final',
            // 'abstract' so they don't have to be checked in every condition
            sToken t1 = tStart;
            while (identifierIs(t1, SHARED_TOKEN) ||
                   identifierIs(t1, EXTERNAL_TOKEN) ||
                   identifierIs(t1, FINAL_TOKEN) ||
                   identifierIs(t1, ABSTRACT_TOKEN))
                getToken(&t1);
            rewindTo(&tStart);

            if (t1.type == ttImport) {
                CodeSegment seg;
                seg.offset = t1.pos;

                seg.type = SymbolType::Import;
                segs << seg;

                // import we don't support just skip
                skipCodeBlock();
            } else if (t1.type == ttEnum) // Handle enumerations
                segs << parseEnumeration();
            else if (t1.type == ttTypedef) // Handle primitive typedefs
                segs << parseTypedef();
            else if (t1.type == ttClass)
                segs << parseClass();
            else if (t1.type == ttMixin)
                segs << parseMixin();
            else if (t1.type == ttInterface)
                segs << parseInterface();
            else if (t1.type == ttFuncDef) {
                segs << parseFuncDef();
            } else if (t1.type == ttConst || t1.type == ttScope ||
                       t1.type == ttAuto || isDataType(t1)) {
                // class properties parsing now are in deep parsing,
                // so there is no need to parse there
                if (isVarDecl()) {
                    auto begin = t1.pos;
                    skipCodeBlock();
                    getToken(&t1);
                    auto end = t1.pos;

                    CodeSegment seg;
                    // seg.name is empty
                    seg.offset = begin;
                    seg.scope = currentNs;
                    seg.type = SymbolType::Variable;
                    seg.codes = _code.sliced(begin, end - begin + 1);
                    rewindTo(&t1);

                    segs.append(seg);
                } else {
                    segs << parseFunction();
                }
            } else if (t1.type == ttEndStatement) {
                // Ignore a semicolon by itself
                getToken(&t1);
            } else if (t1.type == ttNamespace)
                parseNamespace();
            else if (t1.type == ttEnd)
                return segs;
            else if (inBlock && t1.type == ttEndStatementBlock)
                return segs;
            else {
                rewindErrorTo(&t1);
            }
        }

        if (_isSyntaxError) {
            skipCodeBlock();
            _isSyntaxError = false;
        }
    }
    UNREACHABLE_RETURN;
}

void QAsCodeParser::skipCodeBlock() {
    // Search for either ';' or '{' or end
    sToken t1;
    getToken(&t1);
    while (t1.type != ttEndStatement && t1.type != ttEnd &&
           t1.type != ttStartStatementBlock)
        getToken(&t1);

    if (t1.type == ttStartStatementBlock) {
        // Find the end of the block and skip nested blocks
        int level = 1;
        while (level > 0) {
            getToken(&t1);
            if (t1.type == ttStartStatementBlock)
                level++;
            if (t1.type == ttEndStatementBlock)
                level--;
            if (t1.type == ttEnd)
                break;
        }
    }
}

QByteArray QAsCodeParser::getSymbolString(const sToken &t) {
    return QByteArray(_code.data() + t.pos, t.length);
}

QAsCodeParser::CodeSegment QAsCodeParser::parseMixin() {
    sToken t;
    getToken(&t);

    if (t.type != ttMixin) {
        rewindErrorTo(&t);
        return {};
    }

    return parseClass();
}

QAsCodeParser::CodeSegment QAsCodeParser::parseClass() {
    CodeSegment seg;
    sToken t;
    getToken(&t);

    // Allow the keywords 'shared', 'abstract', 'final', and 'external' before
    // 'class'
    while (identifierIs(t, SHARED_TOKEN) || identifierIs(t, ABSTRACT_TOKEN) ||
           identifierIs(t, FINAL_TOKEN) || identifierIs(t, EXTERNAL_TOKEN)) {
        rewindTo(&t);
        parseIdentifier();
        getToken(&t);
    }

    if (t.type != ttClass) {
        rewindErrorTo(&t);
        return seg;
    }

    if (engine->ep.allowImplicitHandleTypes) {
        // Parse 'implicit handle class' construct
        getToken(&t);

        if (t.type != ttHandle)
            rewindTo(&t);
    }

    auto cls = parseIdentifier();
    seg.name = getSymbolString(cls);

    seg.scope = currentNs;
    seg.type = SymbolType::Class;

    // External shared declarations are ended with ';'
    getToken(&t);
    seg.offset = t.pos;
    if (t.type == ttEndStatement) {
        rewindTo(&t);
        parseToken(ttEndStatement);
        return seg;
    }

    rewindTo(&t);
    auto begin = t.pos;
    t = superficiallyParseStatementBlock();
    auto end = t.pos;
    seg.codes = _code.sliced(begin, end - begin + 1);
    return seg;
}

QAsCodeParser::CodeSegment QAsCodeParser::parseTypedef() {
    CodeSegment seg;
    sToken token;

    getToken(&token);
    if (token.type != ttTypedef) {
        rewindErrorTo(&token);
        return seg;
    }

    auto begin = token.pos;
    skipCodeBlock();
    getToken(&token);
    rewindTo(&token);
    auto end = token.pos;

    // seg.name is empty
    seg.scope = currentNs;
    seg.offset = begin;
    seg.type = SymbolType::TypeDef;
    seg.codes = _code.sliced(begin, end - begin);
    return seg;
}

QAsCodeParser::CodeSegment QAsCodeParser::parseEnumeration() {
    CodeSegment seg;
    sToken token;

    // Optional 'shared' and 'external' token
    getToken(&token);
    auto ebegin = token.pos;
    while (identifierIs(token, SHARED_TOKEN) ||
           identifierIs(token, EXTERNAL_TOKEN)) {
        rewindTo(&token);

        parseIdentifier();

        if (_isSyntaxError)
            return seg;

        getToken(&token);
    }

    // Check for enum
    if (token.type != ttEnum) {
        rewindErrorTo(&token);
        return seg;
    }

    // Get the identifier
    getToken(&token);
    if (ttIdentifier != token.type) {
        rewindErrorTo(&token);
        return seg;
    }

    seg.name = getSymbolString(token);
    seg.scope = currentNs;
    seg.offset = ebegin;
    seg.type = SymbolType::Enum;

    // External shared declarations are ended with ';'
    getToken(&token);
    if (token.type == ttEndStatement) {
        rewindTo(&token);
        parseToken(ttEndStatement);
        return seg;
    }

    auto begin = token.pos;
    rewindTo(&token);
    token = superficiallyParseStatementBlock();
    auto end = token.pos;
    seg.codes = _code.sliced(begin, end - begin + 1);
    return seg;
}

void QAsCodeParser::parseNamespace() {
    sToken t1;
    QByteArrayList ns;

    getToken(&t1);
    if (t1.type != ttNamespace) {
        rewindErrorTo(&t1);
    }

    auto id = parseIdentifier();
    if (_isSyntaxError)
        return;

    ns.append(getSymbolString(id));

    getToken(&t1);
    while (t1.type == ttScope) {
        auto id = parseIdentifier();

        if (_isSyntaxError)
            return;

        ns.append(getSymbolString(id));
        getToken(&t1);
    }

    if (t1.type != ttStartStatementBlock) {
        rewindErrorTo(&t1);
        return;
    }

    currentNs = ns;
    sToken start = t1;
    parseScript(true);
    currentNs.clear();

    if (!_isSyntaxError) {
        getToken(&t1);
        if (t1.type != ttEndStatementBlock) {
            rewindErrorTo(&start);
            return;
        }
    }
}

QList<QAsCodeParser::Symbol>
QAsCodeParser::parseGlobalVarDecls(const QByteArrayList &ns,
                                   const QByteArray &code) {
    _code = code;
    return parseDeclaration(ns, false, true);
}

QAsCodeParser::CodeSegment QAsCodeParser::parseFunctionMethod(Visiblity &vis) {
    CodeSegment seg;

    sToken t1;
    getToken(&t1);

    vis = Visiblity::Public;

    // A class method can start with 'private' or 'protected'
    if (t1.type == ttPrivate) {
        rewindTo(&t1);
        parseToken(ttPrivate);
        getToken(&t1);
        vis = Visiblity::Private;
    } else if (t1.type == ttProtected) {
        rewindTo(&t1);
        parseToken(ttProtected);
        getToken(&t1);
        vis = Visiblity::Protected;
    }

    if (_isSyntaxError)
        return seg;

    // If it is a global function, or a method, except constructor and
    // destructor, then the return type is parsed
    sToken t2;
    getToken(&t2);
    rewindTo(&t1);
    if (t1.type != ttBitNot && t2.type != ttOpenParenthesis) {
        auto id = parseType(true);
        if (_isSyntaxError)
            return seg;
        seg.additonalInfos.append(id);

        parseTypeMod(false);
        if (_isSyntaxError)
            return seg;
    }

    // If this is a class destructor then it starts with ~, and no return type
    // is declared
    if (t1.type == ttBitNot) {
        parseToken(ttBitNot);
        if (_isSyntaxError)
            return seg;
    }

    auto iden = parseIdentifier();
    if (_isSyntaxError)
        return seg;
    seg.name = getSymbolString(iden);
    seg.offset = iden.pos;

    auto params = parseParameterListContent();
    if (_isSyntaxError)
        return seg;

    if (seg.additonalInfos.isEmpty()) {
        seg.additonalInfos.append(QByteArray());
    }
    if (params.isEmpty()) {
        seg.additonalInfos.append(QByteArray());
    } else {
        QByteArrayList args;
        for (auto &p : params) {
            args.append(p.type + ' ' + p.name);
        }
        seg.additonalInfos.append(args.join(", "));
    }

    getToken(&t1);
    rewindTo(&t1);

    // Is the method a const?
    if (t1.type == ttConst)
        parseToken(ttConst);

    // TODO: Should support abstract methods, in which case no statement block
    // should be provided
    parseMethodAttributes();
    if (_isSyntaxError)
        return seg;

    // External shared functions must be ended with ';'
    getToken(&t1);
    rewindTo(&t1);

    seg.scope = currentNs;

    if (t1.type == ttEndStatement) {
        parseToken(ttEndStatement);
        return seg;
    }

    auto begin = t1.pos;

    // We should just find the end of the statement block here.
    t1 = superficiallyParseStatementBlock();
    auto end = t1.pos;
    seg.codes = _code.sliced(begin, end - begin + 1);
    return seg;
}

QAsCodeParser::Symbol
QAsCodeParser::parseFuncDefContent(const QByteArrayList &ns,
                                   const QByteArray &code) {
    reset();
    _code = code;
    return parseFuncDefContent(ns);
}

QPair<QByteArrayList, QList<QAsCodeParser::Symbol>>
QAsCodeParser::parseClassContent(qsizetype offset, const QByteArrayList &ns,
                                 const QByteArray &code) {
    reset();
    _code = code;

    QByteArrayList inhertSyms;
    QList<Symbol> syms;

    sToken t;
    getToken(&t);

    // Optional list of interfaces that are being implemented and classes that
    // are being inherited
    if (t.type == ttColon) {
        QByteArray inhertSym;

        // assuming it as an interface
        inhertSym = parseOptionalScope().join("::");
        if (!inhertSym.isEmpty()) {
            inhertSym += "::";
        }
        inhertSym += getSymbolString(parseIdentifier());
        inhertSyms.append(inhertSym);
        getToken(&t);
        while (t.type == ttListSeparator) {
            inhertSym = parseOptionalScope().join("::");
            if (!inhertSym.isEmpty()) {
                inhertSym += "::";
            }
            inhertSym += getSymbolString(parseIdentifier());
            inhertSyms.append(inhertSym);
            getToken(&t);
        }
    }

    if (t.type != ttStartStatementBlock) {
        rewindErrorTo(&t);
        return {inhertSyms, syms};
    }

    // Parse properties
    getToken(&t);
    rewindTo(&t);
    while (t.type != ttEndStatementBlock && t.type != ttEnd) {
        // Is it a property or a method?
        if (t.type == ttFuncDef) {
            auto fndef = parseFuncDefContent(ns);
            syms.append(fndef);
        } else if (isFuncDecl(true)) {
            Symbol sym;
            auto fn = parseFunctionMethod(sym.vis);

            // add function symbols
            sym.symtype = SymbolType::Function;
            sym.scope = fn.scope;
            sym.offset = fn.offset;
            sym.name = fn.name;
            sym.type = fn.additonalInfos.at(0);
            sym.additonalInfo = fn.additonalInfos.at(1);
            syms.append(sym);

            // deep parsing
            if (offset >= fn.offset && offset < fn.end()) {
                auto ss =
                    parseStatementBlock(fn.scope, fn.codes, offset - fn.offset);
                syms.append(ss);
            }
        } else if (isVirtualPropertyDecl()) {
            auto vp = parseVirtualPropertyDecl(true, false);
            syms.append(vp);
        } else if (isVarDecl()) {
            auto decl = parseDeclaration(ns, true);
            syms.append(decl);
        } else if (t.type == ttEndStatement)
            // Skip empty declarations
            getToken(&t);
        else {
            rewindErrorTo(&t);
            return {inhertSyms, syms};
        }

        if (_isSyntaxError)
            return {inhertSyms, syms};

        getToken(&t);
        rewindTo(&t);
    }

    getToken(&t);
    if (t.type != ttEndStatementBlock) {
        rewindErrorTo(&t);
        return {inhertSyms, syms};
    }

    return {inhertSyms, syms};
}

QPair<QByteArrayList, QList<QAsCodeParser::Symbol>>
QAsCodeParser::parseInterfaceContent(qsizetype offset, const QByteArrayList &ns,
                                     const QByteArray &code) {
    QByteArrayList inhertSyms;
    QList<Symbol> syms;

    sToken t;

    getToken(&t);
    // Can optionally have a list of interfaces that are inherited
    if (t.type == ttColon) {
        QByteArray inhertSym;

        inhertSym = parseOptionalScope().join("::");
        if (!inhertSym.isEmpty()) {
            inhertSym += "::";
        }
        inhertSym += getSymbolString(parseIdentifier());
        inhertSyms.append(inhertSym);
        getToken(&t);
        while (t.type == ttListSeparator) {
            inhertSym = parseOptionalScope().join("::");
            if (!inhertSym.isEmpty()) {
                inhertSym += "::";
            }
            inhertSym += getSymbolString(parseIdentifier());
            inhertSyms.append(inhertSym);
            getToken(&t);
        }
    }

    if (t.type != ttStartStatementBlock) {
        rewindErrorTo(&t);
        return {};
    }

    // Parse interface methods
    getToken(&t);
    rewindTo(&t);
    while (t.type != ttEndStatementBlock && t.type != ttEnd) {
        if (isVirtualPropertyDecl()) {
            auto vp = parseVirtualPropertyDecl(true, true);
            syms.append(vp);
        } else if (t.type == ttEndStatement) {
            // Skip empty declarations
            getToken(&t);
        } else {
            // Parse the method signature
            auto im = parseInterfaceMethod();
            syms.append(im);
        }

        if (_isSyntaxError)
            return {inhertSyms, syms};

        getToken(&t);
        rewindTo(&t);
    }

    getToken(&t);
    if (t.type != ttEndStatementBlock) {
        rewindErrorTo(&t);
        return {inhertSyms, syms};
    }

    return {inhertSyms, syms};
}

QAsCodeParser::Symbol QAsCodeParser::parseInterfaceMethod() {
    Symbol sym;

    auto ret = parseType(true);
    if (_isSyntaxError)
        return sym;
    sym.type = ret;

    parseTypeMod(false);
    if (_isSyntaxError)
        return sym;

    auto id = parseIdentifier();
    if (_isSyntaxError)
        return sym;
    sym.name = getSymbolString(id);
    sym.offset = id.pos;

    auto args = parseParameterListContent();
    if (_isSyntaxError)
        return sym;

    sym.children = args;

    // Parse an optional const after the method definition
    sToken t1;
    getToken(&t1);
    rewindTo(&t1);
    if (t1.type == ttConst)
        parseToken(ttConst);

    getToken(&t1);
    if (t1.type != ttEndStatement) {
        rewindErrorTo(&t1);
        return sym;
    }

    sym.symtype = SymbolType::Function;
    return sym;
}

QAsCodeParser::Symbol
QAsCodeParser::parseVirtualPropertyDecl(bool isMethod, bool isInterface) {
    Symbol sym;

    sToken t1, t2;
    getToken(&t1);
    getToken(&t2);
    rewindTo(&t1);

    // A class method can start with 'private' or 'protected'
    if (isMethod && t1.type == ttPrivate) {
        parseToken(ttPrivate);
        sym.vis = Visiblity::Private;
    } else if (isMethod && t1.type == ttProtected) {
        parseToken(ttProtected);
        sym.vis = Visiblity::Protected;
    }

    if (_isSyntaxError)
        return sym;

    auto id = parseType(true);
    if (_isSyntaxError)
        return sym;
    sym.type = id;

    parseTypeMod(false);
    if (_isSyntaxError)
        return sym;

    auto iden = parseIdentifier();
    if (_isSyntaxError)
        return sym;
    sym.name = getSymbolString(iden);
    sym.offset = iden.pos;

    getToken(&t1);
    if (t1.type != ttStartStatementBlock) {
        rewindErrorTo(&t1);
        return sym;
    }

    for (;;) {
        getToken(&t1);

        CodeSegment seg;

        if (identifierIs(t1, GET_TOKEN) || identifierIs(t1, SET_TOKEN)) {
            rewindTo(&t1);

            auto id = parseIdentifier();
            auto name = getSymbolString(id);
            seg.name = name;

            if (isMethod) {
                getToken(&t1);
                rewindTo(&t1);
                if (t1.type == ttConst)
                    parseToken(ttConst);

                if (!isInterface) {
                    parseMethodAttributes();
                    if (_isSyntaxError)
                        return sym;
                }
            }

            if (!isInterface) {
                getToken(&t1);
                if (t1.type == ttStartStatementBlock) {
                    rewindTo(&t1);
                    superficiallyParseStatementBlock();
                    // TODO: support deep parsing ?
                    if (_isSyntaxError)
                        return sym;
                } else if (t1.type == ttEndStatement) {
                    rewindTo(&t1);
                    parseToken(ttEndStatement);
                    if (_isSyntaxError)
                        return sym;
                } else {
                    rewindErrorTo(&t1);
                    return sym;
                }
            } else {
                getToken(&t1);
                if (t1.type == ttEndStatement) {
                    rewindTo(&t1);
                    parseToken(ttEndStatement);
                    if (_isSyntaxError)
                        return sym;
                } else {
                    rewindErrorTo(&t1);
                    return sym;
                }
            }
        } else if (t1.type == ttEndStatementBlock)
            break;
        else {
            rewindErrorTo(&t1);
            return sym;
        }
    }

    sym.symtype = SymbolType::Variable;
    return sym;
}

QList<QAsCodeParser::Symbol> QAsCodeParser::parseEnumerationContent(
    const QByteArrayList &ns, const QByteArray &name, const QByteArray &code) {
    reset();
    _code = code;

    QList<QAsCodeParser::Symbol> syms;
    sToken token;
    size_t eoff;

    getToken(&token);
    if (token.type == ttEndStatement) {
        rewindTo(&token);
        parseToken(ttEndStatement);
        return syms;
    }

    // check for the start of the declaration block
    if (token.type != ttStartStatementBlock) {
        rewindTo(&token);
        rewindErrorTo(&token);
        return syms;
    }

    while (ttEnd != token.type) {
        getToken(&token);

        if (ttEndStatementBlock == token.type) {
            rewindTo(&token);
            break;
        }

        if (ttIdentifier != token.type) {
            rewindErrorTo(&token);
            return syms;
        }

        // Add the enum element
        Symbol se;
        se.type = "int";
        se.symtype = SymbolType::Enum;
        se.name = getSymbolString(token);
        se.scope = ns;
        se.scope.append(name);
        syms.append(se);

        if (token.type == ttAssignment) {
            rewindTo(&token);

            superficiallyParseVarInit();

            if (_isSyntaxError) {
                return syms;
            }

            // se.additonalInfo = getSymbolString(t);
        }

        getToken(&token);
        if (ttListSeparator != token.type) {
            rewindTo(&token);
            break;
        }
    }

    // check for the end of the declaration block
    getToken(&token);
    if (token.type != ttEndStatementBlock) {
        rewindTo(&token);
        rewindErrorTo(&token);
        return syms;
    }

    return syms;
}

QAsCodeParser::Symbol
QAsCodeParser::parseTypedefContent(const QByteArrayList &ns,
                                   const QByteArray &code) {
    reset();
    _code = code;

    QAsCodeParser::Symbol sym;
    sToken token;

    getToken(&token);
    auto begin = token.length;
    if (token.type != ttTypedef) {
        rewindErrorTo(&token);
        return sym;
    }

    // Parse the base type
    getToken(&token);
    rewindTo(&token);

    // Make sure it is a primitive type (except ttVoid)
    if (!isPrimType(token.type) || token.type == ttVoid) {
        rewindErrorTo(&token);
        return sym;
    }

    auto r = parseRealType();
    auto i = parseIdentifier();

    // Check for the end of the typedef
    getToken(&token);
    if (token.type != ttEndStatement) {
        rewindTo(&token);
        rewindErrorTo(&token);
    }

    sym.name = getSymbolString(i);
    sym.type = getSymbolString(r);
    sym.symtype = SymbolType::TypeDef;
    sym.offset = begin;
    sym.scope = currentNs;

    return sym;
}

bool QAsCodeParser::isVarDecl() {
    // Set start point so that we can rewind
    sToken t;
    getToken(&t);
    rewindTo(&t);

    // A class property decl can be preceded by 'private' or 'protected'
    sToken t1;
    getToken(&t1);
    if (t1.type != ttPrivate && t1.type != ttProtected)
        rewindTo(&t1);

    // A variable decl starts with the type
    if (!findTokenAfterType(t1)) {
        rewindTo(&t);
        return false;
    }

    // Jump to the token after the type
    rewindTo(&t1);
    getToken(&t1);

    // The declaration needs to have a name
    if (t1.type != ttIdentifier) {
        rewindTo(&t);
        return false;
    }

    // It can be followed by an initialization
    getToken(&t1);
    if (t1.type == ttEndStatement || t1.type == ttAssignment ||
        t1.type == ttListSeparator) {
        rewindTo(&t);
        return true;
    }
    if (t1.type == ttOpenParenthesis) {
        // If the closing parenthesis is followed by a statement block,
        // function decorator, or end-of-file, then treat it as a function.
        // A function decl may have nested parenthesis so we need to check
        // for this too.
        int nest = 0;
        while (t1.type != ttEnd) {
            if (t1.type == ttOpenParenthesis)
                nest++;
            else if (t1.type == ttCloseParenthesis) {
                nest--;
                if (nest == 0)
                    break;
            }
            getToken(&t1);
        }

        if (t1.type == ttEnd) {
            rewindTo(&t);
            return false;
        } else {
            getToken(&t1);
            rewindTo(&t);
            if (t1.type == ttStartStatementBlock ||
                t1.type == ttIdentifier || // function decorator
                t1.type == ttEnd)
                return false;
        }

        rewindTo(&t);
        return true;
    }

    rewindTo(&t);
    return false;
}

bool QAsCodeParser::isVirtualPropertyDecl() {
    // Set start point so that we can rewind
    sToken t;
    getToken(&t);
    rewindTo(&t);

    // A class property decl can be preceded by 'private' or 'protected'
    sToken t1;
    getToken(&t1);
    if (t1.type != ttPrivate && t1.type != ttProtected)
        rewindTo(&t1);

    // A variable decl starts with the type
    if (!findTokenAfterType(t1)) {
        rewindTo(&t);
        return false;
    }

    // Move to the token after the type
    rewindTo(&t1);
    getToken(&t1);

    // The decl must have an identifier
    if (t1.type != ttIdentifier) {
        rewindTo(&t);
        return false;
    }

    // To be a virtual property it must also have a block for the get/set
    // functions
    getToken(&t1);
    if (t1.type == ttStartStatementBlock) {
        rewindTo(&t);
        return true;
    }

    rewindTo(&t);
    return false;
}

bool QAsCodeParser::isFuncDecl(
    bool isMethod) { // Set start point so that we can rewind
    sToken t;
    getToken(&t);
    rewindTo(&t);

    if (isMethod) {
        // A class method decl can be preceded by 'private' or 'protected'
        sToken t1, t2;
        getToken(&t1);
        if (t1.type != ttPrivate && t1.type != ttProtected)
            rewindTo(&t1);

        // A class constructor starts with identifier followed by parenthesis
        // A class destructor starts with the ~ token
        getToken(&t1);
        getToken(&t2);
        rewindTo(&t1);
        if ((t1.type == ttIdentifier && t2.type == ttOpenParenthesis) ||
            t1.type == ttBitNot) {
            rewindTo(&t);
            return true;
        }
    }

    // A function decl starts with a type
    sToken t1;
    if (!findTokenAfterType(t1)) {
        rewindTo(&t);
        return false;
    }

    // Move to the token after the type
    rewindTo(&t1);
    getToken(&t1);

    // There can be an ampersand if the function returns a reference
    if (t1.type == ttAmp) {
        rewindTo(&t);
        return true;
    }

    if (t1.type != ttIdentifier) {
        rewindTo(&t);
        return false;
    }

    getToken(&t1);
    if (t1.type == ttOpenParenthesis) {
        // If the closing parenthesis is not followed by a
        // statement block then it is not a function.
        // It's possible that there are nested parenthesis due to default
        // arguments so this should be checked for.
        int nest = 0;
        getToken(&t1);
        while ((nest || t1.type != ttCloseParenthesis) && t1.type != ttEnd) {
            if (t1.type == ttOpenParenthesis)
                nest++;
            if (t1.type == ttCloseParenthesis)
                nest--;

            getToken(&t1);
        }

        if (t1.type == ttEnd)
            return false;
        else {
            if (isMethod) {
                // A class method can have a 'const' token after the parameter
                // list
                getToken(&t1);
                if (t1.type != ttConst)
                    rewindTo(&t1);
            }

            // A function may also have any number of additional attributes
            bool hasAttribs = false;
            for (;;) {
                getToken(&t1);
                if (!identifierIs(t1, FINAL_TOKEN) &&
                    !identifierIs(t1, OVERRIDE_TOKEN) &&
                    !identifierIs(t1, EXPLICIT_TOKEN) &&
                    !identifierIs(t1, PROPERTY_TOKEN) &&
                    !identifierIs(t1, DELETE_TOKEN)) {
                    rewindTo(&t1);
                    break;
                }
                hasAttribs = true;
            }

            getToken(&t1);
            rewindTo(&t);

            // If the function has an attribute, e.g. delete, it can be
            // terminated with a ; if no implementation is expected otherwise it
            // must have a statement block
            if ((t1.type == ttEndStatement && hasAttribs) ||
                t1.type == ttStartStatementBlock)
                return true;
        }

        rewindTo(&t);
        return false;
    }

    rewindTo(&t);
    return false;
}

bool QAsCodeParser::findTokenAfterType(
    sToken &nextToken) { // Set a rewind point
    sToken t, t1;
    getToken(&t);

    // A type can start with a const
    t1 = t;
    if (t1.type == ttConst)
        getToken(&t1);

    sToken t2;
    if (t1.type != ttAuto) {
        // The type may be initiated with the scope operator
        if (t1.type == ttScope)
            getToken(&t1);

        // The type may be preceded with a multilevel scope
        getToken(&t2);
        while (t1.type == ttIdentifier) {
            if (t2.type == ttScope) {
                getToken(&t1);
                getToken(&t2);
                continue;
            } else if (t2.type == ttLessThan) {
                // Template types can also be used as scope identifiers
                rewindTo(&t2);
                if (checkTemplateType(t1)) {
                    sToken t3;
                    getToken(&t3);
                    if (t3.type == ttScope) {
                        getToken(&t1);
                        getToken(&t2);
                        continue;
                    }
                }
            }

            break;
        }
        rewindTo(&t2);
    }

    // We don't validate if the identifier is an actual declared type at this
    // moment as it may wrongly identify the statement as a non-declaration if
    // the user typed the name incorrectly. The real type is validated in
    // ParseDeclaration where a proper error message can be given.
    if (!isPrimType(t1.type) && t1.type != ttIdentifier && t1.type != ttAuto) {
        rewindTo(&t);
        return false;
    }

    if (!checkTemplateType(t1)) {
        rewindTo(&t);
        return false;
    }

    // Object handles can be interleaved with the array brackets
    // Even though declaring variables with & is invalid we'll accept
    // it here to give an appropriate error message later
    getToken(&t2);
    while (t2.type == ttHandle || t2.type == ttAmp ||
           t2.type == ttOpenBracket) {
        if (t2.type == ttHandle) {
            // A handle can optionally be read-only
            sToken t3;
            getToken(&t3);
            if (t3.type != ttConst)
                rewindTo(&t3);
        } else if (t2.type == ttOpenBracket) {
            getToken(&t2);
            if (t2.type != ttCloseBracket) {
                rewindTo(&t);
                return false;
            }
        } else if (t2.type == ttAmp) {
            // & can be followed by in, out, or inout
            sToken t3;
            getToken(&t3);
            if (t3.type != ttIn && t3.type != ttOut && t3.type != ttInOut)
                rewindTo(&t3);
        }

        getToken(&t2);
    }

    // Return the next token so the caller can jump directly to it if desired
    nextToken = t2;

    // Rewind to start point
    rewindTo(&t);

    return true;
}

bool QAsCodeParser::isConstant(int tokenType) {
    if (tokenType == ttIntConstant || tokenType == ttFloatConstant ||
        tokenType == ttDoubleConstant || tokenType == ttStringConstant ||
        tokenType == ttMultilineStringConstant ||
        tokenType == ttHeredocStringConstant || tokenType == ttTrue ||
        tokenType == ttFalse || tokenType == ttBitsConstant ||
        tokenType == ttNull)
        return true;

    return false;
}

bool QAsCodeParser::isOperator(int tokenType) {
    if (tokenType == ttPlus || tokenType == ttMinus || tokenType == ttStar ||
        tokenType == ttSlash || tokenType == ttPercent ||
        tokenType == ttStarStar || tokenType == ttAnd || tokenType == ttOr ||
        tokenType == ttXor || tokenType == ttEqual || tokenType == ttNotEqual ||
        tokenType == ttLessThan || tokenType == ttLessThanOrEqual ||
        tokenType == ttGreaterThan || tokenType == ttGreaterThanOrEqual ||
        tokenType == ttAmp || tokenType == ttBitOr || tokenType == ttBitXor ||
        tokenType == ttBitShiftLeft || tokenType == ttBitShiftRight ||
        tokenType == ttBitShiftRightArith || tokenType == ttIs ||
        tokenType == ttNotIs)
        return true;

    return false;
}

bool QAsCodeParser::isPreOperator(int tokenType) {
    if (tokenType == ttMinus || tokenType == ttPlus || tokenType == ttNot ||
        tokenType == ttInc || tokenType == ttDec || tokenType == ttBitNot ||
        tokenType == ttHandle)
        return true;
    return false;
}

bool QAsCodeParser::isPostOperator(int tokenType) {
    if (tokenType == ttInc ||         // post increment
        tokenType == ttDec ||         // post decrement
        tokenType == ttDot ||         // member access
        tokenType == ttOpenBracket || // index operator
        tokenType ==
            ttOpenParenthesis) // argument list for call on function pointer
        return true;
    return false;
}

bool QAsCodeParser::isAssignOperator(int tokenType) {
    if (tokenType == ttAssignment || tokenType == ttAddAssign ||
        tokenType == ttSubAssign || tokenType == ttMulAssign ||
        tokenType == ttDivAssign || tokenType == ttModAssign ||
        tokenType == ttPowAssign || tokenType == ttAndAssign ||
        tokenType == ttOrAssign || tokenType == ttXorAssign ||
        tokenType == ttShiftLeftAssign || tokenType == ttShiftRightLAssign ||
        tokenType == ttShiftRightAAssign)
        return true;

    return false;
}

bool QAsCodeParser::typeExist(const QString &t) {
    Q_UNUSED(t);
    // TODO: don't check
    return true;
}

bool QAsCodeParser::checkTemplateType(const sToken &t) {
    // Is this a template type?
    auto type = getSymbolString(t);
    if (engine->IsTemplateType(type)) {
        // If the next token is a < then parse the sub-type too
        sToken t1;
        getToken(&t1);
        if (t1.type != ttLessThan) {
            rewindTo(&t1);
            return true;
        }

        for (;;) {
            // There might optionally be a 'const'
            getToken(&t1);
            if (t1.type == ttConst)
                getToken(&t1);

            // The type may be initiated with the scope operator
            if (t1.type == ttScope)
                getToken(&t1);

            // There may be multiple levels of scope operators
            sToken t2;
            getToken(&t2);
            while (t1.type == ttIdentifier && t2.type == ttScope) {
                getToken(&t1);
                getToken(&t2);
            }
            rewindTo(&t2);

            // Now there must be a data type
            if (!isDataType(t1))
                return false;

            if (!checkTemplateType(t1))
                return false;

            getToken(&t1);

            // Is it a handle or array?
            while (t1.type == ttHandle || t1.type == ttOpenBracket) {
                if (t1.type == ttOpenBracket) {
                    getToken(&t1);
                    if (t1.type != ttCloseBracket)
                        return false;
                } else if (t1.type == ttHandle) {
                    // after @ there can be a const
                    getToken(&t1);
                    if (t1.type != ttConst)
                        rewindTo(&t1);
                }

                getToken(&t1);
            }

            // Was this the last template subtype?
            if (t1.type != ttListSeparator)
                break;
        }

        // Accept >> and >>> tokens too. But then force the tokenizer to move
        // only 1 character ahead (thus splitting the token in two).
        if (_code[QString::size_type(t1.pos)] != '>')
            return false;
        else if (t1.length != 1) {
            // We need to break the token, so that only the first character is
            // parsed
            setPos(t1.pos + 1);
        }
    }

    return true;
}

sToken QAsCodeParser::parseToken(int token) {
    sToken t1;

    getToken(&t1);
    if (t1.type != token) {
        rewindErrorTo(&t1);
    }

    return t1;
}

sToken QAsCodeParser::parseOneOf(int *tokens, int count) {
    sToken t1;

    getToken(&t1);
    int n;
    for (n = 0; n < count; n++) {
        if (tokens[n] == t1.type)
            break;
    }
    if (n == count) {
        rewindErrorTo(&t1);
        return t1;
    }

    return t1;
}
