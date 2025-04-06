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

#include "AngelScript/sdk/angelscript/source/as_scriptengine.h"
#include "AngelScript/sdk/angelscript/source/as_tokenizer.h"

/* Some information about AngelScript and this class:
 * * This class is designed to get variables and functions symbol.
 * * Strings are also included.
 * * AngelScript's class and enum can not be nested unlike C++.
 * * We only support class / mixin class / interface / function / enum.
 */

QAsCodeParser::QAsCodeParser(asCScriptEngine *engine) : engine(engine) {
    Q_ASSERT(engine);
    checkValidTypes = false;
    isParsingAppInterface = false;
}

QAsCodeParser::QAsCodeParser(asIScriptEngine *engine) {
    this->engine = dynamic_cast<asCScriptEngine *>(engine);
    Q_ASSERT(engine);
    checkValidTypes = false;
    isParsingAppInterface = false;
}

QAsCodeParser::~QAsCodeParser() {}

QList<QAsCodeParser::CodeSegment>
QAsCodeParser::parse(const QByteArray &codes) {
    reset();
    code = codes;
    return parseScript(false);
}

QList<QAsCodeParser::Symbol>
QAsCodeParser::parseAndIntell(qsizetype offset, const QByteArray &codes) {
    return parseIntell(offset, parse(codes));
}

QList<QAsCodeParser::Symbol>
QAsCodeParser::parseIntell(qsizetype offset,
                           const QList<QAsCodeParser::CodeSegment> &segs) {

    QList<Symbol> ret;

    // first: global entries
    for (auto &seg : segs) {
        Symbol sym;
        sym.symtype = seg.type;
        sym.scope = seg.scope;
        sym.offset = seg.offset;
        sym.name = seg.name;

        switch (seg.type) {
        case SymbolType::Function: {
            sym.type = seg.additonalInfos.at(0);
            sym.additonalInfo = seg.additonalInfos.at(1);
            sym.name = seg.name;
        } break;
        case SymbolType::Enum:
            break;
        case SymbolType::Variable:
        case SymbolType::Class:
        case SymbolType::TypeDef:
        case SymbolType::FnDef:
        case SymbolType::VarsDecl:
        case SymbolType::Invalid:
            continue;
        }

        ret.append(sym);
    }

    // TODO: deep parsing with offset

    return ret;
}

void QAsCodeParser::reset() {
    errorWhileParsing = false;
    isSyntaxError = false;
    checkValidTypes = false;
    isParsingAppInterface = false;

    sourcePos = 0;

    lastToken.pos = size_t(-1);
}

