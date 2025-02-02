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
#include "class/qcodenode.h"

#include <QString>

// This class is the modification of as_parser.
// You can modified it to support more features.
/** It's a complex thing to fully support AngelScript code intellisense.
 ** I just support basic code completion like local or global
 *  variables/functions.
 ** If you are interested in implement a well-featured intellisense like
 *  Qt creator or Visual Studio, PRs are welcomed !!!
 */

// 这个类基于 as_parser 这个文件进行修改得到。
// 你可以修改该类以支持更多的功能。
/* 完美支持 AngelScript 代码智能提示是一个很复杂的事情。
 * 我只提供基础的局部或全局变量和函数的代码填充。
 * 如果你对实现一个像 Qt creator 或者 Visual Studio 一样功能丰富的智能提示，
 * 欢迎 PR ！！！
 */

class QAsCodeParser {
public:
    QAsCodeParser(asCScriptEngine *engine);
    QAsCodeParser(asIScriptEngine *engine);
    ~QAsCodeParser();

public:
    enum class SymbolType {
        Invalid,
        FnDecl,
        Import,
        Value,     // a common value
        Variable,  // a variable
        Enum,      // an enum
        Class,     // a class type
        Namespace, // a namespace
        Function,  // a function
        TypeDef,   // a typedef
        FnDef,     // a funcdef
        Property,  // a property
    };

    enum class Visiblity { Public, Private, Protected };

    struct Symbol;

    struct CodeSegment {
        bool valid = false;
        QByteArrayList ns;
        QByteArray ret;
        QByteArray name;
        qsizetype nameInSrc = -1;
        QList<Symbol> args;
        QByteArray code;

        bool isValid() const { return valid; }
    };

    struct Symbol {
        SymbolType type = SymbolType::Invalid;
        QString name;
        qsizetype nameInSrc = -1;
        QString typeStr;
        Visiblity vis = Visiblity::Public;
        QList<Symbol> content;

        QByteArrayList ns;                     // namespaces
        QMap<qsizetype, CodeSegment> codesegs; // used in class

        // size_t scope = 0; // 0 for all

        bool isValid() const { return type != SymbolType::Invalid; }
    };

    using SymbolTable = QMap<size_t, Symbol>;

private:
    // QMap< offset , CodeSegment>
    QMap<qsizetype, CodeSegment> m_segs; // global functions

public:
    SymbolTable parse(const QByteArray &codes);

    SymbolTable parseIntell(qsizetype offset, const QByteArray &codes);

private:
    void ParseScript(bool inBlock);

    QByteArray getSymbolString(const sToken &t);

    QByteArrayList getRealNamespace(const QByteArrayList &ns);

    // parse tokens
    sToken ParseIdentifier();
    sToken ParseToken(int token);
    sToken ParseRealType();
    sToken ParseDataType(bool allowVariableType = false,
                         bool allowAuto = false);
    sToken ParseOneOf(int *tokens, int count);
    sToken ParseType(bool allowConst, bool allowVariableType = false,
                     bool allowAuto = false);

    // Statements
    sToken SuperficiallyParseVarInit();

    // parse and get symbols
    Symbol ParseImport();
    void ParseEnumeration();
    void ParseTypedef();
    void ParseClass();
    void ParseMixin();
    void ParseInterface();
    Symbol ParseFuncDef();
    void ParseNamespace();
    void ParseReturn();
    void ParseBreak();
    void ParseContinue();
    void ParseTryCatch();
    void ParseIf();
    void ParseLambda();
    void ParseStatement();
    CodeSegment ParseFunction(bool isMethod = false);
    void ParseExpressionStatement();
    void ParseListPattern();
    void ParseStatementBlock();

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

    Symbol ParseVirtualPropertyDecl(bool isMethod, bool isInterface);
    QList<Symbol> ParseParameterList();

    // parse but not get symbols
    void ParseTypeMod(bool isParam);
    void ParseFunctionCall();
    void SuperficiallyParseStatementBlock();
    void ParseInitList();
    void ParseCast();
    void ParseVariableAccess();
    void ParseConstructCall();
    void ParseConstant();

private:
    void Reset();

    bool IsVarDecl();
    bool IsVirtualPropertyDecl();
    bool IsFuncDecl(bool isMethod);
    bool IsLambda();
    bool IsFunctionCall();

    void GetToken(sToken *token);
    void RewindTo(const sToken *token);
    void RewindErrorTo(sToken *token);
    void SetPos(size_t pos);

    bool IsRealType(int tokenType);
    bool IdentifierIs(const sToken &t, const char *str);
    bool IsDataType(const sToken &token);

private:
    bool CheckTemplateType(const sToken &t);
    bool ParseTemplTypeList(bool required = true);
    void ParseMethodAttributes();

    void ParseArgList(bool withParenthesis = true);

    void SuperficiallyParseExpression();

private:
    bool FindTokenAfterType(sToken &nextToken);
    bool FindIdentifierAfterScope(sToken &nextToken);
    bool IsConstant(int tokenType);
    bool IsOperator(int tokenType);
    bool IsPreOperator(int tokenType);
    bool IsPostOperator(int tokenType);
    bool IsAssignOperator(int tokenType);

    bool DoesTypeExist(const QString &t);

    Symbol ParseFunctionDefinition();

    QList<Symbol> ParseDeclaration(bool isClassProp = false,
                                   bool isGlobalVar = false);

    Symbol ParseInterfaceMethod();

    void ParseStringConstant();

private:
    bool errorWhileParsing;
    bool isSyntaxError;
    bool checkValidTypes;
    bool isParsingAppInterface;

    asCScriptEngine *engine;

    QByteArrayList _curns; // current namespaces
    // size_t _curscope = 0;
    SymbolTable _symtable;

    QByteArray code;
    QByteArray tempString; // Used for reduzing amount of dynamic allocations

    sToken lastToken;
    size_t sourcePos;
};

#endif // QASCODEPARSER_H
