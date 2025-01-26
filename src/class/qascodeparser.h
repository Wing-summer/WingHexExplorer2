#ifndef QASCODEPARSER_H
#define QASCODEPARSER_H

#include "AngelScript/sdk/angelscript/source/as_scriptnode.h"
#include "class/qcodenode.h"

#include <QString>

class QAsCodeParser {
public:
    QAsCodeParser(asCScriptEngine *engine);
    ~QAsCodeParser();

public:
    enum class SymbolType {
        Invalid,
        Value,     // a common value
        Variable,  // a variable
        Enum,      // an enum
        Class,     // a class type
        Namespace, // a namespace
        Function,  // a function
        TypeDef,   // a typedef
    };

    struct Symbol;

    struct Symbol {
        SymbolType type = SymbolType::Invalid;
        QString name;
        qsizetype typeID;      // if not a Variable, -1
        QList<Symbol> content; // only avaliable for enums and classes

        QStringList ns; // namespaces

        size_t group = 0; // 0 for all
        size_t scope = 0; // 0 for all
    };

    using SymbolTable = QMap<size_t, Symbol>;

public:
    void parse(const QString &codes);

private:
    void ParseScript(bool inBlock);

    QString getSymbolString(const sToken &t);

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
    void ParseImport();
    void ParseEnumeration();
    void ParseTypedef();
    void ParseClass();
    void ParseMixin();
    void ParseInterface();
    void ParseFuncDef();
    void ParseNamespace();
    void ParseReturn();
    void ParseBreak();
    void ParseContinue();
    void ParseTryCatch();
    void ParseIf();
    void ParseLambda();
    void ParseStatement();
    void ParseFunction(bool isMethod = false);
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

    void ParseOptionalScope();

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

    void ParseFunctionDefinition();

    void ParseDeclaration(bool isClassProp = false, bool isGlobalVar = false);

    void ParseInterfaceMethod();

    void ParseStringConstant();

private:
    bool errorWhileParsing;
    bool isSyntaxError;
    bool checkValidTypes;
    bool isParsingAppInterface;

    asCScriptEngine *engine;

    QStringList _curns; // current namespaces
    SymbolTable _symtable;

    QString code;
    QString tempString; // Used for reduzing amount of dynamic allocations

    sToken lastToken;
    size_t sourcePos;
};

#endif // QASCODEPARSER_H
