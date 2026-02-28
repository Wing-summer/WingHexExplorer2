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

#include "scriptmachine.h"

#include "grammar/ASConsole/AngelscriptConsoleLexer.h"
#include "grammar/ASConsole/AngelscriptConsoleParser.h"

#include "AngelScript/sdk/add_on/autowrapper/aswrappedcall.h"
#include "AngelScript/sdk/add_on/scriptgrid/scriptgrid.h"
#include "AngelScript/sdk/add_on/scripthandle/scripthandle.h"
#include "AngelScript/sdk/add_on/scriptmath/scriptmath.h"
#include "AngelScript/sdk/add_on/scriptmath/scriptmathcomplex.h"
#include "AngelScript/sdk/add_on/weakref/weakref.h"
#include "AngelScript/sdk/angelscript/source/as_scriptengine.h"

#include "class/angelscriptconsolevisitor.h"
#include "class/appmanager.h"
#include "class/asbuilder.h"
#include "class/logger.h"
#include "class/pluginsystem.h"
#include "class/settingmanager.h"
#include "define.h"

#include "scriptaddon/scriptany.h"
#include "scriptaddon/scriptcolor.h"
#include "scriptaddon/scriptcrypto.h"
#include "scriptaddon/scriptdatetime.h"
#include "scriptaddon/scriptenv.h"
#include "scriptaddon/scriptfile.h"
#include "scriptaddon/scriptfilesystem.h"
#include "scriptaddon/scriptjson.h"
#include "scriptaddon/scriptoptbox.h"
#include "scriptaddon/scriptqdictionary.h"
#include "scriptaddon/scriptqstring.h"
#include "scriptaddon/scriptreflection.h"
#include "scriptaddon/scriptregex.h"
#include "scriptaddon/scripturl.h"

#include "class/fmtlibext.h"

#include <fmt/ranges.h>

#include <QClipboard>
#include <QMimeData>
#include <QProcess>
#include <QScopeGuard>

bool ScriptMachine::init() {
    if (isInited()) {
        return true;
    }

    qRegisterMetaType<MessageInfo>();

    _engine = asCreateScriptEngine();
    if (!ScriptMachine::configureEngine()) {
        _engine->ShutDownAndRelease();
        _engine = nullptr;
        return false;
    }

    // create the debugger
    _workspace = new asIDBFileWorkspace("", _engine);
    _debugger = new asDebugger(_workspace);
    return true;
}

bool ScriptMachine::isInited() const { return _engine != nullptr; }

bool ScriptMachine::isRunning(ConsoleMode mode) const {
    return _ctx.value(mode) != nullptr;
}

bool ScriptMachine::checkEngineConfigError() const {
    if (_engine) {
        auto e = static_cast<asCScriptEngine *>(_engine);
        if (e) {
            e->PrepareEngine();
            return e->configFailed;
        }
    }
    return true;
}

