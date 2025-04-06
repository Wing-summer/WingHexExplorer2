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

#ifndef QASCODEPARSER_H
#define QASCODEPARSER_H

#include "AngelScript/sdk/angelscript/source/as_scriptnode.h"

#include <QList>
#include <QMap>
#include <QString>

// This class is the modification of as_parser.
// You can modified it to support more features.
/** It's a complex thing to fully support AngelScript code intellisense.
 ** I just support basic code completion.
 ** If you are interested in implement a well-featured intellisense like
 *  Qt creator or Visual Studio, PRs are welcomed !!!
 */

class QAsCodeParser {
public:
    QAsCodeParser(asCScriptEngine *engine);
    QAsCodeParser(asIScriptEngine *engine);
    ~QAsCodeParser();

public:
    enum class SymbolType {
        Invalid,
        Variable, // variable or property in class
        Enum,     // an enum
        Class,    // a class type
        Function, // a function
        TypeDef,  // a typedef
        FnDef,    // a funcdef
        VarsDecl, // global variable decalration
    };

    enum class Visiblity { Public, Private, Protected };

    /**
     * @brief The CodeSegment class
     */
    struct CodeSegment {
        QByteArrayList scope;
        QString name;
        SymbolType type = QAsCodeParser::SymbolType::Invalid;
        int offset = -1;
        QString codes;

        QByteArrayList additonalInfos; // for other additonal infos

    public:
        bool isValid() const { return type != SymbolType::Invalid; }
        bool hasCodes() const { return !codes.isEmpty(); }
    };

    struct Symbol {
        SymbolType symtype = QAsCodeParser::SymbolType::Invalid;
        QByteArrayList scope;
        QString name;
        QByteArray type;
        int offset = -1;
        Visiblity vis = QAsCodeParser::Visiblity::Public;
        QByteArray additonalInfo; // for other additonal info

        QList<Symbol> children;
    };

public:
    // First, we should parse and split the code into segments
    QList<QAsCodeParser::CodeSegment> parse(const QByteArray &codes);

    // Then, we can deep parsing for code completion
    QList<Symbol> parseIntell(qsizetype offset,
                              const QList<QAsCodeParser::CodeSegment> &segs);

    // so, a helper function?
    QList<Symbol> parseAndIntell(qsizetype offset, const QByteArray &codes);

private:
    QList<QAsCodeParser::CodeSegment> parseScript(bool inBlock);

    void appendValidSeg(QList<QAsCodeParser::CodeSegment> &container,
                        QAsCodeParser::CodeSegment seg);

    void skipCodeBlock();
    QByteArray getSymbolString(const sToken &t);
    QByteArrayList getRealNamespace(const QByteArrayList &ns);

    // Statements
    sToken superficiallyParseVarInit();
    sToken superficiallyParseStatementBlock();
    void parseMethodAttributes();

private:
    CodeSegment parseEnumeration();
    CodeSegment parseTypedef();
    CodeSegment parseClass();
    CodeSegment parseMixin();
    CodeSegment parseInterface();
    CodeSegment parseFuncDef();
    CodeSegment parseFunction();

private:
    // parse tokens
    sToken parseIdentifier();
    sToken parseToken(int token);
    sToken parseRealType();
    sToken parseDataType(bool allowVariableType = false,
                         bool allowAuto = false);
    sToken parseOneOf(int *tokens, int count);
    sToken parseType(bool allowConst, bool allowVariableType = false,
                     bool allowAuto = false);

    void parseNamespace();

private:
    // deep parsing
    void parseStatementBlock();
    void parseStatement();

private:
    void parseEnumerationContent();
    void parseTypedefContent();
    void parseClassContent();
    void parseMixinContent();
    void parseInterfaceContent();
    Symbol parseFuncDefContent();
    void ParseReturn();
    void ParseBreak();
    void ParseContinue();
    void ParseTryCatch();
    void ParseIf();
    void ParseLambda();
    CodeSegment parseFunction(bool isMethod);
    void ParseExpressionStatement();
    void ParseListPattern();

    void ParseAssignment();
    void ParseAssignOperator();
    void ParseSwitch();
    void ParseCase();
    void ParseFor();
    void ParseWhile();
    void ParseDoWhile();
    void ParseCondition();
    void ParseExpression();
    void ParseExprTerm();
    void ParseExprOperator();
    void ParseExprPreOp();
    void ParseExprPostOp();
    void ParseExprValue();

    QByteArrayList ParseOptionalScope();

    Symbol parseVirtualPropertyDecl(bool isMethod, bool isInterface);
    QList<Symbol> parseParameterListContent();

    // parse but not get symbols
    void ParseTypeMod(bool isParam);
    void ParseFunctionCall();
    void ParseInitList();
    void ParseCast();
    void ParseVariableAccess();
    void ParseConstructCall();
    void ParseConstant();

private:
    void reset();

    bool isVarDecl();
    bool isVirtualPropertyDecl();
    bool isFuncDecl(bool isMethod);
    bool isLambda();
    bool isFunctionCall();

    void getToken(sToken *token);
    void rewindTo(const sToken *token);
    void rewindErrorTo(sToken *token);
    void SetPos(size_t pos);

    bool isRealType(int tokenType);
    bool identifierIs(const sToken &t, const char *str);
    bool isDataType(const sToken &token);

private:
    bool checkTemplateType(const sToken &t);
    bool parseTemplTypeList(bool required = true);

    void parseArgList(bool withParenthesis = true);

    void superficiallyParseExpression();

private:
    bool findTokenAfterType(sToken &nextToken);
    bool findIdentifierAfterScope(sToken &nextToken);
    bool isConstant(int tokenType);
    bool isOperator(int tokenType);
    bool isPreOperator(int tokenType);
    bool isPostOperator(int tokenType);
    bool isAssignOperator(int tokenType);

    bool typeExist(const QString &t);

    Symbol parseFunctionDefinition();

    QList<Symbol> parseDeclaration(bool isClassProp = false,
                                   bool isGlobalVar = false);

    Symbol parseInterfaceMethod();

    void ParseStringConstant();

private:
    bool errorWhileParsing;
    bool isSyntaxError;
    bool checkValidTypes;
    bool isParsingAppInterface;

    asCScriptEngine *engine;

    // size_t _curscope = 0;

    QByteArray code;
    QByteArray tempString; // Used for reduzing amount of dynamic allocations

    sToken lastToken;
    size_t sourcePos;

    QByteArrayList currentNs;
};

#endif // QASCODEPARSER_H
