#include "scriptconsolemachine.h"

#include "AngelScript/add_on/scripthelper/scripthelper.h"
#include "src/plugin/pluginsystem.h"

ScriptConsoleMachine::ScriptConsoleMachine(
    std::function<QString(void)> &getInputFn, QObject *parent)
    : ScriptMachine(getInputFn, parent) {
    if (!configureEngine(_engine)) {
        _engine->ShutDownAndRelease();
        _engine = nullptr;
    }
}

ScriptConsoleMachine::~ScriptConsoleMachine() {
    if (_immediateContext) {
        _immediateContext->Release();
        _immediateContext = nullptr;
    }
}

bool ScriptConsoleMachine::executeScript(const QString &script,
                                         bool isInDebug) {
    Q_UNUSED(isInDebug);
    return execString(_engine, script);
}

bool ScriptConsoleMachine::configureEngine(asIScriptEngine *engine) {
    // Immediate execution script context for console
    _immediateContext = engine->CreateContext();
    _immediateContext->SetExceptionCallback(
        asMETHOD(ScriptConsoleMachine, exceptionCallback), this,
        asCALL_THISCALL);
    PluginSystem::instance().angelApi()->installAPI(engine);
    return _immediateContext != nullptr;
}

bool ScriptConsoleMachine::execString(asIScriptEngine *engine,
                                      const QString &code) {
    return ExecuteString(_engine, preProcessCode(code).toUtf8(),
                         engine->GetModule("Console", asGM_ALWAYS_CREATE),
                         _immediateContext) >= 0;
}

void ScriptConsoleMachine::exceptionCallback(asIScriptContext *context) {
    QString message =
        tr("- Exception '%1' in '%2'\n")
            .arg(context->GetExceptionString(),
                 context->GetExceptionFunction()->GetDeclaration()) +
        getCallStack(context);

    const char *section;
    MessageInfo msg;
    msg.row = context->GetExceptionLineNumber(&msg.col, &section);
    msg.type = asMSGTYPE_ERROR;
    msg.message = message;
    emit onOutput(MessageType::Error, msg);
}

QString ScriptConsoleMachine::getCallStack(asIScriptContext *context) {
    QString str = tr("AngelScript callstack:\n");

    // Append the call stack
    for (asUINT i = 0; i < context->GetCallstackSize(); i++) {
        asIScriptFunction *func;
        const char *scriptSection;
        int line, column;
        func = context->GetFunction(i);
        line = context->GetLineNumber(i, &column, &scriptSection);
        str += (QStringLiteral("\t") + scriptSection + QStringLiteral(":") +
                func->GetDeclaration() + QStringLiteral(":") +
                QString::number(line) + QStringLiteral(",") +
                QString::number(column) + QStringLiteral("\n"));
    }

    return str;
}

QString ScriptConsoleMachine::preProcessCode(const QString &code) {
    if (code.startsWith(QStringLiteral("addvar "))) {

    } else if (code.startsWith(QStringLiteral("addfn"))) {

    } else if (code.startsWith(QStringLiteral("rmvar"))) {

    } else if (code.startsWith(QStringLiteral("rmfn"))) {

    } else if (code.startsWith(QStringLiteral("lsfn"))) {

    } else if (code.startsWith(QStringLiteral("lsvar"))) {

    } else {
    }

    return code;
}