bool ScriptMachine::configureEngine() {
    if (_engine == nullptr) {
        return false;
    }

    // we need utf8, the default is what we want
    _engine->SetEngineProperty(asEP_EXPAND_DEF_ARRAY_TO_TMPL, true);
    _engine->SetEngineProperty(asEP_DISALLOW_EMPTY_LIST_ELEMENTS, true);
    _engine->SetEngineProperty(asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE, false);
    _engine->SetEngineProperty(asEP_ALLOW_MULTILINE_STRINGS, false);
    _engine->SetEngineProperty(asEP_USE_CHARACTER_LITERALS, false);
    _engine->SetEngineProperty(asEP_DISABLE_INTEGER_DIVISION, false);
    _engine->SetEngineProperty(asEP_PRIVATE_PROP_AS_PROTECTED, false);
    _engine->SetEngineProperty(asEP_ALTER_SYNTAX_NAMED_ARGS, 0);
    _engine->SetEngineProperty(asEP_ALLOW_UNICODE_IDENTIFIERS, true);
    _engine->SetEngineProperty(asEP_REQUIRE_ENUM_SCOPE,
                               true); // enum class like

    // The script compiler will send any compiler messages to the callback
    auto r = _engine->SetMessageCallback(asFUNCTION(messageCallback), this,
                                         asCALL_CDECL);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    _engine->SetFunctionUserDataCleanupCallback(
        &ScriptMachine::cleanUpPluginSysIDFunction,
        AsUserDataType::UserData_PluginFn);

    registerEngineAddon(_engine);

    _engine->SetDefaultAccessMask(0x1);

    // Register a couple of extra functions for the scripts
    r = _engine->RegisterGlobalFunction(
        "void print(const ? &in obj, const ? &in ...)", asFUNCTION(print),
        asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }
    r = _engine->RegisterGlobalFunction(
        "void printf(const string &in fmt, const ? &in ...)",
        asFUNCTION(printf), asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }
    r = _engine->RegisterGlobalFunction(
        "void println(const ? &in obj, const ? &in ...)", asFUNCTION(println),
        asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction(
        "void warnprint(const ? &in obj, const ? &in ...)",
        asFUNCTION(warnprint), asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }
    r = _engine->RegisterGlobalFunction(
        "void warnprintf(const string &in fmt, const ? &in ...)",
        asFUNCTION(warnprintf), asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }
    r = _engine->RegisterGlobalFunction(
        "void warnprintln(const ? &in obj, const ? &in ...)",
        asFUNCTION(warnprintln), asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction(
        "void infoprint(const ? &in obj, const ? &in ...)",
        asFUNCTION(infoprint), asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }
    r = _engine->RegisterGlobalFunction(
        "void infoprintf(const string &in fmt, const ? &in ...)",
        asFUNCTION(infoprintf), asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }
    r = _engine->RegisterGlobalFunction(
        "void infoprintln(const ? &in obj, const ? &in ...)",
        asFUNCTION(infoprintln), asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction(
        "void errprint(const ? &in obj, const ? &in ...)", asFUNCTION(errprint),
        asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }
    r = _engine->RegisterGlobalFunction(
        "void errprintf(const string &in fmt, const ? &in ...)",
        asFUNCTION(errprintf), asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }
    r = _engine->RegisterGlobalFunction(
        "void errprintln(const ? &in obj, const ? &in ...)",
        asFUNCTION(errprintln), asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction("string input()",
                                        asMETHOD(ScriptMachine, input),
                                        asCALL_THISCALL_ASGLOBAL, this);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction("string stringify(? &in obj)",
                                        asMETHOD(ScriptMachine, stringify),
                                        asCALL_THISCALL_ASGLOBAL, this);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction(
        "int exec(string &out output, const string &in exe, "
        "const string &in params = \"\", int timeout = 3000)",
        asFUNCTION(execSystemCmd), asCALL_CDECL);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    _ctxMgr = new CContextMgr;
    _ctxMgr->RegisterCoRoutineSupport(_engine);
    _ctxMgr->SetGetTimeCallback([]() -> asUINT {
        return AppManager::instance()->currentMSecsSinceEpoch();
    });
    _ctxMgr->RegisterThreadSupport(_engine);

    // Tell the engine to use our context pool. This will also
    // allow us to debug internal script calls made by the engine
    r = _engine->SetContextCallbacks(requestContextCallback,
                                     returnContextCallback, this);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    PluginSystem::instance().angelApi()->installAPI(this);

    // create module for Console
    auto mod = _engine->GetModule("WINGCONSOLE", asGM_ALWAYS_CREATE);
    mod->SetAccessMask(0x1);

    return true;
}

QString ScriptMachine::getCallStack(asIScriptContext *context) {
    QString str = QStringLiteral("AngelScript callstack:\n");

    // Append the call stack
    for (asUINT i = 0; i < context->GetCallstackSize(); i++) {
        asIScriptFunction *func;
        const char *scriptSection;
        int line, column;
        func = context->GetFunction(i);
        line = context->GetLineNumber(i, &column, &scriptSection);
        str +=
            (QStringLiteral("\t") + QString::fromUtf8(scriptSection) +
             QStringLiteral(":") + QString::fromUtf8(func->GetDeclaration()) +
             QStringLiteral(":") + QString::number(line) + QStringLiteral(",") +
             QString::number(column) + QStringLiteral("\n"));
    }

    return str;
}

void ScriptMachine::printf(asIScriptGeneric *args) {
    __outputfmt(MessageType::Print, args);
}

void ScriptMachine::destoryMachine() {
    if (_engine) {
        _ctxMgr->AbortAll();
        delete _ctxMgr;

        delete _debugger;
        delete _workspace;

        _engine->ShutDownAndRelease();
        _engine = nullptr;
    }
}

void ScriptMachine::setCustomEvals(
    const QHash<std::string_view, WingHex::IWingAngel::Evaluator> &evals) {
    _debugger->setCustomEvals(evals);
}

void ScriptMachine::exceptionCallback(asIScriptContext *context) {
    if (context) {
        ConsoleMode mode = ConsoleMode(reinterpret_cast<asPWORD>(
            context->GetUserData(AsUserDataType::UserData_ContextMode)));
        QString message =
            QStringLiteral("- Exception '%1' in '%2'\n")
                .arg(context->GetExceptionString(),
                     context->GetExceptionFunction()->GetDeclaration()) +
            getCallStack(context);

        const char *section;
        int col;
        MessageInfo msg;
        msg.mode = mode;
        msg.row = context->GetExceptionLineNumber(&col, &section);
        msg.col = col;
        msg.type = MessageType::Error;
        msg.message = message;

        outputMessage(msg);
    }
}

void ScriptMachine::attachDebugBreak(asIScriptContext *ctx) {
    if (!ctx)
        ctx = asGetActiveContext();

    checkDebugger(ctx);

    if (_debugger)
        _debugger->DebugBreak(ctx);
}

void ScriptMachine::checkDebugger(asIScriptContext *ctx) {
    if (_debugger == nullptr) {
        return;
    }
    // hook the context
    if (_debugger->HasWork()) {
        _debugger->HookContext(ctx, true);
    } else {
        _debugger->HookContext(ctx, false);
        ctx->SetLineCallback(asFUNCTION(ScriptMachine::lineCallback), nullptr,
                             asCALL_CDECL);
    }
}

void ScriptMachine::__output(MessageType type, asIScriptGeneric *args) {
    auto context = asGetActiveContext();
    if (context) {
        ConsoleMode mode = ConsoleMode(reinterpret_cast<asPWORD>(
            context->GetUserData(AsUserDataType::UserData_ContextMode)));

        auto &m = ScriptMachine::instance();

        MessageInfo info;
        info.mode = mode;
        info.type = type;

        for (int i = 0; i < args->GetArgCount(); ++i) {
            void *ref = args->GetArgAddress(i);
            int typeId = args->GetArgTypeId(i);
            info.message.append(m.stringify(ref, typeId));
        }

        m.outputMessage(info);
    }
}

void ScriptMachine::print(asIScriptGeneric *args) {
    __output(MessageType::Print, args);
}

void ScriptMachine::__outputln(MessageType type, asIScriptGeneric *args) {
    auto context = asGetActiveContext();
    if (context) {
        ConsoleMode mode = ConsoleMode(reinterpret_cast<asPWORD>(
            context->GetUserData(AsUserDataType::UserData_ContextMode)));

        auto &m = ScriptMachine::instance();

        MessageInfo info;
        info.mode = mode;
        info.type = type;

        for (int i = 0; i < args->GetArgCount(); ++i) {
            void *ref = args->GetArgAddress(i);
            int typeId = args->GetArgTypeId(i);

            if (typeId) {
                info.message = (m.stringify(ref, typeId).append('\n'));
                m.outputMessage(info);
            }
        }
    }
}

void ScriptMachine::__outputfmt(MessageType type, asIScriptGeneric *args) {
    auto context = asGetActiveContext();
    if (context) {
        auto fmt = static_cast<const QString *>(args->GetArgObject(0));
        fmt::dynamic_format_arg_store<fmt::format_context> store;
        auto total = args->GetArgCount();

        auto &m = ScriptMachine::instance();
        for (int i = 1; i < total; ++i) {
            auto typeId = args->GetArgTypeId(i);
            void *ref = args->GetArgAddress(i);

            switch (typeId) {
            case asTYPEID_BOOL:
                store.push_back(*static_cast<bool *>(ref));
                break;
            case asTYPEID_INT8:
                store.push_back(*static_cast<qint8 *>(ref));
                break;
            case asTYPEID_INT16:
                store.push_back(*static_cast<qint16 *>(ref));
                break;
            case asTYPEID_INT32:
                store.push_back(*static_cast<qint32 *>(ref));
                break;
            case asTYPEID_INT64:
                store.push_back(*static_cast<qint64 *>(ref));
                break;
            case asTYPEID_UINT8:
                store.push_back(*static_cast<quint8 *>(ref));
                break;
            case asTYPEID_UINT16:
                store.push_back(*static_cast<quint16 *>(ref));
                break;
            case asTYPEID_UINT32:
                store.push_back(*static_cast<quint32 *>(ref));
                break;
            case asTYPEID_UINT64:
                store.push_back(*static_cast<quint64 *>(ref));
                break;
            case asTYPEID_FLOAT:
                store.push_back(*static_cast<float *>(ref));
                break;
            case asTYPEID_DOUBLE:
                store.push_back(*static_cast<double *>(ref));
                break;
            default: {
                if (m.isAngelChar(typeId)) {
                    // char
                    store.push_back(*resolveObjAs<QChar>(ref, typeId));
                } else if (m.isAngelString(typeId)) {
                    // string
                    store.push_back(*resolveObjAs<QString>(ref, typeId));
                } else if (m.isAngelArray(typeId)) {
                    // array<?>
                    store.push_back(CScriptArrayView(
                        resolveObjAs<CScriptArray>(ref, typeId)));
                } else if (m.isAngelDictionary(typeId)) {
                    // dictionary
                    store.push_back(CScriptDictionaryView(
                        resolveObjAs<CScriptDictionary>(ref, typeId)));
                } else {
                    store.push_back(m.stringify_helper(ref, typeId));
                }
            } break;
            }
        }

        auto fmtStr = fmt->toUtf8();
        ConsoleMode mode = ConsoleMode(reinterpret_cast<asPWORD>(
            context->GetUserData(AsUserDataType::UserData_ContextMode)));
        MessageInfo info;
        info.mode = mode;
        try {
            auto r = fmt::vformat({}, fmtStr.data(), store);
            info.type = type;
            info.message = QString::fromStdString(r);
        } catch (const std::exception &ex) {
            info.type = MessageType::Error;
            info.message = QString::fromStdString(ex.what());
        }
        m.outputMessage(info);
    }
}

void ScriptMachine::println(asIScriptGeneric *args) {
    __outputln(MessageType::Print, args);
}

void ScriptMachine::warnprint(asIScriptGeneric *args) {
    __output(MessageType::Warn, args);
}

void ScriptMachine::warnprintf(asIScriptGeneric *args) {
    __outputfmt(MessageType::Warn, args);
}

void ScriptMachine::warnprintln(asIScriptGeneric *args) {
    __outputln(MessageType::Warn, args);
}

void ScriptMachine::errprint(asIScriptGeneric *args) {
    __output(MessageType::Error, args);
}

void ScriptMachine::errprintf(asIScriptGeneric *args) {
    __outputfmt(MessageType::Error, args);
}

void ScriptMachine::errprintln(asIScriptGeneric *args) {
    __outputln(MessageType::Error, args);
}

void ScriptMachine::infoprint(asIScriptGeneric *args) {
    __output(MessageType::Info, args);
}

void ScriptMachine::infoprintf(asIScriptGeneric *args) {
    __outputfmt(MessageType::Info, args);
}

void ScriptMachine::infoprintln(asIScriptGeneric *args) {
    __outputln(MessageType::Info, args);
}

QString ScriptMachine::input() {
    auto context = asGetActiveContext();
    if (context) {
        ConsoleMode mode = ConsoleMode(reinterpret_cast<asPWORD>(
            context->GetUserData(AsUserDataType::UserData_ContextMode)));

        auto cbs = _regcalls.value(mode);
        if (cbs.getInputFn) {
            return cbs.getInputFn();
        }
    }
    return {};
}

void ScriptMachine::outputMessage(const MessageInfo &info) {
    auto cbs = _regcalls.value(info.mode);
    if (cbs.printMsgFn) {
        cbs.printMsgFn(info);
    }
}

QString ScriptMachine::getGlobalDecls() const {
    if (!_cachedGlobalStrs.isEmpty()) {
        return _cachedGlobalStrs;
    }

    auto mod = module(ScriptMachine::Interactive);
    if (mod == nullptr) {
        return {};
    }

    auto total = mod->GetGlobalVarCount();
    for (asUINT n = 0; n < total; n++) {
        auto decl = mod->GetGlobalVarDeclaration(n, true);
        _cachedGlobalStrs.append(decl).append(';');
    }
    return _cachedGlobalStrs;
}

int ScriptMachine::execSystemCmd(QString &out, const QString &exe,
                                 const QString &params, int timeout) {
    if (Utilities::isRoot()) {
        auto ctx = asGetActiveContext();
        if (ctx) {
            ctx->SetException(
                "Running this with root privileges is not allowed");
        }
        return -1;
    }
    QProcess ps;
    ps.setProgram(exe);
    ps.setArguments(QProcess::splitCommand(params));
    ps.start();
    if (ps.waitForFinished(timeout)) {
        out = QString::fromUtf8(ps.readAllStandardOutput());
        return ps.exitCode();
    } else {
        ps.kill();
        return -1;
    }
}

QString ScriptMachine::stringify(void *ref, int typeId) {
    return QString::fromStdString(stringify_helper(ref, typeId));
}

std::string ScriptMachine::getAsTypeName(int typeId) {
    auto type = _engine->GetTypeInfoById(typeId);
    std::string name;

    if (!type) {
        // a primitive
        switch (typeId & asTYPEID_MASK_SEQNBR) {
        case asTYPEID_BOOL:
            name = "bool";
            break;
        case asTYPEID_INT8:
            name = "int8";
            break;
        case asTYPEID_INT16:
            name = "int16";
            break;
        case asTYPEID_INT32:
            name = "int32";
            break;
        case asTYPEID_INT64:
            name = "int64";
            break;
        case asTYPEID_UINT8:
            name = "uint8";
            break;
        case asTYPEID_UINT16:
            name = "uint16";
            break;
        case asTYPEID_UINT32:
            name = "uint32";
            break;
        case asTYPEID_UINT64:
            name = "uint64";
            break;
        case asTYPEID_FLOAT:
            name = "float";
            break;
        case asTYPEID_DOUBLE:
            name = "double";
            break;
        default:
            name = "???";
            break;
        }
    } else {
        auto ns = type->GetNamespace();
        if (ns && strlen(ns)) {
            name = fmt::format(FMT_STRING("{}::{}"), ns, type->GetName());
        } else {
            name = type->GetName();
        }
    }
    return name;
}

std::string ScriptMachine::stringify_helper(const void *ref, int typeId) {
    ASSERT(ref && typeId);
    if (ref == nullptr) {
        return {};
    }

    switch (typeId & asTYPEID_MASK_SEQNBR) {
    case asTYPEID_BOOL:
        return fmt::to_string(*static_cast<const bool *>(ref));
    case asTYPEID_INT8:
        return fmt::to_string(*static_cast<const qint8 *>(ref));
    case asTYPEID_INT16:
        return fmt::to_string(*static_cast<const qint16 *>(ref));
    case asTYPEID_INT32:
        return fmt::to_string(*static_cast<const qint32 *>(ref));
    case asTYPEID_INT64:
        return fmt::to_string(*static_cast<const qint64 *>(ref));
    case asTYPEID_UINT8:
        return fmt::to_string(*static_cast<const quint8 *>(ref));
    case asTYPEID_UINT16:
        return fmt::to_string(*static_cast<const quint16 *>(ref));
    case asTYPEID_UINT32:
        return fmt::to_string(*static_cast<const quint32 *>(ref));
    case asTYPEID_UINT64:
        return fmt::to_string(*static_cast<const quint64 *>(ref));
    case asTYPEID_FLOAT:
        return fmt::to_string(*static_cast<const float *>(ref));
    case asTYPEID_DOUBLE:
        return fmt::to_string(*static_cast<const double *>(ref));
    }

    if (isAngelChar(typeId)) {
        // char
        auto pch = resolveObjAs<QChar>(ref, typeId);
        return QString(*pch).toStdString();
    } else if (isAngelString(typeId)) {
        // string
        return resolveObjAs<QString>(ref, typeId)->toStdString();
    } else if (isAngelArray(typeId)) {
        // array<?>
        return fmt::to_string(
            CScriptArrayView(resolveObjAs<CScriptArray>(ref, typeId)));
    } else if (isAngelDictionary(typeId)) {
        // dictionary
        return fmt::to_string(CScriptDictionaryView(
            resolveObjAs<CScriptDictionary>(ref, typeId)));
    } else if (isAngelDicValue(typeId)) {
        // dictonaryValue
        auto dicv = resolveObjAs<CScriptDictValue>(ref, typeId);
        return stringify_helper(const_cast<void *>(dicv->GetAddressOfValue()),
                                dicv->GetTypeId());
    } else if (isAngelAny(typeId)) {
        // any
        auto obj = resolveObjAs<CScriptAny>(ref, typeId);
        return stringify_helper(const_cast<void *>(obj->GetAddressOfValue()),
                                obj->GetTypeId());
    }

    // if type has toString() function
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        auto info = _engine->GetTypeInfoById(typeId);
        if (info) {
            auto func = info->GetMethodByDecl("string toString() const");
            if (func) {
                ctx->PushState();
                ctx->Prepare(func);
                ctx->SetObject(
                    const_cast<void *>(resolveObjAs<void>(ref, typeId)));
                ctx->Execute();
                auto rstr = *static_cast<QString *>(ctx->GetReturnObject());
                ctx->PopState();
                return rstr.toStdString();
            }
        }
    }

    return fmt::format(FMT_STRING("{}<#{}>"), getAsTypeName(typeId),
                       fmt::ptr(ref));
}

bool ScriptMachine::executeScript(
    ConsoleMode mode, const QString &script, bool isInDebug, int *retCode,
    std::function<void(const QHash<QString, AsPreprocesser::Result> &)>
        sections) {
    ASSERT(mode != Interactive);
    if (QThread::currentThread() != qApp->thread()) {
        Logger::warning(QStringLiteral("Code must be exec in the main thread"));
        return false;
    }

    if (script.isEmpty()) {
        return true;
    }

    // Compile the script
    auto mod = createModuleIfNotExist(mode);
    // script-running is not allowed in interactive mode
    if (mod == nullptr) {
        return false;
    }

    QScopeGuard guard([mod, this]() {
        endEvaluateDefine();
        // Before leaving, allow the engine to clean up remaining objects by
        // discarding the module and doing a full garbage collection so that
        // this can also be debugged if desired
        mod->Discard();
        _engine->SetUserData(0, AsUserDataType::UserData_ContextMode);
    });

    beginEvaluateDefine();

    asPWORD isDbg = 0;
    if (mode == Scripting) {
        if (isInDebug) {
            isDbg = 1;
        }
    }
    _engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, isDbg == 0);

    asBuilder builder(_engine);
    for (auto &m : PluginSystem::instance().scriptMarcos()) {
        builder.defineMacroWord(m);
    }

    // Set the pragma callback so we can detect
    builder.setPragmaCallback(&ScriptMachine::pragmaCallback);
    builder.setErrorHandler(
        [this, mode](const AsPreprocesser::PreprocError &error) {
            MessageInfo info;
            info.mode = mode;
            info.row = error.line;
            info.col = error.column;
            info.message = error.message;

            switch (error.severity) {
            case AsPreprocesser::Severity::Info:
                info.type = MessageType::Info;
                break;
            case AsPreprocesser::Severity::Warning:
                info.type = MessageType::Warn;
                break;
            case AsPreprocesser::Severity::Error:
                info.type = MessageType::Error;
                break;
            }

            outputMessage(info);
        });

    asPWORD umode = asPWORD(mode);
    _engine->SetUserData(reinterpret_cast<void *>(umode),
                         AsUserDataType::UserData_ContextMode);

    auto r = builder.loadSectionFromFile(script);
    if (r < 0) {
        MessageInfo info;
        info.mode = mode;
        info.message = QStringLiteral("Script failed to pre-processed");
        info.type = MessageType::Error;
        outputMessage(info);
        return false;
    }

    r = builder.build(mod);
    if (r < 0) {
        MessageInfo info;
        info.mode = mode;
        info.message = QStringLiteral("Script failed to build");
        info.type = MessageType::Error;
        outputMessage(info);
        return false;
    }

    // Find the main function
    asIScriptFunction *func = mod->GetFunctionByDecl("int main()");
    if (func == nullptr) {
        // Try again with "void main()"
        func = mod->GetFunctionByDecl("void main()");
    }

    if (func == nullptr) {
        MessageInfo info;
        info.mode = mode;
        info.message =
            QStringLiteral("Cannot find 'int main()' or 'void main()'");
        info.type = MessageType::Error;
        outputMessage(info);
        return false;
    }

    if (sections) {
        auto data = builder.scriptData();
        sections(data);
    }

    if (isInDebug) {
        // Allow the user to initialize the debugging before moving on
        MessageInfo info;
        info.mode = mode;
        info.message = QStringLiteral("Debugging, waiting for commands.");
        info.type = MessageType::Info;
        outputMessage(info);
    }

    // Set up a context to execute the script
    // The context manager will request the context from the
    // pool, which will automatically attach the debugger
    asIScriptContext *ctx = _ctxMgr->AddContext(_engine, func, true);

    _ctx[mode] = ctx;

    if (mode == Background) {
        MessageInfo info;
        info.mode = mode;
        info.message = QStringLiteral("Run > ") + script;
        info.type = MessageType::ExecInfo;
        outputMessage(info);
    }

    mod->SetUserData(reinterpret_cast<void *>(isDbg),
                     AsUserDataType::UserData_isDbg);

    ctx->SetUserData(reinterpret_cast<void *>(isDbg),
                     AsUserDataType::UserData_isDbg);
    ctx->SetUserData(reinterpret_cast<void *>(umode),
                     AsUserDataType::UserData_ContextMode);
    ctx->SetUserData(reinterpret_cast<void *>(
                         AppManager::instance()->currentMSecsSinceEpoch()),
                     AsUserDataType::UserData_Timer);
    auto timeOutRaw = SettingManager::instance().scriptTimeout();
    auto timeOut = asPWORD(timeOutRaw) * 60000; // min -> ms
    ctx->SetUserData(reinterpret_cast<void *>(timeOut),
                     AsUserDataType::UserData_TimeOut);
    ctx->SetUserData(const_cast<QString *>(&script),
                     AsUserDataType::UserData_Section_StringPtr);

    ctx->SetExceptionCallback(asMETHOD(ScriptMachine, exceptionCallback), this,
                              asCALL_THISCALL);

    // collect the handle info
    auto &api = PluginSystem::instance();
    auto handles = api.scriptHandles();

    // Execute the script until completion
    // The script may create co-routines. These will automatically
    // be managed by the context manager
    while (_ctxMgr->ExecuteScripts()) {
        qApp->processEvents();
    }

    _ctx[mode] = nullptr;

    // Check if the main script finished normally
    r = ctx->GetState();
    if (r != asEXECUTION_FINISHED) {
        if (r == asEXECUTION_EXCEPTION) {
            r = -1;
        } else if (r == asEXECUTION_ABORTED) {
            MessageInfo info;
            info.mode = mode;
            info.message = QStringLiteral("The script was aborted");
            info.type = MessageType::Error;
            outputMessage(info);
            r = -1;
        } else {
            auto e = QMetaEnum::fromType<asEContextState>();
            MessageInfo info;
            info.mode = mode;
            info.message =
                QStringLiteral("The script terminated unexpectedly (") +
                QString::fromLatin1(e.valueToKey(r)) + QStringLiteral(")");
            info.type = MessageType::Error;
            outputMessage(info);
            r = -1;
        }
    } else {
        // Get the return value from the script
        if (func->GetReturnTypeId() == asTYPEID_INT32) {
            r = *(int *)ctx->GetAddressOfReturnValue();
        } else
            r = 0;
    }

    MessageInfo info;
    info.mode = mode;
    info.message =
        QStringLiteral("The script exited with ") + QString::number(r);
    info.type = MessageType::ExecInfo;
    outputMessage(info);

    // Return the context after retrieving the return value
    _ctxMgr->DoneWithContext(ctx);

    // Before leaving, allow the engine to clean up remaining objects by
    // discarding the module and doing a full garbage collection so that
    // this can also be debugged if desired
    api.cleanScriptHandles(handles);
    _engine->GarbageCollect();

    if (isDbg) {
        _debugger->reset();
    }

    if (retCode) {
        *retCode = r;
    }

    return r >= 0;
}

void ScriptMachine::beginEvaluateDefine() {
    ASSERT(_eMod == nullptr);
    if (_eMod == nullptr) {
        _eMod = _engine->GetModule("WINGDEF", asGM_ALWAYS_CREATE);
        _eMod->SetAccessMask(0x2);
    }
}

QVariant ScriptMachine::evaluateDefine(const QString &code) {
    ASSERT(_eMod);
    if (!_eMod) {
        return {};
    }

    asIScriptFunction *func = nullptr;

    auto ccode = code;
    ccode.prepend("any f(){any ret;ret.store(").append(");return ret;}");
    // start to compile

    auto cr = _eMod->CompileFunction(nullptr, ccode.toUtf8(), 0, 0, &func);
    if (cr < 0) {
        return {};
    }

    // Set up a context to execute the script
    // The context manager will request the context from the
    // pool, which will automatically attach the debugger
    asIScriptContext *ctx = _ctxMgr->AddContext(_engine, func, true);

    ctx->SetUserData(reinterpret_cast<void *>(asPWORD(
                         AppManager::instance()->currentMSecsSinceEpoch())),
                     AsUserDataType::UserData_Timer);

    asPWORD isDbg = 0;
    ctx->SetUserData(reinterpret_cast<void *>(isDbg),
                     AsUserDataType::UserData_isDbg);
    _eMod->SetUserData(reinterpret_cast<void *>(isDbg),
                       AsUserDataType::UserData_isDbg);
    ctx->SetUserData(0, AsUserDataType::UserData_ContextMode);

    ctx->SetExceptionCallback(asMETHOD(ScriptMachine, exceptionCallback), this,
                              asCALL_THISCALL);

    // Execute the script until completion
    // The script may create co-routines. These will automatically
    // be managed by the context manager
    while (_ctxMgr->ExecuteScripts()) {
        qApp->processEvents();
    }

    QVariant result;
    // Check if the main script finished normally
    int r = ctx->GetState();
    if (r == asEXECUTION_FINISHED) {
        auto ret = reinterpret_cast<CScriptAny *>(ctx->GetReturnObject());
        int typeID = ret->GetTypeId();
        switch (typeID) {
        case asTYPEID_BOOL: {
            bool r;
            ret->Retrieve(&r, asTYPEID_BOOL);
            result = r;
        } break;
        case asTYPEID_INT8: {
            qint8 r;
            ret->Retrieve(&r, asTYPEID_INT8);
            result = r;
        } break;
        case asTYPEID_INT16: {
            qint16 r;
            ret->Retrieve(&r, asTYPEID_INT16);
            result = r;
        } break;
        case asTYPEID_INT32: {
            qint32 r;
            ret->Retrieve(&r, asTYPEID_INT32);
            result = r;
        } break;
        case asTYPEID_INT64: {
            qint64 r;
            ret->Retrieve(&r, asTYPEID_INT64);
            result = r;
        } break;
        case asTYPEID_UINT8: {
            quint8 r;
            ret->Retrieve(&r, asTYPEID_UINT8);
            result = r;
        } break;
        case asTYPEID_UINT16: {
            quint16 r;
            ret->Retrieve(&r, asTYPEID_UINT16);
            result = r;
        } break;
        case asTYPEID_UINT32: {
            quint32 r;
            ret->Retrieve(&r, asTYPEID_UINT32);
            result = r;
        } break;
        case asTYPEID_UINT64: {
            quint64 r;
            ret->Retrieve(&r, asTYPEID_UINT64);
            result = r;
        } break;
        case asTYPEID_FLOAT: {
            float r;
            ret->Retrieve(&r, asTYPEID_FLOAT);
            result = r;
        } break;
        case asTYPEID_DOUBLE: {
            double r;
            ret->Retrieve(r);
            result = r;
        } break;
        default:
            break;
        }
    }

    func->Release();

    // Return the context after retrieving the return value
    _ctxMgr->DoneWithContext(ctx);
    _engine->GarbageCollect();

    return result;
}

void ScriptMachine::endEvaluateDefine() {
    if (_eMod) {
        _eMod->Discard();
        _eMod = nullptr;
    }
}

void ScriptMachine::abortDbgScript() {
    if (_debugger) {
        abortScript(ConsoleMode::Scripting);
        _debugger->Resume();
    }
}

void ScriptMachine::abortScript(ConsoleMode mode) {
    auto ctx = _ctx.value(mode, nullptr);
    if (ctx) {
        ctx->Abort();
        _ctx[mode] = nullptr;
    }
}

void ScriptMachine::messageCallback(const asSMessageInfo *msg, void *param) {
    ConsoleMode mode;
    auto ins = static_cast<ScriptMachine *>(param);
    auto ctx = asGetActiveContext();
    if (ctx) {
        mode = ConsoleMode(reinterpret_cast<asPWORD>(
            ctx->GetUserData(AsUserDataType::UserData_ContextMode)));
    } else {
        mode = ConsoleMode(reinterpret_cast<asPWORD>(
            ins->engine()->GetUserData(AsUserDataType::UserData_ContextMode)));
    }

    MessageType t = MessageType::Print;
    switch (msg->type) {
    case asMSGTYPE_ERROR:
        t = MessageType::Error;
        break;
    case asMSGTYPE_WARNING:
        t = MessageType::Warn;
        break;
    case asMSGTYPE_INFORMATION:
        t = MessageType::Info;
        break;
    }

    MessageInfo info;
    info.mode = mode;
    info.row = msg->row;
    info.col = msg->col;
    info.section = QString::fromUtf8(msg->section);
    info.message = QString::fromUtf8(msg->message);
    info.type = t;
    ins->outputMessage(info);
}

void ScriptMachine::cleanUpPluginSysIDFunction(asIScriptFunction *) {
    // do nothing
    // UserData_API is readonly and it will delete later by its allocator
    // UserData_PluginFn is just an id, not a valid pointer to data
}

ScriptMachine &ScriptMachine::instance() {
    static ScriptMachine ins;
    return ins;
}

ScriptMachine::ScriptMachine() {}

asIScriptModule *ScriptMachine::createModule(ConsoleMode mode) {
    if (isModuleExists(mode)) {
        return nullptr;
    }

    asIScriptModule *mod = nullptr;

    switch (mode) {
    case Interactive:
        mod = nullptr;
    case Scripting:
        mod = _engine->GetModule("WINGSCRIPT", asGM_ALWAYS_CREATE);
        mod->SetAccessMask(0x1);
        break;
    case Background:
        mod = _engine->GetModule("WINGSRV", asGM_ALWAYS_CREATE);
        mod->SetAccessMask(0x1);
        break;
    }

    return mod;
}

asIScriptModule *ScriptMachine::createModuleIfNotExist(ConsoleMode mode) {
    asIScriptModule *mod = nullptr;
    switch (mode) {
    case Interactive:
        mod = _engine->GetModule("WINGCONSOLE", asGM_ONLY_IF_EXISTS);
        mod->SetAccessMask(0x1);
        break;
    case Scripting:
        mod = _engine->GetModule("WINGSCRIPT", asGM_CREATE_IF_NOT_EXISTS);
        mod->SetAccessMask(0x1);
        break;
    case Background:
        mod = _engine->GetModule("WINGSRV", asGM_CREATE_IF_NOT_EXISTS);
        mod->SetAccessMask(0x1);
        break;
    }

    return mod;
}

asIScriptModule *ScriptMachine::module(ConsoleMode mode) const {
    switch (mode) {
    case Interactive:
        return _engine->GetModule("WINGCONSOLE", asGM_ONLY_IF_EXISTS);
    case Scripting:
        return _engine->GetModule("WINGSCRIPT", asGM_ONLY_IF_EXISTS);
    case Background:
        return _engine->GetModule("WINGSRV", asGM_ONLY_IF_EXISTS);
    }
    return nullptr;
}

bool ScriptMachine::isModuleExists(ConsoleMode mode) {
    return module(mode) != nullptr;
}

asIScriptContext *ScriptMachine::requestContextCallback(asIScriptEngine *engine,
                                                        void *param) {
    asIScriptContext *ctx = nullptr;
    auto p = reinterpret_cast<ScriptMachine *>(param);
    ASSERT(p);

    // Check if there is a free context available in the pool
    if (p->_ctxPool.size()) {
        ctx = p->_ctxPool.back();
        p->_ctxPool.pop_back();
    } else {
        // No free context was available so we'll have to create a new one
        ctx = engine->CreateContext();
    }

    // Attach the debugger
    p->checkDebugger(ctx);

    return ctx;
}

void ScriptMachine::lineCallback(asIScriptContext *, void *) {
    auto app = AppManager::instance();
    static auto oldTime = app->currentMSecsSinceEpoch();
    auto curTime = app->currentMSecsSinceEpoch();
    if (curTime - oldTime >= 50) {
        QApplication::processEvents();
        oldTime = curTime;
    }
}

void ScriptMachine::returnContextCallback(asIScriptEngine *engine,
                                          asIScriptContext *ctx, void *param) {
    Q_UNUSED(engine);

    if (ctx) {
        // We can also check for possible script exceptions here if so desired

        // Unprepare the context to free any objects it may still hold (e.g.
        // return value) This must be done before making the context available
        // for re-use, as the clean up may trigger other script executions, e.g.
        // if a destructor needs to call a function.
        ctx->Unprepare();

        auto p = reinterpret_cast<ScriptMachine *>(param);
        ASSERT(p);

        // Place the context into the pool for when it will be needed again
        p->_ctxPool.push_back(ctx);
    }
}

void ScriptMachine::debug_break() {
    auto ctx = asGetActiveContext();
    if (ctx) {
        auto &m = ScriptMachine::instance();
        auto isDbg = reinterpret_cast<asPWORD>(
            ctx->GetUserData(AsUserDataType::UserData_isDbg));
        if (isDbg && m._ctx[ConsoleMode::Scripting] == ctx) {
            m._debugger->DebugBreak(ctx);
        } else {
            ctx->SetException("debug::setBreak can be only used in scripting");
        }
    }
}

quint64 ScriptMachine::debug_elapsedTime() {
    auto ctx = asGetActiveContext();
    if (ctx) {
        return AppManager::instance()->currentMSecsSinceEpoch() -
               reinterpret_cast<quint64>(
                   ctx->GetUserData(AsUserDataType::UserData_Timer));
    }
    return 0;
}

QString ScriptMachine::debug_backtrace() {
    auto ctx = asGetActiveContext();
    if (ctx) {
        auto &m = ScriptMachine::instance();
        if (m._ctx[ConsoleMode::Interactive] == ctx) {
            ctx->SetException("debug::break cannot be used in console");
            return {};
        }

        auto cs = ctx->GetCallstackSize();

        QString ret;
        for (asUINT i = 0; i < cs; i++) {
            auto f = ctx->GetFunction(i);
            int col;
            const char *section;
            int row = ctx->GetLineNumber(i, &col, &section);
            ret.append(QStringLiteral("%1 %2[%3:%4]\n")
                           .arg(QString::fromUtf8(
                                    f->GetDeclaration(true, false, true)),
                                QString::fromUtf8(section))
                           .arg(row)
                           .arg(col));
        }

        return ret;
    }

    return {};
}

PragmaResult ScriptMachine::pragmaCallback(const QString &pragmaText,
                                           AsPreprocesser *builder,
                                           const QString &sectionname) {
    asIScriptEngine *engine = builder->getEngine();

    // Filter the pragmaText so only what is of interest remains
    // With this the user can add comments and use different whitespaces
    // without affecting the result
    asUINT pos = 0;
    asUINT length = 0;
    QStringList tokens;
    auto pcodes = pragmaText.toUtf8();
    while (pos < pragmaText.size()) {
        asETokenClass tokenClass =
            engine->ParseToken(pcodes.data() + pos, 0, &length);
        if (tokenClass == asTC_IDENTIFIER || tokenClass == asTC_KEYWORD ||
            tokenClass == asTC_VALUE) {
            auto token = pcodes.mid(pos, length);
            tokens << QString::fromUtf8(token);
        }
        if (tokenClass == asTC_UNKNOWN)
            return {};
        pos += length;
    }

    auto pn = tokens.takeFirst();
    return PluginSystem::instance().processPragma(sectionname, pn, tokens);
}

void ScriptMachine::registerEngineAddon(asIScriptEngine *engine) {
    // all modules can access
    engine->SetDefaultAccessMask(0x3);

    auto r = engine->RegisterTypedef("byte", "uint8"); // register alias
    ASSERT(r >= 0);

    RegisterScriptArray(engine, true);
    RegisterQString(engine);
    RegisterScriptRegex(engine);
    RegisterQStringUtils(engine);
    RegisterQStringRegExSupport(engine);

    r = engine->SetDefaultNamespace("math");
    ASSERT(r >= 0);

    RegisterScriptMath(engine);
    RegisterScriptMathComplex(engine);
    engine->SetDefaultNamespace("");

    RegisterScriptWeakRef(engine);
    RegisterScriptAny(engine);
    RegisterScriptDictionary(engine);
    RegisterScriptGrid(engine);
    RegisterScriptHandle(engine);
    RegisterColor(engine);
    RegisterQJson(engine);
    RegisterEnv(engine);
    RegisterScriptFile(engine);
    RegisterScriptDateTime(engine);
    RegisterScriptFileSystem(engine);
    RegisterScriptUrl(engine);
    RegisterScriptCrypto(engine);
    RegisterASReflection(engine);
    RegisterOptBox(engine);

    engine->SetDefaultAccessMask(0x1);
    registerExceptionRoutines(engine);
    registerEngineAssert(engine);
    registerEngineClipboard(engine);
    registerEngineDebug(engine);

    // cache typeids and typeinfos we all must use
    auto type = engine->GetTypeInfoByName("char");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_CharTypeInfo);

    type = engine->GetTypeInfoByName("string");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_StringTypeInfo);

    type = engine->GetTypeInfoByName("datetime");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_DateTimeTypeInfo);

    type = engine->GetTypeInfoByName("array");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_ArrayTypeInfo);

    type = engine->GetTypeInfoByName("dictionary");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_DictionaryTypeInfo);

    type = engine->GetTypeInfoByName("dictionaryValue");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_DictionaryValueTypeInfo);

    type = engine->GetTypeInfoByName("any");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_AnyTypeInfo);

    type = engine->GetTypeInfoByName("json::value");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_JsonValueTypeInfo);

    type = engine->GetTypeInfoByDecl("array<byte>");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_ByteArrayTypeInfo);

    type = engine->GetTypeInfoByDecl("array<string>");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_StringListTypeInfo);

    type = engine->GetTypeInfoByDecl("array<char>");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_CharArrayTypeInfo);
}

