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

#include "scriptmachine.h"

#include "class/application/startuptrace.h"
#include SCRIPTMACHINE_LUAU_BINDING

#include <utility>

#include <QDebug>
#include <QScopeGuard>
#include <QScopedValueRollback>
#include <QSet>

#include "lua.h"
#include "lualib.h"

#include "Luau/CodeGen.h"
#include "Luau/Common.h"
#include "Luau/Compiler.h"
#include "luau/CodeGen/include/Luau/CodeGenOptions.h"

#include "LuaBridge/LuaBridge.h"

#include "class/application/applicationclock.h"
#include "class/application/applicationmode.h"
#include "class/plugin/pluginsystem.h"
#include "class/settings/settingmanager.h"
#include "define.h"
#include "luau/lsp/luaulanguageserver.h"
#include "luau/luauinspector.h"
#include "luau/luauutil.h"
#include "luau/wingluaurequire.h"
#include "scriptexecution.h"
#include "utilities.h"

extern "C" {
int luaopen_cffi(lua_State *L);
}

LUAU_FASTFLAG(LuauCyclicRequireShortCircuit)

namespace {

class DbgStackPusher {
public:
    DbgStackPusher(lua_State *L) : m_state(L) {
        auto &m = ScriptMachine::instance();
        auto thd = m.contextData(m_state);
        if (thd == nullptr) {
            return;
        }
        auto bridge = thd->debugger;
        if (bridge == nullptr) {
            return;
        }
        bridge->pushThreadStack(m_state);
    }

    ~DbgStackPusher() {
        auto &m = ScriptMachine::instance();
        auto thd = m.contextData(m_state);
        if (thd == nullptr) {
            return;
        }
        auto bridge = thd->debugger;
        if (bridge == nullptr) {
            return;
        }
        bridge->popThreadStack();
    }

private:
    lua_State *m_state;
};

} // namespace

bool ScriptMachine::init(WingScriptBind::ScriptBindingEngine::ObjectCodec codec,
                         ScriptRuntimeOptions options) {
    if (isInited()) {
        return true;
    }

    m_runtimeOptions = options;
    qRegisterMetaType<MessageInfo>();

    FFlag::LuauCyclicRequireShortCircuit.value = true;

    m_main = luaL_newstate();
    if (m_main == nullptr) {
        return false;
    }
    auto cleanup = qScopeGuard([this] { destoryMachine(); });
    try {
        if (!configureEngine(m_main, std::move(codec))) {
            return false;
        }
    } catch (const std::exception &error) {
        qCritical("Script binding initialization failed: %s", error.what());
        return false;
    } catch (...) {
        qCritical("Script binding initialization failed");
        return false;
    }

    // only REPL thread can be reused, other threads should be re-created
    // when needed
    auto interIdx = consoleModeIdx(ConsoleMode::Interactive);
    auto &l = m_ctx[interIdx];
    auto s = lua_newthread(m_main);
    if (s == nullptr) {
        return false;
    }

    auto id = lua_ref(m_main, -1);
    if (id == LUA_REFNIL) {
        return false;
    }
    lua_pop(m_main, 1);
    l.state = s;
    l.refID = id;

    for (auto i = 0; i < ConsoleModeCount; i++) {
        auto &l = m_ctx[i];
        auto d = &m_tdata[i];
        d->mode = ConsoleMode(i + 1);
        d->parent = &l;
        l.data = d;
    }

    auto d = &m_tdata[interIdx];
    lua_setthreaddata(s, d);
    luaL_sandboxthread(s);

    // config callbacks
    auto &&cbs = lua_callbacks(m_main);
    cbs->interrupt = ScriptMachine::onLuauInterrupt;
    cbs->userthread = ScriptMachine::onLuauThreadCreated;

    // init inspect options
    m_printOptions.setMode(InspectMode::Pretty);
    m_printOptions.depth = 8;
    m_printOptions.maxItems = 50;
    m_printOptions.quoteString = false;

    if (m_runtimeOptions.debugger) {
        m_debugger = new LuauDebugger;
        m_debugger->pushThreadStack(m_main);
    }

    m_regcalls.resize(ConsoleModeCount, {});

    m_inited = true;
    cleanup.dismiss();
    return true;
}

