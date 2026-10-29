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
#include SCRIPTMACHINE_LUAU_BINDING

#include "Luau/CodeGen.h"
#include "Luau/Common.h"
#include "Luau/Compiler.h"
#include "class/application/appmanager.h"
#include "class/application/logger.h"
#include "class/plugin/pluginsystem.h"
#include "class/settings/settingmanager.h"
#include "define.h"
#include "lua.h"
#include "lualib.h"
#include "luau/CodeGen/include/Luau/CodeGenOptions.h"
#include "luau/lsp/luaulanguageserver.h"
#include "luau/luauinspector.h"
#include "luau/luauutil.h"
#include "luau/wingluaurequire.h"
#include "utilities.h"

#include "LuaBridge/LuaBridge.h"

#include <QScopeGuard>
#include <QSet>

extern "C" {
int luaopen_cffi(lua_State *L);
}

LUAU_FASTFLAG(LuauCyclicRequireShortCircuit)

namespace {

class DbgStackPusher {
public:
    DbgStackPusher(lua_State *L) : L_(L) {
        auto &m = ScriptMachine::instance();
        auto thd = m.contextData(L_);
        if (thd == nullptr) {
            return;
        }
        auto bridge = thd->debugger;
        if (bridge == nullptr) {
            return;
        }
        bridge->pushThreadStack(L_);
    }
    ~DbgStackPusher() {
        auto &m = ScriptMachine::instance();
        auto thd = m.contextData(L_);
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
    lua_State *L_;
};

} // namespace

bool ScriptMachine::init(
    WingScriptBind::ScriptBindingEngine::ObjectCodec codec) {
    if (isInited()) {
        return true;
    }

    qRegisterMetaType<MessageInfo>();

    FFlag::LuauCyclicRequireShortCircuit.value = true;

    _main = luaL_newstate();
    if (_main == nullptr) {
        return false;
    }
    auto cleanup = qScopeGuard([this] { destoryMachine(); });
    try {
        if (!configureEngine(_main, std::move(codec))) {
            return false;
        }
    } catch (const std::exception &error) {
        Logger::critical(QString::fromUtf8(error.what()));
        return false;
    } catch (...) {
        Logger::critical(
            QStringLiteral("Script binding initialization failed"));
        return false;
    }

    // only REPL thread can be reused, other threads should be re-created
    // when needed
    auto interIdx = consoleModeIdx(ConsoleMode::Interactive);
    auto &l = _ctx[interIdx];
    auto s = lua_newthread(_main);
    if (s == nullptr) {
        return false;
    }

    auto id = lua_ref(_main, -1);
    if (id == LUA_REFNIL) {
        return false;
    }
    lua_pop(_main, 1);
    l.state = s;
    l.refID = id;

    for (auto i = 0; i < ConsoleModeCount; i++) {
        auto &l = _ctx[i];
        auto d = &_tdata[i];
        d->mode = ConsoleMode(i + 1);
        d->parent = &l;
        l.data = d;
    }

    auto d = &_tdata[interIdx];
    lua_setthreaddata(s, d);
    luaL_sandboxthread(s);

    // config callbacks
    auto &&cbs = lua_callbacks(_main);
    cbs->interrupt = ScriptMachine::onLuauInterrupt;
    cbs->userthread = ScriptMachine::onLuauThreadCreated;

    // init inspect options
    _printOptions.setMode(InspectMode::Pretty);
    _printOptions.depth = 8;
    _printOptions.maxItems = 50;
    _printOptions.quoteString = false;

    // create the debugger
    _debugger = new LuauDebugger;
    _debugger->pushThreadStack(_main);

    _regcalls.resize(ConsoleModeCount, {});

    _inited = true;
    cleanup.dismiss();
    return true;
}

void ScriptMachine::finishScriptRegistration() {
    if (!_inited || !_bindings) {
        throw WingScriptBind::ScriptException(
            WingScriptBind::Exception::InvalidScope);
    }
    if (QThread::currentThread() != qApp->thread()) {
        throw WingScriptBind::ScriptException(
            WingScriptBind::Exception::WrongThread);
    }
    if (_scriptRegistrationFinished) {
        return;
    }

    luaL_sandbox(_main);

    // setup language server
    auto &lsp = LuauLanguageServer::instance();
    LuauLanguageServer::InitializationOptions opts;
    opts.defaultMode = Luau::Mode::Nonstrict;
    opts.pluginSymbols = _bindings->symbolCatalog();
    const auto &ropts = _luaReq.options();
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
    for (auto &file : _bindings->definitionFiles()) {
        if (loadedDefinitions.contains(file)) {
            continue;
        }
        loadedDefinitions.insert(file);
        opts.pluginDefinitionFiles.append(file);
    }
    lsp.initialize(opts);

    _scriptRegistrationFinished = true;
}

bool ScriptMachine::isInited() const { return _inited; }

bool ScriptMachine::scriptRegistrationFinished() const noexcept {
    return _scriptRegistrationFinished;
}

WingScriptBind::IScriptEngine *ScriptMachine::scriptEngine() const noexcept {
    return _bindings.get();
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

    qtluau::setLuaContextResolver(L, [](lua_State *state) -> const void * {
        auto *data = lua_getthreaddata(state);
        return data ? data : state;
    });

    WingLuauRequire::Options ropts;
    auto lualib = QStringLiteral("luaulib");

    QDir appDir(QApplication::applicationDirPath());
    ropts.systemRoot = appDir.absoluteFilePath(lualib);
    QDir usrDir(Utilities::getAppDataPath());
    ropts.userRoot = usrDir.absoluteFilePath(lualib);
    ropts.onLuauFileLoading = [](lua_State *L, lua_State *ML,
                                 const QString &path,
                                 const QByteArray &source) -> bool {
        auto data = contextData(L);
        if (data == nullptr) {
            return false;
        }

        // set the thread data for the new thread
        lua_setthreaddata(ML, data);

        auto dbg = data->debugger;
        if (dbg) {
            dbg->onLuaFileLoaded(L, path, source);
        }
        return true;
    };

    if (!_luaReq.initRequire(L, ropts)) {
        return false;
    }

    luabridge::registerMainThread(L);

    luaL_openlibs(L);

    // register cffi module
    luaopen_cffi(L);
    auto b = _luaReq.registerBuiltinValue("cffi");
    WING_ASSERT(b);
    lua_pop(L, 1);
    _luaReq.installRequire();
    _luaReq.installProxyRequire();

    luabridge::enableExceptions(L);

    codec = ScriptMachineBindings::initialize(L, std::move(codec), _luaReq);

    PluginSystem::instance().installAPI(L);

    _bindings = std::make_unique<WingScriptBind::ScriptBindingEngine>(
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
        msg.append(LuauInspector::inspect(L, i, _printOptions));
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

void ScriptMachine::destoryMachine() {
    _inited = false;
    _scriptRegistrationFinished = false;
    delete _debugger;
    _debugger = nullptr;
    for (auto &c : _ctx) {
        c.destory();
    }
    if (_main) {
        qtluau::clearLuaCallbacks(_main);
        qtluau::setUiWidgetResolver({});
        qtluau::setUiWidgetCreatedCallback({});
        qtluau::setUiWidgetFinalizer({});
        qtluau::setWidgetContentResolver({});
        qtluau::setWidgetContentMutators({}, {});
        qtluau::setWindowMenuAccessors({}, {});
    }
    _bindings.reset();
    if (_main) {
        lua_close(_main);
    }
    _main = nullptr;
}

QVector<lua_State *> ScriptMachine::getThreadAncestors(lua_State *L) const {
    if (L == nullptr) {
        return {};
    }
    auto thd = contextData(L);
    if (thd) {
        // The LuauDebugger tracks coroutine nesting (thread_stack_), so when a
        // debugger is attached we let it build the chain: running coroutine
        // first, then its resumer, up to the main thread.  This is what the
        // variable registry walks to resolve locals/upvalues.
        auto dbg = thd->debugger;
        if (dbg) {
            return dbg->getThreadAncestors(L);
        }
    }
    return {L, _main};
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
    auto *data = contextData(L);
    luaL_argcheck(
        L, data && data->mode > Console_Begin && data->mode < Console_End, 1,
        "Input requires a script context");
    auto &machine = instance();
    const auto mode = ConsoleMode(data->mode);
    const auto callback =
        machine._regcalls.value(consoleModeIdx(mode)).getInputFn;
    const QString result = callback ? callback() : QString();
    if (machine.context(mode)->requestStop) {
        luaL_error(L, "Script execution was aborted");
    }
    luabridge::LuaRef(L, qtluau::QtValueRef<QString>(result)).push();
    return 1;
}

int ScriptMachine::stringify(lua_State *L) {
    luaL_checkany(L, 1);
    luabridge::LuaRef(L, qtluau::QtValueRef<QString>(
                             LuauInspector::inspect(L, 1, _printOptions)))
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
        data
            ? AppManager::instance()->currentMSecsSinceEpoch() - data->startTime
            : 0;
    lua_pushinteger64(L, static_cast<int64_t>(elapsed));
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

    constexpr const auto INVALID_CONTEXT_ERROR =
        "Thread context data is invalid";

    auto lastTime = d->lastInteruptTime;
    if (lastTime < d->startTime) {
        LuauUtil::throwError(L, INVALID_CONTEXT_ERROR);
        return;
    }
    auto nowTime = AppManager::instance()->currentMSecsSinceEpoch();
    if (nowTime < lastTime) {
        LuauUtil::throwError(L, INVALID_CONTEXT_ERROR);
        return;
    }

    if (d->timeOutTime) {
        if (nowTime - lastTime > d->timeOutTime) {
            LuauUtil::throwError(L, "Thread execution is timed-out");
            return;
        }
    }

    if (lua_isyieldable(L)) {
        constexpr auto TIME_OUT_TIME = 10; // 10ms
        if (nowTime - lastTime >= TIME_OUT_TIME) {
            lua_yield(L, 0);
        }
        d->lastInteruptTime = nowTime;
    }
}

void ScriptMachine::onLuauThreadCreated(lua_State *LP, lua_State *L) {
    if (LP) {
        // create
        auto pd = reinterpret_cast<LuauThreadData *>(lua_getthreaddata(LP));
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
    auto cbs = _regcalls.value(consoleModeIdx(info.mode));
    if (cbs.printMsgFn) {
        cbs.printMsgFn(info);
    }
}

void ScriptMachine::executeScript(ConsoleMode mode, const QString &fileName,
                                  bool isInDebug,
                                  const std::function<void(bool)> &onFinished) {
    Q_ASSERT(onFinished);
    if (_main == nullptr || !_scriptRegistrationFinished) {
        Logger::warning(QStringLiteral("The script engine is not initialized"));
        onFinished(true);
        return;
    }

    WING_ASSERT(mode != Interactive);
    // script-running is not allowed in interactive mode
    if (mode == Interactive) {
        onFinished(true);
        return;
    }

    if (fileName.isEmpty()) {
        onFinished(true);
        return;
    }

    if (QThread::currentThread() != qApp->thread()) {
        Logger::warning(QStringLiteral("Code must be exec in the main thread"));
        onFinished(true);
        return;
    }

    auto ctx = context(mode);
    if (isRunning(mode)) {
        onFinished(false);
        return;
    }

    QFile script(fileName);
    if (script.size() > DEFAULT_SCRIPT_FILE_SIZE_LIMIT) {
        MessageInfo info;
        info.type = MessageType::Error;
        info.mode = mode;
        info.message = QStringLiteral("Excessive script file size");
        outputMessage(info);
        onFinished(true);
        return;
    }

    if (!script.open(QFile::ReadOnly | QFile::Text)) {
        MessageInfo info;
        info.type = MessageType::Error;
        info.mode = mode;
        info.message = QStringLiteral("Permission denied");
        outputMessage(info);
        onFinished(true);
        return;
    }

    auto T = lua_newthread(_main);
    if (T == nullptr) {
        onFinished(true);
        return;
    }
    auto refID = lua_ref(_main, -1);
    if (refID == LUA_REFNIL) {
        lua_pop(_main, 1);
        onFinished(true);
        return;
    }
    lua_pop(_main, 1);

    ctx->state = T;
    ctx->refID = refID;
    lua_setthreaddata(T, ctx->data);
    luaL_sandboxthread(T);

    auto source = script.readAll();

    // Compile the script
    auto chunkname = fileName.toUtf8();
    chunkname.prepend('@');
    Luau::CompileOptions opts;
    if (mode == Scripting) {
        if (isInDebug) {
            opts.debugLevel = 2;
            opts.optimizationLevel = 1;
        } else {
            opts.debugLevel = 0;
            opts.optimizationLevel = 2;
        }
    }

    auto bytecode = Luau::compile(source.data(), opts);
    if (luau_load(T, chunkname.data(), bytecode.data(), bytecode.size(), 0) ==
        LUA_OK) {
        if (!isInDebug && Luau::CodeGen::isSupported()) {
            // try to use jit
            Luau::CodeGen::CompilationOptions nativeOptions;
            Luau::CodeGen::compile(T, -1, nativeOptions);
        }
    } else {
        size_t len;
        const char *msg = lua_tolstring(T, -1, &len);
        MessageInfo info;
        info.type = MessageType::Error;
        info.mode = mode;
        info.message = QString::fromUtf8(msg, len);
        outputMessage(info);
        lua_pop(T, 1);
        onFinished(true);
        return;
    }

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
    d->startTime = AppManager::instance()->currentMSecsSinceEpoch();
    d->lastInteruptTime = d->startTime;
    // min -> ms
    d->timeOutTime =
        quint64(SettingManager::instance().scriptTimeout()) * 60000;

    if (isInDebug) {
        d->debugger = _debugger;
        _debugger->attach(T);
        _debugger->onLuaFileLoaded(T, fileName, source);
    }

    // collect the handle info
    auto &api = PluginSystem::instance();
    auto handles = api.scriptHandles();

    auto runner = new LuauScheduler(ctx);
    QObject::connect(
        runner, &LuauScheduler::finished, runner,
        [this, runner, mode, fileName, handles, onFinished](int status) {
            auto &R = _ctx[consoleModeIdx(mode)];
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

            R.destory();
            _luaReq.clearRequireCache();

            // Before leaving, allow the engine to clean up
            // remaining objects by discarding the module and doing
            // a full garbage collection so that this can also be
            // debugged if desired
            auto &api = PluginSystem::instance();
            api.cleanScriptHandles(handles);

            onFinished(true);
        });
    runner->start();
}

void ScriptMachine::abortDbgScript() {
    if (_debugger) {
        _debugger->terminate();
    }
    abortScript(ConsoleMode::Scripting);
}

void ScriptMachine::abortScript(ConsoleMode mode) {
    context(mode)->requestStop = true;
    qtluau::clearLuaContext(context(mode)->state);
    Q_EMIT inputCancelled(mode);
}

ScriptMachine &ScriptMachine::instance() {
    static ScriptMachine ins;
    return ins;
}

ScriptMachine::ScriptMachine() {}

LuauThread *ScriptMachine::context(ConsoleMode mode) const {
    Q_ASSERT(mode > Console_Begin && mode < Console_End);
    return &_ctx[consoleModeIdx(mode)];
}

lua_State *ScriptMachine::contextState(ConsoleMode mode) const {
    Q_ASSERT(mode > Console_Begin && mode < Console_End);
    return _ctx[consoleModeIdx(mode)].state;
}

LuauThreadData *ScriptMachine::contextData(ConsoleMode mode) const {
    auto ctx = contextState(mode);
    return contextData(ctx);
}

void ScriptMachine::registerCallBack(ConsoleMode mode,
                                     const RegCallBacks &callbacks) {
    _regcalls[consoleModeIdx(mode)] = callbacks;
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

LuauDebugger *ScriptMachine::debugger() const { return _debugger; }

void ScriptMachine::executeCode(ConsoleMode mode, const QString &code,
                                const std::function<void(bool)> &onFinished) {

#define RETURN_NEED_MORE_CODE                                                  \
    onFinished(false);                                                         \
    return

#define RETURN_DEFAULT                                                         \
    onFinished(true);                                                          \
    return

    if (QThread::currentThread() != qApp->thread()) {
        Logger::warning(QStringLiteral("Code must be exec in the main thread"));
        RETURN_DEFAULT;
    }

    if (_main == nullptr || !_scriptRegistrationFinished) {
        Logger::warning(QStringLiteral("The script engine is not initialized"));
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
    d->startTime = AppManager::instance()->currentMSecsSinceEpoch();
    d->lastInteruptTime = d->startTime;
    // min -> ms
    d->timeOutTime =
        quint64(SettingManager::instance().scriptTimeout()) * 60000;

    auto runner = new LuauScheduler(ctx);
    QObject::connect(runner, &LuauScheduler::finished, runner,
                     [this, mode, onFinished](int status) {
                         auto &&R = _ctx[consoleModeIdx(mode)];
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
                                 error +=
                                     QStringLiteral("\nstack backtrace:\n");
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
                     });

    runner->start();
#undef RETURN_DEFAULT
#undef RETURN_NEED_MORE_CODE
}