void ScriptMachine::registerEngineAssert(asIScriptEngine *engine) {
    int r;

    // The string type must be available
    ASSERT(engine->GetTypeInfoByDecl("string"));

    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") == 0) {
        r = engine->RegisterGlobalFunction("void assert(bool expression)",
                                           asFUNCTION(scriptAssert),
                                           asCALL_CDECL);
        ASSERT(r >= 0);

        r = engine->RegisterGlobalFunction(
            "void assert_x(bool expression, const string &in msg)",
            asFUNCTION(scriptAssert_X), asCALL_CDECL);
        ASSERT(r >= 0);

    } else {
        r = engine->RegisterGlobalFunction("void assert(bool expression)",
                                           WRAP_FN(scriptAssert),
                                           asCALL_GENERIC);
        ASSERT(r >= 0);

        r = engine->RegisterGlobalFunction(
            "void assert_x(bool expression, const string &in msg)",
            WRAP_FN(scriptAssert_X), asCALL_GENERIC);
        ASSERT(r >= 0);
    }
}

void ScriptMachine::registerEngineClipboard(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("clipboard");
    ASSERT(r >= 0);

    // The string type must be available
    ASSERT(engine->GetTypeInfoByDecl("string"));

    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") == 0) {
        r = engine->RegisterGlobalFunction(
            "void setText(const string &in text)", asFUNCTION(clip_setText),
            asCALL_CDECL);
        ASSERT(r >= 0);

        r = engine->RegisterGlobalFunction(
            "string text()", asFUNCTION(clip_getText), asCALL_CDECL);
        ASSERT(r >= 0);

        r = engine->RegisterGlobalFunction(
            "void setBinary(const uint8[]@ data)", asFUNCTION(clip_setBinary),
            asCALL_CDECL);
        ASSERT(r >= 0);

        r = engine->RegisterGlobalFunction(
            "uint8[]@ getBinary()", asFUNCTION(clip_getBinary), asCALL_CDECL);
        ASSERT(r >= 0);

    } else {
        r = engine->RegisterGlobalFunction(
            "void setText(const string &in text)", WRAP_FN(clip_setText),
            asCALL_GENERIC);
        ASSERT(r >= 0);

        r = engine->RegisterGlobalFunction(
            "string text()", asFUNCTION(clip_getText), asCALL_GENERIC);
        ASSERT(r >= 0);

        r = engine->RegisterGlobalFunction(
            "void setBinary(const uint8[]@ data)", WRAP_FN(clip_setBinary),
            asCALL_GENERIC);
        ASSERT(r >= 0);

        r = engine->RegisterGlobalFunction(
            "uint8[]@ getBinary()", WRAP_FN(clip_getBinary), asCALL_GENERIC);
        ASSERT(r >= 0);
    }

    engine->SetDefaultNamespace("");
}