void ScriptMachine::finishScriptRegistration() {
    if (!m_inited || !m_bindings) {
        throw WingScriptBind::ScriptException(
            WingScriptBind::Exception::InvalidScope);
    }
    if (QThread::currentThread() != qApp->thread()) {
        throw WingScriptBind::ScriptException(
            WingScriptBind::Exception::WrongThread);
    }
    if (m_scriptRegistrationFinished) {
        return;
    }

    luaL_sandbox(m_main);

    if (!m_runtimeOptions.languageServer) {
        m_scriptRegistrationFinished = true;
        return;
    }

    StartupTrace::PhaseTimer startupPhases;
    // setup language server
    auto &lsp = LuauLanguageServer::instance();
    LuauLanguageServer::InitializationOptions opts;
    opts.defaultMode = Luau::Mode::Nonstrict;
    opts.pluginSymbols = m_bindings->symbolCatalog();
    const auto &ropts = m_luaReq.options();
    opts.readCfgRoots = {ropts.userRoot, ropts.systemRoot};
    opts.userModuleRoot = ropts.userRoot;
    opts.systemModuleRoot = ropts.systemRoot;
    opts.definitionsFiles = ScriptMachineBindings::definitions();
    opts.builtinModuleFiles = ScriptMachineBindings::moduleFiles();
    opts.symbolCatalogFile = bindings::symbolCatalogFile();
    QSet<QString> loadedDefinitions;
    for (const auto &file : std::as_const(opts.definitionsFiles)) {
        loadedDefinitions.insert(QFileInfo(file).canonicalFilePath());
    }
    for (auto &file : m_bindings->definitionFiles()) {
        if (loadedDefinitions.contains(file)) {
            continue;
        }
        loadedDefinitions.insert(file);
        opts.pluginDefinitionFiles.append(file);
    }
    startupPhases.checkpoint("scripting.lsp_options");
    lsp.initialize(opts);
    startupPhases.checkpoint("scripting.lsp_initialize");

    m_scriptRegistrationFinished = true;
}

bool ScriptMachine::isInited() const { return m_inited; }

bool ScriptMachine::scriptRegistrationFinished() const noexcept {
    return m_scriptRegistrationFinished;
}

WingScriptBind::IScriptEngine *ScriptMachine::scriptEngine() const noexcept {
    return m_bindings.get();
}

bool ScriptMachine::isRunning(ConsoleMode mode) const {
    Q_ASSERT(mode > Console_Begin && mode < Console_End);
    return context(mode)->isRunning;
}

