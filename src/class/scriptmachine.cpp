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

#include "Luau/CodeGen.h"
#include "Luau/Common.h"
#include "Luau/Compiler.h"
#include "debugger/luauinspector.h"
#include "debugger/luauutil.h"
#include "lua.h"
#include "lualib.h"

#include "class/appmanager.h"
#include "class/logger.h"
#include "class/pluginsystem.h"
#include "class/settingmanager.h"
#include "define.h"

#include "LuaBridge/LuaBridge.h"
#include "luau/CodeGen/include/Luau/CodeGenOptions.h"
#include "luau/Require/include/Luau/Require.h"

#include <QClipboard>
#include <QMimeData>
#include <QScopeGuard>

extern "C" {
int luaopen_cffi(lua_State *L);
}

LUAU_FASTFLAG(LuauAutoStack)

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

bool ScriptMachine::init() {
    if (isInited()) {
        return true;
    }

    qRegisterMetaType<MessageInfo>();

    _main = luaL_newstate();
    if (_main == nullptr) {
        return false;
    }
    if (!configureEngine(_main)) {
        lua_close(_main);
        _main = nullptr;
        return false;
    }

    // TODO: only REPL thread can be reused, other threads should be re-created
    // when needed
    auto interIdx = consoleModeIdx(ConsoleMode::Interactive);
    auto &l = _ctx[interIdx];
    auto s = lua_newthread(_main);
    if (s == nullptr) {
        lua_close(_main);
        _main = nullptr;
        return false;
    }

    auto id = lua_ref(_main, -1);
    if (id == LUA_REFNIL) {
        lua_close(_main);
        _main = nullptr;
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
    // cbs->userthread = ScriptMachine::onLuauThreadCreated;
    luaL_sandbox(_main);

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
    return true;
}

bool ScriptMachine::isInited() const { return _inited; }

bool ScriptMachine::isRunning(ConsoleMode mode) const {
    Q_ASSERT(mode > Console_Begin && mode < Console_End);
    return context(mode)->isRunning;
}

bool ScriptMachine::checkEngineConfigError() const {
    // TODO
    // if (_engine) {
    //     auto e = static_cast<asCScriptEngine *>(_engine);
    //     if (e) {
    //         e->PrepareEngine();
    //         return e->configFailed;
    //     }
    // }
    return false;
}

bool ScriptMachine::configureEngine(lua_State *L) {
    if (L == nullptr) {
        return false;
    }

    luabridge::registerMainThread(L);

    luaL_openlibs(L);

    luabridge::enableExceptions(L);

    luabridge::getGlobalNamespace(L)
        .addFunction("require", &ScriptMachine::onLuauRequire)
        .addFunction("print", &ScriptMachine::print)
        .addFunction("println", &ScriptMachine::println)
        .addFunction("warnprint", &ScriptMachine::warnprint)
        .addFunction("warnprintln", &ScriptMachine::warnprintln)
        .addFunction("infoprint", &ScriptMachine::infoprint)
        .addFunction("infoprintln", &ScriptMachine::infoprintln)
        .addFunction("errprint", &ScriptMachine::errprint)
        .addFunction("errprintln", &ScriptMachine::errprintln);

    luabridge::getGlobalNamespace(L)
        .beginNamespace("coroutine")
        .addFunction("wrap", &ScriptMachine::cowrap)
        .addFunction("resume", &ScriptMachine::coresume)
        .endNamespace();

    luabridge::getGlobalNamespace(L)
        .beginNamespace("cffi")
        .addFunction("import", &ScriptMachine::injectLuauCffi)
        .endNamespace();

    // TODO

    // r = _engine->RegisterGlobalFunction("string input()",
    //                                     asMETHOD(ScriptMachine, input),
    //                                     asCALL_THISCALL_ASGLOBAL, this);

    // PluginSystem::instance().angelApi()->installAPI(this);

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
        if (i > 1) {
            msg.append(sep);
        }
        msg.append(LuauInspector::inspect(L, i, _printOptions));
    }

    MessageInfo info;
    info.type = type;
    info.mode = ConsoleMode(contextData(L)->mode);
    info.message = msg;
    ScriptMachine::instance().outputMessage(info);
    return 0;
}

void ScriptMachine::destoryMachine() {
    delete _debugger;
    _debugger = nullptr;
    for (auto &c : _ctx) {
        c.destory();
    }
    lua_close(_main);
    _main = nullptr;
}

QVector<lua_State *> ScriptMachine::getThreadAncestors(lua_State *L) const {
    if (L == nullptr) {
        return {};
    }
    auto thd = contextData(L);
    if (thd) {
        auto dbg = thd->debugger;
        if (dbg) {
            return dbg->getThreadAncestors(L);
        }
    }
    return {L, _main};
}

void ScriptMachine::setCustomEvals(
    const QHash<std::string_view, WingHex::IWingAngel::Evaluator> &evals) {
    // _debugger->setCustomEvals(evals);
}

// void ScriptMachine::attachDebugBreak(asIScriptContext *ctx) {
//     if (!ctx)
//         ctx = asGetActiveContext();

//     checkDebugger(ctx);

//     if (_debugger)
//         _debugger->DebugBreak(ctx);
// }

