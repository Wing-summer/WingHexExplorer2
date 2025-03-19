#include "asconsolecompletion.h"

#include "control/scriptingconsole.h"

AsConsoleCompletion::AsConsoleCompletion(asIScriptEngine *engine,
                                         ScriptingConsole *p)
    : AsCompletion(engine, p), _console(p) {}

QList<CodeInfoTip> AsConsoleCompletion::parseDocument() {
    // TODO
    return {};
}