bool ScriptMachine::configureEngine(
    lua_State *L, WingScriptBind::ScriptBindingEngine::ObjectCodec codec) {
    if (L == nullptr) {
        return false;
    }

    qtluau::LuaCallbackHooks hooks;
    hooks.schedule = [this](qtluau::LuaCallbackInvocation invocation) {
        auto execution = m_executions.value(invocation.context);
        if (!execution) {
            return false;
        }
        execution->enqueueCallback(std::move(invocation));
        return true;
    };
    hooks.end = [this](const QString &error) {
        if (!error.isEmpty() && m_callbackFailure) {
            m_callbackFailure(error);
        }
    };
    qtluau::setLuaCallbackHooks(L, std::move(hooks));
    qtluau::setLuaContextResolver(L, [](lua_State *state) -> const void * {
        auto *data = lua_getthreaddata(state);
        return data ? data : state;
    });

    WingLuauRequire::Options ropts;
    ropts.readonlyBuiltins = m_runtimeOptions.retainContext;
    auto lualib = QStringLiteral("luaulib");

    QDir appDir(QApplication::applicationDirPath());
    ropts.systemRoot = appDir.absoluteFilePath(lualib);
    QDir usrDir(Utilities::getAppDataPath());
    ropts.userRoot = usrDir.absoluteFilePath(lualib);
    ropts.onLuauFileLoading = [](lua_State *L, lua_State *ml,
                                 const QString &path,
                                 const QByteArray &source) -> bool {
        auto data = contextData(L);
        if (data == nullptr) {
            return false;
        }

        // set the thread data for the new thread
        lua_setthreaddata(ml, data);

        auto dbg = data->debugger;
        if (dbg) {
            dbg->onLuaFileLoaded(L, path, source);
        }
        return true;
    };

    if (!m_luaReq.initRequire(L, ropts)) {
        return false;
    }

    luabridge::registerMainThread(L);

    luaL_openlibs(L);

    // register cffi module
    luaopen_cffi(L);
    auto b = m_luaReq.registerBuiltinValue("cffi");
    WING_ASSERT(b);
    lua_pop(L, 1);
    m_luaReq.installRequire();
    m_luaReq.installProxyRequire();

    luabridge::enableExceptions(L);

    codec = ScriptMachineBindings::initialize(L, std::move(codec), m_luaReq,
                                              m_runtimeOptions.ui);

    PluginSystem::instance().installAPI(L);

    m_bindings = std::make_unique<WingScriptBind::ScriptBindingEngine>(
        L, std::move(codec));

    if (Luau::CodeGen::isSupported()) {
        Luau::CodeGen::create(L);
    }
    return true;
}

LuauThreadData *ScriptMachine::contextData(lua_State *l) {
    return l ? static_cast<LuauThreadData *>(lua_getthreaddata(l)) : nullptr;
}

constexpr int ScriptMachine::consoleModeIdx(ConsoleMode mode) {
    return mode - 1;
}

int ScriptMachine::__output(MessageType type, lua_State *L) {
    return __outputsep(type, L, ' ');
}

int ScriptMachine::__outputln(MessageType type, lua_State *L) {
    return __outputsep(type, L, '\n');
}

int ScriptMachine::__outputsep(MessageType type, lua_State *L, QChar sep) {
    int n = lua_gettop(L);

    QString msg;
    msg.reserve(256);
    for (int i = 1; i <= n; i++) {
        msg.append(LuauInspector::inspect(L, i, m_printOptions));
        if (i > 1 || sep == '\n') {
            msg.append(sep);
        }
    }

    MessageInfo info;
    info.type = type;
    info.mode = ConsoleMode(contextData(L)->mode);
    info.message = msg;
    ScriptMachine::instance().outputMessage(info);
    return 0;
}

LuauScheduler *ScriptMachine::createScheduler(LuauThread *context) {
    auto &scheduler =
        m_consoleSchedulers[consoleModeIdx(ConsoleMode(context->data->mode))];
    if (!scheduler) {
        scheduler = std::make_unique<LuauScheduler>(*this, context);
    }
    return scheduler.get();
}

int ScriptMachine::resumeThread(LuauThread *thread, int arguments) {
    Q_ASSERT(!m_executing);
    const QScopedValueRollback executing(m_executing, true);
    auto *execution = thread->data->execution;
    return execution ? execution->resumeThread(thread->state, arguments)
                     : lua_resume(thread->state, nullptr, arguments);
}

void ScriptMachine::destoryMachine() {
    m_callbackFailure = {};
    m_inited = false;
    m_scriptRegistrationFinished = false;
    const auto executions = std::exchange(m_executions, {});
    for (auto execution : executions) {
        delete execution;
    }
    for (auto &scheduler : m_consoleSchedulers) {
        scheduler.reset();
    }
    delete m_debugger;
    m_debugger = nullptr;
    for (auto &c : m_ctx) {
        c.destory();
    }
    if (m_main) {
        qtluau::clearLuaCallbacks(m_main);
        qtluau::setUiWidgetResolver({});
        qtluau::setUiWidgetCreatedCallback({});
        qtluau::setUiWidgetFinalizer({});
        qtluau::setWidgetContentResolver({});
        qtluau::setWidgetContentMutators({}, {});
        qtluau::setWindowMenuAccessors({}, {});
    }
    m_bindings.reset();
    if (m_main) {
        lua_close(m_main);
    }
    m_main = nullptr;
}