// void ScriptMachine::checkDebugger(asIScriptContext *ctx) {
//     if (_debugger == nullptr) {
//         return;
//     }
//     // hook the context
//     if (_debugger->HasWork()) {
//         _debugger->HookContext(ctx, true);
//     } else {
//         _debugger->HookContext(ctx, false);
//         ctx->SetLineCallback(asFUNCTION(ScriptMachine::lineCallback),
//         nullptr,
//                              asCALL_CDECL);
//     }
// }

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
    for (int i = 1; i <= top; ++i)
        lua_pushvalue(L, i);
    lua_call(L, top, 1);

    if (auto *cl = LuauUtil::getCFunction(L, -1)) {
        auto *cont = cl->c.cont;
        lua_checkstack(L, 1);
        lua_pushcclosurek(
            L,
            [](lua_State *L) {
                DbgStackPusher _(L);
                return forward(L, lua_upvalueindex(1));
            },
            nullptr, 1, cont);
    }
    return 1;
}

int ScriptMachine::coresume(lua_State *L) {
    DbgStackPusher _(L);
    return forward(L, lua_upvalueindex(1));
}

int ScriptMachine::forward(lua_State *L, int index) {
    int top = lua_gettop(L);
    lua_checkstack(L, 1 + top);
    lua_pushvalue(L, index);
    for (int i = 1; i <= top; ++i)
        lua_pushvalue(L, i);
    lua_call(L, top, LUA_MULTRET);
    return lua_gettop(L) - top;
}

int ScriptMachine::injectLuauCffi(lua_State *L) {
    lua_pushcfunction(L, luaopen_cffi, "luaopen_cffi");
    lua_call(L, 0, 1);        // leaves the cffi table on the stack
    lua_setglobal(L, "cffi"); // now usable from Luau as `cffi`
    return 0;
}

QString ScriptMachine::input() {
    // auto context = asGetActiveContext();
    // if (context) {
    //     ConsoleMode mode = ConsoleMode(reinterpret_cast<asPWORD>(
    //         context->GetUserData(AsUserDataType::UserData_ContextMode)));

    //     auto cbs = _regcalls.value(mode);
    //     if (cbs.getInputFn) {
    //         return cbs.getInputFn();
    //     }
    // }
    return {};
}

int ScriptMachine::onLuauRequire(lua_State *L) {
    size_t len;
    auto raw_module_path = luaL_checklstring(L, 1, &len);
    auto module_path = QString::fromUtf8(raw_module_path, len);

    // TODO

    lua_Debug ar;
    lua_getinfo(L, 1, "s", &ar);
    std::string source_path = ar.source;
    // if (source_path.empty())
    //     return path;

    auto normalized_path = LuauUtil::normalizeLuauRequirePath(module_path);
    luaL_findtable(L, LUA_REGISTRYINDEX, "_MODULES", 1);

    std::array suffixes{".luau", ".lua", "/init.luau", "/init.lua"};

    std::string source_code;
    QString resolved_path;
    auto r_path = resolved_path.toUtf8();
    for (const char *suffix : suffixes) {
        // resolved_path = normalized_path + suffix;

        lua_getfield(L, -1, r_path);
        if (!lua_isnil(L, -1))
            return finishLuauRequire(L);

        lua_pop(L, 1);

        // std::optional<std::string> source =
        // file_utils::readFile(resolved_path); if (source) {
        //     source_code = source.value();
        //     break;
        // }
    }

    if (source_code.empty()) {
        luaL_errorL(L, "error requiring module");
    }

    lua_State *GL = lua_mainthread(L);
    lua_State *ML = lua_newthread(GL);
    lua_xmove(GL, L, 1);

    std::string bytecode = Luau::compile(source_code, {});
    if (luau_load(ML, r_path, bytecode.data(), bytecode.size(), 0) == 0) {
        // NOTICE: Call debugger when file is loaded
        auto *debugger =
            reinterpret_cast<LuauDebugger *>(lua_getthreaddata(GL));
        if (debugger) {
            debugger->onLuaFileLoaded(ML, resolved_path, false);
        }

        int status = lua_resume(ML, L, 0);

        if (status == 0) {
            if (lua_gettop(ML) == 0) {
                lua_pushstring(ML, "module must return a value");
            } else if (!lua_istable(ML, -1) && !lua_isfunction(ML, -1)) {
                lua_pushstring(ML, "module must return a table or function");
            }
        } else if (status == LUA_YIELD) {
            lua_pushstring(ML, "module can not yield");
        } else if (!lua_isstring(ML, -1)) {
            lua_pushstring(ML, "unknown error while running module");
        }
    }

    lua_xmove(ML, L, 1);
    lua_pushvalue(L, -1);
    lua_setfield(L, -4, r_path);

    // L stack: _MODULES ML result
    return finishLuauRequire(L);
}

int ScriptMachine::finishLuauRequire(lua_State *L) {
    if (lua_isstring(L, -1)) {
        lua_error(L);
    }
    return 1;
}