void ScriptMachine::registerEngineDebug(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("debug");
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction("void setBreak()",
                                       asFUNCTION(debug_break), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "uint64 elapsedTime()", asFUNCTION(debug_elapsedTime), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "string backtrace()", asFUNCTION(debug_backtrace), asCALL_CDECL);
    ASSERT(r >= 0);

    engine->SetDefaultNamespace("");
}

void ScriptMachine::registerCallBack(ConsoleMode mode,
                                     const RegCallBacks &callbacks) {
    _regcalls.insert(mode, callbacks);
}

bool ScriptMachine::isAngelChar(int typeID) const {
    auto type = static_cast<asITypeInfo *>(
        _engine->GetUserData(AsUserDataType::UserData_CharTypeInfo));
    return typeID == type->GetTypeId();
}

bool ScriptMachine::isAngelString(int typeID) const {
    auto type = static_cast<asITypeInfo *>(
        _engine->GetUserData(AsUserDataType::UserData_StringTypeInfo));
    return typeID == type->GetTypeId();
}

bool ScriptMachine::isAngelArray(int typeID) const {
    if (typeID & asTYPEID_TEMPLATE) {
        auto t = _engine->GetTypeInfoById(typeID);
        t = _engine->GetTypeInfoByName(t->GetName());
        ASSERT(t);
        typeID = t->GetTypeId();
    }
    auto type = static_cast<asITypeInfo *>(
        _engine->GetUserData(AsUserDataType::UserData_ArrayTypeInfo));
    return typeID == type->GetTypeId();
}