QVector<lua_State *> ScriptMachine::getThreadAncestors(lua_State *L) const {
    if (L == nullptr) {
        return {};
    }
    auto thd = contextData(L);
    if (thd) {
        // The LuauDebugger tracks coroutine nesting (m_threadStack), so when a
        // debugger is attached we let it build the chain: running coroutine
        // first, then its resumer, up to the main thread.  This is what the
        // variable registry walks to resolve locals/upvalues.
        auto dbg = thd->debugger;
        if (dbg) {
            return dbg->getThreadAncestors(L);
        }
    }
    return {L, m_main};
}

int ScriptMachine::print(lua_State *L) {
    return __output(MessageType::Print, L);
}

int ScriptMachine::println(lua_State *L) {
    return __outputln(MessageType::Print, L);
}

int ScriptMachine::warnprint(lua_State *L) {
    return __output(MessageType::Warn, L);
}

int ScriptMachine::warnprintln(lua_State *L) {
    return __outputln(MessageType::Warn, L);
}

int ScriptMachine::errprint(lua_State *L) {
    return __output(MessageType::Error, L);
}

int ScriptMachine::errprintln(lua_State *L) {
    return __outputln(MessageType::Error, L);
}

int ScriptMachine::infoprint(lua_State *L) {
    return __output(MessageType::Info, L);
}

int ScriptMachine::infoprintln(lua_State *L) {
    return __outputln(MessageType::Info, L);
}

int ScriptMachine::cowrap(lua_State *L) {
    if (auto *data = contextData(L); data && data->execution) {
        return data->execution->wrapCoroutine(L);
    }
    int top = lua_gettop(L);
    lua_checkstack(L, 1 + top);
    lua_pushvalue(L, lua_upvalueindex(1));
    for (int i = 1; i <= top; ++i) {
        lua_pushvalue(L, i);
    }
    lua_call(L, top, 1);

    if (auto *cl = LuauUtil::getCFunction(L, -1)) {
        (void)cl;
        lua_checkstack(L, 1);
        lua_pushcclosurek(
            L,
            [](lua_State *L) {
                DbgStackPusher _(L);
                return forward(L, lua_upvalueindex(1));
            },
            nullptr, 1, &ScriptMachine::forwardContinuation);
    }
    return 1;
}

int ScriptMachine::coresume(lua_State *L) {
    if (auto *data = contextData(L); data && data->execution) {
        return data->execution->resumeCoroutine(L, lua_upvalueindex(1));
    }
    DbgStackPusher _(L);
    return forward(L, lua_upvalueindex(1));
}

int ScriptMachine::forwardContinuation(lua_State *L, int status) {
    if (status != LUA_OK) {
        lua_error(L);
    }
    return lua_gettop(L);
}

int ScriptMachine::forward(lua_State *L, int index) {
    const int count = lua_gettop(L);
    lua_checkstack(L, 1);
    lua_pushvalue(L, index);
    lua_insert(L, 1);
    lua_call(L, count, LUA_MULTRET);
    return lua_gettop(L);
}

int ScriptMachine::input(lua_State *L) {
    if (ApplicationMode::headless()) {
        luaL_error(L, "Standard input is unavailable in headless mode");
    }
    auto *data = contextData(L);
    luaL_argcheck(
        L, data && data->mode > Console_Begin && data->mode < Console_End, 1,
        "Input requires a script context");
    auto &machine = instance();
    const auto mode = ConsoleMode(data->mode);
    const auto callback =
        machine.m_regcalls.value(consoleModeIdx(mode)).getInputFn;
    QString result;
    try {
        result = callback ? callback() : QString();
    } catch (const std::exception &error) {
        luaL_error(L, "%s", error.what());
    }
    if (machine.context(mode)->requestStop) {
        luaL_error(L, "Script execution was aborted");
    }
    luabridge::LuaRef(L, qtluau::QtValueRef<QString>(result)).push();
    return 1;
}

