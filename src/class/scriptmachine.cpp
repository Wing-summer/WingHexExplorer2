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
#include "grammar/ASConsole/AngelscriptConsoleParserBaseVisitor.h"

#include "AngelScript/sdk/add_on/autowrapper/aswrappedcall.h"
#include "AngelScript/sdk/add_on/scriptany/scriptany.h"
#include "AngelScript/sdk/add_on/scriptarray/scriptarray.h"
#include "AngelScript/sdk/add_on/scriptdictionary/scriptdictionary.h"
#include "AngelScript/sdk/add_on/scriptgrid/scriptgrid.h"
#include "AngelScript/sdk/add_on/scripthandle/scripthandle.h"
#include "AngelScript/sdk/add_on/scripthelper/scripthelper.h"
#include "AngelScript/sdk/add_on/scriptmath/scriptmath.h"
#include "AngelScript/sdk/add_on/scriptmath/scriptmathcomplex.h"
#include "AngelScript/sdk/add_on/weakref/weakref.h"
#include "AngelScript/sdk/angelscript/source/as_scriptengine.h"

#include "angelobjstring.h"
#include "class/appmanager.h"
#include "class/asbuilder.h"
#include "class/logger.h"
#include "class/pluginsystem.h"
#include "class/settingmanager.h"
#include "define.h"
#include "scriptaddon/scriptcolor.h"
#include "scriptaddon/scriptenv.h"
#include "scriptaddon/scriptfile.h"
#include "scriptaddon/scriptjson.h"
#include "scriptaddon/scriptqstring.h"
#include "scriptaddon/scriptregex.h"

#include <QClipboard>
#include <QMimeData>
#include <QProcess>
#include <QScopeGuard>

class StringFormatter {
public:
    static QString format(const QString &input, uint indentSize = 4) {
        QString output;
        int level = 0;
        bool inString = false;
        QChar stringDelim;
        bool escape = false;

        for (int i = 0; i < input.size(); ++i) {
            QChar c = input[i];

            if (inString) {
                output += c;
                if (escape) {
                    escape = false;
                } else if (c == '\\') {
                    escape = true;
                } else if (c == stringDelim) {
                    inString = false;
                }
                continue;
            }

            if (isQuote(c)) {
                inString = true;
                stringDelim = c;
                output += c;
                continue;
            }

            switch (c.unicode()) {
            case '{':
                output += QStringLiteral("{\n");
                ++level;
                output += QString(level * indentSize, ' ');
                break;

            case '}':
                output += QStringLiteral("\n");
                --level;
                output +=
                    QString(level * indentSize, ' ') + QStringLiteral("}");
                break;

            case ',':
                output +=
                    QStringLiteral(",\n") + QString(level * indentSize, ' ');
                break;

            default:
                if (c.isSpace()) {
                    // collapse multiple spaces outside strings
                    if (!output.isEmpty() && !output.endsWith(' ')) {
                        output += ' ';
                    }
                } else {
                    output += c;
                }
                break;
            }
        }

        return output;
    }

private:
    static bool isQuote(QChar c) { return c == '"' || c == '\''; }
};

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

    // Let the debugger hold an engine pointer that can be used by the
    // callbacks
    _debugger->setEngine(_engine);

    return true;
}

bool ScriptMachine::isInited() const { return _engine != nullptr; }

bool ScriptMachine::isRunning(ConsoleMode mode) const {
    return _ctx.value(mode) != nullptr;
}

