/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#ifndef SCRIPTMACHINE_H
#define SCRIPTMACHINE_H

#include "AngelScript/sdk/add_on/contextmgr/contextmgr.h"
#include "AngelScript/sdk/angelscript/include/angelscript.h"

#include "as-debugger/as_debugger.h"
#include "class/aspreprocesser.h"

#include "asdebugger.h"

#include <QObject>
#include <QQueue>

class CScriptArray;

class ScriptMachine {
    Q_GADGET
public:
    // we have three console modes
    enum ConsoleMode {
        Interactive = 1, // in a shell
        Scripting,       // in scripting dialog
        Background,      // run codes from other way
    };

public:
    enum class MessageType { Info, Warn, Error, Print };

    struct MessageInfo {
        ConsoleMode mode = ConsoleMode::Background;
        QString section;
        qint64 row = -1;
        qint64 col = -1;
        MessageType type = MessageType::Info;
        QString message;
    };

public:
    // only for refection
    enum class asEContextState {
        asEXECUTION_FINISHED = ::asEContextState::asEXECUTION_FINISHED,
        asEXECUTION_SUSPENDED = ::asEContextState::asEXECUTION_SUSPENDED,
        asEXECUTION_ABORTED = ::asEContextState::asEXECUTION_ABORTED,
        asEXECUTION_EXCEPTION = ::asEContextState::asEXECUTION_EXCEPTION,
        asEXECUTION_PREPARED = ::asEContextState::asEXECUTION_PREPARED,
        asEXECUTION_UNINITIALIZED =
            ::asEContextState::asEXECUTION_UNINITIALIZED,
        asEXECUTION_ACTIVE = ::asEContextState::asEXECUTION_ACTIVE,
        asEXECUTION_ERROR = ::asEContextState::asEXECUTION_ERROR,
        asEXECUTION_DESERIALIZATION =
            ::asEContextState::asEXECUTION_DESERIALIZATION
    };
    Q_ENUM(asEContextState)

public:
    struct RegCallBacks {
        std::function<QString()> getInputFn;
        std::function<void()> clearFn;
        std::function<void(const ScriptMachine::MessageInfo &)> printMsgFn;
    };

private:
    explicit ScriptMachine();
    Q_DISABLE_COPY_MOVE(ScriptMachine)

private:
    asIScriptModule *createModule(ConsoleMode mode);
    asIScriptModule *createModuleIfNotExist(ConsoleMode mode);
    bool isModuleExists(ConsoleMode mode);

public:
    asIScriptModule *module(ConsoleMode mode);

    static ScriptMachine &instance();
    void destoryMachine();

public:
    bool init();
    bool isInited() const;
    bool isRunning(ConsoleMode mode) const;
    bool isEngineConfigError() const;

    static void registerEngineAddon(asIScriptEngine *engine);
    static void registerEngineAssert(asIScriptEngine *engine);
    static void registerEngineClipboard(asIScriptEngine *engine);
    static void registerEngineDebug(asIScriptEngine *engine);

    void registerCallBack(ConsoleMode mode, const RegCallBacks &callbacks);

public:
    asDebugger *debugger() const;

    asIScriptEngine *engine() const;

    void outputMessage(const MessageInfo &info);

public:
    static void scriptAssert(bool b);
    static void scriptAssert_X(bool b, const QString &msg);

    static void clip_setText(const QString &text);
    static void clip_setBinary(const CScriptArray &array);
    static QString clip_getText();
    static CScriptArray *clip_getBinary();

    static void scriptThrow(const QString &msg);

    static QString scriptGetExceptionInfo();

    static void registerExceptionRoutines(asIScriptEngine *engine);

public:
    // debug or release?
    bool isDebugMode(ConsoleMode mode = Scripting);

public:
    bool executeCode(ScriptMachine::ConsoleMode mode, const QString &code);
    // only scripting mode can be debugged
    bool executeScript(
        ScriptMachine::ConsoleMode mode, const QString &script,
        bool isInDebug = false, int *retCode = nullptr,
        std::function<void(const QHash<QString, AsPreprocesser::Result> &)>
            sections = {});

    QVariant evaluateDefine(const QString &code);

    void abortDbgScript();
    void abortScript(ScriptMachine::ConsoleMode mode);
    void abortScript();

protected:
    bool configureEngine();
    void beginEvaluateDefine();
    void endEvaluateDefine();

    QString getCallStack(asIScriptContext *context);

private:
    static void print(asIScriptGeneric *args);
    static void println(asIScriptGeneric *args);

    QString getInput();

    static int execSystemCmd(QString &out, const QString &exe,
                             const QString &params, int timeout);

    static QString beautify(const QString &str, uint indent);

    QString stringify(void *ref, int typeId);
    QString stringify_helper(const std::shared_ptr<asIDBVariable> &var);

private:
    static void messageCallback(const asSMessageInfo *msg, void *param);

    static void cleanUpPluginSysIDFunction(asIScriptFunction *fn);

    static asIScriptContext *requestContextCallback(asIScriptEngine *engine,
                                                    void *param);
    static void returnContextCallback(asIScriptEngine *engine,
                                      asIScriptContext *ctx, void *param);

    static std::optional<WingHex::PragmaResult>
    pragmaCallback(const QString &pragmaText, AsPreprocesser *builder,
                   const QString &sectionname);

    static void debug_break();

    static QString debug_backtrace();

    void exceptionCallback(asIScriptContext *context);

private:
    void attachDebugBreak(asIScriptContext *ctx);

private:
    asIScriptEngine *_engine = nullptr;
    CContextMgr *_ctxMgr = nullptr;
    asIScriptModule *_eMod = nullptr;

    QQueue<asIScriptContext *> _ctxPool;

    QMap<ConsoleMode, RegCallBacks> _regcalls;
    QMap<ConsoleMode, asIScriptContext *> _ctx;
    ConsoleMode _curMsgMode = ConsoleMode::Background;

    qint64 lineOffset = 0;
    qint64 colOffset = 0;

private:
    void checkDebugger(asIScriptContext *ctx);

private:
    asDebugger *_debugger = nullptr;
    asIDBWorkspace *_workspace = nullptr;
};

Q_DECLARE_METATYPE(ScriptMachine::MessageInfo)

#endif // SCRIPTMACHINE_H
