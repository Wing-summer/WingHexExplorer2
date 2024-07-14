#ifndef SCRIPTMACHINE_H
#define SCRIPTMACHINE_H

#include "../AngelScript/add_on/contextmgr/contextmgr.h"
#include "../AngelScript/add_on/debugger/debugger.h"
#include "../AngelScript/add_on/scriptbuilder/scriptbuilder.h"
#include "../AngelScript/angelscript/include/angelscript.h"
#include <QObject>

class ScriptMachine : public QObject {
    Q_OBJECT
public:
    explicit ScriptMachine(QObject *parent = nullptr);

    virtual ~ScriptMachine();

    bool isRunning() const;

public slots:
    bool runScript(const QString &script, bool isInDebug = false);
    bool compileScript(const QString &script);

private:
    static asIScriptContext *requestContextCallback(asIScriptEngine *engine,
                                                    void *param);
    static void returnContextCallback(asIScriptEngine *engine,
                                      asIScriptContext *ctx, void *param);

    static std::string stringToString(void *obj, int expandMembers,
                                      CDebugger *dbg);

    static std::string arrayToString(void *obj, int expandMembers,
                                     CDebugger *dbg);

    static std::string dictionaryToString(void *obj, int expandMembers,
                                          CDebugger *dbg);

    static std::string dateTimeToString(void *obj, int expandMembers,
                                        CDebugger *dbg);

    static int pragmaCallback(const std::string &pragmaText,
                              CScriptBuilder &builder, void *userParam);

signals:

private:
    asIScriptEngine *_engine = nullptr;
    CDebugger *_debugger = nullptr;
    CContextMgr *_ctxMgr = nullptr;
    QVector<asIScriptContext *> _ctxPool;
};

#endif // SCRIPTMACHINE_H