int ScriptMachine::stringify(lua_State *L) {
    luaL_checkany(L, 1);
    luabridge::LuaRef(L, qtluau::QtValueRef<QString>(
                             LuauInspector::inspect(L, 1, m_printOptions)))
        .push();
    return 1;
}

int ScriptMachine::assertX(lua_State *L) {
    if (!lua_toboolean(L, 1)) {
        const auto text =
            qtluau::qtFromLua<QString>(luabridge::LuaRef::fromStack(L, 2))
                .toUtf8();
        luaL_error(L, "%s", text.constData());
    }
    return 0;
}

int ScriptMachine::debugBreak(lua_State *L) {
    auto *data = contextData(L);
    if (!data || data->mode != Scripting || !data->debugger) {
        luaL_error(L, "debug.set_break requires an active scripting debugger");
    }
    data->debugger->pause();
    return 0;
}

int ScriptMachine::debugElapsedTime(lua_State *L) {
    const auto *data = contextData(L);
    const auto elapsed =
        data ? ApplicationClock::elapsed() - data->startTime : 0;
    qtluau::qtToLua(L, elapsed).push();
    return 1;
}

void ScriptMachine::onLuauInterrupt(lua_State *L, int gc) {
    Q_UNUSED(gc);
    if (L == nullptr) {
        return;
    }

    if (gc >= 0) {
        // we should not interrupt the thread when gc is running, because it may
        // cause crash
        return;
    }

    auto d = contextData(L);
    if (d == nullptr) {
        LuauUtil::throwError(L, "Thread context data not found");
        return;
    }

    if (d->execution) {
        d->execution->interrupt(L);
        return;
    }

    constexpr const auto invalidContextError = "Thread context data is invalid";

    auto lastTime = d->lastInteruptTime;
    if (lastTime < d->startTime) {
        LuauUtil::throwError(L, invalidContextError);
        return;
    }
    auto &sm = instance();
    if ((sm.m_interruptionCheck && sm.m_interruptionCheck()) ||
        (d->parent && d->parent->requestStop)) {
        LuauUtil::throwError(L, "Script execution was aborted");
        return;
    }
    auto nowTime = ApplicationClock::elapsed();
    if (nowTime < lastTime) {
        LuauUtil::throwError(L, invalidContextError);
        return;
    }

    if (d->timeOutTime) {
        if (nowTime - d->startTime > d->timeOutTime) {
            LuauUtil::throwError(L, "Thread execution is timed-out");
            return;
        }
    }

    if (lua_isyieldable(L)) {
        constexpr auto timeOutTime = 10; // 10ms
        if (nowTime - lastTime >= timeOutTime) {
            lua_yield(L, 0);
        }
        d->lastInteruptTime = nowTime;
    }
}

void ScriptMachine::onLuauThreadCreated(lua_State *lp, lua_State *L) {
    if (lp) {
        // create
        auto pd = reinterpret_cast<LuauThreadData *>(lua_getthreaddata(lp));
        if (pd) {
            lua_setthreaddata(L, pd);
        }
        luaL_sandboxthread(L);
    } else {
        // destroy
        lua_setthreaddata(L, nullptr);
    }
}

void ScriptMachine::outputMessage(const MessageInfo &info) {
    auto cbs = m_regcalls.value(consoleModeIdx(info.mode));
    if (cbs.printMsgFn) {
        cbs.printMsgFn(info);
    }
}

void ScriptMachine::executeScript(ConsoleMode mode, const QString &fileName,
                                  bool isInDebug,
                                  const std::function<void(bool)> &onFinished) {
    executeScriptWithResult(mode, fileName, isInDebug,
                            [onFinished](ExecutionResult result) {
                                onFinished(result != ExecutionResult::Busy);
                            });
}