bool ScriptMachine::isAngelDictionary(int typeID) const {
    auto type = static_cast<asITypeInfo *>(
        _engine->GetUserData(AsUserDataType::UserData_DictionaryTypeInfo));
    return typeID == type->GetTypeId();
}

bool ScriptMachine::isAngelDicValue(int typeID) const {
    auto type = static_cast<asITypeInfo *>(
        _engine->GetUserData(AsUserDataType::UserData_DictionaryValueTypeInfo));
    return typeID == type->GetTypeId();
}

bool ScriptMachine::isAngelAny(int typeID) const {
    auto type = static_cast<asITypeInfo *>(
        _engine->GetUserData(AsUserDataType::UserData_AnyTypeInfo));
    return typeID == type->GetTypeId();
}

void ScriptMachine::setFileEnableOverwrite(bool b) {
    CScriptFile::ENABLE_OVERWRITE = b;
}

void ScriptMachine::setFileSystemWrite(bool b) {
    CScriptFileSystem::ENABLE_WRITE = b;
}

bool ScriptMachine::fileEnableOverwrite() const {
    return CScriptFile::ENABLE_OVERWRITE;
}

bool ScriptMachine::fileSystemWrite() const {
    return CScriptFileSystem::ENABLE_WRITE;
}

void ScriptMachine::scriptAssert(bool b) {
    auto ctx = asGetActiveContext();
    if (ctx) {
        if (!b) {
            ctx->SetException("Assert failed", false);
        }
    }
}

