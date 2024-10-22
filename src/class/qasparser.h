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
    QCodeNode *asNode2CodeNode(asCScriptNode *node);

public:
    bool parse(const QString &filename);

private:
    asIScriptEngine *_engine;

    QScopedPointer<asCScriptCode> m_code;
};

#endif // !_QCPP_PARSER_H_
