#ifndef ASCONSOLECOMPLETION_H
#define ASCONSOLECOMPLETION_H

#include "ascompletion.h"

class ScriptingConsole;

class AsConsoleCompletion : public AsCompletion {
    Q_OBJECT
public:
    explicit AsConsoleCompletion(asIScriptEngine *engine, ScriptingConsole *p);
    virtual ~AsConsoleCompletion() = default;

    // AsCompletion interface
protected:
    virtual QList<CodeInfoTip> parseDocument() override;

private:
    ScriptingConsole *_console;
};

#endif // ASCONSOLECOMPLETION_H
