#ifndef _QAS_PARSER_H_
#define _QAS_PARSER_H_

#include "class/asbuilder.h"

#include <QScopedPointer>

class asCScriptCode;
class asCScriptNode;
class QCodeNode;

class QAsParser : protected asBuilder {
public:
    explicit QAsParser(asIScriptEngine *engine);
    virtual ~QAsParser();

private:
    void processNode(asCScriptCode *code, asCScriptNode *raw, QCodeNode *node);

public:
    bool parse(const QString &filename);
    bool parse(const QString &code, const QString &section);

    QList<QCodeNode *> codeNodes() const;

private:
    asIScriptEngine *_engine;
    QScopedPointer<asCScriptCode> m_code;
    QList<QCodeNode *> _nodes;
};

// I don't know why it doesn't pass the compiler under Windows when
// these codes are in the qasparser.cpp
inline QList<QCodeNode *> QAsParser::codeNodes() const { return _nodes; }

#endif // !_QCPP_PARSER_H_
