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

QAsCodeParser::SymbolTable QAsCodeParser::parse(const QByteArray &codes) {
    Reset();
    code = codes;
    ParseScript(false);

    SymbolTable ret = _symtable;

    if (!m_segs.isEmpty()) {
        // only add function signature as symbol
        // you can extend it with more features
        // PRS are welcomed.
        for (auto p = m_segs.constKeyValueBegin();
             p != m_segs.constKeyValueEnd(); ++p) {
            auto v = p->second;

            Symbol fn;
            fn.type = SymbolType::Function;
            fn.name = v.name;
            fn.nameInSrc = v.nameInSrc;
            fn.content = v.args;
            fn.ns = v.ns;
            fn.typeStr = v.ret;
            ret.insert(p->first, fn);
        }
    }

    return ret;
}

QAsCodeParser::SymbolTable QAsCodeParser::parseIntell(qsizetype offset,
                                                      const QByteArray &codes) {
    parse(codes);
    // then we will parse the symbols we want

    SymbolTable ret;
    auto pend = _symtable.lowerBound(offset);
    for (auto p = _symtable.cbegin(); p != pend; p++) {
        ret.insert(p.key(), p.value());
    }

    if (!m_segs.isEmpty()) {
        // only add function signature as symbol
        // you can extend it with more features
        // PRS are welcomed.
        auto pend = m_segs.lowerBound(offset);
        for (auto p = m_segs.cbegin(); p != pend; ++p) {
            auto v = p.value();

            Symbol fn;
            fn.type = SymbolType::Function;
            fn.name = v.name;
            fn.nameInSrc = v.nameInSrc;
            fn.content = v.args;
            fn.ns = v.ns;
            fn.typeStr = v.ret;
            ret.insert(p.key(), fn);
        }
    }

    return _symtable;
}

void QAsCodeParser::Reset() {
    errorWhileParsing = false;
    isSyntaxError = false;
    checkValidTypes = false;
    isParsingAppInterface = false;

    sourcePos = 0;

    lastToken.pos = size_t(-1);
}

