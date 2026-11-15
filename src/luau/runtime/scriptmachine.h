/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include <array>
#include <memory>

#include <QHash>
#include <QObject>
#include <QSet>

#include "luau/luaudebugger.h"
#include "luau/luauinspector.h"
#include "luau/scriptbindingengine.h"
#include "luau/wingluaurequire.h"
#include "luauscheduler.h"
#include "scriptchunk.h"
#include "scriptexecution.h"

struct ScriptRuntimeOptions {
    bool ui = true;
    bool debugger = true;
    bool languageServer = true;
    bool retainContext = false;
};

class PluginIOService;

class ScriptMachine : public QObject, public LuauExecutor {
    Q_OBJECT
    friend class ScriptMachineBindings;
    friend class ScriptExecution;

public:
    // Execution sources; Background tasks have independent contexts.
    enum ConsoleMode : int {
        Console_Begin = 0,
        Interactive, // in a shell
        Scripting,   // in scripting dialog
        Background,  // run codes from other way
        Console_End
    };

    static constexpr auto ConsoleModeCount = Console_End - Console_Begin - 1;

public:
    enum class MessageType { Unknown, Info, Warn, Error, Print, ExecInfo };

    struct MessageInfo {
        ConsoleMode mode = ConsoleMode::Background;
        quint64 taskId = 0;
        QString section;
        qint64 row = -1;
        qint64 col = -1;
        MessageType type = MessageType::Info;
        QString message;
    };

public:
    struct RegCallBacks {
        std::function<QString()> getInputFn;
        std::function<void()> clearFn;
        std::function<void(const ScriptMachine::MessageInfo &)> printMsgFn;
    };

public:
    LuauThread *context(ConsoleMode mode) const;
    LuauThreadData *contextData(ConsoleMode mode) const;
    lua_State *contextState(ConsoleMode mode) const;

    static LuauThreadData *contextData(lua_State *l);

private:
    explicit ScriptMachine();
    Q_DISABLE_COPY_MOVE(ScriptMachine)

    bool
    configureEngine(lua_State *l,
                    WingScriptBind::ScriptBindingEngine::ObjectCodec codec);
    constexpr static int consoleModeIdx(ConsoleMode mode);
    LuauScheduler *createScheduler(LuauThread *context);

    inline bool isExecuting() const override { return m_executing; }

    int resumeThread(LuauThread *thread, int arguments) override;

public:
    static ScriptMachine &instance();
    void destoryMachine();

    QVector<lua_State *> getThreadAncestors(lua_State *L) const;

public:
    inline void setInterruptionCheck(std::function<bool()> check) {
        m_interruptionCheck = std::move(check);
    }

    inline void
    setCallbackFailureHandler(std::function<void(const QString &)> handler) {
        m_callbackFailure = std::move(handler);
    }

    bool init(WingScriptBind::ScriptBindingEngine::ObjectCodec codec = {},
              ScriptRuntimeOptions options = {});
    void finishScriptRegistration();
    bool scriptRegistrationFinished() const noexcept;
    bool isInited() const;
    WingScriptBind::IScriptEngine *scriptEngine() const noexcept;
    bool isRunning(ConsoleMode mode) const;

    void registerCallBack(ConsoleMode mode, const RegCallBacks &callbacks);

public:
    LuauDebugger *debugger() const;

    void outputMessage(const MessageInfo &info);

public:
    // debug or release?
    bool isDebugMode(ConsoleMode mode = Scripting);

public:
    // @return true if execution finished, else need more codes input
    void executeCode(ScriptMachine::ConsoleMode mode, const QString &code,
                     const std::function<void(bool)> &onFinished);

    enum class ExecutionResult {
        Success,
        Busy,
        FileError,
        Failed,
        Aborted,
        Capacity
    };
    static constexpr int MaxBackgroundTasks = 16;

    struct BackgroundCreationResult {
        ExecutionResult result = ExecutionResult::Failed;
        ScriptExecution *execution = nullptr;
    };

    BackgroundCreationResult
    createBackgroundExecution(const QString &path,
                              ScriptExecution::Lifetime lifetime);
    int activeBackgroundTasks() const;
    ScriptExecution *backgroundTask(quint64 taskId) const;
    void abortBackgroundTask(quint64 taskId);
    void setBackgroundInputHandler(std::function<QString(quint64)> handler);
    void executeScriptWithResult(
        ConsoleMode mode, const QString &fileName, bool isInDebug,
        const std::function<void(ExecutionResult)> &onFinished);