void ScriptMachine::scriptAssert_X(bool b, const QString &msg) {
    auto ctx = asGetActiveContext();
    if (ctx) {
        if (!b) {
            auto m = msg;
            if (m.isEmpty()) {
                m = QStringLiteral("Assert failed");
            }
            ctx->SetException(m.toUtf8(), false);
        }
    }
}

void ScriptMachine::clip_setText(const QString &text) {
    qApp->clipboard()->setText(text);
}

void ScriptMachine::clip_setBinary(const CScriptArray &array) {
    QByteArray buffer;
    buffer.reserve(array.GetSize());
    array.AddRef();
    for (asUINT i = 0; i < array.GetSize(); ++i) {
        auto item = reinterpret_cast<const asBYTE *>(array.At(i));
        buffer.append(*item);
    }
    array.Release();

    auto c = qApp->clipboard();
    auto mime = new QMimeData;
    mime->setData(QStringLiteral("application/octet-stream"),
                  buffer); // don't use setText()
    c->setMimeData(mime);
}

QString ScriptMachine::clip_getText() { return qApp->clipboard()->text(); }

CScriptArray *ScriptMachine::clip_getBinary() {
    QClipboard *c = qApp->clipboard();

    QByteArray data;
    auto d = c->mimeData();
    data = d->data(QStringLiteral("application/octet-stream"));

    auto engine = ScriptMachine::instance().engine();
    auto len = data.size();
    auto arr =
        CScriptArray::Create(engine->GetTypeInfoByDecl("array<uint8>"), len);
    arr->AddRef();
    for (int i = 0; i < len; ++i) {
        auto addr = arr->At(i);
        *reinterpret_cast<char *>(addr) = data.at(i);
    }
    arr->Release();
    return arr;
}

