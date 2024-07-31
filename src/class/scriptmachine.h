#ifndef SCRIPTMACHINE_H
#define SCRIPTMACHINE_H

#include "AngelScript/add_on/contextmgr/contextmgr.h"
#include "AngelScript/add_on/scriptarray/scriptarray.h"
#include "AngelScript/add_on/scriptbuilder/scriptbuilder.h"
#include "AngelScript/angelscript/include/angelscript.h"

#include "asdebugger.h"

#include <QObject>

class ScriptMachine : public QObject {
    Q_OBJECT
public:
    enum class MessageType { Info, Warn, Error };

public:
    explicit ScriptMachine(std::function<std::string(void)> &getInputFn,
                           QObject *parent = nullptr);

    virtual ~ScriptMachine();

    bool inited();

    bool isRunning() const;

public slots:
    bool executeScript(const QString &script, const QStringList &params,
                       bool isInDebug = false);

private:
    bool configureEngine(asIScriptEngine *engine);
    bool compileScript(const QString &script);

private:
    CScriptArray *getCommandLineArgs();

    void printString(const std::string &str);

    std::string getInput();

    static int execSystemCmd(const std::string &exe, const std::string &params,
                             std::string &out);

    static int execSystemCmd(const std::string &exe, const std::string &params);

private:
    static void messageCallback(const asSMessageInfo *msg, void *param);

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
    void onOutputString(MessageType type, const QStringList &messages);

private:
    asIScriptEngine *_engine = nullptr;
    asDebugger *_debugger = nullptr;
    CContextMgr *_ctxMgr = nullptr;
    QVector<asIScriptContext *> _ctxPool;
    CScriptArray *_commandLineArgs = nullptr;
    QStringList _params;

    std::function<void(void)> _getcmdLineArgsFn;
    std::function<void(const std::string &)> _printStringFn;
    std::function<std::string(void)> _getInputFn;
};

#endif // SCRIPTMACHINE_H