ScriptMachine::ScriptLoadResult
ScriptMachine::loadScript(lua_State *thread, const QString &path,
                          ConsoleMode mode, bool debug, bool native) {
    QFile file(path);
    if (file.size() > DEFAULT_SCRIPT_FILE_SIZE_LIMIT) {
        return {ExecutionResult::FileError,
                {},
                QStringLiteral("Excessive script file size")};
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {ExecutionResult::FileError,
                {},
                QStringLiteral("Cannot read script: ") + file.errorString()};
    }
    auto source = file.read(DEFAULT_SCRIPT_FILE_SIZE_LIMIT + 1);
    if (source.size() > DEFAULT_SCRIPT_FILE_SIZE_LIMIT) {
        return {ExecutionResult::FileError,
                {},
                QStringLiteral("Excessive script file size")};
    }
    if (file.error() != QFileDevice::NoError) {
        return {ExecutionResult::FileError,
                {},
                QStringLiteral("Cannot read script: ") + file.errorString()};
    }
    Luau::CompileOptions options;
    if (mode == Scripting) {
        options.debugLevel = debug ? 2 : 0;
        options.optimizationLevel = debug ? 1 : 2;
    }
    const auto bytecode =
        Luau::compile(std::string(source.constData(), source.size()), options);
    const QByteArray chunk = QByteArray("@") + path.toUtf8();
    if (luau_load(thread, chunk.constData(), bytecode.data(), bytecode.size(),
                  0) != LUA_OK) {
        const auto message = QString::fromUtf8(lua_tostring(thread, -1));
        lua_pop(thread, 1);
        return {ExecutionResult::Failed, {}, message};
    }
    if (native && Luau::CodeGen::isSupported()) {
        Luau::CodeGen::CompilationOptions nativeOptions;
        Luau::CodeGen::compile(thread, -1, nativeOptions);
    }
    return {ExecutionResult::Success, std::move(source), {}};
}