void ScriptMachine::scriptThrow(const QString &msg) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        ctx->SetException(msg.toUtf8());
    }
}

bool ScriptMachine::isDebugMode(ConsoleMode mode) {
    if (mode == Scripting) {
        auto mod = module(mode);
        if (mod) {
            return reinterpret_cast<asPWORD>(
                mod->GetUserData(AsUserDataType::UserData_isDbg));
        }
    }

    return false;
}

asIScriptEngine *ScriptMachine::engine() const { return _engine; }

asDebugger *ScriptMachine::debugger() const { return _debugger; }

bool ScriptMachine::executeCode(ConsoleMode mode, const QString &code) {
    if (QThread::currentThread() != qApp->thread()) {
        Logger::warning(QStringLiteral("Code must be exec in the main thread"));
        return false;
    }

    if (code.isEmpty()) {
        return true;
    }

    asIScriptModule *mod = createModuleIfNotExist(mode);
    _engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, false);

    QScopeGuard guard([mod, this, mode]() {
        // Before leaving, allow the engine to clean up remaining objects by
        // discarding the module and doing a full garbage collection so that
        // this can also be debugged if desired
        if (mode != Interactive) {
            mod->Discard();
        }
        _engine->SetUserData(0, AsUserDataType::UserData_ContextMode);
    });

    // first, valid the input
    auto ccode = code.toUtf8();
    antlr4::ANTLRInputStream input(ccode.constData(), ccode.length());
    AngelscriptConsoleLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);

    AngelscriptConsoleParser parser(&tokens);
    parser.removeErrorListeners();
    parser.setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());

    AngelScriptConsoleVisitor visitor(tokens);
    bool isDecl = true;
    try {
        visitor.visit(parser.script());
    } catch (...) {
        isDecl = false;
    }

    if (isDecl) {
        auto decls = visitor.declCode();
        if (!decls.isEmpty()) {
            for (auto &s : visitor.declCode()) {
                auto r = mod->CompileGlobalVar(nullptr, s, 0);
                if (r < 0) {
                    MessageInfo info;
                    info.mode = mode;
                    info.message =
                        QStringLiteral("Invalid global variable declaration:") +
                        QString::fromUtf8(s);
                    info.type = MessageType::Error;
                    outputMessage(info);
                }
            }
            if (mode == ScriptMachine::Interactive) {
                _cachedGlobalStrs.clear();
            }
        }
        return true;
    }

    asIScriptFunction *func = nullptr;

    ccode = code.toUtf8();
    if (ccode.isEmpty()) {
        return true;
    }

    // ok, wrap the codes
    ccode.prepend("void f(){\n").append("\n}");

    asPWORD umode = asPWORD(mode);
    _engine->SetUserData(reinterpret_cast<void *>(umode),
                         AsUserDataType::UserData_ContextMode);

    // start to compile
    auto cr = mod->CompileFunction(nullptr, ccode, 0, 0, &func);
    if (cr < 0) {
        return false;
    }

    // Set up a context to execute the script
    // The context manager will request the context from the
    // pool, which will automatically attach the debugger
    asIScriptContext *ctx = _ctxMgr->AddContext(_engine, func, true);
    _ctx[mode] = ctx;

    asPWORD isDbg = 0;
    mod->SetUserData(reinterpret_cast<void *>(isDbg),
                     AsUserDataType::UserData_isDbg);

    ctx->SetUserData(reinterpret_cast<void *>(isDbg),
                     AsUserDataType::UserData_isDbg);
    ctx->SetUserData(reinterpret_cast<void *>(umode),
                     AsUserDataType::UserData_ContextMode);
    ctx->SetUserData(reinterpret_cast<void *>(
                         AppManager::instance()->currentMSecsSinceEpoch()),
                     AsUserDataType::UserData_Timer);
    auto timeOutRaw = SettingManager::instance().scriptTimeout();
    auto timeOut = asPWORD(timeOutRaw) * 60000; // min -> ms
    ctx->SetUserData(reinterpret_cast<void *>(timeOut),
                     AsUserDataType::UserData_TimeOut);

    ctx->SetExceptionCallback(asMETHOD(ScriptMachine, exceptionCallback), this,
                              asCALL_THISCALL);

    // Execute the script until completion
    // The script may create co-routines. These will automatically
    // be managed by the context manager
    while (_ctxMgr->ExecuteScripts()) {
        qApp->processEvents();
    }

    _ctx[mode] = nullptr;

    // Check if the main script finished normally
    int r = ctx->GetState();
    if (r != asEXECUTION_FINISHED) {
        if (r == asEXECUTION_EXCEPTION) {
            r = -1;
        } else if (r == asEXECUTION_ABORTED) {
            MessageInfo info;
            info.mode = mode;
            info.message = QStringLiteral("The script was aborted");
            info.type = MessageType::Error;
            outputMessage(info);
            r = -1;
        } else {
            auto e = QMetaEnum::fromType<asEContextState>();
            MessageInfo info;
            info.mode = mode;
            info.message =
                QStringLiteral("The script terminated unexpectedly (") +
                QString::fromLatin1(e.valueToKey(r)) + QStringLiteral(")");
            info.type = MessageType::Error;
            outputMessage(info);
            r = -1;
        }
    } else {
        r = 0;
    }

    // Return the context after retrieving the return value
    _ctxMgr->DoneWithContext(ctx);

    // Before leaving, allow the engine to clean up remaining objects by
    // discarding the module and doing a full garbage collection so that
    // this can also be debugged if desired
    _engine->GarbageCollect();

    func->Release();

    return r >= 0;
}