    ScriptExecution *createExecution(const QString &path);
    bool registerBuiltin(const QString &name,
                         const std::function<void(lua_State *)> &push);

    // only scripting mode can be debugged
    void executeScript(ScriptMachine::ConsoleMode mode, const QString &fileName,
                       bool isInDebug,
                       const std::function<void(bool)> &onFinished);

    void executeChunk(ConsoleMode mode, const QByteArray &bytes,
                      const QString &name, ScriptChunkFormat format,
                      bool isInDebug,
                      const std::function<void(bool)> &onFinished);

    void abortDbgScript();
    void abortScript(ScriptMachine::ConsoleMode mode);

private:
    void observeBackgroundCompletion(
        ScriptExecution *execution,
        const std::function<void(ExecutionResult)> &onFinished);
    static int __output(MessageType type, lua_State *L);
    static int __outputln(MessageType type, lua_State *L);
    static int __outputsep(MessageType type, lua_State *L, QChar sep);

    static int print(lua_State *L);
    static int println(lua_State *L);

    static int warnprint(lua_State *L);
    static int warnprintln(lua_State *L);

    static int errprint(lua_State *L);
    static int errprintln(lua_State *L);

    static int infoprint(lua_State *L);
    static int infoprintln(lua_State *L);

    static int cowrap(lua_State *L);
    static int coresume(lua_State *L);
    static int forward(lua_State *L, int index);
    static int forwardContinuation(lua_State *L, int status);

    static int input(lua_State *L);
    static int stringify(lua_State *L);
    static int assertX(lua_State *L);
    static int debugBreak(lua_State *L);
    static int debugElapsedTime(lua_State *L);

signals:
    void sigOnInputCancelled(ScriptMachine::ConsoleMode mode);
    void sigOnBackgroundInputCancelled(quint64 taskId);
    void sigOnBackgroundTaskStateChanged(quint64 taskId,
                                         ScriptExecution::State state);
    void sigOnBackgroundTasksChanged();

private:
    static void onLuauInterrupt(lua_State *L, int gc);
    static void onLuauThreadCreated(lua_State *lp, lua_State *L);

private:
    void outputError(ConsoleMode mode, const QString &message,
                     const QString &section = {});
    void executeChunkWithResult(
        ConsoleMode mode, const QByteArray &bytes, const QString &name,
        ScriptChunkFormat format, bool debug,
        const std::function<void(ExecutionResult)> &finished);

    struct ScriptLoadResult {
        ExecutionResult result = ExecutionResult::Failed;
        QByteArray source;
        QString message;
    };

    static ScriptLoadResult
    loadChunk(lua_State *thread, const QByteArray &bytes, const QString &path,
              ScriptChunkFormat format, ConsoleMode mode, bool debug,
              bool native);
    static ScriptLoadResult loadScript(lua_State *thread, const QString &path,
                                       ConsoleMode mode, bool debug,
                                       bool native);

private:
    std::array<std::unique_ptr<LuauScheduler>, ConsoleModeCount>
        m_consoleSchedulers;
    QHash<const void *, ScriptExecution *> m_executions;
    QSet<QByteArray> m_consoleModuleCacheKeys;
    quint64 m_nextBackgroundTaskId = 0;
    std::function<QString(quint64)> m_backgroundInput;
    bool m_executing = false;
    bool m_inited = false;
    bool m_scriptRegistrationFinished = false;
    lua_State *m_main = nullptr;
    std::unique_ptr<WingScriptBind::ScriptBindingEngine> m_bindings;
    std::unique_ptr<PluginIOService> m_pluginIoService;
    LuauDebugger *m_debugger = nullptr;
    mutable LuauThread m_ctx[ConsoleModeCount]{};
    mutable LuauThreadData m_tdata[ConsoleModeCount]{};

    WingLuauRequire m_luaReq;

    inline static InspectOptions m_printOptions;
    ScriptRuntimeOptions m_runtimeOptions;
    std::function<bool()> m_interruptionCheck;
    std::function<void(const QString &)> m_callbackFailure;
    QVector<RegCallBacks> m_regcalls;
};

Q_DECLARE_METATYPE(ScriptMachine::MessageInfo)

#endif // SCRIPTMACHINE_H