void ScriptMachine::executeScriptWithResult(
    ConsoleMode mode, const QString &fileName, bool isInDebug,
    const std::function<void(ExecutionResult)> &onFinished) {
    Q_ASSERT(onFinished);
    if (m_main == nullptr || !m_scriptRegistrationFinished) {
        qWarning("The script engine is not initialized");
        onFinished(ExecutionResult::Failed);
        return;
    }

    WING_ASSERT(mode != Interactive);
    // script-running is not allowed in interactive mode
    if (mode == Interactive) {
        onFinished(ExecutionResult::Failed);
        return;
    }

    if (fileName.isEmpty()) {
        onFinished(ExecutionResult::Failed);
        return;
    }

    if (QThread::currentThread() != qApp->thread()) {
        qWarning("Code must execute on the main thread");
        onFinished(ExecutionResult::Failed);
        return;
    }

    auto ctx = context(mode);
    if (isRunning(mode)) {
        onFinished(ExecutionResult::Busy);
        return;
    }

    auto T = lua_newthread(m_main);
    if (T == nullptr) {
        onFinished(ExecutionResult::Failed);
        return;
    }
    auto refID = lua_ref(m_main, -1);
    if (refID == LUA_REFNIL) {
        lua_pop(m_main, 1);
        onFinished(ExecutionResult::Failed);
        return;
    }
    lua_pop(m_main, 1);

    ctx->state = T;
    ctx->refID = refID;
    lua_setthreaddata(T, ctx->data);
    luaL_sandboxthread(T);

    const auto loaded = loadScript(T, fileName, mode, isInDebug, !isInDebug);
    if (loaded.result != ExecutionResult::Success) {
        MessageInfo info;
        info.type = MessageType::Error;
        info.mode = mode;
        info.message = loaded.message;
        outputMessage(info);
        ctx->destory();
        onFinished(loaded.result);
        return;
    }
    const auto &source = loaded.source;

    if (isInDebug) {
        // Allow the user to initialize the debugging before moving on
        MessageInfo info;
        info.mode = mode;
        info.section = fileName;
        info.message = QStringLiteral("Debugging, waiting for commands.");
        info.type = MessageType::Info;
        outputMessage(info);
    }

    if (mode == Background) {
        MessageInfo info;
        info.mode = mode;
        info.message = QStringLiteral("Run > ") + fileName;
        info.type = MessageType::ExecInfo;
        outputMessage(info);
    }

    auto d = ctx->data;
    Q_ASSERT(d);
    d->startTime = ApplicationClock::elapsed();
    d->lastInteruptTime = d->startTime;
    // min -> ms
    d->timeOutTime =
        quint64(SettingManager::instance().scriptTimeout()) * 60000;

    if (isInDebug) {
        d->debugger = m_debugger;
        m_debugger->attach(T);
        m_debugger->onLuaFileLoaded(T, fileName, source);
    }

    // collect the handle info
    auto &api = PluginSystem::instance();
    auto handles = api.scriptHandles(T);

    auto runner = createScheduler(ctx);
    QObject::connect(
        runner, &LuauScheduler::sigOnFinished, runner,
        [this, runner, mode, fileName, handles, onFinished](int status) {
            auto &R = m_ctx[consoleModeIdx(mode)];
            auto T = R.state;

            if (status != LUA_OK) {
                auto err = runner->executeError();
                if (!err.isEmpty()) {
                    MessageInfo info;
                    info.mode = mode;
                    info.section = fileName;
                    info.message = err;
                    info.type = MessageType::Error;
                    outputMessage(info);
                }
            }

            MessageInfo info;
            info.mode = mode;
            info.section = fileName;
            info.message = QStringLiteral("The script exited with ") +
                           QString::number(status);
            info.type = MessageType::ExecInfo;
            outputMessage(info);

            auto &dbg = R.data->debugger;
            if (dbg) {
                dbg->detach();
            }

            const bool stopped = R.requestStop;
            if (!m_runtimeOptions.retainContext) {
                // Release this execution's handles while its state is alive.
                PluginSystem::instance().cleanScriptHandles(T, handles);
                R.destory();
                m_luaReq.clearRequireCache();
            }

            onFinished(stopped            ? ExecutionResult::Aborted
                       : status == LUA_OK ? ExecutionResult::Success
                                          : ExecutionResult::Failed);
        },
        Qt::SingleShotConnection);
    runner->start();
}

void ScriptMachine::abortDbgScript() {
    if (m_debugger) {
        m_debugger->terminate();
    }
    abortScript(ConsoleMode::Scripting);
}

void ScriptMachine::abortScript(ConsoleMode mode) {
    context(mode)->requestStop = true;
    qtluau::clearLuaContext(context(mode)->state);
    Q_EMIT sigOnInputCancelled(mode);
}

ScriptMachine &ScriptMachine::instance() {
    static ScriptMachine ins;
    return ins;
}

ScriptMachine::ScriptMachine() {}

ScriptExecution *ScriptMachine::createExecution(const QString &path) {
    if (!m_scriptRegistrationFinished) {
        return nullptr;
    }
    auto execution = new ScriptExecution(*this, path);
    const auto key = execution->contextKey();
    m_executions.insert(key, execution);
    connect(execution, &QObject::destroyed, this,
            [this, key] { m_executions.remove(key); });
    return execution;
}

bool ScriptMachine::registerBuiltin(
    const QString &name, const std::function<void(lua_State *)> &push) {
    if (!m_main || m_scriptRegistrationFinished) {
        return false;
    }
    push(m_main);
    const auto registered = m_luaReq.registerBuiltinValue(name);
    lua_pop(m_main, 1);
    return registered;
}

LuauThread *ScriptMachine::context(ConsoleMode mode) const {
    Q_ASSERT(mode > Console_Begin && mode < Console_End);
    return &m_ctx[consoleModeIdx(mode)];
}

lua_State *ScriptMachine::contextState(ConsoleMode mode) const {
    Q_ASSERT(mode > Console_Begin && mode < Console_End);
    return m_ctx[consoleModeIdx(mode)].state;
}