void QAsCodeParser::getToken(sToken *token) {
    // Check if the token has already been parsed
    if (lastToken.pos == sourcePos) {
        *token = lastToken;
        sourcePos += token->length;

        if (token->type == ttWhiteSpace || token->type == ttOnelineComment ||
            token->type == ttMultilineComment)
            getToken(token);

        return;
    }

    // Parse new token
    size_t sourceLength = code.length();
    do {
        if (sourcePos >= sourceLength) {
            token->type = ttEnd;
            token->length = 0;
        } else
            token->type =
                engine->tok.GetToken(code.data() + sourcePos,
                                     sourceLength - sourcePos, &token->length);

        token->pos = sourcePos;

        // Update state
        sourcePos += token->length;
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
    lastToken = *token;

    sourcePos = token->pos;
}

void QAsCodeParser::SetPos(size_t pos) {
    lastToken.pos = size_t(-1);
    sourcePos = pos;
}

void QAsCodeParser::rewindErrorTo(sToken *token) {
    rewindTo(token);

    isSyntaxError = true;
    errorWhileParsing = true;
}

QAsCodeParser::Symbol QAsCodeParser::parseFunctionDefinition() {
    auto ret = parseType(true);
    if (isSyntaxError)
        return {};

    ParseTypeMod(false);
    if (isSyntaxError)
        return {};

    auto ns = ParseOptionalScope();

    auto id = parseIdentifier();
    if (isSyntaxError)
        return {};

    auto params = parseParameterListContent();
    if (isSyntaxError)
        return {};

    // ok, add symbol
    Symbol se;
    // se.type = SymbolType::FnDecl;
    // se.typeStr = getSymbolString(id);
    // se.name = getSymbolString(id);
    // se.nameInSrc = id.pos;
    // se.content = params;
    // se.ns = getRealNamespace(ns);

    // Parse an optional 'const' after the function definition (used for
    // object methods)
    sToken t1;
    getToken(&t1);
    rewindTo(&t1);
    if (t1.type == ttConst)
        parseToken(ttConst);

    // Parse optional attributes
    parseMethodAttributes();

    return se;
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
            auto t = parseType(true, isParsingAppInterface);
            if (isSyntaxError)
                return ret;

            ParseTypeMod(true);
            if (isSyntaxError)
                return ret;

            // Parse optional identifier
            getToken(&t1);
            if (t1.type == ttIdentifier) {
                rewindTo(&t1);

                auto iden = parseIdentifier();
                Symbol se;
                se.name = getSymbolString(iden);
                // se.type = SymbolType::Variable;
                // se.typeStr = getSymbolString(t);
                ret.append(se);

                if (isSyntaxError)
                    return ret;

                getToken(&t1);
            }

            // Parse optional expression for the default arg
            if (t1.type == ttAssignment) {
                // Do a superficial parsing of the default argument
                // The actual parsing will be done when the argument is compiled
                // for a function call
                superficiallyParseExpression();
                if (isSyntaxError)
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

sToken QAsCodeParser::parseType(bool allowConst, bool allowVariableType,
                                bool allowAuto) {
    sToken t;

    if (allowConst) {
        getToken(&t);
        rewindTo(&t);
        if (t.type == ttConst) {
            parseToken(ttConst);
            if (isSyntaxError)
                return t;
        }
    }

    // Parse scope prefix
    ParseOptionalScope();

    // Parse the actual type
    auto dt = parseDataType(allowVariableType, allowAuto);
    if (isSyntaxError)
        return t;

    // If the datatype is a template type, then parse the subtype within the < >
    getToken(&t);
    rewindTo(&t);

    tempString = getSymbolString(dt);
    if (engine->IsTemplateType(tempString) && t.type == ttLessThan) {
        parseTemplTypeList();
        if (isSyntaxError)
            return t;
    }

    // Parse [] and @
    getToken(&t);
    rewindTo(&t);
    while (t.type == ttOpenBracket || t.type == ttHandle) {
        if (t.type == ttOpenBracket) {
            parseToken(ttOpenBracket);
            if (isSyntaxError)
                return t;

            getToken(&t);
            if (t.type != ttCloseBracket) {
                rewindErrorTo(&t);
                return t;
            }
        } else {
            parseToken(ttHandle);
            if (isSyntaxError)
                return t;

            getToken(&t);
            rewindTo(&t);
            if (t.type == ttConst) {
                parseToken(ttConst);
                if (isSyntaxError)
                    return t;
            }
        }

        getToken(&t);
        rewindTo(&t);
    }

    return dt;
}

void QAsCodeParser::ParseTypeMod(bool isParam) {
    sToken t;

    // Parse possible & token
    getToken(&t);
    rewindTo(&t);
    if (t.type == ttAmp) {
        parseToken(ttAmp);
        if (isSyntaxError)
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
        if (isSyntaxError)
            return;
    }

    // Parse possible if_handle_then_const token
    getToken(&t);
    rewindTo(&t);
    if (identifierIs(t, IF_HANDLE_TOKEN)) {
        parseToken(ttIdentifier);
        if (isSyntaxError)
            return;
    }
}

QByteArrayList QAsCodeParser::ParseOptionalScope() {
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
        tempString = getSymbolString(t1);
        if (engine->IsTemplateType(tempString)) {
            rewindTo(&t1);
            parseIdentifier();
            if (parseTemplTypeList(false)) {
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
    }

    // The identifier is not part of the scope
    rewindTo(&t1);

    return scopes;
}

sToken QAsCodeParser::parseRealType() {
    sToken t1;

    getToken(&t1);
    if (!isRealType(t1.type)) {
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

bool QAsCodeParser::parseTemplTypeList(bool required) {
    sToken t;
    bool isValid = true;

    // Remember the last child, so we can restore the state if needed
    // asCScriptNode *last = node->lastChild;

    // Starts with '<'
    getToken(&t);
    if (t.type != ttLessThan) {
        if (required) {
            rewindErrorTo(&t);
        }
        return false;
    }

    // At least one type
    // TODO: child funcdef: Make this work with !required
    parseType(true, false);
    if (isSyntaxError)
        return false;

    getToken(&t);

    // Parse template types by list separator
    while (t.type == ttListSeparator) {
        // TODO: child funcdef: Make this work with !required
        parseType(true, false);
        if (isSyntaxError)
            return false;
        getToken(&t);
    }

    // End with '>'
    // Accept >> and >>> tokens too. But then force the tokenizer to move
    // only 1 character ahead (thus splitting the token in two).
    if (code[QString::size_type(t.pos)] != '>') {
        if (required) {
            rewindErrorTo(&t);
        } else
            isValid = false;
    } else {
        // Break the token so that only the first > is parsed
        SetPos(t.pos + 1);
    }

    if (!required && !isValid) {
        return false;
    }

    // The template type list was parsed OK
    return true;
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

void QAsCodeParser::ParseListPattern() {
    sToken t1;

    getToken(&t1);
    if (t1.type != ttStartStatementBlock) {
        rewindErrorTo(&t1);
        return;
    }

    sToken start = t1;

    bool isBeginning = true;
    bool afterType = false;
    while (!isSyntaxError) {
        getToken(&t1);
        if (t1.type == ttEndStatementBlock) {
            if (!afterType) {
                rewindErrorTo(&t1);
            }
            break;
        } else if (t1.type == ttStartStatementBlock) {
            if (afterType) {
                rewindErrorTo(&t1);
            }
            rewindTo(&t1);
            ParseListPattern();
            afterType = true;
        } else if (t1.type == ttIdentifier &&
                   (identifierIs(t1, "repeat") ||
                    identifierIs(t1, "repeat_same"))) {
            if (!isBeginning) {
                rewindErrorTo(&t1);
            }
            rewindTo(&t1);
            parseIdentifier();
        } else if (t1.type == ttEnd) {
            rewindErrorTo(&start);
            break;
        } else if (t1.type == ttListSeparator) {
            if (!afterType) {
                rewindErrorTo(&t1);
            }
            afterType = false;
        } else {
            if (afterType) {
                rewindErrorTo(&t1);
            }
            rewindTo(&t1);
            // NOTE
            parseType(true, true);
            afterType = true;
        }

        isBeginning = false;
    }
}

bool QAsCodeParser::isRealType(int tokenType) {
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
        if (checkValidTypes) {
            // Check if this is an existing type, regardless of namespace
            tempString = getSymbolString(token);
            if (!typeExist(tempString))
                return false;
        }
#endif
        return true;
    }

    if (isRealType(token.type))
        return true;

    return false;
}

bool QAsCodeParser::identifierIs(const sToken &t, const char *str) {
    if (t.type != ttIdentifier)
        return false;

    return code.sliced(t.pos, t.length) == QByteArray(str);
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
    while (level > 0 && !isSyntaxError) {
        getToken(&t1);
        if (t1.type == ttEndStatementBlock)
            level--;
        else if (t1.type == ttStartStatementBlock)
            level++;
        else if (t1.type == ttNonTerminatedStringConstant) {
            rewindErrorTo(&t1);
            return t1;
        } else if (t1.type == ttEnd) {
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
            if (isSyntaxError)
                return seg;
        } else
            break;

        getToken(&t1);
    }

    if (isSyntaxError)
        return seg;

    // If it is a global function, or a method, except constructor and
    // destructor, then the return type is parsed
    sToken t2;
    getToken(&t2);
    rewindTo(&t1);

    QByteArray rettype;

    if (t1.type != ttBitNot && t2.type != ttOpenParenthesis) {
        auto id = parseType(true);
        if (isSyntaxError)
            return seg;
        rettype = getSymbolString(id);

        ParseTypeMod(false);
        if (isSyntaxError)
            return seg;
    } else {
        rewindErrorTo(&t1);
        return seg;
    }

    // the first is returning value type
    seg.additonalInfos.append(rettype);

    auto iden = parseIdentifier();
    if (isSyntaxError)
        return seg;
    seg.name = getSymbolString(iden);
    seg.offset = iden.pos;

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
        seg.additonalInfos.append(code.sliced(begin, end - begin));
    }

    // TODO: Should support abstract methods, in which case no statement block
    // should be provided
    parseMethodAttributes();
    if (isSyntaxError)
        return seg;

    // External shared functions must be ended with ';'
    getToken(&t1);
    rewindTo(&t1);

    seg.scope = currentNs;
    seg.type = SymbolType::Function;

    if (t1.type == ttEndStatement) {
        parseToken(ttEndStatement);
        return seg;
    }

    auto begin = t1.pos;
    // We should just find the end of the statement block here.
    t1 = superficiallyParseStatementBlock();
    auto end = t1.pos;
    seg.codes = code.sliced(begin, end - begin + 1);
    return seg;
}

QAsCodeParser::CodeSegment QAsCodeParser::parseFuncDef() {
    CodeSegment seg;
    sToken t1;

    // Allow keywords 'external' and 'shared' before 'interface'
    getToken(&t1);
    while (identifierIs(t1, SHARED_TOKEN) || identifierIs(t1, EXTERNAL_TOKEN)) {
        rewindTo(&t1);

        if (isSyntaxError)
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
    auto end = t1.pos;

    // seg.name is empty
    seg.scope = currentNs;
    seg.offset = begin;
    seg.type = SymbolType::FnDef;
    seg.codes = code.sliced(begin, end - begin + 1);
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

        if (isSyntaxError)
            return seg;

        getToken(&t);
    }

    if (t.type != ttInterface) {
        rewindErrorTo(&t);
        return seg;
    }

    auto id = parseIdentifier();
    seg.name = getSymbolString(id);
    seg.offset = id.pos;
    seg.scope = currentNs;
    seg.type = SymbolType::Class;

    // External shared declarations are ended with ';'
    getToken(&t);
    if (t.type == ttEndStatement) {
        rewindTo(&t);
        parseToken(ttEndStatement);
        return seg;
    }

    auto begin = t.pos;
    rewindTo(&t);
    t = superficiallyParseStatementBlock();
    auto end = t.pos;
    seg.codes = code.sliced(begin, end - begin + 1);

    return seg;
}

sToken QAsCodeParser::superficiallyParseVarInit() {
    sToken t;
    getToken(&t);

    if (t.type == ttAssignment) {
        getToken(&t);
        sToken start = t;

        // Find the end of the expression
        int indentParan = 0;
        int indentBrace = 0;
        while (indentParan || indentBrace ||
               (t.type != ttListSeparator && t.type != ttEndStatement &&
                t.type != ttEndStatementBlock)) {
            if (t.type == ttOpenParenthesis)
                indentParan++;
            else if (t.type == ttCloseParenthesis)
                indentParan--;
            else if (t.type == ttStartStatementBlock)
                indentBrace++;
            else if (t.type == ttEndStatementBlock)
                indentBrace--;
            else if (t.type == ttNonTerminatedStringConstant) {
                rewindErrorTo(&t);
                break;
            } else if (t.type == ttEnd) {
                rewindErrorTo(&start);
                break;
            }
            getToken(&t);
        }

        // Rewind so that the next token read is the list separator, end
        // statement, or end statement block
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

    return t;
}

void QAsCodeParser::parseStatementBlock() {
    sToken t1;

    getToken(&t1);
    if (t1.type != ttStartStatementBlock) {
        rewindErrorTo(&t1);
        return;
    }

    sToken start = t1;

    for (;;) {
        while (!isSyntaxError) {
            getToken(&t1);
            if (t1.type == ttEndStatementBlock) {

                // Statement block is finished
                return;
            } else {
                rewindTo(&t1);

                if (isVarDecl())
                    parseDeclaration();
                else
                    parseStatement();
            }
        }

        if (isSyntaxError) {
            // Search for either ';', '{', '}', or end
            getToken(&t1);
            while (t1.type != ttEndStatement && t1.type != ttEnd &&
                   t1.type != ttStartStatementBlock &&
                   t1.type != ttEndStatementBlock) {
                getToken(&t1);
            }

            // Skip this statement block
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
            } else if (t1.type == ttEndStatementBlock) {
                rewindTo(&t1);
            } else if (t1.type == ttEnd) {
                rewindErrorTo(&start);
                return;
            }

            isSyntaxError = false;
        }
    }
    UNREACHABLE_RETURN;
}

void QAsCodeParser::parseStatement() {
    sToken t1;

    getToken(&t1);
    rewindTo(&t1);

    if (t1.type == ttIf)
        ParseIf();
    else if (t1.type == ttFor)
        ParseFor();
    else if (t1.type == ttWhile)
        ParseWhile();
    else if (t1.type == ttReturn)
        ParseReturn();
    else if (t1.type == ttStartStatementBlock)
        parseStatementBlock();
    else if (t1.type == ttBreak)
        ParseBreak();
    else if (t1.type == ttContinue)
        ParseContinue();
    else if (t1.type == ttDo)
        ParseDoWhile();
    else if (t1.type == ttSwitch)
        ParseSwitch();
    else if (t1.type == ttTry)
        ParseTryCatch();
    else {
        if (isVarDecl()) {
            rewindErrorTo(&t1);
            return;
        }
        ParseExpressionStatement();
    }
}

void QAsCodeParser::ParseExpressionStatement() {
    sToken t;
    getToken(&t);
    if (t.type == ttEndStatement) {
        return;
    }

    rewindTo(&t);

    ParseAssignment();
    if (isSyntaxError)
        return;

    getToken(&t);
    if (t.type != ttEndStatement) {
        rewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseSwitch() {
    sToken t;
    getToken(&t);
    if (t.type != ttSwitch) {
        rewindErrorTo(&t);
        return;
    }

    getToken(&t);
    if (t.type != ttOpenParenthesis) {
        rewindErrorTo(&t);
        return;
    }

    ParseAssignment();
    if (isSyntaxError)
        return;

    getToken(&t);
    if (t.type != ttCloseParenthesis) {
        rewindErrorTo(&t);
        return;
    }

    getToken(&t);
    if (t.type != ttStartStatementBlock) {
        rewindErrorTo(&t);
        return;
    }

    while (!isSyntaxError) {
        getToken(&t);

        if (t.type == ttEndStatementBlock)
            break;

        rewindTo(&t);

        if (t.type != ttCase && t.type != ttDefault) {
            rewindErrorTo(&t);
            return;
        }

        ParseCase();
        if (isSyntaxError)
            return;
    }

    if (t.type != ttEndStatementBlock) {
        rewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseCase() {
    sToken t;
    getToken(&t);
    if (t.type != ttCase && t.type != ttDefault) {
        rewindErrorTo(&t);
        return;
    }

    if (t.type == ttCase) {
        ParseExpression();
    }

    getToken(&t);
    if (t.type != ttColon) {
        rewindErrorTo(&t);
        return;
    }

    // Parse statements until we find either of }, case, default, and break
    getToken(&t);
    rewindTo(&t);
    while (t.type != ttCase && t.type != ttDefault &&
           t.type != ttEndStatementBlock && t.type != ttBreak) {
        if (isVarDecl())
            // Variable declarations are not allowed, but we parse it anyway to
            // give a good error message
            parseDeclaration();
        else
            parseStatement();
        if (isSyntaxError)
            return;

        getToken(&t);
        rewindTo(&t);
    }

    // If the case was ended with a break statement, add it to the node
    if (t.type == ttBreak)
        ParseBreak();
}

void QAsCodeParser::ParseIf() {
    sToken t;
    getToken(&t);
    if (t.type != ttIf) {
        rewindErrorTo(&t);
        return;
    }

    getToken(&t);
    if (t.type != ttOpenParenthesis) {
        rewindErrorTo(&t);
        return;
    }

    ParseAssignment();
    if (isSyntaxError)
        return;

    getToken(&t);
    if (t.type != ttCloseParenthesis) {
        rewindErrorTo(&t);
        return;
    }

    parseStatement();
    if (isSyntaxError)
        return;

    getToken(&t);
    if (t.type != ttElse) {
        // No else statement return already
        rewindTo(&t);
        return;
    }

    parseStatement();
}

void QAsCodeParser::ParseFor() {

    sToken t;
    getToken(&t);
    if (t.type != ttFor) {
        rewindErrorTo(&t);
        return;
    }

    getToken(&t);
    if (t.type != ttOpenParenthesis) {
        rewindErrorTo(&t);
        return;
    }

    if (isVarDecl())
        parseDeclaration();
    else
        ParseExpressionStatement();
    if (isSyntaxError)
        return;

    ParseExpressionStatement();
    if (isSyntaxError)
        return;

    getToken(&t);
    if (t.type != ttCloseParenthesis) {
        rewindTo(&t);

        // Parse N increment statements separated by ,
        for (;;) {

            ParseAssignment();
            if (isSyntaxError)
                return;

            getToken(&t);
            if (t.type == ttListSeparator)
                continue;
            else if (t.type == ttCloseParenthesis)
                break;
            else {
                rewindErrorTo(&t);
                return;
            }
        }
    }

    parseStatement();
}

void QAsCodeParser::ParseWhile() {
    sToken t;
    getToken(&t);
    if (t.type != ttWhile) {
        rewindErrorTo(&t);
        return;
    }

    getToken(&t);
    if (t.type != ttOpenParenthesis) {
        rewindErrorTo(&t);
        return;
    }

    ParseAssignment();
    if (isSyntaxError)
        return;

    getToken(&t);
    if (t.type != ttCloseParenthesis) {
        rewindErrorTo(&t);
        return;
    }

    parseStatement();
}

void QAsCodeParser::ParseDoWhile() {

    sToken t;
    getToken(&t);
    if (t.type != ttDo) {
        rewindErrorTo(&t);
        return;
    }

    parseStatement();
    if (isSyntaxError)
        return;

    getToken(&t);
    if (t.type != ttWhile) {
        rewindErrorTo(&t);
        return;
    }

    getToken(&t);
    if (t.type != ttOpenParenthesis) {
        rewindErrorTo(&t);
        return;
    }

    ParseAssignment();
    if (isSyntaxError)
        return;

    getToken(&t);
    if (t.type != ttCloseParenthesis) {
        rewindErrorTo(&t);
        return;
    }

    getToken(&t);
    if (t.type != ttEndStatement) {
        rewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseReturn() {
    sToken t;
    getToken(&t);
    if (t.type != ttReturn) {
        rewindErrorTo(&t);
        return;
    }

    getToken(&t);
    if (t.type == ttEndStatement) {

        return;
    }

    rewindTo(&t);

    ParseAssignment();
    if (isSyntaxError)
        return;

    getToken(&t);
    if (t.type != ttEndStatement) {
        rewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseBreak() {
    sToken t;
    getToken(&t);
    if (t.type != ttBreak) {
        rewindErrorTo(&t);
        return;
    }

    getToken(&t);
    if (t.type != ttEndStatement) {
        rewindErrorTo(&t);
    }

    return;
}

void QAsCodeParser::ParseContinue() {
    sToken t;
    getToken(&t);
    if (t.type != ttContinue) {
        rewindErrorTo(&t);
        return;
    }

    getToken(&t);
    if (t.type != ttEndStatement) {
        rewindErrorTo(&t);
    }
}

void QAsCodeParser::ParseTryCatch() {
    sToken t;
    getToken(&t);
    if (t.type != ttTry) {
        rewindErrorTo(&t);
        return;
    }

    parseStatementBlock();
    if (isSyntaxError)
        return;

    getToken(&t);
    if (t.type != ttCatch) {
        rewindErrorTo(&t);
        return;
    }

    parseStatementBlock();
    if (isSyntaxError)
        return;
}

QList<QAsCodeParser::Symbol> QAsCodeParser::parseDeclaration(bool isClassProp,
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
    auto dt = parseType(true, false, !isClassProp);
    if (isSyntaxError)
        return ret;

    // sym.typeStr = getSymbolString(dt);

    for (;;) {
        // Parse identifier
        auto id = parseIdentifier();
        if (isSyntaxError)
            return ret;

        sym.name = getSymbolString(id);
        // sym.nameInSrc = id.pos;

        if (isClassProp || isGlobalVar) {
            // Only superficially parse the initialization RewindErrorTo for the
            // class property
            getToken(&t);
            rewindTo(&t);
            if (t.type == ttAssignment || t.type == ttOpenParenthesis) {
                superficiallyParseVarInit();
                if (isSyntaxError)
                    return ret;
            }
        } else {
            // If next token is assignment, parse expression
            getToken(&t);
            if (t.type == ttOpenParenthesis) {
                rewindTo(&t);
                parseArgList();
                if (isSyntaxError)
                    return ret;
            } else if (t.type == ttAssignment) {
                getToken(&t);
                rewindTo(&t);
                if (t.type == ttStartStatementBlock) {
                    ParseInitList();
                    if (isSyntaxError)
                        return ret;
                } else {
                    ParseAssignment();
                    if (isSyntaxError)
                        return ret;
                }
            } else
                rewindTo(&t);
        }

        // continue if list separator, else terminate with end statement
        getToken(&t);
        if (t.type == ttListSeparator) {
            // sym.type = SymbolType::Variable;
            ret.append(sym);
            continue;
        } else if (t.type == ttEndStatement) {
            // sym.type = SymbolType::Variable;
            ret.append(sym);
            return ret;
        } else {
            rewindErrorTo(&t);
            return ret;
        }
    }
    UNREACHABLE_RETURN;
}

QList<QAsCodeParser::CodeSegment> QAsCodeParser::parseScript(bool inBlock) {
    QList<CodeSegment> segs;

    for (;;) {
        while (!isSyntaxError) {
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
                // import we don't support just skip
                skipCodeBlock();
            } else if (t1.type == ttEnum) // Handle enumerations
                appendValidSeg(segs, parseEnumeration());
            else if (t1.type == ttTypedef) // Handle primitive typedefs
                appendValidSeg(segs, parseTypedef());
            else if (t1.type == ttClass)
                appendValidSeg(segs, parseClass());
            else if (t1.type == ttMixin)
                appendValidSeg(segs, parseMixin());
            else if (t1.type == ttInterface)
                appendValidSeg(segs, parseInterface());
            else if (t1.type == ttFuncDef) {
                appendValidSeg(segs, parseFuncDef());
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
                    seg.type = SymbolType::VarsDecl;
                    seg.codes = code.sliced(begin, end - begin + 1);
                    rewindTo(&t1);

                    segs.append(seg);
                } else {
                    appendValidSeg(segs, parseFunction());
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

        if (isSyntaxError) {
            skipCodeBlock();
            isSyntaxError = false;
        }
    }
    UNREACHABLE_RETURN;
}

void QAsCodeParser::appendValidSeg(QList<CodeSegment> &container,
                                   CodeSegment seg) {
    if (seg.isValid()) {
        container.append(seg);
    }
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
    return QByteArray(code.data() + t.pos, t.length);
}

QByteArrayList QAsCodeParser::getRealNamespace(const QByteArrayList &ns) {
    // if (ns.isEmpty()) {
    //     return _curns + ns;
    // } else {
    return ns;
    // }
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
    seg.offset = cls.pos;
    seg.scope = currentNs;
    seg.type = SymbolType::Class;

    // External shared declarations are ended with ';'
    getToken(&t);
    if (t.type == ttEndStatement) {
        rewindTo(&t);
        parseToken(ttEndStatement);
        return seg;
    }

    rewindTo(&t);
    auto begin = t.pos;
    t = superficiallyParseStatementBlock();
    auto end = t.pos;
    seg.codes = code.sliced(begin, end - begin + 1);
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
    auto end = token.pos;

    // seg.name is empty
    seg.scope = currentNs;
    seg.offset = begin;
    seg.type = SymbolType::TypeDef;
    seg.codes = code.sliced(begin, end - begin + 1);
    return seg;
}

QAsCodeParser::CodeSegment QAsCodeParser::parseEnumeration() {
    CodeSegment seg;
    sToken token;

    // Optional 'shared' and 'external' token
    getToken(&token);
    while (identifierIs(token, SHARED_TOKEN) ||
           identifierIs(token, EXTERNAL_TOKEN)) {
        rewindTo(&token);

        parseIdentifier();

        if (isSyntaxError)
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
    seg.offset = token.pos;
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
    seg.codes = code.sliced(begin, end - begin + 1);
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
    if (isSyntaxError)
        return;

    ns.append(getSymbolString(id));

    getToken(&t1);
    while (t1.type == ttScope) {
        auto id = parseIdentifier();

        if (isSyntaxError)
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

    if (!isSyntaxError) {
        getToken(&t1);
        if (t1.type != ttEndStatementBlock) {
            rewindErrorTo(&start);
            return;
        }
    }
}

QAsCodeParser::CodeSegment QAsCodeParser::parseFunction(bool isMethod) {
    CodeSegment fn;

    // TODO: Why isn't ParseFunctionDefinition used?
    sToken t1;
    getToken(&t1);

    if (!isMethod) {
        // A global function can be marked as shared and external
        while (t1.type == ttIdentifier) {
            if (identifierIs(t1, SHARED_TOKEN) ||
                identifierIs(t1, EXTERNAL_TOKEN)) {
                rewindTo(&t1);
                parseIdentifier();
                if (isSyntaxError)
                    return fn;
            } else
                break;

            getToken(&t1);
        }
    }

    // A class method can start with 'private' or 'protected'
    if (isMethod && t1.type == ttPrivate) {
        rewindTo(&t1);
        parseToken(ttPrivate);
        getToken(&t1);
    } else if (isMethod && t1.type == ttProtected) {
        rewindTo(&t1);
        parseToken(ttProtected);
        getToken(&t1);
    }
    if (isSyntaxError)
        return fn;

    // If it is a global function, or a method, except constructor and
    // destructor, then the return type is parsed
    sToken t2;
    getToken(&t2);
    rewindTo(&t1);
    if (!isMethod || (t1.type != ttBitNot && t2.type != ttOpenParenthesis)) {
        auto id = parseType(true);
        if (isSyntaxError)
            return fn;
        // fn.ret = getSymbolString(id);

        ParseTypeMod(false);
        if (isSyntaxError)
            return fn;
    }

    // If this is a class destructor then it starts with ~, and no return type
    // is declared
    if (isMethod && t1.type == ttBitNot) {
        parseToken(ttBitNot);
        if (isSyntaxError)
            return fn;
    }

    auto iden = parseIdentifier();
    if (isSyntaxError)
        return fn;
    fn.name = getSymbolString(iden);
    // fn.nameInSrc = iden.pos;

    auto params = parseParameterListContent();
    if (isSyntaxError)
        return fn;
    // fn.args = params;

    if (isMethod) {
        getToken(&t1);
        rewindTo(&t1);

        // Is the method a const?
        if (t1.type == ttConst)
            parseToken(ttConst);
    }

    // TODO: Should support abstract methods, in which case no statement block
    // should be provided
    parseMethodAttributes();
    if (isSyntaxError)
        return fn;

    // External shared functions must be ended with ';'
    getToken(&t1);
    rewindTo(&t1);
    if (t1.type == ttEndStatement) {
        parseToken(ttEndStatement);
        // fn.ns = _curns;
        return fn;
    }

    // We should just find the end of the statement block here. The statements
    // will be parsed on request by the compiler once it starts the compilation.
    superficiallyParseStatementBlock();
    // fn.ns = _curns;
    // fn.code = code.mid(t1.pos, sourcePos - t1.pos);
    // fn.valid = true;
    return fn;
}

QAsCodeParser::Symbol QAsCodeParser::parseFuncDefContent() {
    Symbol sym;

    // Allow keywords 'external' and 'shared' before 'interface'
    sToken t1;
    getToken(&t1);
    while (identifierIs(t1, SHARED_TOKEN) || identifierIs(t1, EXTERNAL_TOKEN)) {
        rewindTo(&t1);

        if (isSyntaxError)
            return sym;

        getToken(&t1);
    }

    if (t1.type != ttFuncDef) {
        rewindErrorTo(&t1);
        return sym;
    }

    parseType(true);
    if (isSyntaxError)
        return sym;

    ParseTypeMod(false);
    if (isSyntaxError)
        return sym;

    auto iden = parseIdentifier();
    if (isSyntaxError)
        return sym;
    sym.name = getSymbolString(iden);
    // sym.nameInSrc = iden.pos;

    auto args = parseParameterListContent();
    if (isSyntaxError)
        return sym;
    // sym.content = args;

    getToken(&t1);
    if (t1.type != ttEndStatement) {
        rewindErrorTo(&t1);
        return sym;
    }

    // sym.type = SymbolType::FnDecl;
    return sym;
}

void QAsCodeParser::parseClassContent() {
    Symbol clssym;

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
        return;
    }

    if (engine->ep.allowImplicitHandleTypes) {
        // Parse 'implicit handle class' construct
        getToken(&t);

        if (t.type != ttHandle)
            rewindTo(&t);
    }

    auto cls = parseIdentifier();
    clssym.name = getSymbolString(cls);
    // clssym.nameInSrc = cls.pos;
    // clssym.type = SymbolType::Class;

    // External shared declarations are ended with ';'
    getToken(&t);
    if (t.type == ttEndStatement) {
        rewindTo(&t);
        parseToken(ttEndStatement);
        return;
    }

    // Optional list of interfaces that are being implemented and classes that
    // are being inherited
    if (t.type == ttColon) {
        // TODO: dont support temperily, later
        ParseOptionalScope();
        parseIdentifier();
        getToken(&t);
        while (t.type == ttListSeparator) {
            ParseOptionalScope();
            parseIdentifier();
            getToken(&t);
        }
    }

    if (t.type != ttStartStatementBlock) {
        rewindErrorTo(&t);
        return;
    }

    // Parse properties
    getToken(&t);
    rewindTo(&t);
    while (t.type != ttEndStatementBlock && t.type != ttEnd) {
        // Is it a property or a method?
        if (t.type == ttFuncDef) {
            auto fndef = parseFuncDefContent();
            // if (fndef.isValid()) {
            //     clssym.content.append(fndef);
            // }
        } else if (isFuncDecl(true)) {
            auto fn = parseFunction(true);
            // if (fn.isValid()) {
            //     clssym.codesegs.insert(fn.nameInSrc, fn);
            // }
        } else if (isVirtualPropertyDecl()) {
            auto vp = parseVirtualPropertyDecl(true, false);
            // if (vp.isValid()) {
            //     clssym.content.append(vp);
            // }
        } else if (isVarDecl()) {
            auto decl = parseDeclaration(true);
            // clssym.content.append(decl);
        } else if (t.type == ttEndStatement)
            // Skip empty declarations
            getToken(&t);
        else {
            rewindErrorTo(&t);
            return;
        }

        if (isSyntaxError)
            return;

        getToken(&t);
        rewindTo(&t);
    }

    getToken(&t);
    if (t.type != ttEndStatementBlock) {
        rewindErrorTo(&t);
        return;
    }

    // _symtable.insert(cls.pos, clssym);
}

void QAsCodeParser::parseMixinContent() {
    sToken t;
    getToken(&t);

    if (t.type != ttMixin) {
        rewindErrorTo(&t);
        return;
    }

    // A mixin token must be followed by a class declaration
    parseClassContent();
}

void QAsCodeParser::ParseInitList() {
    sToken t1;

    getToken(&t1);
    if (t1.type != ttStartStatementBlock) {
        rewindErrorTo(&t1);
        return;
    }

    getToken(&t1);
    if (t1.type == ttEndStatementBlock) {
        // Statement block is finished
        return;
    } else {
        rewindTo(&t1);
        for (;;) {
            getToken(&t1);
            if (t1.type == ttListSeparator) {
                // No expression

                getToken(&t1);
                if (t1.type == ttEndStatementBlock) {
                    // No expression
                    return;
                }
                rewindTo(&t1);
            } else if (t1.type == ttEndStatementBlock) {
                // No expression
                // Statement block is finished
                return;
            } else if (t1.type == ttStartStatementBlock) {
                rewindTo(&t1);
                ParseInitList();

                if (isSyntaxError)
                    return;

                getToken(&t1);
                if (t1.type == ttListSeparator)
                    continue;
                else if (t1.type == ttEndStatementBlock) {

                    // Statement block is finished
                    return;
                } else {
                    rewindErrorTo(&t1);
                    return;
                }
            } else {
                rewindTo(&t1);
                // NOTE
                ParseAssignment();
                if (isSyntaxError)
                    return;

                getToken(&t1);
                if (t1.type == ttListSeparator)
                    continue;
                else if (t1.type == ttEndStatementBlock) {
                    // Statement block is finished
                    return;
                } else {
                    rewindErrorTo(&t1);
                    return;
                }
            }
        }
    }
    UNREACHABLE_RETURN;
}

void QAsCodeParser::parseInterfaceContent() {
    Symbol sym;

    sToken t;

    // Allow keywords 'external' and 'shared' before 'interface'
    getToken(&t);
    while (identifierIs(t, SHARED_TOKEN) || identifierIs(t, EXTERNAL_TOKEN)) {
        rewindTo(&t);
        parseIdentifier();

        if (isSyntaxError)
            return;

        getToken(&t);
    }

    if (t.type != ttInterface) {
        rewindErrorTo(&t);
        return;
    }

    auto id = parseIdentifier();
    sym.name = getSymbolString(id);
    // sym.nameInSrc = id.pos;

    // External shared declarations are ended with ';'
    getToken(&t);
    if (t.type == ttEndStatement) {
        rewindTo(&t);
        parseToken(ttEndStatement);
        return;
    }

    // Can optionally have a list of interfaces that are inherited
    if (t.type == ttColon) {
        ParseOptionalScope();
        parseIdentifier();
        getToken(&t);
        while (t.type == ttListSeparator) {
            ParseOptionalScope();
            parseIdentifier();
            getToken(&t);
        }
    }

    if (t.type != ttStartStatementBlock) {
        rewindErrorTo(&t);
        return;
    }

    // Parse interface methods
    getToken(&t);
    rewindTo(&t);
    while (t.type != ttEndStatementBlock && t.type != ttEnd) {
        if (isVirtualPropertyDecl()) {
            auto vp = parseVirtualPropertyDecl(true, true);
            // if (vp.isValid()) {
            //     sym.content.append(vp);
            // }
        } else if (t.type == ttEndStatement) {
            // Skip empty declarations
            getToken(&t);
        } else {
            // Parse the method signature
            auto im = parseInterfaceMethod();
            // if (im.isValid()) {
            //     sym.content.append(im);
            // }
        }

        if (isSyntaxError)
            return;

        getToken(&t);
        rewindTo(&t);
    }

    getToken(&t);
    if (t.type != ttEndStatementBlock) {
        rewindErrorTo(&t);
        return;
    }
}

QAsCodeParser::Symbol QAsCodeParser::parseInterfaceMethod() {
    Symbol sym;

    auto ret = parseType(true);
    if (isSyntaxError)
        return sym;
    // sym.typeStr = getSymbolString(ret);

    ParseTypeMod(false);
    if (isSyntaxError)
        return sym;

    auto id = parseIdentifier();
    if (isSyntaxError)
        return sym;
    sym.name = getSymbolString(id);
    // sym.nameInSrc = id.pos;

    auto args = parseParameterListContent();
    if (isSyntaxError)
        return sym;
    // sym.content = args;

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

    // sym.type = SymbolType::FnDecl;
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

    if (isSyntaxError)
        return sym;

    auto id = parseType(true);
    if (isSyntaxError)
        return sym;
    // sym.typeStr = getSymbolString(id);

    ParseTypeMod(false);
    if (isSyntaxError)
        return sym;

    auto iden = parseIdentifier();
    if (isSyntaxError)
        return sym;
    sym.name = getSymbolString(iden);
    // sym.nameInSrc = iden.pos;

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
                    if (isSyntaxError)
                        return sym;
                }
            }

            if (!isInterface) {
                getToken(&t1);
                if (t1.type == ttStartStatementBlock) {
                    rewindTo(&t1);
                    superficiallyParseStatementBlock();

                    // seg.valid = true;
                    // seg.code = code.mid(t1.pos, sourcePos - t1.pos);
                    // sym.codesegs.insert(t1.pos, seg);

                    if (isSyntaxError)
                        return sym;
                } else if (t1.type == ttEndStatement) {
                    rewindTo(&t1);
                    parseToken(ttEndStatement);
                    if (isSyntaxError)
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
                    if (isSyntaxError)
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

    // sym.type = SymbolType::Property;
    return sym;
}

void QAsCodeParser::parseEnumerationContent() {
    Symbol sym;
    sToken token;
    size_t eoff;

    // Optional 'shared' and 'external' token
    getToken(&token);
    while (identifierIs(token, SHARED_TOKEN) ||
           identifierIs(token, EXTERNAL_TOKEN)) {
        rewindTo(&token);

        parseIdentifier();

        if (isSyntaxError)
            return;

        getToken(&token);
    }

    // Check for enum
    if (token.type != ttEnum) {
        rewindErrorTo(&token);
        return;
    }

    // Get the identifier
    getToken(&token);
    if (ttIdentifier != token.type) {
        rewindErrorTo(&token);
        return;
    }

    // ok, init symbol
    eoff = token.pos;
    sym.name = getSymbolString(token);
    // sym.nameInSrc = token.pos;
    // sym.type = SymbolType::Enum;
    // sym.ns = _curns;

    // External shared declarations are ended with ';'
    getToken(&token);
    if (token.type == ttEndStatement) {
        rewindTo(&token);
        parseToken(ttEndStatement);
        return;
    }

    // check for the start of the declaration block
    if (token.type != ttStartStatementBlock) {
        rewindTo(&token);
        rewindErrorTo(&token);
        return;
    }

    while (ttEnd != token.type) {
        getToken(&token);

        if (ttEndStatementBlock == token.type) {
            rewindTo(&token);
            break;
        }

        if (ttIdentifier != token.type) {
            rewindErrorTo(&token);
            return;
        }

        // Add the enum element
        Symbol se;
        // se.type = SymbolType::Value;
        se.name = getSymbolString(token);
        getToken(&token);
        // sym.content.append(se);

        if (token.type == ttAssignment) {
            rewindTo(&token);

            superficiallyParseVarInit();

            if (isSyntaxError)
                return;

            getToken(&token);
        }

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
        return;
    }

    // _symtable.insert(eoff, sym);
}

void QAsCodeParser::parseTypedefContent() {
    Symbol sym;
    size_t eoff;

    sToken token;

    getToken(&token);
    if (token.type != ttTypedef) {
        rewindErrorTo(&token);
        return;
    }

    eoff = token.pos;

    // Parse the base type
    getToken(&token);
    rewindTo(&token);

    // Make sure it is a primitive type (except ttVoid)
    if (!isRealType(token.type) || token.type == ttVoid) {
        rewindErrorTo(&token);
        return;
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

    Symbol st;
    st.name = getSymbolString(r);

    // sym.content.append(st);
    // sym.ns = _curns;

    // _symtable.insert(eoff, sym);
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

bool QAsCodeParser::isLambda() {
    bool isLambda = false;
    sToken t;
    getToken(&t);
    if (t.type == ttIdentifier && identifierIs(t, FUNCTION_TOKEN)) {
        sToken t2;
        getToken(&t2);
        if (t2.type == ttOpenParenthesis) {
            // Skip until )
            while (t2.type != ttCloseParenthesis && t2.type != ttEnd)
                getToken(&t2);

            // The next token must be a {
            getToken(&t2);
            if (t2.type == ttStartStatementBlock)
                isLambda = true;
        }
    }

    rewindTo(&t);
    return isLambda;
}

bool QAsCodeParser::isFunctionCall() {
    sToken s;
    sToken t1, t2;

    getToken(&s);
    t1 = s;

    // A function call may be prefixed with scope resolution
    if (t1.type == ttScope)
        getToken(&t1);
    getToken(&t2);

    while (t1.type == ttIdentifier && t2.type == ttScope) {
        getToken(&t1);
        getToken(&t2);
    }

    // A function call starts with an identifier followed by an argument list
    // The parser doesn't have enough RewindErrorTormation about scope to
    // determine if the identifier is a datatype, so even if it happens to be
    // the parser will identify the expression as a function call rather than a
    // construct call. The compiler will sort this out later
    if (t1.type != ttIdentifier) {
        rewindTo(&s);
        return false;
    }

    if (t2.type == ttOpenParenthesis) {
        rewindTo(&s);
        return true;
    }

    rewindTo(&s);
    return false;
}

void QAsCodeParser::ParseAssignment() {
    ParseCondition();
    if (isSyntaxError)
        return;

    sToken t;
    getToken(&t);
    rewindTo(&t);

    if (isAssignOperator(t.type)) {
        ParseAssignOperator();
        if (isSyntaxError)
            return;

        ParseAssignment();
        if (isSyntaxError)
            return;
    }
}

void QAsCodeParser::ParseAssignOperator() {
    sToken t;
    getToken(&t);
    if (!isAssignOperator(t.type)) {
        rewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseCondition() {
    ParseExpression();

    if (isSyntaxError)
        return;

    sToken t;
    getToken(&t);
    if (t.type == ttQuestion) {
        ParseAssignment();
        if (isSyntaxError)
            return;

        getToken(&t);
        if (t.type != ttColon) {
            rewindErrorTo(&t);
            return;
        }

        ParseAssignment();
        if (isSyntaxError)
            return;
    } else
        rewindTo(&t);
}

void QAsCodeParser::ParseExpression() {
    ParseExprTerm();
    if (isSyntaxError)
        return;

    for (;;) {
        sToken t;
        getToken(&t);
        rewindTo(&t);

        if (!isOperator(t.type))
            return;

        ParseExprOperator();
        if (isSyntaxError)
            return;

        ParseExprTerm();
        if (isSyntaxError)
            return;
    }
    UNREACHABLE_RETURN;
}

void QAsCodeParser::ParseExprTerm() {
    // Check if the expression term is an initialization of a temp object with
    // init list, i.e. type = {...}
    sToken t;
    getToken(&t);
    sToken t2 = t, t3;
    rewindTo(&t);
    if (isDataType(t2) && checkTemplateType(t2) && findTokenAfterType(t2)) {
        // Move to token after the type
        rewindTo(&t2);

        // The next token must be a = followed by a {
        getToken(&t2);
        getToken(&t3);
        if (t2.type == ttAssignment && t3.type == ttStartStatementBlock) {
            // It is an initialization, now parse it for real
            rewindTo(&t);
            parseType(false);
            getToken(&t2);
            ParseInitList();
            return;
        }
    }
    // Or an anonymous init list, i.e. {...}
    else if (t.type == ttStartStatementBlock) {
        rewindTo(&t);
        ParseInitList();
        return;
    }

    // It wasn't an initialization, so it must be an ordinary expression term
    rewindTo(&t);

    for (;;) {
        getToken(&t);
        rewindTo(&t);
        if (!isPreOperator(t.type))
            break;

        ParseExprPreOp();
        if (isSyntaxError)
            return;
    }

    ParseExprValue();
    if (isSyntaxError)
        return;

    for (;;) {
        getToken(&t);
        rewindTo(&t);
        if (!isPostOperator(t.type))
            return;

        ParseExprPostOp();
        if (isSyntaxError)
            return;
    }
    UNREACHABLE_RETURN;
}

void QAsCodeParser::ParseExprOperator() {
    sToken t;
    getToken(&t);
    if (!isOperator(t.type)) {
        rewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseExprPreOp() {
    sToken t;
    getToken(&t);
    if (!isPreOperator(t.type)) {
        rewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseExprPostOp() {
    sToken t;
    getToken(&t);
    if (!isPostOperator(t.type)) {
        rewindErrorTo(&t);
        return;
    }

    if (t.type == ttDot) {
        sToken t1, t2;
        getToken(&t1);
        getToken(&t2);
        rewindTo(&t1);
        if (t2.type == ttOpenParenthesis)
            ParseFunctionCall();
        else
            parseIdentifier();
    } else if (t.type == ttOpenBracket) {
        parseArgList(false);

        getToken(&t);
        if (t.type != ttCloseBracket) {
            rewindErrorTo(&t);
            return;
        }

    } else if (t.type == ttOpenParenthesis) {
        rewindTo(&t);
        parseArgList();
    }
}

void QAsCodeParser::ParseExprValue() {
    sToken t1, t2;
    getToken(&t1);
    getToken(&t2);
    rewindTo(&t1);

    // 'void' is a special expression that doesn't do anything (normally used
    // for skipping output arguments)
    if (t1.type == ttVoid)
        parseToken(ttVoid);
    else if (isRealType(t1.type))
        ParseConstructCall();
    else if (t1.type == ttIdentifier || t1.type == ttScope) {
        // Check if the expression is an anonymous function
        if (isLambda()) {
            ParseLambda();
        } else {
            // Determine the last identifier in order to check if it is a type
            findIdentifierAfterScope(t2);
            rewindTo(&t2);

            // Get The token after the identifier to determine if it is an [
            sToken t;
            getToken(&t);
            getToken(&t);

            bool isDataType = this->isDataType(t2);
            bool isTemplateType = false;
            if (isDataType) {
                // Is this a template type?
                tempString = getSymbolString(t2);
                if (engine->IsTemplateType(tempString))
                    isTemplateType = true;
            }

            getToken(&t2);

            // Rewind so the real parsing can be done, after deciding what to
            // parse
            rewindTo(&t1);

            // Check if this is a construct call
            // Just 'type()' isn't considered a construct call, because type may
            // just be a function/method name. The compiler will have to sort
            // this out, since the parser doesn't have enough
            // RewindErrorTormation.
            if (isDataType && (t.type == ttOpenBracket &&
                               t2.type == ttCloseBracket)) // type[]()
                ParseConstructCall();
            else if (isTemplateType && t.type == ttLessThan) // type<t>()
                ParseConstructCall();
            else if (isFunctionCall())
                ParseFunctionCall();
            else
                ParseVariableAccess();
        }
    } else if (t1.type == ttCast)
        ParseCast();
    else if (isConstant(t1.type))
        ParseConstant();
    else if (t1.type == ttOpenParenthesis) {
        getToken(&t1);
        ParseAssignment();
        if (isSyntaxError)
            return;

        getToken(&t1);
        if (t1.type != ttCloseParenthesis) {
            rewindErrorTo(&t1);
        }

    } else {
        rewindErrorTo(&t1);
    }
}

void QAsCodeParser::parseArgList(bool withParenthesis) {
    sToken t1;
    if (withParenthesis) {
        getToken(&t1);
        if (t1.type != ttOpenParenthesis) {
            rewindErrorTo(&t1);
            return;
        }
    }

    getToken(&t1);
    if (t1.type == ttCloseParenthesis || t1.type == ttCloseBracket) {
        if (withParenthesis) {
            if (t1.type != ttCloseParenthesis) {

                rewindErrorTo(&t1);
            }
        } else
            rewindTo(&t1);

        // Argument list has ended
        return;
    } else {
        rewindTo(&t1);

        for (;;) {
            // Determine if this is a named argument
            sToken tl, t2;
            getToken(&tl);
            getToken(&t2);
            rewindTo(&tl);

            // Named arguments uses the syntax: arg : expr
            // This avoids confusion when the argument has the same name as a
            // local variable, i.e. var = expr It also avoids conflict with
            // expressions to that creates anonymous objects initialized with
            // lists, i.e. type = {...} The alternate syntax: arg = expr, is
            // supported to provide backwards compatibility with 2.29.0
            // TODO: 3.0.0: Remove the alternate syntax
            if (tl.type == ttIdentifier &&
                (t2.type == ttColon || (engine->ep.alterSyntaxNamedArgs &&
                                        t2.type == ttAssignment))) {

                // NOTE add symbol

                parseIdentifier();
                getToken(&t2);

                ParseAssignment();
            } else
                ParseAssignment();

            if (isSyntaxError)
                return;

            // Check if list continues
            getToken(&t1);
            if (t1.type == ttListSeparator)
                continue;
            else {
                if (withParenthesis) {
                    if (t1.type != ttCloseParenthesis) {
                        rewindErrorTo(&t1);
                    }
                } else
                    rewindTo(&t1);

                return;
            }
        }
    }
}

void QAsCodeParser::ParseFunctionCall() {
    // Parse scope prefix
    ParseOptionalScope();

    // Parse the function name followed by the argument list
    parseIdentifier();
    if (isSyntaxError)
        return;

    parseArgList();
}

void QAsCodeParser::ParseVariableAccess() {
    // Parse scope prefix
    ParseOptionalScope();

    // Parse the variable name
    parseIdentifier();
}

void QAsCodeParser::ParseConstructCall() {
    // NOTE

    parseType(false);
    if (isSyntaxError)
        return;

    parseArgList();
}

void QAsCodeParser::ParseCast() {
    sToken t1;
    getToken(&t1);
    if (t1.type != ttCast) {
        rewindErrorTo(&t1);
        return;
    }

    getToken(&t1);
    if (t1.type != ttLessThan) {
        rewindErrorTo(&t1);
        return;
    }

    // Parse the data type
    parseType(true);
    if (isSyntaxError)
        return;

    getToken(&t1);
    if (t1.type != ttGreaterThan) {
        rewindErrorTo(&t1);
        return;
    }

    getToken(&t1);
    if (t1.type != ttOpenParenthesis) {
        rewindErrorTo(&t1);
        return;
    }

    ParseAssignment();
    if (isSyntaxError)
        return;

    getToken(&t1);
    if (t1.type != ttCloseParenthesis) {
        rewindErrorTo(&t1);
        return;
    }
}

void QAsCodeParser::ParseConstant() {
    sToken t;
    getToken(&t);
    if (!isConstant(t.type)) {
        rewindErrorTo(&t);
        return;
    }

    // We want to gather a list of string constants to concatenate as children
    if (t.type == ttStringConstant || t.type == ttMultilineStringConstant ||
        t.type == ttHeredocStringConstant)
        rewindTo(&t);

    while (t.type == ttStringConstant || t.type == ttMultilineStringConstant ||
           t.type == ttHeredocStringConstant) {
        // NOTE
        ParseStringConstant();

        getToken(&t);
        rewindTo(&t);
    }
}

void QAsCodeParser::ParseStringConstant() {
    sToken t;
    getToken(&t);
    if (t.type != ttStringConstant && t.type != ttMultilineStringConstant &&
        t.type != ttHeredocStringConstant) {
        rewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseLambda() {
    sToken t;
    getToken(&t);

    if (t.type != ttIdentifier || !identifierIs(t, FUNCTION_TOKEN)) {
        rewindErrorTo(&t);
        return;
    }

    getToken(&t);
    if (t.type != ttOpenParenthesis) {
        rewindErrorTo(&t);
        return;
    }

    getToken(&t);
    while (t.type != ttCloseParenthesis && t.type != ttEnd) {
        rewindTo(&t);

        // Create node to represent the parameter, the datatype and identifier
        // must be children of the node. If no datatype or identifier is given
        // then the node is still there to be counted as the arg
        // NOTE
        // auto param = CreateNode(snUndefined);
        // node->children().append(param);

        // Skip 'const' for the next check
        sToken t1, t2 = t;
        if (t.type == ttConst)
            getToken(&t1);

        // Determine the last identifier after scope in order to check if it is
        // a type
        findIdentifierAfterScope(t2);
        rewindTo(&t);

        // Parse optional type before parameter name
        if (isDataType(t2) && checkTemplateType(t2) && findTokenAfterType(t) &&
            (t.type == ttIdentifier || t.type == ttListSeparator ||
             t.type == ttCloseParenthesis)) {
            // NOTE
            // param->children().append(ParseType(true));
            // if (isSyntaxError)
            //     return ;
            // param->children().append(ParseTypeMod(true));
            // if (isSyntaxError)
            //     return ;
        }

        // Parse optional parameter name
        if (t.type == ttIdentifier) {
            // NOTE
            // param->children().append(ParseIdentifier());
            if (isSyntaxError)
                return;
        }

        getToken(&t);
        if (t.type == ttListSeparator)
            getToken(&t);
        else if (t.type != ttCloseParenthesis) {
            rewindErrorTo(&t);
            return;
        }
    }

    if (t.type != ttCloseParenthesis) {
        rewindErrorTo(&t);
        return;
    }

    // We should just find the end of the statement block here. The statements
    // will be parsed on request by the compiler once it starts the compilation.
    superficiallyParseStatementBlock();
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
    if (!isRealType(t1.type) && t1.type != ttIdentifier && t1.type != ttAuto) {
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

bool QAsCodeParser::findIdentifierAfterScope(sToken &nextToken) {
    sToken t1, t2, t3;

    // Determine the last identifier after scope in order to check if it is a
    // type
    getToken(&t1);
    getToken(&t2);
    rewindTo(&t1);

    if (t1.type != ttScope && t2.type != ttScope) {
        if (t1.type == ttIdentifier) {
            nextToken = t1;
            return true;
        }
        return false;
    }

    if (t1.type == ttScope)
        t3 = t2;
    else
        t3 = t1;
    rewindTo(&t3);
    getToken(&t2);
    while (t3.type == ttIdentifier) {
        t2 = t3;
        getToken(&t3);
        if (t3.type == ttScope)
            getToken(&t3);
        else
            break;
    }
    rewindTo(&t1);
    nextToken = t2;

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
    tempString = getSymbolString(t);
    auto tstr = tempString;
    if (engine->IsTemplateType(tstr.data())) {
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
        if (code[QString::size_type(t1.pos)] != '>')
            return false;
        else if (t1.length != 1) {
            // We need to break the token, so that only the first character is
            // parsed
            SetPos(t1.pos + 1);
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