void QAsCodeParser::GetToken(sToken *token) {
    // Check if the token has already been parsed
    if (lastToken.pos == sourcePos) {
        *token = lastToken;
        sourcePos += token->length;

        if (token->type == ttWhiteSpace || token->type == ttOnelineComment ||
            token->type == ttMultilineComment)
            GetToken(token);

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

void QAsCodeParser::RewindTo(const sToken *token) {
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

void QAsCodeParser::RewindErrorTo(sToken *token) {
    RewindTo(token);

    isSyntaxError = true;
    errorWhileParsing = true;
}

QAsCodeParser::Symbol QAsCodeParser::ParseFunctionDefinition() {
    auto ret = ParseType(true);
    if (isSyntaxError)
        return {};

    ParseTypeMod(false);
    if (isSyntaxError)
        return {};

    auto ns = ParseOptionalScope();

    auto id = ParseIdentifier();
    if (isSyntaxError)
        return {};

    auto params = ParseParameterList();
    if (isSyntaxError)
        return {};

    // ok, add symbol
    Symbol se;
    se.type = SymbolType::FnDecl;
    se.typeStr = getSymbolString(id);
    se.name = getSymbolString(id);
    se.nameInSrc = id.pos;
    se.content = params;
    se.ns = getRealNamespace(ns);

    // Parse an optional 'const' after the function definition (used for
    // object methods)
    sToken t1;
    GetToken(&t1);
    RewindTo(&t1);
    if (t1.type == ttConst)
        ParseToken(ttConst);

    // Parse optional attributes
    ParseMethodAttributes();

    return se;
}

QList<QAsCodeParser::Symbol> QAsCodeParser::ParseParameterList() {
    QList<QAsCodeParser::Symbol> ret;

    sToken t1;
    GetToken(&t1);
    if (t1.type != ttOpenParenthesis) {
        RewindErrorTo(&t1);
        return ret;
    }

    GetToken(&t1);
    if (t1.type == ttCloseParenthesis) {
        // Statement block is finished
        return ret;
    } else {
        // If the parameter list is just (void) then the void token should be
        // ignored
        if (t1.type == ttVoid) {
            sToken t2;
            GetToken(&t2);
            if (t2.type == ttCloseParenthesis) {
                return ret;
            }
        }

        RewindTo(&t1);

        for (;;) {
            // Parse data type
            auto t = ParseType(true, isParsingAppInterface);
            if (isSyntaxError)
                return ret;

            ParseTypeMod(true);
            if (isSyntaxError)
                return ret;

            // Parse optional identifier
            GetToken(&t1);
            if (t1.type == ttIdentifier) {
                RewindTo(&t1);

                auto iden = ParseIdentifier();
                Symbol se;
                se.name = getSymbolString(iden);
                se.type = SymbolType::Variable;
                se.typeStr = getSymbolString(t);
                ret.append(se);

                if (isSyntaxError)
                    return ret;

                GetToken(&t1);
            }

            // Parse optional expression for the default arg
            if (t1.type == ttAssignment) {
                // Do a superficial parsing of the default argument
                // The actual parsing will be done when the argument is compiled
                // for a function call
                SuperficiallyParseExpression();
                if (isSyntaxError)
                    return ret;

                GetToken(&t1);
            }

            // Check if list continues
            if (t1.type == ttCloseParenthesis) {
                return ret;
            } else if (t1.type == ttListSeparator)
                continue;
            else {
                RewindErrorTo(&t1);
                return ret;
            }
        }
    }
    UNREACHABLE_RETURN;
}

void QAsCodeParser::SuperficiallyParseExpression() {
    // Simply parse everything until the first , or ), whichever comes first.
    // Keeping in mind that () and {} can group expressions.
    sToken start;
    GetToken(&start);
    RewindTo(&start);

    asCString stack;
    sToken t;
    for (;;) {
        GetToken(&t);

        if (t.type == ttOpenParenthesis)
            stack += "(";
        else if (t.type == ttCloseParenthesis) {
            if (stack == "") {
                // Expression has ended. This token is not part of expression
                RewindTo(&t);
                break;
            } else if (stack[stack.GetLength() - 1] == '(') {
                // Group has ended
                stack.SetLength(stack.GetLength() - 1);
            } else {
                // Wrong syntax
                RewindTo(&t);
                RewindErrorTo(&t);
                return;
            }
        } else if (t.type == ttListSeparator) {
            if (stack == "") {
                // Expression has ended. This token is not part of expression
                RewindTo(&t);
                break;
            }
        } else if (t.type == ttStartStatementBlock)
            stack += "{";
        else if (t.type == ttEndStatementBlock) {
            if (stack == "" || stack[stack.GetLength() - 1] != '{') {
                // Wrong syntax
                RewindErrorTo(&t);
                return;
            } else {
                // Group has ended
                stack.SetLength(stack.GetLength() - 1);
            }
        } else if (t.type == ttEndStatement) {
            // Wrong syntax (since we're parsing a default arg expression)
            RewindErrorTo(&t);
            return;
        } else if (t.type == ttNonTerminatedStringConstant) {
            RewindErrorTo(&t);
            return;
        } else if (t.type == ttEnd) {
            // Wrong syntax
            RewindErrorTo(&start);
            return;
        }

        // Include the token in the node
    }
}

sToken QAsCodeParser::ParseType(bool allowConst, bool allowVariableType,
                                bool allowAuto) {
    sToken t;

    if (allowConst) {
        GetToken(&t);
        RewindTo(&t);
        if (t.type == ttConst) {
            ParseToken(ttConst);
            if (isSyntaxError)
                return t;
        }
    }

    // Parse scope prefix
    ParseOptionalScope();

    // Parse the actual type
    auto dt = ParseDataType(allowVariableType, allowAuto);
    if (isSyntaxError)
        return t;

    // If the datatype is a template type, then parse the subtype within the < >
    GetToken(&t);
    RewindTo(&t);

    tempString = getSymbolString(dt);
    if (engine->IsTemplateType(tempString) && t.type == ttLessThan) {
        ParseTemplTypeList();
        if (isSyntaxError)
            return t;
    }

    // Parse [] and @
    GetToken(&t);
    RewindTo(&t);
    while (t.type == ttOpenBracket || t.type == ttHandle) {
        if (t.type == ttOpenBracket) {
            ParseToken(ttOpenBracket);
            if (isSyntaxError)
                return t;

            GetToken(&t);
            if (t.type != ttCloseBracket) {
                RewindErrorTo(&t);
                return t;
            }
        } else {
            ParseToken(ttHandle);
            if (isSyntaxError)
                return t;

            GetToken(&t);
            RewindTo(&t);
            if (t.type == ttConst) {
                ParseToken(ttConst);
                if (isSyntaxError)
                    return t;
            }
        }

        GetToken(&t);
        RewindTo(&t);
    }

    return dt;
}

void QAsCodeParser::ParseTypeMod(bool isParam) {
    sToken t;

    // Parse possible & token
    GetToken(&t);
    RewindTo(&t);
    if (t.type == ttAmp) {
        ParseToken(ttAmp);
        if (isSyntaxError)
            return;

        if (isParam) {
            GetToken(&t);
            RewindTo(&t);

            if (t.type == ttIn || t.type == ttOut || t.type == ttInOut) {
                int tokens[3] = {ttIn, ttOut, ttInOut};
                ParseOneOf(tokens, 3);
            }
        }
    }

    // Parse possible + token
    GetToken(&t);
    RewindTo(&t);
    if (t.type == ttPlus) {
        ParseToken(ttPlus);
        if (isSyntaxError)
            return;
    }

    // Parse possible if_handle_then_const token
    GetToken(&t);
    RewindTo(&t);
    if (IdentifierIs(t, IF_HANDLE_TOKEN)) {
        ParseToken(ttIdentifier);
        if (isSyntaxError)
            return;
    }
}

QByteArrayList QAsCodeParser::ParseOptionalScope() {
    QByteArrayList scopes;

    sToken t1, t2;
    GetToken(&t1);
    GetToken(&t2);
    if (t1.type == ttScope) {
        RewindTo(&t1);
        ParseToken(ttScope);
        GetToken(&t1);
        GetToken(&t2);
    }
    while (t1.type == ttIdentifier && t2.type == ttScope) {
        RewindTo(&t1);
        auto id = ParseIdentifier();
        scopes.append(getSymbolString(id));
        ParseToken(ttScope);
        GetToken(&t1);
        GetToken(&t2);
    }

    // The innermost scope may be a template type
    if (t1.type == ttIdentifier && t2.type == ttLessThan) {
        tempString = getSymbolString(t1);
        if (engine->IsTemplateType(tempString)) {
            RewindTo(&t1);
            ParseIdentifier();
            if (ParseTemplTypeList(false)) {
                GetToken(&t2);
                if (t2.type == ttScope) {
                    // Template type is part of the scope
                    // Nothing more needs to be done

                    return scopes;
                } else {
                    // The template type is not part of the scope
                    // Rewind to the template type and end the scope
                    RewindTo(&t1);
                    return scopes;
                }
            }
        }
    }

    // The identifier is not part of the scope
    RewindTo(&t1);

    return scopes;
}

sToken QAsCodeParser::ParseRealType() {
    sToken t1;

    GetToken(&t1);
    if (!IsRealType(t1.type)) {
        RewindErrorTo(&t1);
    }

    return t1;
}

sToken QAsCodeParser::ParseDataType(bool allowVariableType, bool allowAuto) {
    sToken t1;

    GetToken(&t1);
    if (!IsDataType(t1) && !(allowVariableType && t1.type == ttQuestion) &&
        !(allowAuto && t1.type == ttAuto)) {
        RewindErrorTo(&t1);
        return t1;
    }

    return t1;
}

sToken QAsCodeParser::ParseIdentifier() {
    sToken t1;

    GetToken(&t1);
    if (t1.type != ttIdentifier) {
        RewindErrorTo(&t1);
    }

    return t1;
}

bool QAsCodeParser::ParseTemplTypeList(bool required) {
    sToken t;
    bool isValid = true;

    // Remember the last child, so we can restore the state if needed
    // asCScriptNode *last = node->lastChild;

    // Starts with '<'
    GetToken(&t);
    if (t.type != ttLessThan) {
        if (required) {
            RewindErrorTo(&t);
        }
        return false;
    }

    // At least one type
    // TODO: child funcdef: Make this work with !required
    ParseType(true, false);
    if (isSyntaxError)
        return false;

    GetToken(&t);

    // Parse template types by list separator
    while (t.type == ttListSeparator) {
        // TODO: child funcdef: Make this work with !required
        ParseType(true, false);
        if (isSyntaxError)
            return false;
        GetToken(&t);
    }

    // End with '>'
    // Accept >> and >>> tokens too. But then force the tokenizer to move
    // only 1 character ahead (thus splitting the token in two).
    if (code[QString::size_type(t.pos)] != '>') {
        if (required) {
            RewindErrorTo(&t);
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

void QAsCodeParser::ParseMethodAttributes() {
    sToken t1;

    for (;;) {
        GetToken(&t1);
        RewindTo(&t1);

        if (IdentifierIs(t1, FINAL_TOKEN) || IdentifierIs(t1, OVERRIDE_TOKEN) ||
            IdentifierIs(t1, EXPLICIT_TOKEN) ||
            IdentifierIs(t1, PROPERTY_TOKEN) || IdentifierIs(t1, DELETE_TOKEN))
            ;
        else
            break;
    }
}

void QAsCodeParser::ParseListPattern() {
    sToken t1;

    GetToken(&t1);
    if (t1.type != ttStartStatementBlock) {
        RewindErrorTo(&t1);
        return;
    }

    sToken start = t1;

    bool isBeginning = true;
    bool afterType = false;
    while (!isSyntaxError) {
        GetToken(&t1);
        if (t1.type == ttEndStatementBlock) {
            if (!afterType) {
                RewindErrorTo(&t1);
            }
            break;
        } else if (t1.type == ttStartStatementBlock) {
            if (afterType) {
                RewindErrorTo(&t1);
            }
            RewindTo(&t1);
            ParseListPattern();
            afterType = true;
        } else if (t1.type == ttIdentifier &&
                   (IdentifierIs(t1, "repeat") ||
                    IdentifierIs(t1, "repeat_same"))) {
            if (!isBeginning) {
                RewindErrorTo(&t1);
            }
            RewindTo(&t1);
            ParseIdentifier();
        } else if (t1.type == ttEnd) {
            RewindErrorTo(&start);
            break;
        } else if (t1.type == ttListSeparator) {
            if (!afterType) {
                RewindErrorTo(&t1);
            }
            afterType = false;
        } else {
            if (afterType) {
                RewindErrorTo(&t1);
            }
            RewindTo(&t1);
            // NOTE
            ParseType(true, true);
            afterType = true;
        }

        isBeginning = false;
    }
}

bool QAsCodeParser::IsRealType(int tokenType) {
    if (tokenType == ttVoid || tokenType == ttInt || tokenType == ttInt8 ||
        tokenType == ttInt16 || tokenType == ttInt64 || tokenType == ttUInt ||
        tokenType == ttUInt8 || tokenType == ttUInt16 ||
        tokenType == ttUInt64 || tokenType == ttFloat || tokenType == ttBool ||
        tokenType == ttDouble)
        return true;

    return false;
}

bool QAsCodeParser::IsDataType(const sToken &token) {
    if (token.type == ttIdentifier) {
#ifndef AS_NO_COMPILER
        if (checkValidTypes) {
            // Check if this is an existing type, regardless of namespace
            tempString = getSymbolString(token);
            if (!DoesTypeExist(tempString))
                return false;
        }
#endif
        return true;
    }

    if (IsRealType(token.type))
        return true;

    return false;
}

bool QAsCodeParser::IdentifierIs(const sToken &t, const char *str) {
    if (t.type != ttIdentifier)
        return false;

    return code.mid(t.pos, t.length) == QByteArray(str);
}

void QAsCodeParser::SuperficiallyParseStatementBlock() {
    // This function will only superficially parse the statement block in order
    // to find the end of it
    sToken t1;

    GetToken(&t1);
    if (t1.type != ttStartStatementBlock) {
        RewindErrorTo(&t1);
        return;
    }

    sToken start = t1;

    int level = 1;
    while (level > 0 && !isSyntaxError) {
        GetToken(&t1);
        if (t1.type == ttEndStatementBlock)
            level--;
        else if (t1.type == ttStartStatementBlock)
            level++;
        else if (t1.type == ttNonTerminatedStringConstant) {
            RewindErrorTo(&t1);
            break;
        } else if (t1.type == ttEnd) {
            RewindErrorTo(&start);
            break;
        }
    }
}

sToken QAsCodeParser::SuperficiallyParseVarInit() {
    sToken t;
    GetToken(&t);

    if (t.type == ttAssignment) {
        GetToken(&t);
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
                RewindErrorTo(&t);
                break;
            } else if (t.type == ttEnd) {
                RewindErrorTo(&start);
                break;
            }
            GetToken(&t);
        }

        // Rewind so that the next token read is the list separator, end
        // statement, or end statement block
        RewindTo(&t);
    } else if (t.type == ttOpenParenthesis) {
        sToken start = t;

        // Find the end of the argument list
        int indent = 1;
        while (indent) {
            GetToken(&t);
            if (t.type == ttOpenParenthesis)
                indent++;
            else if (t.type == ttCloseParenthesis)
                indent--;
            else if (t.type == ttNonTerminatedStringConstant) {
                RewindErrorTo(&t);
                break;
            } else if (t.type == ttEnd) {
                RewindErrorTo(&start);
                break;
            }
        }
    } else {
        RewindErrorTo(&t);
    }

    return t;
}

void QAsCodeParser::ParseStatementBlock() {
    sToken t1;

    GetToken(&t1);
    if (t1.type != ttStartStatementBlock) {
        RewindErrorTo(&t1);
        return;
    }

    sToken start = t1;

    for (;;) {
        while (!isSyntaxError) {
            GetToken(&t1);
            if (t1.type == ttEndStatementBlock) {

                // Statement block is finished
                return;
            } else {
                RewindTo(&t1);

                if (IsVarDecl())
                    ParseDeclaration();
                else
                    ParseStatement();
            }
        }

        if (isSyntaxError) {
            // Search for either ';', '{', '}', or end
            GetToken(&t1);
            while (t1.type != ttEndStatement && t1.type != ttEnd &&
                   t1.type != ttStartStatementBlock &&
                   t1.type != ttEndStatementBlock) {
                GetToken(&t1);
            }

            // Skip this statement block
            if (t1.type == ttStartStatementBlock) {
                // Find the end of the block and skip nested blocks
                int level = 1;
                while (level > 0) {
                    GetToken(&t1);
                    if (t1.type == ttStartStatementBlock)
                        level++;
                    if (t1.type == ttEndStatementBlock)
                        level--;
                    if (t1.type == ttEnd)
                        break;
                }
            } else if (t1.type == ttEndStatementBlock) {
                RewindTo(&t1);
            } else if (t1.type == ttEnd) {
                RewindErrorTo(&start);
                return;
            }

            isSyntaxError = false;
        }
    }
    UNREACHABLE_RETURN;
}

void QAsCodeParser::ParseStatement() {
    sToken t1;

    GetToken(&t1);
    RewindTo(&t1);

    if (t1.type == ttIf)
        ParseIf();
    else if (t1.type == ttFor)
        ParseFor();
    else if (t1.type == ttWhile)
        ParseWhile();
    else if (t1.type == ttReturn)
        ParseReturn();
    else if (t1.type == ttStartStatementBlock)
        ParseStatementBlock();
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
        if (IsVarDecl()) {
            RewindErrorTo(&t1);
            return;
        }
        ParseExpressionStatement();
    }
}

void QAsCodeParser::ParseExpressionStatement() {
    sToken t;
    GetToken(&t);
    if (t.type == ttEndStatement) {
        return;
    }

    RewindTo(&t);

    ParseAssignment();
    if (isSyntaxError)
        return;

    GetToken(&t);
    if (t.type != ttEndStatement) {
        RewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseSwitch() {
    sToken t;
    GetToken(&t);
    if (t.type != ttSwitch) {
        RewindErrorTo(&t);
        return;
    }

    GetToken(&t);
    if (t.type != ttOpenParenthesis) {
        RewindErrorTo(&t);
        return;
    }

    ParseAssignment();
    if (isSyntaxError)
        return;

    GetToken(&t);
    if (t.type != ttCloseParenthesis) {
        RewindErrorTo(&t);
        return;
    }

    GetToken(&t);
    if (t.type != ttStartStatementBlock) {
        RewindErrorTo(&t);
        return;
    }

    while (!isSyntaxError) {
        GetToken(&t);

        if (t.type == ttEndStatementBlock)
            break;

        RewindTo(&t);

        if (t.type != ttCase && t.type != ttDefault) {
            RewindErrorTo(&t);
            return;
        }

        ParseCase();
        if (isSyntaxError)
            return;
    }

    if (t.type != ttEndStatementBlock) {
        RewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseCase() {
    sToken t;
    GetToken(&t);
    if (t.type != ttCase && t.type != ttDefault) {
        RewindErrorTo(&t);
        return;
    }

    if (t.type == ttCase) {
        ParseExpression();
    }

    GetToken(&t);
    if (t.type != ttColon) {
        RewindErrorTo(&t);
        return;
    }

    // Parse statements until we find either of }, case, default, and break
    GetToken(&t);
    RewindTo(&t);
    while (t.type != ttCase && t.type != ttDefault &&
           t.type != ttEndStatementBlock && t.type != ttBreak) {
        if (IsVarDecl())
            // Variable declarations are not allowed, but we parse it anyway to
            // give a good error message
            ParseDeclaration();
        else
            ParseStatement();
        if (isSyntaxError)
            return;

        GetToken(&t);
        RewindTo(&t);
    }

    // If the case was ended with a break statement, add it to the node
    if (t.type == ttBreak)
        ParseBreak();
}

void QAsCodeParser::ParseIf() {
    sToken t;
    GetToken(&t);
    if (t.type != ttIf) {
        RewindErrorTo(&t);
        return;
    }

    GetToken(&t);
    if (t.type != ttOpenParenthesis) {
        RewindErrorTo(&t);
        return;
    }

    ParseAssignment();
    if (isSyntaxError)
        return;

    GetToken(&t);
    if (t.type != ttCloseParenthesis) {
        RewindErrorTo(&t);
        return;
    }

    ParseStatement();
    if (isSyntaxError)
        return;

    GetToken(&t);
    if (t.type != ttElse) {
        // No else statement return already
        RewindTo(&t);
        return;
    }

    ParseStatement();
}

void QAsCodeParser::ParseFor() {

    sToken t;
    GetToken(&t);
    if (t.type != ttFor) {
        RewindErrorTo(&t);
        return;
    }

    GetToken(&t);
    if (t.type != ttOpenParenthesis) {
        RewindErrorTo(&t);
        return;
    }

    if (IsVarDecl())
        ParseDeclaration();
    else
        ParseExpressionStatement();
    if (isSyntaxError)
        return;

    ParseExpressionStatement();
    if (isSyntaxError)
        return;

    GetToken(&t);
    if (t.type != ttCloseParenthesis) {
        RewindTo(&t);

        // Parse N increment statements separated by ,
        for (;;) {

            ParseAssignment();
            if (isSyntaxError)
                return;

            GetToken(&t);
            if (t.type == ttListSeparator)
                continue;
            else if (t.type == ttCloseParenthesis)
                break;
            else {
                RewindErrorTo(&t);
                return;
            }
        }
    }

    ParseStatement();
}

void QAsCodeParser::ParseWhile() {
    sToken t;
    GetToken(&t);
    if (t.type != ttWhile) {
        RewindErrorTo(&t);
        return;
    }

    GetToken(&t);
    if (t.type != ttOpenParenthesis) {
        RewindErrorTo(&t);
        return;
    }

    ParseAssignment();
    if (isSyntaxError)
        return;

    GetToken(&t);
    if (t.type != ttCloseParenthesis) {
        RewindErrorTo(&t);
        return;
    }

    ParseStatement();
}

void QAsCodeParser::ParseDoWhile() {

    sToken t;
    GetToken(&t);
    if (t.type != ttDo) {
        RewindErrorTo(&t);
        return;
    }

    ParseStatement();
    if (isSyntaxError)
        return;

    GetToken(&t);
    if (t.type != ttWhile) {
        RewindErrorTo(&t);
        return;
    }

    GetToken(&t);
    if (t.type != ttOpenParenthesis) {
        RewindErrorTo(&t);
        return;
    }

    ParseAssignment();
    if (isSyntaxError)
        return;

    GetToken(&t);
    if (t.type != ttCloseParenthesis) {
        RewindErrorTo(&t);
        return;
    }

    GetToken(&t);
    if (t.type != ttEndStatement) {
        RewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseReturn() {
    sToken t;
    GetToken(&t);
    if (t.type != ttReturn) {
        RewindErrorTo(&t);
        return;
    }

    GetToken(&t);
    if (t.type == ttEndStatement) {

        return;
    }

    RewindTo(&t);

    ParseAssignment();
    if (isSyntaxError)
        return;

    GetToken(&t);
    if (t.type != ttEndStatement) {
        RewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseBreak() {
    sToken t;
    GetToken(&t);
    if (t.type != ttBreak) {
        RewindErrorTo(&t);
        return;
    }

    GetToken(&t);
    if (t.type != ttEndStatement) {
        RewindErrorTo(&t);
    }

    return;
}

void QAsCodeParser::ParseContinue() {
    sToken t;
    GetToken(&t);
    if (t.type != ttContinue) {
        RewindErrorTo(&t);
        return;
    }

    GetToken(&t);
    if (t.type != ttEndStatement) {
        RewindErrorTo(&t);
    }
}

void QAsCodeParser::ParseTryCatch() {
    sToken t;
    GetToken(&t);
    if (t.type != ttTry) {
        RewindErrorTo(&t);
        return;
    }

    ParseStatementBlock();
    if (isSyntaxError)
        return;

    GetToken(&t);
    if (t.type != ttCatch) {
        RewindErrorTo(&t);
        return;
    }

    ParseStatementBlock();
    if (isSyntaxError)
        return;
}

QList<QAsCodeParser::Symbol> QAsCodeParser::ParseDeclaration(bool isClassProp,
                                                             bool isGlobalVar) {

    QList<QAsCodeParser::Symbol> ret;
    Symbol sym;

    sToken t;
    GetToken(&t);
    RewindTo(&t);

    // A class property can be preceeded by private
    if (t.type == ttPrivate && isClassProp) {
        ParseToken(ttPrivate);
        sym.vis = Visiblity::Private;
    } else if (t.type == ttProtected && isClassProp) {
        ParseToken(ttProtected);
        sym.vis = Visiblity::Protected;
    }

    // Parse data type
    auto dt = ParseType(true, false, !isClassProp);
    if (isSyntaxError)
        return ret;

    sym.typeStr = getSymbolString(dt);

    for (;;) {
        // Parse identifier
        auto id = ParseIdentifier();
        if (isSyntaxError)
            return ret;

        sym.name = getSymbolString(id);
        sym.nameInSrc = id.pos;

        if (isClassProp || isGlobalVar) {
            // Only superficially parse the initialization RewindErrorTo for the
            // class property
            GetToken(&t);
            RewindTo(&t);
            if (t.type == ttAssignment || t.type == ttOpenParenthesis) {
                SuperficiallyParseVarInit();
                if (isSyntaxError)
                    return ret;
            }
        } else {
            // If next token is assignment, parse expression
            GetToken(&t);
            if (t.type == ttOpenParenthesis) {
                RewindTo(&t);
                ParseArgList();
                if (isSyntaxError)
                    return ret;
            } else if (t.type == ttAssignment) {
                GetToken(&t);
                RewindTo(&t);
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
                RewindTo(&t);
        }

        // continue if list separator, else terminate with end statement
        GetToken(&t);
        if (t.type == ttListSeparator) {
            sym.type = SymbolType::Variable;
            ret.append(sym);
            continue;
        } else if (t.type == ttEndStatement) {
            sym.type = SymbolType::Variable;
            ret.append(sym);
            return ret;
        } else {
            RewindErrorTo(&t);
            return ret;
        }
    }
    UNREACHABLE_RETURN;
}

QAsCodeParser::Symbol QAsCodeParser::ParseImport() {
    sToken t;
    GetToken(&t);
    if (t.type != ttImport) {
        RewindErrorTo(&t);
        return {};
    }

    auto fn = ParseFunctionDefinition();
    if (isSyntaxError)
        return {};

    GetToken(&t);
    if (t.type != ttIdentifier) {
        RewindErrorTo(&t);
        return {};
    }

    tempString = getSymbolString(t);
    if (tempString != FROM_TOKEN) {
        RewindErrorTo(&t);
        return {};
    }

    GetToken(&t);
    if (t.type != ttStringConstant) {
        RewindErrorTo(&t);
        return {};
    }

    GetToken(&t);
    if (t.type != ttEndStatement) {
        RewindErrorTo(&t);
        return {};
    }

    fn.type = SymbolType::Import;
    return fn;
}

void QAsCodeParser::ParseScript(bool inBlock) {
    // Determine type of node
    for (;;) {
        while (!isSyntaxError) {
            sToken tStart;
            GetToken(&tStart);

            // Optimize by skipping tokens 'shared', 'external', 'final',
            // 'abstract' so they don't have to be checked in every condition
            sToken t1 = tStart;
            while (IdentifierIs(t1, SHARED_TOKEN) ||
                   IdentifierIs(t1, EXTERNAL_TOKEN) ||
                   IdentifierIs(t1, FINAL_TOKEN) ||
                   IdentifierIs(t1, ABSTRACT_TOKEN))
                GetToken(&t1);
            RewindTo(&tStart);

            if (t1.type == ttImport) {
                auto import = ParseImport();
                _symtable.insert(import.nameInSrc, import);
            } else if (t1.type == ttEnum)
                ParseEnumeration(); // Handle enumerations
            else if (t1.type == ttTypedef)
                ParseTypedef(); // Handle primitive typedefs
            else if (t1.type == ttClass)
                ParseClass();
            else if (t1.type == ttMixin)
                ParseMixin();
            else if (t1.type == ttInterface)
                ParseInterface();
            else if (t1.type == ttFuncDef) {
                auto fndef = ParseFuncDef();
                if (fndef.isValid()) {
                    _symtable.insert(t1.pos, fndef);
                }
            } else if (t1.type == ttConst || t1.type == ttScope ||
                       t1.type == ttAuto || IsDataType(t1)) {
                if (IsVirtualPropertyDecl()) {
                    auto vpdl = ParseVirtualPropertyDecl(false, false);
                    if (vpdl.isValid()) {
                        _symtable.insert(vpdl.nameInSrc, vpdl);
                    }
                } else if (IsVarDecl()) {
                    auto dl = ParseDeclaration(false, true);
                    for (auto &item : dl) {
                        if (item.isValid()) {
                            _symtable.insert(item.nameInSrc, item);
                        }
                    }
                } else {
                    auto fn = ParseFunction();
                    if (fn.isValid()) {
                        m_segs.insert(fn.nameInSrc, fn);
                    }
                }
            } else if (t1.type == ttEndStatement) {
                // Ignore a semicolon by itself
                GetToken(&t1);
            } else if (t1.type == ttNamespace)
                ParseNamespace();
            else if (t1.type == ttEnd)
                return;
            else if (inBlock && t1.type == ttEndStatementBlock)
                return;
            else {
                RewindErrorTo(&t1);
            }
        }

        if (isSyntaxError) {
            // Search for either ';' or '{' or end
            sToken t1;
            GetToken(&t1);
            while (t1.type != ttEndStatement && t1.type != ttEnd &&
                   t1.type != ttStartStatementBlock)
                GetToken(&t1);

            if (t1.type == ttStartStatementBlock) {
                // Find the end of the block and skip nested blocks
                int level = 1;
                while (level > 0) {
                    GetToken(&t1);
                    if (t1.type == ttStartStatementBlock)
                        level++;
                    if (t1.type == ttEndStatementBlock)
                        level--;
                    if (t1.type == ttEnd)
                        break;
                }
            }

            isSyntaxError = false;
        }
    }
    UNREACHABLE_RETURN;
}

QByteArray QAsCodeParser::getSymbolString(const sToken &t) {
    return QByteArray(code.data() + t.pos, t.length);
}

QByteArrayList QAsCodeParser::getRealNamespace(const QByteArrayList &ns) {
    if (ns.isEmpty()) {
        return _curns + ns;
    } else {
        return ns;
    }
}

void QAsCodeParser::ParseNamespace() {
    sToken t1;

    GetToken(&t1);
    if (t1.type != ttNamespace) {
        RewindErrorTo(&t1);
    }

    auto iden = ParseIdentifier();
    if (isSyntaxError)
        return;

    GetToken(&t1);
    while (t1.type == ttScope) {

        // NOTE
        auto id = ParseIdentifier();
        if (isSyntaxError)
            return;

        GetToken(&t1);
    }

    if (t1.type != ttStartStatementBlock) {
        RewindErrorTo(&t1);
        return;
    }

    sToken start = t1;

    _curns.append(QByteArray(t1.pos, t1.length));

    ParseScript(true);

    if (!isSyntaxError) {
        GetToken(&t1);
        if (t1.type != ttEndStatementBlock) {
            RewindErrorTo(&start);
            return;
        }
    }
}

QAsCodeParser::CodeSegment QAsCodeParser::ParseFunction(bool isMethod) {
    CodeSegment fn;

    // TODO: Why isn't ParseFunctionDefinition used?
    sToken t1;
    GetToken(&t1);

    auto tstart = t1;

    if (!isMethod) {
        // A global function can be marked as shared and external
        while (t1.type == ttIdentifier) {
            if (IdentifierIs(t1, SHARED_TOKEN) ||
                IdentifierIs(t1, EXTERNAL_TOKEN)) {
                RewindTo(&t1);
                ParseIdentifier();
                if (isSyntaxError)
                    return fn;
            } else
                break;

            GetToken(&t1);
        }
    }

    // A class method can start with 'private' or 'protected'
    if (isMethod && t1.type == ttPrivate) {
        RewindTo(&t1);
        ParseToken(ttPrivate);
        GetToken(&t1);
    } else if (isMethod && t1.type == ttProtected) {
        RewindTo(&t1);
        ParseToken(ttProtected);
        GetToken(&t1);
    }
    if (isSyntaxError)
        return fn;

    // If it is a global function, or a method, except constructor and
    // destructor, then the return type is parsed
    sToken t2;
    GetToken(&t2);
    RewindTo(&t1);
    if (!isMethod || (t1.type != ttBitNot && t2.type != ttOpenParenthesis)) {
        auto id = ParseType(true);
        if (isSyntaxError)
            return fn;
        fn.ret = getSymbolString(id);

        ParseTypeMod(false);
        if (isSyntaxError)
            return fn;
    }

    // If this is a class destructor then it starts with ~, and no return type
    // is declared
    if (isMethod && t1.type == ttBitNot) {
        ParseToken(ttBitNot);
        if (isSyntaxError)
            return fn;
    }

    auto iden = ParseIdentifier();
    if (isSyntaxError)
        return fn;
    fn.name = getSymbolString(iden);
    fn.nameInSrc = iden.pos;

    auto params = ParseParameterList();
    if (isSyntaxError)
        return fn;
    fn.args = params;

    if (isMethod) {
        GetToken(&t1);
        RewindTo(&t1);

        // Is the method a const?
        if (t1.type == ttConst)
            ParseToken(ttConst);
    }

    // TODO: Should support abstract methods, in which case no statement block
    // should be provided
    ParseMethodAttributes();
    if (isSyntaxError)
        return fn;

    // External shared functions must be ended with ';'
    GetToken(&t1);
    RewindTo(&t1);
    if (t1.type == ttEndStatement) {
        ParseToken(ttEndStatement);
        fn.ns = _curns;
        return fn;
    }

    // We should just find the end of the statement block here. The statements
    // will be parsed on request by the compiler once it starts the compilation.
    SuperficiallyParseStatementBlock();
    fn.ns = _curns;
    fn.code = code.mid(t1.pos, sourcePos - t1.pos);
    fn.valid = true;
    return fn;
}

QAsCodeParser::Symbol QAsCodeParser::ParseFuncDef() {
    Symbol sym;
    size_t eoff;

    // Allow keywords 'external' and 'shared' before 'interface'
    sToken t1;
    GetToken(&t1);
    while (IdentifierIs(t1, SHARED_TOKEN) || IdentifierIs(t1, EXTERNAL_TOKEN)) {
        RewindTo(&t1);

        if (isSyntaxError)
            return sym;

        GetToken(&t1);
    }

    if (t1.type != ttFuncDef) {
        RewindErrorTo(&t1);
        return sym;
    }

    ParseType(true);
    if (isSyntaxError)
        return sym;

    ParseTypeMod(false);
    if (isSyntaxError)
        return sym;

    auto iden = ParseIdentifier();
    if (isSyntaxError)
        return sym;
    sym.name = getSymbolString(iden);
    sym.nameInSrc = iden.pos;

    auto args = ParseParameterList();
    if (isSyntaxError)
        return sym;
    sym.content = args;

    GetToken(&t1);
    if (t1.type != ttEndStatement) {
        RewindErrorTo(&t1);
        return sym;
    }

    sym.type = SymbolType::FnDecl;
    return sym;
}

void QAsCodeParser::ParseClass() {
    Symbol clssym;

    sToken t;
    GetToken(&t);

    // Allow the keywords 'shared', 'abstract', 'final', and 'external' before
    // 'class'
    while (IdentifierIs(t, SHARED_TOKEN) || IdentifierIs(t, ABSTRACT_TOKEN) ||
           IdentifierIs(t, FINAL_TOKEN) || IdentifierIs(t, EXTERNAL_TOKEN)) {
        RewindTo(&t);
        ParseIdentifier();
        GetToken(&t);
    }

    if (t.type != ttClass) {
        RewindErrorTo(&t);
        return;
    }

    if (engine->ep.allowImplicitHandleTypes) {
        // Parse 'implicit handle class' construct
        GetToken(&t);

        if (t.type != ttHandle)
            RewindTo(&t);
    }

    auto cls = ParseIdentifier();
    clssym.name = getSymbolString(cls);
    clssym.nameInSrc = cls.pos;

    // External shared declarations are ended with ';'
    GetToken(&t);
    if (t.type == ttEndStatement) {
        RewindTo(&t);
        ParseToken(ttEndStatement);
        return;
    }

    // Optional list of interfaces that are being implemented and classes that
    // are being inherited
    if (t.type == ttColon) {
        // TODO: dont support temperily, later
        ParseOptionalScope();
        ParseIdentifier();
        GetToken(&t);
        while (t.type == ttListSeparator) {
            ParseOptionalScope();
            ParseIdentifier();
            GetToken(&t);
        }
    }

    if (t.type != ttStartStatementBlock) {
        RewindErrorTo(&t);
        return;
    }

    // Parse properties
    GetToken(&t);
    RewindTo(&t);
    while (t.type != ttEndStatementBlock && t.type != ttEnd) {
        // Is it a property or a method?
        if (t.type == ttFuncDef) {
            auto fndef = ParseFuncDef();
            if (fndef.isValid()) {
                clssym.content.append(fndef);
            }
        } else if (IsFuncDecl(true)) {
            auto fn = ParseFunction(true);
            if (fn.isValid()) {
                clssym.codesegs.insert(fn.nameInSrc, fn);
            }
        } else if (IsVirtualPropertyDecl()) {
            auto vp = ParseVirtualPropertyDecl(true, false);
            if (vp.isValid()) {
                clssym.content.append(vp);
            }
        } else if (IsVarDecl()) {
            auto decl = ParseDeclaration(true);
            clssym.content.append(decl);
        } else if (t.type == ttEndStatement)
            // Skip empty declarations
            GetToken(&t);
        else {
            RewindErrorTo(&t);
            return;
        }

        if (isSyntaxError)
            return;

        GetToken(&t);
        RewindTo(&t);
    }

    GetToken(&t);
    if (t.type != ttEndStatementBlock) {
        RewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseMixin() {
    sToken t;
    GetToken(&t);

    if (t.type != ttMixin) {
        RewindErrorTo(&t);
        return;
    }

    // A mixin token must be followed by a class declaration
    ParseClass();
}

void QAsCodeParser::ParseInitList() {
    sToken t1;

    GetToken(&t1);
    if (t1.type != ttStartStatementBlock) {
        RewindErrorTo(&t1);
        return;
    }

    GetToken(&t1);
    if (t1.type == ttEndStatementBlock) {
        // Statement block is finished
        return;
    } else {
        RewindTo(&t1);
        for (;;) {
            GetToken(&t1);
            if (t1.type == ttListSeparator) {
                // No expression

                GetToken(&t1);
                if (t1.type == ttEndStatementBlock) {
                    // No expression
                    return;
                }
                RewindTo(&t1);
            } else if (t1.type == ttEndStatementBlock) {
                // No expression
                // Statement block is finished
                return;
            } else if (t1.type == ttStartStatementBlock) {
                RewindTo(&t1);
                ParseInitList();

                if (isSyntaxError)
                    return;

                GetToken(&t1);
                if (t1.type == ttListSeparator)
                    continue;
                else if (t1.type == ttEndStatementBlock) {

                    // Statement block is finished
                    return;
                } else {
                    RewindErrorTo(&t1);
                    return;
                }
            } else {
                RewindTo(&t1);
                // NOTE
                ParseAssignment();
                if (isSyntaxError)
                    return;

                GetToken(&t1);
                if (t1.type == ttListSeparator)
                    continue;
                else if (t1.type == ttEndStatementBlock) {
                    // Statement block is finished
                    return;
                } else {
                    RewindErrorTo(&t1);
                    return;
                }
            }
        }
    }
    UNREACHABLE_RETURN;
}

void QAsCodeParser::ParseInterface() {
    Symbol sym;
    size_t eoff;
    sToken t;

    // Allow keywords 'external' and 'shared' before 'interface'
    GetToken(&t);
    while (IdentifierIs(t, SHARED_TOKEN) || IdentifierIs(t, EXTERNAL_TOKEN)) {
        RewindTo(&t);
        ParseIdentifier();

        if (isSyntaxError)
            return;

        GetToken(&t);
    }

    if (t.type != ttInterface) {
        RewindErrorTo(&t);
        return;
    }

    auto id = ParseIdentifier();
    sym.name = getSymbolString(id);
    sym.nameInSrc = id.pos;

    // External shared declarations are ended with ';'
    GetToken(&t);
    if (t.type == ttEndStatement) {
        RewindTo(&t);
        ParseToken(ttEndStatement);
        return;
    }

    // Can optionally have a list of interfaces that are inherited
    if (t.type == ttColon) {
        ParseOptionalScope();
        ParseIdentifier();
        GetToken(&t);
        while (t.type == ttListSeparator) {
            ParseOptionalScope();
            ParseIdentifier();
            GetToken(&t);
        }
    }

    if (t.type != ttStartStatementBlock) {
        RewindErrorTo(&t);
        return;
    }

    // Parse interface methods
    GetToken(&t);
    RewindTo(&t);
    while (t.type != ttEndStatementBlock && t.type != ttEnd) {
        if (IsVirtualPropertyDecl()) {
            auto vp = ParseVirtualPropertyDecl(true, true);
            if (vp.isValid()) {
                sym.content.append(vp);
            }
        } else if (t.type == ttEndStatement) {
            // Skip empty declarations
            GetToken(&t);
        } else {
            // Parse the method signature
            auto im = ParseInterfaceMethod();
            if (im.isValid()) {
                sym.content.append(im);
            }
        }

        if (isSyntaxError)
            return;

        GetToken(&t);
        RewindTo(&t);
    }

    GetToken(&t);
    if (t.type != ttEndStatementBlock) {
        RewindErrorTo(&t);
        return;
    }
}

QAsCodeParser::Symbol QAsCodeParser::ParseInterfaceMethod() {
    Symbol sym;

    auto ret = ParseType(true);
    if (isSyntaxError)
        return sym;
    sym.typeStr = getSymbolString(ret);

    ParseTypeMod(false);
    if (isSyntaxError)
        return sym;

    auto id = ParseIdentifier();
    if (isSyntaxError)
        return sym;
    sym.name = getSymbolString(id);
    sym.nameInSrc = id.pos;

    auto args = ParseParameterList();
    if (isSyntaxError)
        return sym;
    sym.content = args;

    // Parse an optional const after the method definition
    sToken t1;
    GetToken(&t1);
    RewindTo(&t1);
    if (t1.type == ttConst)
        ParseToken(ttConst);

    GetToken(&t1);
    if (t1.type != ttEndStatement) {
        RewindErrorTo(&t1);
        return sym;
    }

    sym.type = SymbolType::FnDecl;
    return sym;
}

QAsCodeParser::Symbol
QAsCodeParser::ParseVirtualPropertyDecl(bool isMethod, bool isInterface) {
    Symbol sym;

    sToken t1, t2;
    GetToken(&t1);
    GetToken(&t2);
    RewindTo(&t1);

    // A class method can start with 'private' or 'protected'
    if (isMethod && t1.type == ttPrivate) {
        ParseToken(ttPrivate);
        sym.vis = Visiblity::Private;
    } else if (isMethod && t1.type == ttProtected) {
        ParseToken(ttProtected);
        sym.vis = Visiblity::Protected;
    }

    if (isSyntaxError)
        return sym;

    auto id = ParseType(true);
    if (isSyntaxError)
        return sym;
    sym.typeStr = getSymbolString(id);

    ParseTypeMod(false);
    if (isSyntaxError)
        return sym;

    auto iden = ParseIdentifier();
    if (isSyntaxError)
        return sym;
    sym.name = getSymbolString(iden);
    sym.nameInSrc = iden.pos;

    GetToken(&t1);
    if (t1.type != ttStartStatementBlock) {
        RewindErrorTo(&t1);
        return sym;
    }

    for (;;) {
        GetToken(&t1);

        CodeSegment seg;

        if (IdentifierIs(t1, GET_TOKEN) || IdentifierIs(t1, SET_TOKEN)) {
            RewindTo(&t1);

            auto id = ParseIdentifier();
            auto name = getSymbolString(id);
            seg.name = name;

            if (isMethod) {
                GetToken(&t1);
                RewindTo(&t1);
                if (t1.type == ttConst)
                    ParseToken(ttConst);

                if (!isInterface) {
                    ParseMethodAttributes();
                    if (isSyntaxError)
                        return sym;
                }
            }

            if (!isInterface) {
                GetToken(&t1);
                if (t1.type == ttStartStatementBlock) {
                    RewindTo(&t1);
                    SuperficiallyParseStatementBlock();

                    seg.valid = true;
                    seg.code = code.mid(t1.pos, sourcePos - t1.pos);
                    sym.codesegs.insert(t1.pos, seg);

                    if (isSyntaxError)
                        return sym;
                } else if (t1.type == ttEndStatement) {
                    RewindTo(&t1);
                    ParseToken(ttEndStatement);
                    if (isSyntaxError)
                        return sym;
                } else {
                    RewindErrorTo(&t1);
                    return sym;
                }
            } else {
                GetToken(&t1);
                if (t1.type == ttEndStatement) {
                    RewindTo(&t1);
                    ParseToken(ttEndStatement);
                    if (isSyntaxError)
                        return sym;
                } else {
                    RewindErrorTo(&t1);
                    return sym;
                }
            }
        } else if (t1.type == ttEndStatementBlock)
            break;
        else {
            RewindErrorTo(&t1);
            return sym;
        }
    }

    sym.type = SymbolType::Property;
    return sym;
}

void QAsCodeParser::ParseEnumeration() {
    Symbol sym;
    sToken token;
    size_t eoff;

    // Optional 'shared' and 'external' token
    GetToken(&token);
    while (IdentifierIs(token, SHARED_TOKEN) ||
           IdentifierIs(token, EXTERNAL_TOKEN)) {
        RewindTo(&token);

        ParseIdentifier();

        if (isSyntaxError)
            return;

        GetToken(&token);
    }

    // Check for enum
    if (token.type != ttEnum) {
        RewindErrorTo(&token);
        return;
    }

    // Get the identifier
    GetToken(&token);
    if (ttIdentifier != token.type) {
        RewindErrorTo(&token);
        return;
    }

    // ok, init symbol
    eoff = token.pos;
    sym.name = getSymbolString(token);
    sym.nameInSrc = token.pos;
    sym.type = SymbolType::Enum;
    sym.ns = _curns;

    // External shared declarations are ended with ';'
    GetToken(&token);
    if (token.type == ttEndStatement) {
        RewindTo(&token);
        ParseToken(ttEndStatement);
        return;
    }

    // check for the start of the declaration block
    if (token.type != ttStartStatementBlock) {
        RewindTo(&token);
        RewindErrorTo(&token);
        return;
    }

    while (ttEnd != token.type) {
        GetToken(&token);

        if (ttEndStatementBlock == token.type) {
            RewindTo(&token);
            break;
        }

        if (ttIdentifier != token.type) {
            RewindErrorTo(&token);
            return;
        }

        // Add the enum element
        Symbol se;
        se.type = SymbolType::Value;
        se.name = getSymbolString(token);
        GetToken(&token);
        sym.content.append(se);

        if (token.type == ttAssignment) {
            RewindTo(&token);

            SuperficiallyParseVarInit();

            if (isSyntaxError)
                return;

            GetToken(&token);
        }

        if (ttListSeparator != token.type) {
            RewindTo(&token);
            break;
        }
    }

    // check for the end of the declaration block
    GetToken(&token);
    if (token.type != ttEndStatementBlock) {
        RewindTo(&token);
        RewindErrorTo(&token);
        return;
    }

    _symtable.insert(eoff, sym);
}

void QAsCodeParser::ParseTypedef() {
    Symbol sym;
    size_t eoff;

    sToken token;

    GetToken(&token);
    if (token.type != ttTypedef) {
        RewindErrorTo(&token);
        return;
    }

    eoff = token.pos;

    // Parse the base type
    GetToken(&token);
    RewindTo(&token);

    // Make sure it is a primitive type (except ttVoid)
    if (!IsRealType(token.type) || token.type == ttVoid) {
        RewindErrorTo(&token);
        return;
    }

    auto r = ParseRealType();
    auto i = ParseIdentifier();

    // Check for the end of the typedef
    GetToken(&token);
    if (token.type != ttEndStatement) {
        RewindTo(&token);
        RewindErrorTo(&token);
    }

    sym.name = getSymbolString(i);

    Symbol st;
    st.name = getSymbolString(r);

    sym.content.append(st);
    sym.ns = _curns;

    _symtable.insert(eoff, sym);
}

bool QAsCodeParser::IsVarDecl() {
    // Set start point so that we can rewind
    sToken t;
    GetToken(&t);
    RewindTo(&t);

    // A class property decl can be preceded by 'private' or 'protected'
    sToken t1;
    GetToken(&t1);
    if (t1.type != ttPrivate && t1.type != ttProtected)
        RewindTo(&t1);

    // A variable decl starts with the type
    if (!FindTokenAfterType(t1)) {
        RewindTo(&t);
        return false;
    }

    // Jump to the token after the type
    RewindTo(&t1);
    GetToken(&t1);

    // The declaration needs to have a name
    if (t1.type != ttIdentifier) {
        RewindTo(&t);
        return false;
    }

    // It can be followed by an initialization
    GetToken(&t1);
    if (t1.type == ttEndStatement || t1.type == ttAssignment ||
        t1.type == ttListSeparator) {
        RewindTo(&t);
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
            GetToken(&t1);
        }

        if (t1.type == ttEnd) {
            RewindTo(&t);
            return false;
        } else {
            GetToken(&t1);
            RewindTo(&t);
            if (t1.type == ttStartStatementBlock ||
                t1.type == ttIdentifier || // function decorator
                t1.type == ttEnd)
                return false;
        }

        RewindTo(&t);
        return true;
    }

    RewindTo(&t);
    return false;
}

bool QAsCodeParser::IsVirtualPropertyDecl() { // Set start point so that we can
                                              // rewind
    sToken t;
    GetToken(&t);
    RewindTo(&t);

    // A class property decl can be preceded by 'private' or 'protected'
    sToken t1;
    GetToken(&t1);
    if (t1.type != ttPrivate && t1.type != ttProtected)
        RewindTo(&t1);

    // A variable decl starts with the type
    if (!FindTokenAfterType(t1)) {
        RewindTo(&t);
        return false;
    }

    // Move to the token after the type
    RewindTo(&t1);
    GetToken(&t1);

    // The decl must have an identifier
    if (t1.type != ttIdentifier) {
        RewindTo(&t);
        return false;
    }

    // To be a virtual property it must also have a block for the get/set
    // functions
    GetToken(&t1);
    if (t1.type == ttStartStatementBlock) {
        RewindTo(&t);
        return true;
    }

    RewindTo(&t);
    return false;
}

bool QAsCodeParser::IsFuncDecl(
    bool isMethod) { // Set start point so that we can rewind
    sToken t;
    GetToken(&t);
    RewindTo(&t);

    if (isMethod) {
        // A class method decl can be preceded by 'private' or 'protected'
        sToken t1, t2;
        GetToken(&t1);
        if (t1.type != ttPrivate && t1.type != ttProtected)
            RewindTo(&t1);

        // A class constructor starts with identifier followed by parenthesis
        // A class destructor starts with the ~ token
        GetToken(&t1);
        GetToken(&t2);
        RewindTo(&t1);
        if ((t1.type == ttIdentifier && t2.type == ttOpenParenthesis) ||
            t1.type == ttBitNot) {
            RewindTo(&t);
            return true;
        }
    }

    // A function decl starts with a type
    sToken t1;
    if (!FindTokenAfterType(t1)) {
        RewindTo(&t);
        return false;
    }

    // Move to the token after the type
    RewindTo(&t1);
    GetToken(&t1);

    // There can be an ampersand if the function returns a reference
    if (t1.type == ttAmp) {
        RewindTo(&t);
        return true;
    }

    if (t1.type != ttIdentifier) {
        RewindTo(&t);
        return false;
    }

    GetToken(&t1);
    if (t1.type == ttOpenParenthesis) {
        // If the closing parenthesis is not followed by a
        // statement block then it is not a function.
        // It's possible that there are nested parenthesis due to default
        // arguments so this should be checked for.
        int nest = 0;
        GetToken(&t1);
        while ((nest || t1.type != ttCloseParenthesis) && t1.type != ttEnd) {
            if (t1.type == ttOpenParenthesis)
                nest++;
            if (t1.type == ttCloseParenthesis)
                nest--;

            GetToken(&t1);
        }

        if (t1.type == ttEnd)
            return false;
        else {
            if (isMethod) {
                // A class method can have a 'const' token after the parameter
                // list
                GetToken(&t1);
                if (t1.type != ttConst)
                    RewindTo(&t1);
            }

            // A function may also have any number of additional attributes
            bool hasAttribs = false;
            for (;;) {
                GetToken(&t1);
                if (!IdentifierIs(t1, FINAL_TOKEN) &&
                    !IdentifierIs(t1, OVERRIDE_TOKEN) &&
                    !IdentifierIs(t1, EXPLICIT_TOKEN) &&
                    !IdentifierIs(t1, PROPERTY_TOKEN) &&
                    !IdentifierIs(t1, DELETE_TOKEN)) {
                    RewindTo(&t1);
                    break;
                }
                hasAttribs = true;
            }

            GetToken(&t1);
            RewindTo(&t);

            // If the function has an attribute, e.g. delete, it can be
            // terminated with a ; if no implementation is expected otherwise it
            // must have a statement block
            if ((t1.type == ttEndStatement && hasAttribs) ||
                t1.type == ttStartStatementBlock)
                return true;
        }

        RewindTo(&t);
        return false;
    }

    RewindTo(&t);
    return false;
}

bool QAsCodeParser::IsLambda() {
    bool isLambda = false;
    sToken t;
    GetToken(&t);
    if (t.type == ttIdentifier && IdentifierIs(t, FUNCTION_TOKEN)) {
        sToken t2;
        GetToken(&t2);
        if (t2.type == ttOpenParenthesis) {
            // Skip until )
            while (t2.type != ttCloseParenthesis && t2.type != ttEnd)
                GetToken(&t2);

            // The next token must be a {
            GetToken(&t2);
            if (t2.type == ttStartStatementBlock)
                isLambda = true;
        }
    }

    RewindTo(&t);
    return isLambda;
}

bool QAsCodeParser::IsFunctionCall() {
    sToken s;
    sToken t1, t2;

    GetToken(&s);
    t1 = s;

    // A function call may be prefixed with scope resolution
    if (t1.type == ttScope)
        GetToken(&t1);
    GetToken(&t2);

    while (t1.type == ttIdentifier && t2.type == ttScope) {
        GetToken(&t1);
        GetToken(&t2);
    }

    // A function call starts with an identifier followed by an argument list
    // The parser doesn't have enough RewindErrorTormation about scope to
    // determine if the identifier is a datatype, so even if it happens to be
    // the parser will identify the expression as a function call rather than a
    // construct call. The compiler will sort this out later
    if (t1.type != ttIdentifier) {
        RewindTo(&s);
        return false;
    }

    if (t2.type == ttOpenParenthesis) {
        RewindTo(&s);
        return true;
    }

    RewindTo(&s);
    return false;
}

void QAsCodeParser::ParseAssignment() {
    ParseCondition();
    if (isSyntaxError)
        return;

    sToken t;
    GetToken(&t);
    RewindTo(&t);

    if (IsAssignOperator(t.type)) {
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
    GetToken(&t);
    if (!IsAssignOperator(t.type)) {
        RewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseCondition() {
    ParseExpression();

    if (isSyntaxError)
        return;

    sToken t;
    GetToken(&t);
    if (t.type == ttQuestion) {
        ParseAssignment();
        if (isSyntaxError)
            return;

        GetToken(&t);
        if (t.type != ttColon) {
            RewindErrorTo(&t);
            return;
        }

        ParseAssignment();
        if (isSyntaxError)
            return;
    } else
        RewindTo(&t);
}

void QAsCodeParser::ParseExpression() {
    ParseExprTerm();
    if (isSyntaxError)
        return;

    for (;;) {
        sToken t;
        GetToken(&t);
        RewindTo(&t);

        if (!IsOperator(t.type))
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
    GetToken(&t);
    sToken t2 = t, t3;
    RewindTo(&t);
    if (IsDataType(t2) && CheckTemplateType(t2) && FindTokenAfterType(t2)) {
        // Move to token after the type
        RewindTo(&t2);

        // The next token must be a = followed by a {
        GetToken(&t2);
        GetToken(&t3);
        if (t2.type == ttAssignment && t3.type == ttStartStatementBlock) {
            // It is an initialization, now parse it for real
            RewindTo(&t);
            ParseType(false);
            GetToken(&t2);
            ParseInitList();
            return;
        }
    }
    // Or an anonymous init list, i.e. {...}
    else if (t.type == ttStartStatementBlock) {
        RewindTo(&t);
        ParseInitList();
        return;
    }

    // It wasn't an initialization, so it must be an ordinary expression term
    RewindTo(&t);

    for (;;) {
        GetToken(&t);
        RewindTo(&t);
        if (!IsPreOperator(t.type))
            break;

        ParseExprPreOp();
        if (isSyntaxError)
            return;
    }

    ParseExprValue();
    if (isSyntaxError)
        return;

    for (;;) {
        GetToken(&t);
        RewindTo(&t);
        if (!IsPostOperator(t.type))
            return;

        ParseExprPostOp();
        if (isSyntaxError)
            return;
    }
    UNREACHABLE_RETURN;
}

void QAsCodeParser::ParseExprOperator() {
    sToken t;
    GetToken(&t);
    if (!IsOperator(t.type)) {
        RewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseExprPreOp() {
    sToken t;
    GetToken(&t);
    if (!IsPreOperator(t.type)) {
        RewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseExprPostOp() {
    sToken t;
    GetToken(&t);
    if (!IsPostOperator(t.type)) {
        RewindErrorTo(&t);
        return;
    }

    if (t.type == ttDot) {
        sToken t1, t2;
        GetToken(&t1);
        GetToken(&t2);
        RewindTo(&t1);
        if (t2.type == ttOpenParenthesis)
            ParseFunctionCall();
        else
            ParseIdentifier();
    } else if (t.type == ttOpenBracket) {
        ParseArgList(false);

        GetToken(&t);
        if (t.type != ttCloseBracket) {
            RewindErrorTo(&t);
            return;
        }

    } else if (t.type == ttOpenParenthesis) {
        RewindTo(&t);
        ParseArgList();
    }
}

void QAsCodeParser::ParseExprValue() {
    sToken t1, t2;
    GetToken(&t1);
    GetToken(&t2);
    RewindTo(&t1);

    // 'void' is a special expression that doesn't do anything (normally used
    // for skipping output arguments)
    if (t1.type == ttVoid)
        ParseToken(ttVoid);
    else if (IsRealType(t1.type))
        ParseConstructCall();
    else if (t1.type == ttIdentifier || t1.type == ttScope) {
        // Check if the expression is an anonymous function
        if (IsLambda()) {
            ParseLambda();
        } else {
            // Determine the last identifier in order to check if it is a type
            FindIdentifierAfterScope(t2);
            RewindTo(&t2);

            // Get The token after the identifier to determine if it is an [
            sToken t;
            GetToken(&t);
            GetToken(&t);

            bool isDataType = IsDataType(t2);
            bool isTemplateType = false;
            if (isDataType) {
                // Is this a template type?
                tempString = getSymbolString(t2);
                if (engine->IsTemplateType(tempString))
                    isTemplateType = true;
            }

            GetToken(&t2);

            // Rewind so the real parsing can be done, after deciding what to
            // parse
            RewindTo(&t1);

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
            else if (IsFunctionCall())
                ParseFunctionCall();
            else
                ParseVariableAccess();
        }
    } else if (t1.type == ttCast)
        ParseCast();
    else if (IsConstant(t1.type))
        ParseConstant();
    else if (t1.type == ttOpenParenthesis) {
        GetToken(&t1);
        ParseAssignment();
        if (isSyntaxError)
            return;

        GetToken(&t1);
        if (t1.type != ttCloseParenthesis) {
            RewindErrorTo(&t1);
        }

    } else {
        RewindErrorTo(&t1);
    }
}

void QAsCodeParser::ParseArgList(bool withParenthesis) {
    sToken t1;
    if (withParenthesis) {
        GetToken(&t1);
        if (t1.type != ttOpenParenthesis) {
            RewindErrorTo(&t1);
            return;
        }
    }

    GetToken(&t1);
    if (t1.type == ttCloseParenthesis || t1.type == ttCloseBracket) {
        if (withParenthesis) {
            if (t1.type != ttCloseParenthesis) {

                RewindErrorTo(&t1);
            }
        } else
            RewindTo(&t1);

        // Argument list has ended
        return;
    } else {
        RewindTo(&t1);

        for (;;) {
            // Determine if this is a named argument
            sToken tl, t2;
            GetToken(&tl);
            GetToken(&t2);
            RewindTo(&tl);

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

                ParseIdentifier();
                GetToken(&t2);

                ParseAssignment();
            } else
                ParseAssignment();

            if (isSyntaxError)
                return;

            // Check if list continues
            GetToken(&t1);
            if (t1.type == ttListSeparator)
                continue;
            else {
                if (withParenthesis) {
                    if (t1.type != ttCloseParenthesis) {
                        RewindErrorTo(&t1);
                    }
                } else
                    RewindTo(&t1);

                return;
            }
        }
    }
}

void QAsCodeParser::ParseFunctionCall() {
    // Parse scope prefix
    ParseOptionalScope();

    // Parse the function name followed by the argument list
    ParseIdentifier();
    if (isSyntaxError)
        return;

    ParseArgList();
}

void QAsCodeParser::ParseVariableAccess() {
    // Parse scope prefix
    ParseOptionalScope();

    // Parse the variable name
    ParseIdentifier();
}

void QAsCodeParser::ParseConstructCall() {
    // NOTE

    ParseType(false);
    if (isSyntaxError)
        return;

    ParseArgList();
}

void QAsCodeParser::ParseCast() {
    sToken t1;
    GetToken(&t1);
    if (t1.type != ttCast) {
        RewindErrorTo(&t1);
        return;
    }

    GetToken(&t1);
    if (t1.type != ttLessThan) {
        RewindErrorTo(&t1);
        return;
    }

    // Parse the data type
    ParseType(true);
    if (isSyntaxError)
        return;

    GetToken(&t1);
    if (t1.type != ttGreaterThan) {
        RewindErrorTo(&t1);
        return;
    }

    GetToken(&t1);
    if (t1.type != ttOpenParenthesis) {
        RewindErrorTo(&t1);
        return;
    }

    ParseAssignment();
    if (isSyntaxError)
        return;

    GetToken(&t1);
    if (t1.type != ttCloseParenthesis) {
        RewindErrorTo(&t1);
        return;
    }
}

void QAsCodeParser::ParseConstant() {
    sToken t;
    GetToken(&t);
    if (!IsConstant(t.type)) {
        RewindErrorTo(&t);
        return;
    }

    // We want to gather a list of string constants to concatenate as children
    if (t.type == ttStringConstant || t.type == ttMultilineStringConstant ||
        t.type == ttHeredocStringConstant)
        RewindTo(&t);

    while (t.type == ttStringConstant || t.type == ttMultilineStringConstant ||
           t.type == ttHeredocStringConstant) {
        // NOTE
        ParseStringConstant();

        GetToken(&t);
        RewindTo(&t);
    }
}

void QAsCodeParser::ParseStringConstant() {
    sToken t;
    GetToken(&t);
    if (t.type != ttStringConstant && t.type != ttMultilineStringConstant &&
        t.type != ttHeredocStringConstant) {
        RewindErrorTo(&t);
        return;
    }
}

void QAsCodeParser::ParseLambda() {
    sToken t;
    GetToken(&t);

    if (t.type != ttIdentifier || !IdentifierIs(t, FUNCTION_TOKEN)) {
        RewindErrorTo(&t);
        return;
    }

    GetToken(&t);
    if (t.type != ttOpenParenthesis) {
        RewindErrorTo(&t);
        return;
    }

    GetToken(&t);
    while (t.type != ttCloseParenthesis && t.type != ttEnd) {
        RewindTo(&t);

        // Create node to represent the parameter, the datatype and identifier
        // must be children of the node. If no datatype or identifier is given
        // then the node is still there to be counted as the arg
        // NOTE
        // auto param = CreateNode(snUndefined);
        // node->children().append(param);

        // Skip 'const' for the next check
        sToken t1, t2 = t;
        if (t.type == ttConst)
            GetToken(&t1);

        // Determine the last identifier after scope in order to check if it is
        // a type
        FindIdentifierAfterScope(t2);
        RewindTo(&t);

        // Parse optional type before parameter name
        if (IsDataType(t2) && CheckTemplateType(t2) && FindTokenAfterType(t) &&
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

        GetToken(&t);
        if (t.type == ttListSeparator)
            GetToken(&t);
        else if (t.type != ttCloseParenthesis) {
            RewindErrorTo(&t);
            return;
        }
    }

    if (t.type != ttCloseParenthesis) {
        RewindErrorTo(&t);
        return;
    }

    // We should just find the end of the statement block here. The statements
    // will be parsed on request by the compiler once it starts the compilation.
    SuperficiallyParseStatementBlock();
}

bool QAsCodeParser::FindTokenAfterType(
    sToken &nextToken) { // Set a rewind point
    sToken t, t1;
    GetToken(&t);

    // A type can start with a const
    t1 = t;
    if (t1.type == ttConst)
        GetToken(&t1);

    sToken t2;
    if (t1.type != ttAuto) {
        // The type may be initiated with the scope operator
        if (t1.type == ttScope)
            GetToken(&t1);

        // The type may be preceded with a multilevel scope
        GetToken(&t2);
        while (t1.type == ttIdentifier) {
            if (t2.type == ttScope) {
                GetToken(&t1);
                GetToken(&t2);
                continue;
            } else if (t2.type == ttLessThan) {
                // Template types can also be used as scope identifiers
                RewindTo(&t2);
                if (CheckTemplateType(t1)) {
                    sToken t3;
                    GetToken(&t3);
                    if (t3.type == ttScope) {
                        GetToken(&t1);
                        GetToken(&t2);
                        continue;
                    }
                }
            }

            break;
        }
        RewindTo(&t2);
    }

    // We don't validate if the identifier is an actual declared type at this
    // moment as it may wrongly identify the statement as a non-declaration if
    // the user typed the name incorrectly. The real type is validated in
    // ParseDeclaration where a proper error message can be given.
    if (!IsRealType(t1.type) && t1.type != ttIdentifier && t1.type != ttAuto) {
        RewindTo(&t);
        return false;
    }

    if (!CheckTemplateType(t1)) {
        RewindTo(&t);
        return false;
    }

    // Object handles can be interleaved with the array brackets
    // Even though declaring variables with & is invalid we'll accept
    // it here to give an appropriate error message later
    GetToken(&t2);
    while (t2.type == ttHandle || t2.type == ttAmp ||
           t2.type == ttOpenBracket) {
        if (t2.type == ttHandle) {
            // A handle can optionally be read-only
            sToken t3;
            GetToken(&t3);
            if (t3.type != ttConst)
                RewindTo(&t3);
        } else if (t2.type == ttOpenBracket) {
            GetToken(&t2);
            if (t2.type != ttCloseBracket) {
                RewindTo(&t);
                return false;
            }
        } else if (t2.type == ttAmp) {
            // & can be followed by in, out, or inout
            sToken t3;
            GetToken(&t3);
            if (t3.type != ttIn && t3.type != ttOut && t3.type != ttInOut)
                RewindTo(&t3);
        }

        GetToken(&t2);
    }

    // Return the next token so the caller can jump directly to it if desired
    nextToken = t2;

    // Rewind to start point
    RewindTo(&t);

    return true;
}

bool QAsCodeParser::FindIdentifierAfterScope(sToken &nextToken) {
    sToken t1, t2, t3;

    // Determine the last identifier after scope in order to check if it is a
    // type
    GetToken(&t1);
    GetToken(&t2);
    RewindTo(&t1);

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
    RewindTo(&t3);
    GetToken(&t2);
    while (t3.type == ttIdentifier) {
        t2 = t3;
        GetToken(&t3);
        if (t3.type == ttScope)
            GetToken(&t3);
        else
            break;
    }
    RewindTo(&t1);
    nextToken = t2;

    return true;
}

bool QAsCodeParser::IsConstant(int tokenType) {
    if (tokenType == ttIntConstant || tokenType == ttFloatConstant ||
        tokenType == ttDoubleConstant || tokenType == ttStringConstant ||
        tokenType == ttMultilineStringConstant ||
        tokenType == ttHeredocStringConstant || tokenType == ttTrue ||
        tokenType == ttFalse || tokenType == ttBitsConstant ||
        tokenType == ttNull)
        return true;

    return false;
}

bool QAsCodeParser::IsOperator(int tokenType) {
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

bool QAsCodeParser::IsPreOperator(int tokenType) {
    if (tokenType == ttMinus || tokenType == ttPlus || tokenType == ttNot ||
        tokenType == ttInc || tokenType == ttDec || tokenType == ttBitNot ||
        tokenType == ttHandle)
        return true;
    return false;
}

bool QAsCodeParser::IsPostOperator(int tokenType) {
    if (tokenType == ttInc ||         // post increment
        tokenType == ttDec ||         // post decrement
        tokenType == ttDot ||         // member access
        tokenType == ttOpenBracket || // index operator
        tokenType ==
            ttOpenParenthesis) // argument list for call on function pointer
        return true;
    return false;
}

bool QAsCodeParser::IsAssignOperator(int tokenType) {
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

bool QAsCodeParser::DoesTypeExist(const QString &t) {
    // TODO: don't check
    return true;
}

bool QAsCodeParser::CheckTemplateType(const sToken &t) {
    // Is this a template type?
    tempString = getSymbolString(t);
    auto tstr = tempString;
    if (engine->IsTemplateType(tstr.data())) {
        // If the next token is a < then parse the sub-type too
        sToken t1;
        GetToken(&t1);
        if (t1.type != ttLessThan) {
            RewindTo(&t1);
            return true;
        }

        for (;;) {
            // There might optionally be a 'const'
            GetToken(&t1);
            if (t1.type == ttConst)
                GetToken(&t1);

            // The type may be initiated with the scope operator
            if (t1.type == ttScope)
                GetToken(&t1);

            // There may be multiple levels of scope operators
            sToken t2;
            GetToken(&t2);
            while (t1.type == ttIdentifier && t2.type == ttScope) {
                GetToken(&t1);
                GetToken(&t2);
            }
            RewindTo(&t2);

            // Now there must be a data type
            if (!IsDataType(t1))
                return false;

            if (!CheckTemplateType(t1))
                return false;

            GetToken(&t1);

            // Is it a handle or array?
            while (t1.type == ttHandle || t1.type == ttOpenBracket) {
                if (t1.type == ttOpenBracket) {
                    GetToken(&t1);
                    if (t1.type != ttCloseBracket)
                        return false;
                } else if (t1.type == ttHandle) {
                    // after @ there can be a const
                    GetToken(&t1);
                    if (t1.type != ttConst)
                        RewindTo(&t1);
                }

                GetToken(&t1);
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

sToken QAsCodeParser::ParseToken(int token) {
    sToken t1;

    GetToken(&t1);
    if (t1.type != token) {
        RewindErrorTo(&t1);
    }

    return t1;
}

sToken QAsCodeParser::ParseOneOf(int *tokens, int count) {
    sToken t1;

    GetToken(&t1);
    int n;
    for (n = 0; n < count; n++) {
        if (tokens[n] == t1.type)
            break;
    }
    if (n == count) {
        RewindErrorTo(&t1);
        return t1;
    }

    return t1;
}