LuauThreadData *ScriptMachine::contextData(ConsoleMode mode) const {
    auto ctx = contextState(mode);
    return contextData(ctx);
}

void ScriptMachine::registerCallBack(ConsoleMode mode,
                                     const RegCallBacks &callbacks) {
    m_regcalls[consoleModeIdx(mode)] = callbacks;
}

bool ScriptMachine::isDebugMode(ConsoleMode mode) {
    if (mode == Scripting) {
        auto thd = contextData(mode);
        if (thd) {
            return thd->debugger;
        }
    }

    return false;
}

LuauDebugger *ScriptMachine::debugger() const { return m_debugger; }

void ScriptMachine::executeCode(ConsoleMode mode, const QString &code,
                                const std::function<void(bool)> &onFinished) {

#define RETURN_NEED_MORE_CODE                                                  \
    onFinished(false);                                                         \
    return

#define RETURN_DEFAULT                                                         \
    onFinished(true);                                                          \
    return

    if (QThread::currentThread() != qApp->thread()) {
        qWarning("Code must execute on the main thread");
        RETURN_DEFAULT;
    }

    if (m_main == nullptr || !m_scriptRegistrationFinished) {
        qWarning("The script engine is not initialized");
        RETURN_DEFAULT;
    }

    if (code.isEmpty()) {
        RETURN_DEFAULT;
    }

    auto ctx = context(mode);

    if (isRunning(mode)) {
        RETURN_DEFAULT;
    }

    auto T = ctx->state;
    auto source = code.toUtf8();
    auto bytecode = Luau::compile(source.data());

    if (luau_load(T, "=stdin", bytecode.data(), bytecode.size(), 0) != LUA_OK) {
        size_t len;
        const char *msg = lua_tolstring(T, -1, &len);
        auto error = QString::fromUtf8(msg, len);
        lua_pop(T, 1);
        if (error.endsWith(QLatin1String("<eof>"))) {
            RETURN_NEED_MORE_CODE;
        }
        MessageInfo info;
        info.type = MessageType::Error;
        info.mode = mode;
        info.message = error;
        outputMessage(info);
        RETURN_DEFAULT;
    }

    auto d = contextData(mode);
    Q_ASSERT(d);
    d->startTime = ApplicationClock::elapsed();
    d->lastInteruptTime = d->startTime;
    // min -> ms
    d->timeOutTime =
        quint64(SettingManager::instance().scriptTimeout()) * 60000;

    auto runner = createScheduler(ctx);
    QObject::connect(
        runner, &LuauScheduler::sigOnFinished, runner,
        [this, mode, onFinished](int status) {
            auto &&R = m_ctx[consoleModeIdx(mode)];
            auto T = R.state;
            if (status == LUA_OK) {
                int n = lua_gettop(T);
                if (n) {
                    luaL_checkstack(T, LUA_MINSTACK,
                                    "too many results to print");
                    lua_getglobal(T, "_PRETTYPRINT");
                    // If _PRETTYPRINT is nil, then use the
                    // standard print function instead
                    if (lua_isnil(T, -1)) {
                        lua_pop(T, 1);
                        lua_getglobal(T, "print");
                    }
                    lua_insert(T, 1);
                    lua_pcall(T, n, 0, 0);
                }
            } else {
                size_t len;
                const char *str = lua_tolstring(T, -1, &len);
                if (str) {
                    auto error = QString::fromUtf8(str, len);
                    error += QStringLiteral("\nstack backtrace:\n");
                    error += QString::fromUtf8(lua_debugtrace(T));
                    MessageInfo info;
                    info.mode = mode;
                    info.message = error;
                    info.type = MessageType::Error;
                    outputMessage(info);
                }
            }
            R.reset();
            onFinished(true);
        },
        Qt::SingleShotConnection);

    runner->start();
#undef RETURN_DEFAULT
#undef RETURN_NEED_MORE_CODE
}