void ScriptMachine::onLuauInterrupt(lua_State *L, int gc) {
    Q_UNUSED(gc);
    if (L == nullptr) {
        return;
    }

    if (gc >= 0) {
        return;
    }

    auto d = contextData(L);
    if (d == nullptr) {
        lua_pushstring(L, "Thread context data not found");
        lua_error(L);
        return;
    }

    constexpr const auto INVALID_CONTEXT_ERROR =
        "Thread context data is invalid";

    auto lastTime = d->lastInteruptTime;
    if (lastTime < d->startTime) {
        lua_pushstring(L, INVALID_CONTEXT_ERROR);
        lua_error(L);
        return;
    }
    auto nowTime = AppManager::instance()->currentMSecsSinceEpoch();
    if (nowTime < lastTime) {
        lua_pushstring(L, INVALID_CONTEXT_ERROR);
        lua_error(L);
        return;
    }

    if (d->timeOutTime) {
        if (nowTime - lastTime > d->timeOutTime) {
            lua_pushstring(L, "Thread execution is timed-out");
            lua_error(L);
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
        // destory
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
    if (_main == nullptr) {
        Logger::warning(QStringLiteral("The script engine is not initialized"));
        onFinished(true);
        return;
    }

    ASSERT(mode != Interactive);
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
    if (!script.open(QFile::ReadOnly | QFile::Text)) {
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

    // TODO: limit source code size
    auto source = script.readAll();

    // Compile the script
    auto chunkname = ('@' + fileName).toUtf8();
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
        if (!isInDebug) {
            // auto r = Luau::CodeGen::compile(T, -1, {});
            // if (r.hasErrors()) {
            //     MessageInfo info;
            //     info.type = MessageType::Error;
            //     info.mode = mode;
            //     info.section = fileName;
            //     info.message = QStringLiteral("Native codegen failed: ");
            //     outputMessage(info);
            // }
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
        _debugger->attach(T);
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
    // if (_debugger) {
    abortScript(ConsoleMode::Scripting);
    //     _debugger->Resume();
    // }
}

void ScriptMachine::abortScript(ConsoleMode mode) {
    context(mode)->requestStop = true;
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

// void ScriptMachine::registerEngineClipboard(asIScriptEngine *engine) {
//     int r = engine->SetDefaultNamespace("clipboard");
//     ASSERT(r >= 0);

//     // The string type must be available
//     ASSERT(engine->GetTypeInfoByDecl("string"));

//     r = engine->RegisterGlobalFunction("void setText(const string &in text)",
//                                        asFUNCTION(clip_setText),
//                                        asCALL_CDECL);
//     ASSERT(r >= 0);

//     r = engine->RegisterGlobalFunction("string text()",
//                                        asFUNCTION(clip_getText),
//                                        asCALL_CDECL);
//     ASSERT(r >= 0);

//     r = engine->RegisterGlobalFunction("void setBinary(const uint8[]@ data)",
//                                        asFUNCTION(clip_setBinary),
//                                        asCALL_CDECL);
//     ASSERT(r >= 0);

//     r = engine->RegisterGlobalFunction(
//         "uint8[]@ getBinary()", asFUNCTION(clip_getBinary), asCALL_CDECL);
//     ASSERT(r >= 0);

//     engine->SetDefaultNamespace("");
// }

void ScriptMachine::registerCallBack(ConsoleMode mode,
                                     const RegCallBacks &callbacks) {
    _regcalls[consoleModeIdx(mode)] = callbacks;
}

void ScriptMachine::setFileEnableOverwrite(bool b) {
    // CScriptFile::ENABLE_OVERWRITE = b;
}

void ScriptMachine::setFileSystemWrite(bool b) {
    // CScriptFileSystem::ENABLE_WRITE = b;
}

bool ScriptMachine::fileEnableOverwrite() const {
    // return CScriptFile::ENABLE_OVERWRITE;
    return false;
}

bool ScriptMachine::fileSystemWrite() const {
    // return CScriptFileSystem::ENABLE_WRITE;
    return false;
}

void ScriptMachine::clip_setText(const QString &text) {
    qApp->clipboard()->setText(text);
}

void ScriptMachine::clip_setBinary(const CScriptArray &array) {
    // QByteArray buffer;
    // buffer.reserve(array.GetSize());
    // array.AddRef();
    // for (asUINT i = 0; i < array.GetSize(); ++i) {
    //     auto item = reinterpret_cast<const asBYTE *>(array.At(i));
    //     buffer.append(*item);
    // }
    // array.Release();

    // auto c = qApp->clipboard();
    // auto mime = new QMimeData;
    // mime->setData(QStringLiteral("application/octet-stream"),
    //               buffer); // don't use setText()
    // c->setMimeData(mime);
}

QString ScriptMachine::clip_getText() { return qApp->clipboard()->text(); }

CScriptArray *ScriptMachine::clip_getBinary() {
    // QClipboard *c = qApp->clipboard();

    // QByteArray data;
    // auto d = c->mimeData();
    // data = d->data(QStringLiteral("application/octet-stream"));

    // auto engine = ScriptMachine::instance().context();
    // auto len = data.size();
    // auto arr =
    //     CScriptArray::Create(engine->GetTypeInfoByDecl("array<uint8>"), len);
    // arr->AddRef();
    // for (int i = 0; i < len; ++i) {
    //     auto addr = arr->At(i);
    //     *reinterpret_cast<char *>(addr) = data.at(i);
    // }
    // arr->Release();
    // return arr;
    return nullptr;
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

    if (_main == nullptr) {
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
    auto bytecode = Luau::compile(source.data(), Luau::CompileOptions());

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

// void WingAngelAPI::installAPI(ScriptMachine *machine) {
//     ASSERT(machine);
//     auto engine = machine->engine();

//     installBasicTypes(engine);
//     installExtAPI(engine);
//     installLogAPI(engine);
//     installMsgboxAPI(engine);
//     installInputboxAPI(engine);
//     installFileDialogAPI(engine);
//     installColorDialogAPI(engine);

//     installHexReaderAPI(engine);
//     installHexControllerAPI(engine);
//     installInvokeServiceAPI(engine);

//            // plugin script objects will be install later
// }

// void WingAngelAPI::installBasicTypes(asIScriptEngine *engine) {
//     int r = engine->SetDefaultNamespace("msgbox");
//     ASSERT(r >= 0);

//     registerAngelType<QMessageBox::StandardButtons>(engine, "buttons");
//     registerAngelType<QMessageBox::Icon>(engine, "icon");

//     r = engine->SetDefaultNamespace("inputbox");
//     ASSERT(r >= 0);

//     registerAngelType<QLineEdit::EchoMode>(engine, "EchoMode");
//     registerAngelType<Qt::InputMethodHints>(engine, "InputMethodHints");

//     r = engine->SetDefaultNamespace("filedlg");
//     ASSERT(r >= 0);

//     registerAngelType<QFileDialog::Options>(engine, "options");

//     engine->SetDefaultNamespace("");

//     installHexBaseType(engine);
// }

// void WingAngelAPI::installLogAPI(asIScriptEngine *engine) {
//     int r = engine->SetDefaultNamespace("log");
//     ASSERT(r >= 0);

//     registerAPI(
//         engine,
//         asMETHODPR(WingAngelAPI, logInfo, (const QString &) const, void),
//         "void info(const string &in message)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingAngelAPI, logTrace, (const QString &) const, void),
//         "void trace(const string &in message)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingAngelAPI, logDebug, (const QString &) const, void),
//         "void debug(const string &in message)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingAngelAPI, logWarn, (const QString &) const, void),
//         "void warn(const string &in message)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingAngelAPI, logError, (const QString &) const, void),
//         "void error(const string &in message)");

//     engine->SetDefaultNamespace("");
// }

// void WingAngelAPI::installExtAPI(asIScriptEngine *engine) {
//     // toast(message, iconPath)
//     registerAPI(
//         engine,
//         asMETHODPR(WingAngelAPI, _UI_Toast, (const QString &, const QString
//         &),
//                    void),
//         "void toast(const string &in message, const string &in icon =\"\")");
// }

// void WingAngelAPI::installMsgboxAPI(asIScriptEngine *engine) {
//     int r = engine->SetDefaultNamespace("msgbox");
//     ASSERT(r >= 0);

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _MSG_AboutQt, (const QString &),
//                 void), "void aboutQt(const string &in title =\"\")");

//     registerAPI(
//         engine,
//         asMETHODPR(WingAngelAPI, _MSG_Information,
//                    (const QString &, const QString &, int,
//                     QMessageBox::StandardButton),
//                    QMessageBox::StandardButton),
//         "void information(const string &in title, const string &in text, "
//         "int buttons = msgbox::buttons::Ok, "
//         "msgbox::buttons defaultButton = msgbox::buttons::NoButton)");

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _MSG_Question,
//                            (const QString &, const QString &, int,
//                             QMessageBox::StandardButton),
//                            QMessageBox::StandardButton),
//                 "void question(const string &in title, const string &in text,
//                 " "int buttons = msgbox::buttons::Yes | msgbox::buttons::No,
//                 " "msgbox::buttons defaultButton =
//                 msgbox::buttons::NoButton)");

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _MSG_Warning,
//                            (const QString &, const QString &, int,
//                             QMessageBox::StandardButton),
//                            QMessageBox::StandardButton),
//                 "void warning(const string &in title, const string &in text,
//                 " "int buttons = msgbox::buttons::Ok, " "msgbox::buttons
//                 defaultButton = msgbox::buttons::NoButton)");

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _MSG_Critical,
//                            (const QString &, const QString &, int,
//                             QMessageBox::StandardButton),
//                            QMessageBox::StandardButton),
//                 "void critical(const string &in title, const string &in text,
//                 " "int buttons = msgbox::buttons::Ok, " "msgbox::buttons
//                 defaultButton = msgbox::buttons::NoButton)");

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _MSG_msgbox,
//                            (QMessageBox::Icon, const QString &, const QString
//                            &,
//                             int, QMessageBox::StandardButton),
//                            QMessageBox::StandardButton),
//                 "void msgbox(msgbox::icon icon, const string &in title, "
//                 "const string &in text, "
//                 "int buttons = msgbox::buttons::NoButton, "
//                 "msgbox::buttons defaultButton =
//                 msgbox::buttons::NoButton)");

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _MSG_About,
//                            (const QString &, const QString &), void),
//                 "void about(const string &in title, const string &in text)");

//     engine->SetDefaultNamespace("");
// }

// void WingAngelAPI::installInputboxAPI(asIScriptEngine *engine) {
//     int r = engine->SetDefaultNamespace("inputbox");
//     ASSERT(r >= 0);

//     registerAPI(
//         engine,
//         asMETHODPR(WingAngelAPI, _InputBox_GetText,
//                    (const QString &, const QString &, QLineEdit::EchoMode,
//                     const QString &, bool *, int),
//                    QString),
//         "string getText(const string &in title, const string &in label, "
//         "inputbox::EchoMode echo = inputbox::EchoMode::Normal, "
//         "const string &in text = \"\", bool &out ok = void, "
//         "int inputMethodHints = inputbox::InputMethodHints::ImhNone)");

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _InputBox_GetMultiLineText,
//                            (const QString &, const QString &, const QString
//                            &,
//                             bool *, int),
//                            QString),
//                 "string getMultiLineText(const string &in title, "
//                 "const string &in label, "
//                 "const string &in text = \"\", bool &out ok = void, "
//                 "int inputMethodHints =
//                 inputbox::InputMethodHints::ImhNone)");

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _InputBox_GetInt,
//                            (const QString &, const QString &, int, int, int,
//                             int, bool *),
//                            int),
//                 "int getInt(const string &in title, const string &in label, "
//                 "int &in value = 0, int &in minValue = -2147483647, "
//                 "int &in maxValue = 2147483647, "
//                 "int &in step = 1, bool &out ok = void)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingAngelAPI, _InputBox_GetDouble,
//                    (const QString &, const QString &, double, double, double,
//                     int, bool *, double),
//                    double),
//         "double getDouble(const string &in title, const string &in label, "
//         "double &in value = 0, double &in minValue = -2147483647, "
//         "double &in maxValue = 2147483647, int &in decimals = 1, "
//         "bool &out ok = void, double &in step = 1)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingAngelAPI, _InputBox_getItem,
//                    (const QString &, const QString &, const CScriptArray &,
//                    int,
//                     bool, bool *, int),
//                    QString),
//         "string getItem(const string &in title, const string &in label, "
//         "const string[] &in items, int current = 0, "
//         "bool editable = true, bool &out ok = void, "
//         "int inputMethodHints = inputbox::InputMethodHints::ImhNone)");

//     engine->SetDefaultNamespace("");
// }

// void WingAngelAPI::installFileDialogAPI(asIScriptEngine *engine) {
//     int r = engine->SetDefaultNamespace("filedlg");
//     ASSERT(r >= 0);

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _FileDialog_GetExistingDirectory,
//                            (const QString &, const QString &, int), QString),
//                 "string getExistingDirectory(const string &in caption = \"\",
//                 " "const string &in dir = \"\", " "int options =
//                 filedlg::options::ShowDirsOnly)");

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _FileDialog_GetOpenFileName,
//                            (const QString &, const QString &, const QString
//                            &,
//                             QString *, int),
//                            QString),
//                 "string getOpenFileName(const string &in caption = \"\", "
//                 "const string &in dir = \"\", const string &in filter = \"\",
//                 " "string &out selectedFilter = void, int options = 0)");

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _FileDialog_GetSaveFileName,
//                            (const QString &, const QString &, const QString
//                            &,
//                             QString *, int),
//                            QString),
//                 "string getSaveFileName(const string &in caption = \"\", "
//                 "const string &in dir = \"\", const string &in filter = \"\",
//                 " "string &out selectedFilter = void, int options = 0)");

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _FileDialog_getOpenFileNames,
//                            (const QString &, const QString &, const QString
//                            &,
//                             QString *, int),
//                            CScriptArray *),
//                 "string[]@ getOpenFileNames(const string &in caption = \"\",
//                 " "const string &in dir = \"\", const string &in filter =
//                 \"\", " "string &out selectedFilter = void, int options =
//                 0)");

//     engine->SetDefaultNamespace("");
// }

// void WingAngelAPI::installColorDialogAPI(asIScriptEngine *engine) {
//     int r = engine->SetDefaultNamespace("colordlg");
//     ASSERT(r >= 0);

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _Color_get, (const QString &),
//                 QColor), "color getColor(const string &in caption)");

//     engine->SetDefaultNamespace("");
// }

// void WingAngelAPI::installHexBaseType(asIScriptEngine *engine) {
//     registerAngelType<WingHex::ErrFile>(engine, "ErrFile");
//     registerAngelType<WingHex::SelectionMode>(engine, "SelectionMode");

//            // HexPosition
//     auto r = engine->RegisterObjectType(
//         "HexPosition", sizeof(WingHex::HexPosition),
//         asOBJ_VALUE | asOBJ_POD | ::asGetTypeTraits<WingHex::HexPosition>());
//     ASSERT(r >= 0);

//     r = engine->RegisterObjectProperty("HexPosition", QSIZETYPE_WRAP("line"),
//                                        asOFFSET(WingHex::HexPosition, line));
//     ASSERT(r >= 0);

//     r = engine->RegisterObjectProperty("HexPosition", "int column",
//                                        asOFFSET(WingHex::HexPosition,
//                                        column));
//     ASSERT(r >= 0);

//     r = engine->RegisterObjectProperty(
//         "HexPosition", "uint8 lineWidth",
//         asOFFSET(WingHex::HexPosition, lineWidth));
//     ASSERT(r >= 0);

//     r = engine->RegisterObjectProperty(
//         "HexPosition", "int nibbleindex",
//         asOFFSET(WingHex::HexPosition, nibbleindex));
//     ASSERT(r >= 0);

//     r = engine->RegisterObjectMethod("HexPosition",
//     QSIZETYPE_WRAP("offset()"),
//                                      asMETHOD(WingHex::HexPosition, offset),
//                                      asCALL_THISCALL);
//     ASSERT(r >= 0);

//     r = engine->RegisterObjectMethod(
//         "HexPosition", "int opSub(const HexPosition &in) const",
//         asMETHODPR(WingHex::HexPosition, operator-,
//                    (const WingHex::HexPosition &) const, qsizetype),
//         asCALL_THISCALL);
//     ASSERT(r >= 0);

//     r = engine->RegisterObjectMethod(
//         "HexPosition", "bool opEquals(const HexPosition &in) const",
//         asMETHODPR(WingHex::HexPosition, operator==,
//                    (const WingHex::HexPosition &) const, bool),
//         asCALL_THISCALL);
//     ASSERT(r >= 0);

//            // MetadataInfo
//     r = engine->RegisterObjectType(
//         "MetadataInfo", sizeof(WingHex::MetadataInfo),
//         asOBJ_VALUE | asOBJ_POD |
//         ::asGetTypeTraits<WingHex::MetadataInfo>());
//     ASSERT(r >= 0);

//     r = engine->RegisterObjectProperty("MetadataInfo",
//     QSIZETYPE_WRAP("begin"),
//                                        asOFFSET(WingHex::MetadataInfo,
//                                        begin));
//     ASSERT(r >= 0);
//     r = engine->RegisterObjectProperty("MetadataInfo", QSIZETYPE_WRAP("end"),
//                                        asOFFSET(WingHex::MetadataInfo, end));
//     ASSERT(r >= 0);
//     r = engine->RegisterObjectProperty(
//         "MetadataInfo", "color foreground",
//         asOFFSET(WingHex::MetadataInfo, foreground));
//     ASSERT(r >= 0);
//     r = engine->RegisterObjectProperty(
//         "MetadataInfo", "color background",
//         asOFFSET(WingHex::MetadataInfo, background));
//     ASSERT(r >= 0);
//     r = engine->RegisterObjectProperty(
//         "MetadataInfo", "string comment",
//         asOFFSET(WingHex::MetadataInfo, comment));
//     ASSERT(r >= 0);
// }

// void WingAngelAPI::installHexReaderAPI(asIScriptEngine *engine) {
//     int r = engine->SetDefaultNamespace("reader");
//     ASSERT(r >= 0);

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, isCurrentDocEditing,
//                            (void) const, bool),
//                 "bool isCurrentDocEditing()");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, currentDocFile, (void) const, QUrl),
//         "url currentDocFile()");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, currentDocFileName,
//                            (void) const, QString),
//                 "string currentDocFileName()");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, currentDocWorkSpace,
//                            (void) const, QUrl),
//                 "url currentDocWorkSpace()");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, currentDocWorkSpaceName,
//                            (void) const, QString),
//                 "string currentDocWorkSpaceName()");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, isInsertionMode, (void) const,
//         bool), "bool isInsertionMode()");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, isReadOnly, (void) const, bool),
//         "bool isReadOnly()");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, isKeepSize, (void) const, bool),
//         "bool isKeepSize()");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, isLocked, (void) const,
//                 bool), "bool isLocked()");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, currentPos, (void) const,
//                            WingHex::HexPosition),
//                 "HexPosition currentPos()");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, stringVisible, (void) const, bool),
//         "bool stringVisible()");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, addressVisible, (void) const, bool),
//         "bool addressVisible()");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, headerVisible, (void) const, bool),
//         "bool headerVisible()");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, isModified, (void) const, bool),
//         "bool isModified()");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, documentLines, (void) const,
//                            qsizetype),
//                 QSIZETYPE_WRAP("documentLines()"));
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, documentBytes, (void) const,
//                            qsizetype),
//                 QSIZETYPE_WRAP("documentBytes()"));
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, currentRow, (void) const,
//         qsizetype), QSIZETYPE_WRAP("currentRow()"));
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, currentColumn, (void) const,
//                            qsizetype),
//                 QSIZETYPE_WRAP("currentColumn()"));
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, currentOffset, (void) const,
//                            qsizetype),
//                 QSIZETYPE_WRAP("currentOffset()"));
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, selectedLength, (void)
//                 const,
//                            qsizetype),
//                 QSIZETYPE_WRAP("selectedLength()"));

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _HexReader_selectedBytes,
//                 (qsizetype),
//                            CScriptArray *),
//                 "byte[]@ selectedBytes(" QSIZETYPE " index)");
//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _HexReader_selectionBytes, (void),
//                            CScriptArray *),
//                 "byte[][]@ selectionBytes()");

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, selectionStart,
//                            (qsizetype) const, WingHex::HexPosition),
//                 "HexPosition selectionStart(" QSIZETYPE " index)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, selectionEnd,
//                            (qsizetype) const, WingHex::HexPosition),
//                 "HexPosition selectionEnd(" QSIZETYPE " index)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, selectionLength,
//                            (qsizetype) const, qsizetype),
//                 QSIZETYPE_WRAP("selectionLength(" QSIZETYPE " index)"));
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, selectionCount, () const,
//         qsizetype), QSIZETYPE_WRAP("selectionCount()"));

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, addressBase, (void) const,
//         quintptr), QPTR_WRAP("addressBase()"));

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _HexReader_readBytes,
//                            (qsizetype, qsizetype), CScriptArray *),
//                 "byte[]@ readBytes(" QSIZETYPE " offset," QSIZETYPE " len)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, readInt8, (qsizetype) const, qint8),
//         "int8 readInt8(" QSIZETYPE " offset)");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, readUInt8, (qsizetype) const,
//         quint8), "uint8 readUInt8(" QSIZETYPE " offset)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, readInt16, (qsizetype) const,
//         qint16), "int16 readInt16(" QSIZETYPE " offset)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, readUInt16, (qsizetype)
//                 const,
//                            quint16),
//                 "uint16 readUInt16(" QSIZETYPE " offset)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, readInt32, (qsizetype) const,
//         qint32), "int readInt32(" QSIZETYPE " offset)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, readUInt32, (qsizetype)
//                 const,
//                            quint32),
//                 "uint readUInt32(" QSIZETYPE " offset)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, readInt64, (qsizetype) const,
//         qint64), "int64 readInt64(" QSIZETYPE " offset)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, readUInt64, (qsizetype)
//                 const,
//                            quint64),
//                 "uint64 readUInt64(" QSIZETYPE " offset)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, readFloat, (qsizetype) const,
//         float), "float readFloat(" QSIZETYPE " offset)");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, readDouble, (qsizetype) const,
//         double), "double readDouble(" QSIZETYPE " offset)");

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, readString,
//                            (qsizetype, const QString &) const, QString),
//                 "string readString(" QSIZETYPE
//                 " offset, string &in encoding = \"\")");

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _HexReader_findNext,
//                            (qsizetype, const CScriptArray &), qsizetype),
//                 QSIZETYPE_WRAP("findNext(" QSIZETYPE " begin, byte[] &in
//                 ba)"));
//     registerAPI(
//         engine,
//         asMETHODPR(WingAngelAPI, _HexReader_findPrevious,
//                    (qsizetype, const CScriptArray &), qsizetype),
//         QSIZETYPE_WRAP("findPrevious(" QSIZETYPE " begin, byte[] &in ba)"));

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, bookMarkComment,
//                            (qsizetype) const, QString),
//                 "string bookMarkComment(" QSIZETYPE " pos)");

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, existBookMark,
//                            (qsizetype) const, bool),
//                 "bool existBookMark(" QSIZETYPE " pos)");

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, metadataInfo,
//                            (qsizetype) const, WingHex::MetadataInfo),
//                 "MetadataInfo metadataInfo(" QSIZETYPE " offset)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, metadataInfoByIndex,
//                            (qsizetype) const, WingHex::MetadataInfo),
//                 "MetadataInfo metadataInfoByIndex(" QSIZETYPE " index)");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, bookMarkCount, () const, qint64),
//         "int64 bookMarkCount()");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, bookMarkPos, (qsizetype)
//                 const,
//                            qint64),
//                 "int64 bookMarkPos(" QSIZETYPE " index)");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, metadataCount, () const, qint64),
//         "int64 metadataCount()");

//     engine->SetDefaultNamespace("");
// }

// void WingAngelAPI::installHexControllerAPI(asIScriptEngine *engine) {
//     int r = engine->SetDefaultNamespace("ctl");
//     ASSERT(r >= 0);

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, switchDocument, (int),
//                 bool), "bool switchDocument(int handle)");

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, setLockedFile, (bool),
//                 bool), "bool setLockedFile(bool b)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, setKeepSize, (bool), bool),
//                 "bool setKeepSize(bool b)");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, setStringVisible, (bool), bool),
//         "bool setStringVisible(bool b)");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, setAddressVisible, (bool), bool),
//         "bool setAddressVisible(bool b)");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, setHeaderVisible, (bool), bool),
//         "bool setHeaderVisible(bool b)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, setAddressBase, (quintptr), bool),
//         "bool setAddressBase(" QPTR " base)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, beginMarco, (const QString &),
//         bool), "bool beginMarco(string &in name = \"\")");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, endMarco, (void), bool),
//                 "bool endMarco()");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, isMacroEmpty, (void) const, bool),
//         "bool isMacroEmpty()");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, resetMarco, (void), bool),
//                 "bool resetMarco()");

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, writeInt8, (qsizetype, qint8),
//         bool), "bool writeInt8(" QSIZETYPE " offset, int8 value)");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, writeUInt8, (qsizetype, quint8),
//         bool), "bool writeUInt8(" QSIZETYPE " offset, uint8 value)");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, writeInt16, (qsizetype, qint16),
//         bool), "bool writeInt16(" QSIZETYPE " offset, int16 value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, writeUInt16,
//                            (qsizetype, quint16), bool),
//                 "bool writeUInt16(" QSIZETYPE " offset, uint16 value)");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, writeInt32, (qsizetype, qint32),
//         bool), "bool writeInt32(" QSIZETYPE " offset, int value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, writeUInt32,
//                            (qsizetype, quint32), bool),
//                 "bool writeUInt32(" QSIZETYPE " offset, uint value)");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, writeInt64, (qsizetype, qint64),
//         bool), "bool writeInt64(" QSIZETYPE " offset, int64 value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, writeUInt64,
//                            (qsizetype, quint64), bool),
//                 "bool writeUInt64(" QSIZETYPE " offset, uint64 value)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, writeFloat, (qsizetype, float),
//         bool), "bool writeFloat(" QSIZETYPE " offset, float value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, writeDouble,
//                            (qsizetype, double), bool),
//                 "bool writeDouble(" QSIZETYPE " offset, double value)");

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _HexController_writeBytes,
//                            (qsizetype, const CScriptArray &), bool),
//                 "bool writeBytes(" QSIZETYPE " offset, byte[] &in data)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, writeString,
//                            (qsizetype, const QString &, const QString &),
//                            bool),
//                 "bool writeString(" QSIZETYPE " offset, string &in value, "
//                 "string &in encoding = \"\")");

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, insertInt8, (qsizetype, qint8),
//         bool), "bool insertInt8(" QSIZETYPE " offset, int8 value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, insertUInt8,
//                            (qsizetype, quint8), bool),
//                 "bool insertUInt8(" QSIZETYPE " offset, uint8 value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, insertInt16,
//                            (qsizetype, qint16), bool),
//                 "bool insertInt16(" QSIZETYPE " offset, int16 value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, insertUInt16,
//                            (qsizetype, quint16), bool),
//                 "bool insertUInt16(" QSIZETYPE " offset, uint16 value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, insertInt32,
//                            (qsizetype, qint32), bool),
//                 "bool insertInt32(" QSIZETYPE " offset, int value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, insertUInt32,
//                            (qsizetype, quint32), bool),
//                 "bool insertUInt32(" QSIZETYPE " offset, uint value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, insertInt64,
//                            (qsizetype, qint64), bool),
//                 "bool insertInt64(" QSIZETYPE " offset, int64 value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, insertUInt64,
//                            (qsizetype, quint64), bool),
//                 "bool insertUInt64(" QSIZETYPE " offset, uint64 value)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, insertFloat, (qsizetype, float),
//         bool), "bool insertFloat(" QSIZETYPE " offset, float value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, insertDouble,
//                            (qsizetype, double), bool),
//                 "bool insertDouble(" QSIZETYPE " offset, double value)");

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _HexController_insertBytes,
//                            (qsizetype, const CScriptArray &), bool),
//                 "bool insertBytes(" QSIZETYPE " offset, byte[] &in data)");

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, insertString,
//                            (qsizetype, const QString &, const QString &),
//                            bool),
//                 "bool insertString(" QSIZETYPE " offset, string &in value, "
//                 "string &in encoding = \"\")");

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, appendInt8, (qint8), bool),
//                 "bool appendInt8(int8 value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, appendUInt8, (quint8),
//                 bool), "bool appendUInt8(uint8 value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, appendInt16, (qint16),
//                 bool), "bool appendInt16(int16 value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, appendUInt16, (quint16),
//                 bool), "bool appendUInt16(uint16 value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, appendInt32, (qint32),
//                 bool), "bool appendInt32(int value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, appendUInt32, (quint32),
//                 bool), "bool appendUInt32(uint value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, appendInt64, (qint64),
//                 bool), "bool appendInt64(int64 value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, appendUInt64, (quint64),
//                 bool), "bool appendUInt64(uint value)");

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, appendFloat, (float), bool),
//                 "bool appendFloat(float value)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, appendDouble, (double),
//                 bool), "bool appendDouble(double value)");

//     registerAPI(engine,
//                 asMETHODPR(WingAngelAPI, _HexController_appendBytes,
//                            (const CScriptArray &), bool),
//                 "bool appendBytes(byte[] &in data)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, appendString,
//                    (const QString &, const QString &), bool),
//         "bool appendString(string &in value, string &in encoding = \"\")");

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, removeBytes,
//                            (qsizetype, qsizetype), bool),
//                 "bool removeBytes(" QSIZETYPE " offset, " QSIZETYPE " len)");

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, moveTo,
//                            (qsizetype, qsizetype, int, bool), bool),
//                 "bool moveTo(" QSIZETYPE " line, " QSIZETYPE
//                 " column, int nibbleindex = -1, bool clearSelection =
//                 true)");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, moveTo, (qsizetype, bool), bool),
//         "bool moveTo(" QSIZETYPE " offset, bool clearSelection = true)");

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, select,
//                            (qsizetype, qsizetype, WingHex::SelectionMode),
//                            bool),
//                 "bool select(" QSIZETYPE " offset, " QSIZETYPE
//                 " len, SelectionMode mode = SelectionMode::Add)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, setInsertionMode, (bool), bool),
//         "bool setInsertionMode(bool b)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, metadata,
//                    (qsizetype, qsizetype, const QColor &, const QColor &,
//                     const QString &),
//                    bool),
//         "bool metadata(" QSIZETYPE " begin, " QSIZETYPE
//         " length, color &in fgcolor, color &in bgcolor, string &in
//         comment)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, removeMetadata, (qsizetype), bool),
//         "bool removeMetadata(" QSIZETYPE " offset)");

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, clearMetadata, (), bool),
//                 "bool clearMetadata()");

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, foreground,
//                            (qsizetype, qsizetype, const QColor &), bool),
//                 "bool foreground(" QSIZETYPE " begin, " QSIZETYPE
//                 " length, color &in fgcolor)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, background,
//                            (qsizetype, qsizetype, const QColor &), bool),
//                 "bool background(" QSIZETYPE " begin, " QSIZETYPE
//                 " length, color &in bgcolor)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, comment,
//                            (qsizetype, qsizetype, const QString &), bool),
//                 "bool comment(" QSIZETYPE " begin, " QSIZETYPE
//                 " length, string &in comment)");

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, setMetaVisible, (bool),
//                 bool), "bool setMetaVisible(bool b)");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, setMetafgVisible, (bool), bool),
//         "bool setMetafgVisible(bool b)");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, setMetabgVisible, (bool), bool),
//         "bool setMetabgVisible(bool b)");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, setMetaCommentVisible, (bool),
//         bool), "bool setMetaCommentVisible(bool b)");

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, openFile, (const QUrl &),
//                 int), "int openFile(url &in file)");

//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, closeFile, (int), WingHex::ErrFile),
//         "ErrFile closeFile(int handle)");

//     registerAPI(engine, asMETHODPR(WingHex::IWingPlugin, openCurrent, (),
//     int),
//                 "int openCurrent()");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, closeCurrent, (), WingHex::ErrFile),
//         "ErrFile closeCurrent()");

//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, addBookMark,
//                            (qsizetype, const QString &), bool),
//                 "bool addBookMark(" QSIZETYPE " pos, string &in comment)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, modBookMark,
//                            (qsizetype, const QString &), bool),
//                 "bool modBookMark(" QSIZETYPE " pos, string &in comment)");
//     registerAPI(
//         engine,
//         asMETHODPR(WingHex::IWingPlugin, removeBookMark, (qsizetype), bool),
//         "bool removeBookMark(" QSIZETYPE " pos)");
//     registerAPI(engine,
//                 asMETHODPR(WingHex::IWingPlugin, clearBookMark, (), bool),
//                 "bool clearBookMark()");

//     engine->SetDefaultNamespace("");
// }

// void WingAngelAPI::installInvokeServiceAPI(asIScriptEngine *engine) {
//     auto r = engine->RegisterGlobalFunction(
//         "bool invokeService(const string&in puid, const string&in method, "
//         "?&out result, const ?&in ...)",
//         asFUNCTION(_invoke_service), asCALL_GENERIC);
//     ASSERT(r >= 0);

//     r = engine->RegisterGlobalFunction(
//         "bool invokeService(const string&in puid, const string&in method, "
//         "?&out result)",
//         asFUNCTION(_invoke_service), asCALL_GENERIC);
//     ASSERT(r >= 0);

//     r = engine->RegisterGlobalFunction(
//         "bool invokeService(const string&in puid, const string&in method)",
//         asFUNCTION(_invoke_service_p_r), asCALL_GENERIC);
//     ASSERT(r >= 0);

//     r = engine->RegisterGlobalFunction(
//         "bool invokeServiceVoid(const string&in puid, const string&in method,
//         " "const ?&in ...)", asFUNCTION(_invoke_service_p_p),
//         asCALL_GENERIC);
//     ASSERT(r >= 0);
// }
