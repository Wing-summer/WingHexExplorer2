#ifndef SCRIPTCONSOLEMACHINE_H
#define SCRIPTCONSOLEMACHINE_H

#include "model/scriptobjmodel.h"
#include "scriptmachine.h"

class ScriptConsoleMachine : public ScriptMachine {
    Q_OBJECT
public:
    explicit ScriptConsoleMachine(std::function<QString()> &getInputFn,
                                  QObject *parent = nullptr);
    virtual ~ScriptConsoleMachine();

    virtual bool executeScript(const QString &script,
                               bool isInDebug = false) override;
    virtual bool executeCode(const QString &code) override;

    ScriptObjModel *model() const;

signals:
    void onClearConsole();

protected:
    virtual bool configureEngine(asIScriptEngine *engine) override;

private:
    bool execString(asIScriptEngine *engine, const QString &code);

private:
    ScriptObjModel *_model = nullptr;

    std::function<void(void)> _clsfn;
};

#endif // SCRIPTCONSOLEMACHINE_H