bool ScriptMachine::isEngineConfigError() const {
    if (_engine) {
        auto e = dynamic_cast<asCScriptEngine *>(_engine);
        if (e) {
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

    // We will only initialize the global variables once we're
    // ready to execute, so disable the automatic initialization
    _engine->SetEngineProperty(asEP_INIT_GLOBAL_VARS_AFTER_BUILD, false);

    // The script compiler will send any compiler messages to the callback
    auto r = _engine->SetMessageCallback(asFUNCTION(messageCallback), this,
                                         asCALL_CDECL);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    _engine->SetContextUserDataCleanupCallback(
        &ScriptMachine::cleanUpDbgContext,
        AsUserDataType::UserData_ContextDbgInfo);

    _engine->SetFunctionUserDataCleanupCallback(
        &ScriptMachine::cleanUpPluginSysIDFunction,
        AsUserDataType::UserData_PluginFn);

    registerEngineAddon(_engine);

    _rtypes.resize(RegisteredType::tMAXCOUNT);
    _rtypes[RegisteredType::tString] =
        q_check_ptr(_engine->GetTypeInfoByName("string"));
    _rtypes[RegisteredType::tChar] =
        q_check_ptr(_engine->GetTypeInfoByName("char"));
    _rtypes[RegisteredType::tArray] =
        q_check_ptr(_engine->GetTypeInfoByName("array"));
    _rtypes[RegisteredType::tComplex] =
        q_check_ptr(_engine->GetTypeInfoByName("complex"));
    _rtypes[RegisteredType::tWeakref] =
        q_check_ptr(_engine->GetTypeInfoByName("weakref"));
    _rtypes[RegisteredType::tConstWeakref] =
        q_check_ptr(_engine->GetTypeInfoByName("const_weakref"));
    _rtypes[RegisteredType::tAny] =
        q_check_ptr(_engine->GetTypeInfoByName("any"));
    _rtypes[RegisteredType::tDictionary] =
        q_check_ptr(_engine->GetTypeInfoByName("dictionary"));
    _rtypes[RegisteredType::tDictionaryValue] =
        q_check_ptr(_engine->GetTypeInfoByName("dictionaryValue"));
    _rtypes[RegisteredType::tGrid] =
        q_check_ptr(_engine->GetTypeInfoByName("grid"));
    _rtypes[RegisteredType::tRef] =
        q_check_ptr(_engine->GetTypeInfoByName("ref"));
    _rtypes[RegisteredType::tColor] =
        q_check_ptr(_engine->GetTypeInfoByName("color"));

    _engine->SetDefaultAccessMask(0x1);

    // Register a couple of extra functions for the scripts
    r = _engine->RegisterGlobalFunction("void print(const ? &in ...)",
                                        asFUNCTION(print), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction("void println(const ? &in ...)",
                                        asFUNCTION(println), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction("string getInput()",
                                        asMETHOD(ScriptMachine, getInput),
                                        asCALL_THISCALL_ASGLOBAL, this);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction("string stringify(? &in obj)",
                                        asMETHOD(ScriptMachine, stringify),
                                        asCALL_THISCALL_ASGLOBAL, this);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction(
        "string beautify(const string &in str, uint indent = 4)",
        asFUNCTION(beautify), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction(
        "int exec(string &out output, const string &in exe, "
        "const string &in params = \"\", int timeout = 3000)",
        asFUNCTION(execSystemCmd), asCALL_CDECL);
    Q_ASSERT(r >= 0);
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
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    _debugger = new asDebugger(this);

    // Register the to-string callbacks so the user can see
    // the contents of strings
    _debugger->registerToStringCallback(_rtypes[RegisteredType::tString],
                                        &AngelObjString::stringToString);
    _debugger->registerToStringCallback(_rtypes[RegisteredType::tChar],
                                        &AngelObjString::charToString);
    _debugger->registerToStringCallback(_rtypes[RegisteredType::tArray],
                                        &AngelObjString::arrayToString);
    _debugger->registerToStringCallback(_rtypes[RegisteredType::tDictionary],
                                        &AngelObjString::dictionaryToString);
    _debugger->registerToStringCallback(_rtypes[RegisteredType::tColor],
                                        &AngelObjString::colorToString);

    PluginSystem::instance().angelApi()->installAPI(this);

    // create module for Console
    auto mod = _engine->GetModule("WINGCONSOLE", asGM_ALWAYS_CREATE);
    mod->SetAccessMask(0x1);

    return true;
}

QString ScriptMachine::getCallStack(asIScriptContext *context) {
    QString str = tr("AngelScript callstack:\n");

    // Append the call stack
    for (asUINT i = 0; i < context->GetCallstackSize(); i++) {
        asIScriptFunction *func;
        const char *scriptSection;
        int line, column;
        func = context->GetFunction(i);
        line = context->GetLineNumber(i, &column, &scriptSection);
        str += (QStringLiteral("\t") + scriptSection + QStringLiteral(":") +
                func->GetDeclaration() + QStringLiteral(":") +
                QString::number(line) + QStringLiteral(",") +
                QString::number(column) + QStringLiteral("\n"));
    }

    return str;
}

void ScriptMachine::destoryMachine() {
    _ctxMgr->AbortAll();
    delete _ctxMgr;

    _debugger->setEngine(nullptr);
    _engine->ShutDownAndRelease();
    _engine = nullptr;
}

void ScriptMachine::exceptionCallback(asIScriptContext *context) {
    if (context) {
        ConsoleMode mode = ConsoleMode(reinterpret_cast<asPWORD>(
            context->GetUserData(AsUserDataType::UserData_ContextMode)));
        QString message =
            tr("- Exception '%1' in '%2'\n")
                .arg(context->GetExceptionString(),
                     context->GetExceptionFunction()->GetDeclaration()) +
            QStringLiteral("\n") + getCallStack(context);

        const char *section;
        MessageInfo msg;
        msg.mode = mode;
        msg.row = context->GetExceptionLineNumber(&msg.col, &section);
        msg.type = MessageType::Error;
        msg.message = message;

        outputMessage(msg);
    }
}

void ScriptMachine::print(asIScriptGeneric *args) {
    auto context = asGetActiveContext();
    if (context) {
        ConsoleMode mode = ConsoleMode(reinterpret_cast<asPWORD>(
            context->GetUserData(AsUserDataType::UserData_ContextMode)));

        auto &m = ScriptMachine::instance();

        MessageInfo info;
        info.mode = mode;
        info.type = MessageType::Print;

        for (int i = 0; i < args->GetArgCount(); ++i) {
            void *ref = args->GetArgAddress(i);
            int typeId = args->GetArgTypeId(i);

            if (typeId) {
                info.message.append(
                    m.debugger()->toString(ref, typeId, m.engine()));
            }
        }

        m.outputMessage(info);
    }
}

void ScriptMachine::println(asIScriptGeneric *args) {
    auto context = asGetActiveContext();
    if (context) {
        ConsoleMode mode = ConsoleMode(reinterpret_cast<asPWORD>(
            context->GetUserData(AsUserDataType::UserData_ContextMode)));

        auto &m = ScriptMachine::instance();

        MessageInfo info;
        info.mode = mode;
        info.type = MessageType::Print;

        for (int i = 0; i < args->GetArgCount(); ++i) {
            void *ref = args->GetArgAddress(i);
            int typeId = args->GetArgTypeId(i);

            if (typeId) {
                info.message
                    .append(m.debugger()->toString(ref, typeId, m.engine()))
                    .append('\n');
            }
        }

        m.outputMessage(info);
    }
}

QString ScriptMachine::getInput() {
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

bool ScriptMachine::isType(asITypeInfo *tinfo, RegisteredType type) {
    Q_ASSERT(type < RegisteredType::tMAXCOUNT);
    auto t = _rtypes.at(type);
    return tinfo->DerivesFrom(t) || tinfo->Implements(t);
}

int ScriptMachine::execSystemCmd(QString &out, const QString &exe,
                                 const QString &params, int timeout) {
    if (Utilities::isRoot()) {
        auto ctx = asGetActiveContext();
        if (ctx) {
            auto err = tr("ExecNotAllowedInRoot");
            ctx->SetException(err.toUtf8());
        }
        return -1;
    }
    QProcess ps;
    ps.setProgram(exe);
    ps.setArguments(QProcess::splitCommand(params));
    ps.start();
    if (ps.waitForFinished(timeout)) {
        out = ps.readAllStandardOutput();
        return ps.exitCode();
    } else {
        ps.kill();
        return -1;
    }
}

QString ScriptMachine::beautify(const QString &str, uint indent) {
    return StringFormatter::format(str, indent);
}

QString ScriptMachine::stringify(void *ref, int typeId) {
    return _debugger->toString(ref, typeId, _engine);
}

bool ScriptMachine::executeScript(ConsoleMode mode, const QString &script,
                                  bool isInDebug, int *retCode) {
    Q_ASSERT(mode != Interactive);
    if (QThread::currentThread() != qApp->thread()) {
        Logger::warning(tr("Code must be exec in the main thread"));
        return false;
    }

    if (script.isEmpty()) {
        return true;
    }

    // Compile the script
    auto mod = createModule(mode);
    // script-running is not allowed in interactive mode
    if (mod == nullptr) {
        return false;
    }

    QScopeGuard guard([mod]() {
        // Before leaving, allow the engine to clean up remaining objects by
        // discarding the module and doing a full garbage collection so that
        // this can also be debugged if desired
        mod->Discard();
    });

    asPWORD isDbg = 0;
    if (mode == Scripting) {
        if (isInDebug) {
            isDbg = 1;
            _debugger->resetState();
        }
    }
    _engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, isDbg == 0);

    asBuilder builder(_engine);
    for (auto &m : PluginSystem::instance().scriptMarcos()) {
        builder.defineMacroWord(m);
    }

    // Set the pragma callback so we can detect
    builder.setPragmaCallback(&ScriptMachine::pragmaCallback);

    _curMsgMode = mode;
    auto r = builder.loadSectionFromFile(script.toUtf8());
    if (r < 0) {
        MessageInfo info;
        info.mode = mode;
        info.message = tr("Script failed to pre-processed");
        info.type = MessageType::Error;
        outputMessage(info);
        return false;
    }

    r = builder.build(mod);
    if (r < 0) {
        MessageInfo info;
        info.mode = mode;
        info.message = tr("Script failed to build");
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
        info.message = tr("Cannot find 'int main()' or 'void main()'");
        info.type = MessageType::Error;
        outputMessage(info);
        return false;
    }

    if (isInDebug) {
        // Allow the user to initialize the debugging before moving on
        MessageInfo info;
        info.mode = mode;
        info.message = tr("Debugging, waiting for commands.");
        info.type = MessageType::Info;
        outputMessage(info);
    }

    // Once we have the main function, we first need to initialize the
    // global variables Since we've set up the request context callback we
    // will be able to debug the initialization without passing in a
    // pre-created context
    r = mod->ResetGlobalVars(0);
    if (r < 0) {
        MessageInfo info;
        info.mode = mode;
        info.message = tr("Failed while initializing global variables");
        info.type = MessageType::Error;
        outputMessage(info);
        return false;
    }

    // Set up a context to execute the script
    // The context manager will request the context from the
    // pool, which will automatically attach the debugger
    asIScriptContext *ctx = _ctxMgr->AddContext(_engine, func, true);
    _ctx[mode] = ctx;

    if (mode == Background) {
        MessageInfo info;
        info.mode = mode;
        info.message = tr("Run:") + script;
        info.type = MessageType::Info;
        outputMessage(info);
    }

    ctx->SetUserData(reinterpret_cast<void *>(
                         AppManager::instance()->currentMSecsSinceEpoch()),
                     AsUserDataType::UserData_Timer);
    ctx->SetUserData(reinterpret_cast<void *>(isDbg),
                     AsUserDataType::UserData_isDbg);

    auto timeOutRaw = SettingManager::instance().scriptTimeout();
    auto timeOut = asPWORD(timeOutRaw) * 6000; // min -> ms
    ctx->SetUserData(reinterpret_cast<void *>(timeOut),
                     AsUserDataType::UserData_TimeOut);

    mod->SetUserData(reinterpret_cast<void *>(isDbg),
                     AsUserDataType::UserData_isDbg);

    asPWORD umode = asPWORD(mode);
    ctx->SetUserData(reinterpret_cast<void *>(umode),
                     AsUserDataType::UserData_ContextMode);

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
    r = ctx->GetState();
    if (r != asEXECUTION_FINISHED) {
        if (r == asEXECUTION_EXCEPTION) {
            MessageInfo info;
            info.mode = mode;
            info.message = tr("The script failed with an exception") +
                           QStringLiteral("\n") +
                           QString::fromStdString(GetExceptionInfo(ctx, true));
            info.type = MessageType::Error;
            outputMessage(info);
            r = -1;
        } else if (r == asEXECUTION_ABORTED) {
            MessageInfo info;
            info.mode = mode;
            info.message = tr("The script was aborted");
            info.type = MessageType::Error;
            outputMessage(info);
            r = -1;
        } else {
            auto e = QMetaEnum::fromType<asEContextState>();
            MessageInfo info;
            info.mode = mode;
            info.message = tr("The script terminated unexpectedly") +
                           QStringLiteral(" (") + e.valueToKey(r) +
                           QStringLiteral(")");
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
    info.message = tr("The script exited with ") + QString::number(r);
    info.type = MessageType::Info;
    outputMessage(info);

    // Return the context after retrieving the return value
    _ctxMgr->DoneWithContext(ctx);

    // Before leaving, allow the engine to clean up remaining objects by
    // discarding the module and doing a full garbage collection so that
    // this can also be debugged if desired
    _engine->GarbageCollect();

    if (isDbg) {
        _debugger->clearBreakPoint();
        Q_EMIT onDebugFinished();
    }

    if (retCode) {
        *retCode = r;
    }

    return r >= 0;
}

QVariant ScriptMachine::evaluateDefine(const QString &code) {
    QByteArray name =
        QByteArrayLiteral("WINGDEF") +
        QByteArray::number(AppManager::instance()->currentMSecsSinceEpoch());

    auto mod = _engine->GetModule(name.data(), asGM_ALWAYS_CREATE);
    mod->SetAccessMask(0x2);

    asIScriptFunction *func = nullptr;
    QScopeGuard guard([mod]() {
        // Before leaving, allow the engine to clean up remaining objects by
        // discarding the module and doing a full garbage collection so that
        // this can also be debugged if desired
        mod->Discard();
    });

    auto ccode = code;
    ccode.prepend("any f(){ return (").append(");}");
    // start to compile

    auto cr = mod->CompileFunction(nullptr, ccode.toUtf8(), 0, 0, &func);
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
    mod->SetUserData(reinterpret_cast<void *>(isDbg),
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
        ret->Release();
    }

    func->Release();

    // Return the context after retrieving the return value
    _ctxMgr->DoneWithContext(ctx);
    _engine->GarbageCollect();

    return result;
}

void ScriptMachine::abortDbgScript() {
    if (_debugger->getEngine()) {
        _debugger->runDebugAction(asDebugger::ABORT);
    }
}

void ScriptMachine::abortScript(ConsoleMode mode) {
    auto ctx = _ctx.value(mode, nullptr);
    if (ctx) {
        ctx->Abort();
    }
}

void ScriptMachine::abortScript() {
    abortScript(ConsoleMode::Interactive);
    abortScript(ConsoleMode::Scripting);
    abortScript(ConsoleMode::Background);
}

void ScriptMachine::messageCallback(const asSMessageInfo *msg, void *param) {
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
    auto ins = static_cast<ScriptMachine *>(param);

    MessageInfo info;
    info.mode = ins->_curMsgMode;
    info.row = msg->row;
    info.col = msg->col;
    info.section = msg->section;
    info.message = msg->message;
    info.type = t;
    ins->outputMessage(info);
}

void ScriptMachine::cleanUpDbgContext(asIScriptContext *context) {
    auto dbgContext =
        context->GetUserData(AsUserDataType::UserData_ContextDbgInfo);
    asDebugger::deleteDbgContextInfo(dbgContext);
}

void ScriptMachine::cleanUpPluginSysIDFunction(asIScriptFunction *) {
    // do nothing
    // UserData_API is readonly and it will delete later by its allocator
    // UserData_PluginFn is just an id, not a valid pointer to data
}

asITypeInfo *ScriptMachine::typeInfo(RegisteredType type) const {
    if (type < RegisteredType::tMAXCOUNT && type >= 0) {
        return _rtypes.at(type);
    }
    return nullptr;
}

ScriptMachine &ScriptMachine::instance() {
    static ScriptMachine ins;
    return ins;
}

ScriptMachine::ScriptMachine() : QObject() {}

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
        mod->SetAccessMask(0x2);
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
        mod->SetAccessMask(0x2);
        break;
    }

    return mod;
}

asIScriptModule *ScriptMachine::module(ConsoleMode mode) {
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
    Q_ASSERT(p);

    // Check if there is a free context available in the pool
    if (p->_ctxPool.size()) {
        ctx = p->_ctxPool.back();
        p->_ctxPool.pop_back();
    } else {
        // No free context was available so we'll have to create a new one
        ctx = engine->CreateContext();
    }

    // Attach the debugger if needed
    if (ctx && p->_debugger->getEngine()) {
        // Set the line callback for the debugging
        ctx->SetLineCallback(asMETHOD(asDebugger, lineCallback), p->_debugger,
                             asCALL_THISCALL);
    }

    return ctx;
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
        Q_ASSERT(p);

        // Place the context into the pool for when it will be needed again
        p->_ctxPool.push_back(ctx);
    }
}

int ScriptMachine::pragmaCallback(const QString &pragmaText,
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
            return -1;
        pos += length;
    }

    auto pn = tokens.takeFirst();
    if (PluginSystem::instance().dispatchEvent(
            IWingPlugin::RegisteredEvent::ScriptPragma,
            {sectionname, pn, tokens})) {
        return 0;
    }

    // The #pragma directive was not accepted
    return -1;
}

void ScriptMachine::registerEngineAddon(asIScriptEngine *engine) {
    // all modules can access
    engine->SetDefaultAccessMask(0x3);
    RegisterScriptArray(engine, true);
    RegisterQString(engine);
    RegisterScriptRegex(engine);
    RegisterQStringUtils(engine);
    RegisterQStringRegExSupport(engine);
    RegisterScriptMath(engine);
    RegisterScriptMathComplex(engine);
    RegisterScriptWeakRef(engine);
    RegisterScriptAny(engine);
    RegisterScriptDictionary(engine);
    RegisterScriptGrid(engine);
    RegisterScriptHandle(engine);
    RegisterColor(engine);
    RegisterQJson(engine);
    RegisterEnv(engine);

    engine->SetDefaultAccessMask(0x1);
    RegisterScriptFile(engine);
    registerExceptionRoutines(engine);
    registerEngineAssert(engine);
    registerEngineClipboard(engine);
}

void ScriptMachine::registerEngineAssert(asIScriptEngine *engine) {
    int r;

    // The string type must be available
    Q_ASSERT(engine->GetTypeInfoByDecl("string"));

    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") == 0) {
        r = engine->RegisterGlobalFunction("void assert(bool expression)",
                                           asFUNCTION(scriptAssert),
                                           asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);

        r = engine->RegisterGlobalFunction(
            "void assert_x(bool expression, const string &in msg)",
            asFUNCTION(scriptAssert_X), asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
    } else {
        r = engine->RegisterGlobalFunction("void assert(bool expression)",
                                           WRAP_FN(scriptAssert),
                                           asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);

        r = engine->RegisterGlobalFunction(
            "void assert_x(bool expression, const string &in msg)",
            WRAP_FN(scriptAssert_X), asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
    }
}

void ScriptMachine::registerEngineClipboard(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("clipboard");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // The string type must be available
    Q_ASSERT(engine->GetTypeInfoByDecl("string"));

    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") == 0) {
        r = engine->RegisterGlobalFunction(
            "void setText(const string &in text)", asFUNCTION(clip_setText),
            asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterGlobalFunction(
            "string text()", asFUNCTION(clip_getText), asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterGlobalFunction(
            "void setBinary(const uint8[]@ data)", asFUNCTION(clip_setBinary),
            asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterGlobalFunction(
            "uint8[]@ getBinary()", asFUNCTION(clip_getBinary), asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
    } else {
        r = engine->RegisterGlobalFunction(
            "void setText(const string &in text)", WRAP_FN(clip_setText),
            asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterGlobalFunction(
            "string text()", asFUNCTION(clip_getText), asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterGlobalFunction(
            "void setBinary(const uint8[]@ data)", WRAP_FN(clip_setBinary),
            asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterGlobalFunction(
            "uint8[]@ getBinary()", WRAP_FN(clip_getBinary), asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
    }

    engine->SetDefaultNamespace("");
}

void ScriptMachine::registerCallBack(ConsoleMode mode,
                                     const RegCallBacks &callbacks) {
    _regcalls.insert(mode, callbacks);
}

void ScriptMachine::scriptAssert(bool b) {
    auto ctx = asGetActiveContext();
    if (ctx) {
        if (!b) {
            QString buffer = tr("Assert failed");
            ctx->SetException(buffer.toUtf8(), false);
        }
    }
}

void ScriptMachine::scriptAssert_X(bool b, const QString &msg) {
    auto ctx = asGetActiveContext();
    if (ctx) {
        if (!b) {
            auto m = msg;
            if (m.isEmpty()) {
                m = tr("Assert failed");
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
        Logger::warning(tr("Code must be exec in the main thread"));
        return false;
    }

    if (code.isEmpty()) {
        return true;
    }

    asIScriptModule *mod = createModuleIfNotExist(mode);
    _engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, false);

    // first, preparse the code
    auto buffer = code.toUtf8();
    antlr4::ANTLRInputStream input(buffer.constData(), buffer.length());
    AngelscriptConsoleLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);

    AngelscriptConsoleParser parser(&tokens);
    parser.removeErrorListeners();
    parser.setBuildParseTree(false);
    parser.setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());

    AngelscriptConsoleParserBaseVisitor visitor;

    try {
        visitor.visit(parser.script());
    } catch (...) {
        return false;
    }

    asIScriptFunction *func = nullptr;
    // check whether there is any enum/class
    QByteArray ccode;
    if (true) {
        // ok, wrap the codes
        ccode.prepend("void f(){").append("}");
        // start to compile
        _curMsgMode = mode;
        auto cr = mod->CompileFunction(nullptr, ccode, 0, 0, &func);
        if (cr < 0) {
            MessageInfo info;
            info.mode = mode;
            info.message = tr("Script failed to build");
            info.type = MessageType::Error;
            outputMessage(info);
            return false;
        }

        // Set up a context to execute the script
        // The context manager will request the context from the
        // pool, which will automatically attach the debugger
        asIScriptContext *ctx = _ctxMgr->AddContext(_engine, func, true);
        _ctx[mode] = ctx;

        ctx->SetUserData(reinterpret_cast<void *>(asPWORD(
                             AppManager::instance()->currentMSecsSinceEpoch())),
                         AsUserDataType::UserData_Timer);

        asPWORD isDbg = 0;
        ctx->SetUserData(reinterpret_cast<void *>(isDbg),
                         AsUserDataType::UserData_isDbg);
        mod->SetUserData(reinterpret_cast<void *>(isDbg),
                         AsUserDataType::UserData_isDbg);

        auto timeOutRaw = SettingManager::instance().scriptTimeout();
        auto timeOut = asPWORD(timeOutRaw) * 6000; // min -> ms
        ctx->SetUserData(reinterpret_cast<void *>(timeOut),
                         AsUserDataType::UserData_TimeOut);

        asPWORD umode = asPWORD(mode);
        ctx->SetUserData(reinterpret_cast<void *>(umode),
                         AsUserDataType::UserData_ContextMode);

        ctx->SetExceptionCallback(asMETHOD(ScriptMachine, exceptionCallback),
                                  this, asCALL_THISCALL);

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
                MessageInfo info;
                info.mode = mode;
                info.message =
                    tr("The script failed with an exception") +
                    QStringLiteral("\n") +
                    QString::fromStdString(GetExceptionInfo(ctx, true));
                info.type = MessageType::Error;
                outputMessage(info);
                r = -1;
            } else if (r == asEXECUTION_ABORTED) {
                MessageInfo info;
                info.mode = mode;
                info.message = tr("The script was aborted");
                info.type = MessageType::Error;
                outputMessage(info);
                r = -1;
            } else {
                auto e = QMetaEnum::fromType<asEContextState>();
                MessageInfo info;
                info.mode = mode;
                info.message = tr("The script terminated unexpectedly") +
                               QStringLiteral(" (") + e.valueToKey(r) +
                               QStringLiteral(")");
                info.type = MessageType::Error;
                outputMessage(info);
                r = -1;
            }
        } else {
            r = 0;
        }

        func->Release();

        // Return the context after retrieving the return value
        _ctxMgr->DoneWithContext(ctx);
        _engine->GarbageCollect();

        return r >= 0;
    } else {
        // if (std::all_of(ret.begin(), ret.end(),
        //                 [](const QAsCodeParser::CodeSegment &seg) {
        //                     return seg.type ==
        //                            QAsCodeParser::SymbolType::Variable;
        //                 })) {
        //     _curMsgMode = mode;

        //     // for (auto &s : ret) {
        //     //     auto r = mod->CompileGlobalVar(nullptr, s.codes, 0);
        //     //     if (r < 0) {
        //     //         MessageInfo info;
        //     //         info.mode = mode;
        //     //         info.message = tr("BadDecl:") + s.codes;
        //     //         info.type = MessageType::Error;
        //     //         outputMessage(info);
        //     //     }
        //     // }

        //     if (mod->ResetGlobalVars() < 0) {
        //         MessageInfo info;
        //         info.mode = mode;
        //         info.message = tr("GlobalBadDecl");
        //         info.type = MessageType::Error;
        //         outputMessage(info);
        //     }
        //     return true;
        // }

        MessageInfo info;
        info.mode = mode;
        info.message = tr("Script failed to build");
        info.type = MessageType::Error;
        outputMessage(info);
    }
    return false;
}

QString ScriptMachine::scriptGetExceptionInfo() {
    asIScriptContext *ctx = asGetActiveContext();
    if (!ctx)
        return {};

    const char *msg = ctx->GetExceptionString();
    if (msg == 0)
        return {};

    return QString(msg);
}

void ScriptMachine::registerExceptionRoutines(asIScriptEngine *engine) {
    int r;

    // The string type must be available
    Q_ASSERT(engine->GetTypeInfoByDecl("string"));

    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") == 0) {
        r = engine->RegisterGlobalFunction("void throw(const string &in)",
                                           asFUNCTION(scriptThrow),
                                           asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);

        r = engine->RegisterGlobalFunction("string getExceptionInfo()",
                                           asFUNCTION(scriptGetExceptionInfo),
                                           asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);

    } else {
        r = engine->RegisterGlobalFunction("void throw(const string &in)",
                                           WRAP_FN(scriptThrow),
                                           asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);

        r = engine->RegisterGlobalFunction("string getExceptionInfo()",
                                           WRAP_FN(scriptGetExceptionInfo),
                                           asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
    }
}
