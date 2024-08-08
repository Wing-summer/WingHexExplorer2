#ifndef SCRIPTCONSOLEMACHINE_H
#define SCRIPTCONSOLEMACHINE_H

#include "scriptmachine.h"

class ScriptConsoleMachine : public ScriptMachine {
    Q_OBJECT
public:
    explicit ScriptConsoleMachine(std::function<QString()> &getInputFn,
                                  QObject *parent = nullptr);
    virtual ~ScriptConsoleMachine();

    virtual bool executeScript(const QString &script,
                               bool isInDebug = false) override;

protected:
    virtual bool configureEngine(asIScriptEngine *engine) override;

private:
    bool execString(asIScriptEngine *engine, const QString &code);

    void exceptionCallback(asIScriptContext *context);

    QString getCallStack(asIScriptContext *context);

    QString preProcessCode(const QString &code);

private:
    asIScriptContext *_immediateContext = nullptr;
};

#endif // SCRIPTCONSOLEMACHINE_H