QString ScriptMachine::scriptGetExceptionInfo() {
    asIScriptContext *ctx = asGetActiveContext();
    if (!ctx)
        return {};

    const char *msg = ctx->GetExceptionString();
    if (msg == 0)
        return {};

    return QString::fromUtf8(msg);
}

void ScriptMachine::registerExceptionRoutines(asIScriptEngine *engine) {
    int r;

    // The string type must be available
    ASSERT(engine->GetTypeInfoByDecl("string"));

    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") == 0) {
        r = engine->RegisterGlobalFunction("void throw(const string &in)",
                                           asFUNCTION(scriptThrow),
                                           asCALL_CDECL);
        ASSERT(r >= 0);

        r = engine->RegisterGlobalFunction("string getExceptionInfo()",
                                           asFUNCTION(scriptGetExceptionInfo),
                                           asCALL_CDECL);
        ASSERT(r >= 0);

    } else {
        r = engine->RegisterGlobalFunction("void throw(const string &in)",
                                           WRAP_FN(scriptThrow),
                                           asCALL_GENERIC);
        ASSERT(r >= 0);

        r = engine->RegisterGlobalFunction("string getExceptionInfo()",
                                           WRAP_FN(scriptGetExceptionInfo),
                                           asCALL_GENERIC);
        ASSERT(r >= 0);
    }
}
