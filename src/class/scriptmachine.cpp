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
#include "class/qascodeparser.h"
#include "class/settingmanager.h"
#include "define.h"
#include "scriptaddon/aspromise.hpp"
#include "scriptaddon/scriptcolor.h"
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

#define INS_1 "const ?&in = null"
#define INS_2 INS_1 ", " INS_1
#define INS_4 INS_2 ", " INS_2
#define INS_8 INS_4 ", " INS_4

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
    r = _engine->RegisterGlobalFunction(
        "void print(const ? &in obj, const ? &in = null," INS_8 ")",
        asFUNCTION(print), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction(
        "void println(const ? &in obj, const ? &in = null," INS_8 ")",
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
        builder.defineWord(m);
    }

    // Set the pragma callback so we can detect
    builder.setPragmaCallback(&ScriptMachine::pragmaCallback, this);
    builder.setIncludeCallback(&ScriptMachine::includeCallback, this);

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

int ScriptMachine::evaluateDefine(const QString &code, bool &result) {
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
    ccode.prepend("bool f(){ return bool(").append(");}");
    // start to compile

    auto cr = mod->CompileFunction(nullptr, ccode.toUtf8(), 0, 0, &func);
    if (cr < 0) {
        return cr;
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

    // Check if the main script finished normally
    int r = ctx->GetState();
    if (r != asEXECUTION_FINISHED) {
        r = -1;
    } else {
        result = bool(ctx->GetReturnByte());
        r = 0;
    }

    func->Release();

    // Return the context after retrieving the return value
    _ctxMgr->DoneWithContext(ctx);
    _engine->GarbageCollect();
    return r;
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

    auto m = processTranslation(msg->message, ins);
    if (m.isEmpty()) {
        return;
    }
    MessageInfo info;
    info.mode = ins->_curMsgMode;
    info.row = msg->row;
    info.col = msg->col;
    info.section = msg->section;
    info.message = m;
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

int ScriptMachine::pragmaCallback(const QByteArray &pragmaText,
                                  AsPreprocesser *builder,
                                  const QString &sectionname, void *userParam) {
    Q_UNUSED(userParam);

    asIScriptEngine *engine = builder->getEngine();

    // Filter the pragmaText so only what is of interest remains
    // With this the user can add comments and use different whitespaces
    // without affecting the result

    asUINT pos = 0;
    asUINT length = 0;
    QStringList tokens;
    while (pos < pragmaText.size()) {
        asETokenClass tokenClass =
            engine->ParseToken(pragmaText.data() + pos, 0, &length);
        if (tokenClass == asTC_IDENTIFIER || tokenClass == asTC_KEYWORD ||
            tokenClass == asTC_VALUE) {
            auto token = pragmaText.mid(pos, length);
            tokens << token;
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

int ScriptMachine::includeCallback(const QString &include, bool quotedInclude,
                                   const QString &from, AsPreprocesser *builder,
                                   void *userParam) {
    Q_UNUSED(userParam);

    QFileInfo info(include);
    bool isAbsolute = info.isAbsolute();
    bool hasNoExt = info.suffix().isEmpty();
    QString inc;
    if (quotedInclude) {
        if (isAbsolute) {
            inc = include;
        } else {
            auto pwd = QFileInfo(from).absoluteDir();
            inc = pwd.absoluteFilePath(include);
        }
    } else {
        // absolute include is not allowed in #include<>
        if (isAbsolute) {
            return asERROR;
        }

        QDir dir(qApp->applicationDirPath());
        if (!dir.cd(QStringLiteral("aslib"))) {
            // someone crash the software
            return asERROR;
        }
        inc = dir.absoluteFilePath(include);
    }

    if (hasNoExt) {
        inc += QStringLiteral(".as");
    }

    return builder->loadSectionFromFile(inc);
}

QString ScriptMachine::processTranslation(const char *content,
                                          ScriptMachine *machine) {
    static QHash<QRegularExpression, TranslateFunc> exps{
        {QRegularExpression(QStringLiteral("^'(.*?)' is already declared")),
         [](const QStringList &contents) -> QString {
             return tr("'%1' is already declared").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Abstract class '(.*?)' cannot be instantiated")),
         [](const QStringList &contents) -> QString {
             return tr("Abstract class '%1' cannot be instantiated")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Accessing private property '(.*?)' of parent class")),
         [](const QStringList &contents) -> QString {
             return tr("Accessing private property '%1' of parent class")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Attribute '(.*?)' informed multiple times")),
         [](const QStringList &contents) -> QString {
             return tr("Attribute '%1' informed multiple times")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Can't form arrays of subtype '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Can't form arrays of subtype '%1'").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Can't inherit from class '(.*?)' marked as final")),
         [](const QStringList &contents) -> QString {
             return tr("Can't inherit from class '%1' marked as final")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Cannot access non-static member '(.*?)' like this")),
         [](const QStringList &contents) -> QString {
             return tr("Cannot access non-static member '%1' like this")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Can't construct handle '(.*?)'. Use ref cast instead")),
         [](const QStringList &contents) -> QString {
             return tr("Can't construct handle '%1'. Use ref cast instead")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Can't implicitly convert from '(.*?)' to '(.*?)'.")),
         [](const QStringList &contents) -> QString {
             return tr("Can't implicitly convert from '%1' to '%2'.")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(QStringLiteral("^Compiling (.*)")),
         [](const QStringList &contents) -> QString {
             return tr("Compiling %1").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral("^Compiling auto generated (.*)")),
         [](const QStringList &contents) -> QString {
             return tr("Compiling auto generated %1").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Implemented property accessor '(.*?)' "
                            "does not expect index argument")),
         [](const QStringList &contents) -> QString {
             return tr("Implemented property accessor '%1' does not expect "
                       "index argument")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Implemented property accessor '(.*?)' expects index argument")),
         [](const QStringList &contents) -> QString {
             return tr("Implemented property accessor '%1' expects index "
                       "argument")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral("^Data type can't be '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Data type can't be '%1'").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^All subsequent parameters after the first default "
             "value must have default values in function '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("All subsequent parameters after the first default "
                       "value must have default values in function '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^The method in the derived class must have the "
                            "same return type as in the base class: '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("The method in the derived class must have the "
                       "same return type as in the base class: '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^The name of the destructor '(.*?)::~(.*?)' must be the "
             "same as the class")),
         [](const QStringList &contents) -> QString {
             return tr("The name of the destructor '%1::~%2' must be "
                       "the same as the class")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(QStringLiteral("^Expected '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Expected '%1'").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral("^Expected '(.*?)' or '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Expected '%1' or '%2'")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(QStringLiteral(
             "^Expression must be of boolean type, instead found '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Expression must be of boolean type, instead found '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Expression '(.*?)' is a data type")),
         [](const QStringList &contents) -> QString {
             return tr("Expression '%1' is a data type").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^External shared entity '(.*?)' not found")),
         [](const QStringList &contents) -> QString {
             return tr("External shared entity '%1' not found")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^External shared entity '(.*?)' cannot "
                            "redefine the original entity")),
         [](const QStringList &contents) -> QString {
             return tr("External shared entity '%1' cannot "
                       "redefine the original entity")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Failed while compiling default arg "
                            "for parameter (.*?) in function '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Failed while compiling default arg "
                       "for parameter %1 in function '%2'")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(QStringLiteral("^Function '(.*?)' not found")),
         [](const QStringList &contents) -> QString {
             return tr("Function '%1' not found").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^The property '(.*?)' has mismatching types for the "
             "get and set accessors")),
         [](const QStringList &contents) -> QString {
             return tr("The property '%1' has mismatching types for the "
                       "get and set accessors")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Variable '(.*?)' hides another variable of same name "
             "in outer scope")),
         [](const QStringList &contents) -> QString {
             return tr("Variable '%1' hides another variable of same name "
                       "in outer scope")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Identifier '(.*?)' is not a data type")),
         [](const QStringList &contents) -> QString {
             return tr("Identifier '%1' is not a data type")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Identifier '(.*?)' is not a data type in global namespace")),
         [](const QStringList &contents) -> QString {
             return tr("Identifier '%1' is not a data type in global namespace")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Identifier '(.*?)' is not a data type in "
                            "namespace '(.*?)' or parent")),
         [](const QStringList &contents) -> QString {
             return tr("Identifier '%1' is not a data type in "
                       "namespace '%2' or parent")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(QStringLiteral("^Illegal operation on '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Illegal operation on '%1'").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral("^Illegal variable name '(.*?)'.")),
         [](const QStringList &contents) -> QString {
             return tr("Illegal variable name '%1'.").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Illegal access to inherited private property '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Illegal access to inherited private property '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Initialization lists cannot be used with '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Initialization lists cannot be used with '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral("^Attempting to instantiate invalid "
                                           "template type '(.*?)<(.*?)>'")),
         [](const QStringList &contents) -> QString {
             return tr("Attempting to instantiate invalid "
                       "template type '%1<%2>'")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(QStringLiteral("^Instead found '(.*?)'")),
         [machine](const QStringList &contents) -> QString {
             return tr("Instead found '%1'").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Instead found identifier '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Instead found identifier '%1'").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Instead found reserved keyword '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Instead found reserved keyword '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Interface '(.*?)' cannot be instantiated")),
         [](const QStringList &contents) -> QString {
             return tr("Interface '%1' cannot be instantiated")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Invalid unicode escape sequence, expected (.*?) hex digits")),
         [](const QStringList &contents) -> QString {
             return tr("Invalid unicode escape sequence, expected %1 hex "
                       "digits")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Method '(.*?)' declared as final and cannot be overridden")),
         [](const QStringList &contents) -> QString {
             return tr("Method '%1' declared as final and cannot be overridden")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Method '(.*?)' marked as override but does not "
                            "replace any base class or interface method")),
         [](const QStringList &contents) -> QString {
             return tr("Method '%1' marked as override but does not "
                       "replace any base class or interface method")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Method '(.*?)::(.*?)' is missing the return type, "
             "nor is it the same name as object to be a constructor")),
         [](const QStringList &contents) -> QString {
             return tr("Method '%1::%2' is missing the return type, "
                       "nor is it the same name as object to be a constructor")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(
             QStringLiteral("^Method '(.*?)' is not part of object '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Method '%1' is not part of object '%2'")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(
             QStringLiteral("^Missing implementation of '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Missing implementation of '%1'").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral("^Missing definition of '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Missing definition of '%1'").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Mixin class cannot be declared as '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Mixin class cannot be declared as '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Multiple matching signatures to '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Multiple matching signatures to '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Found multiple get accessors for property '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Found multiple get accessors for property '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Found multiple set accessors for property '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Found multiple set accessors for property '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Namespace '(.*?)' doesn't exist.")),
         [](const QStringList &contents) -> QString {
             return tr("Namespace '%1' doesn't exist.").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Name conflict. '(.*?)' is an extended data type.")),
         [](const QStringList &contents) -> QString {
             return tr("Name conflict. ") +
                    tr("'%1' is an extended data type.").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Name conflict. '(.*?)' is a global property.")),
         [](const QStringList &contents) -> QString {
             return tr("Name conflict. ") +
                    tr("'%1' is a global property.").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Name conflict. '(.*?)' is a named type.")),
         [](const QStringList &contents) -> QString {
             return tr("Name conflict. ") +
                    tr("'%1' is a named type.").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Name conflict. '(.*?)' is a funcdef.")),
         [](const QStringList &contents) -> QString {
             return tr("Name conflict. ") +
                    tr("'%1' is a funcdef.").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Name conflict. '(.*?)' is a global function.")),
         [](const QStringList &contents) -> QString {
             return tr("Name conflict. ") +
                    tr("'%1' is a global function.").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Name conflict. '(.*?)' is a mixin class.")),
         [](const QStringList &contents) -> QString {
             return tr("Name conflict. ") +
                    tr("'%1' is a mixin class.").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Name conflict. '(.*?)' is a virtual property.")),
         [](const QStringList &contents) -> QString {
             return tr("Name conflict. ") +
                    tr("'%1' is a virtual property.").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Name conflict. '(.*?)' is a class.")),
         [](const QStringList &contents) -> QString {
             return tr("Name conflict. ") +
                    tr("'%1' is a class.").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Name conflict. '(.*?)' is an interface.")),
         [](const QStringList &contents) -> QString {
             return tr("Name conflict. ") +
                    tr("'%1' is an interface.").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Name conflict. '(.*?)' is an object property.")),
         [](const QStringList &contents) -> QString {
             return tr("Name conflict. ") +
                    tr("'%1' is an object property.").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Name conflict. '(.*?)' is a class method.")),
         [](const QStringList &contents) -> QString {
             return tr("Name conflict. ") +
                    tr("'%1' is a class method.").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Name conflict. '(.*?)' is already used.")),
         [](const QStringList &contents) -> QString {
             return tr("Name conflict. ") +
                    tr("'%1' is already used.").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^No appropriate opHndlAssign method found in '(.*?)' "
             "for handle assignment")),
         [](const QStringList &contents) -> QString {
             return tr("No appropriate opHndlAssign method found in '%1' "
                       "for handle assignment")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^No conversion from '(.*?)' to '(.*?)' available.")),
         [](const QStringList &contents) -> QString {
             return tr("No conversion from '%1' to '%2' available.")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(QStringLiteral(
             "^No conversion from '(.*?)' to math type available.")),
         [](const QStringList &contents) -> QString {
             return tr("No conversion from '%1' to math type available.")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^No default constructor for object of type '(.*?)'.")),
         [](const QStringList &contents) -> QString {
             return tr("No default constructor for object of type '%1'.")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^No appropriate opAssign method found in '(.*?)' for "
             "value assignment")),
         [](const QStringList &contents) -> QString {
             return tr("No appropriate opAssign method found in '%1' for "
                       "value assignment")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^No copy constructor for object of type '(.*?)'.")),
         [](const QStringList &contents) -> QString {
             return tr("No copy constructor for object of type '%1'.")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^No matching signatures to '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("No matching signatures to '%1'").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^No matching operator that takes the type '(.*?)' found")),
         [](const QStringList &contents) -> QString {
             return tr("No matching operator that takes the type '%1' found")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^No matching operator that takes the types '(.*?)' "
             "and '(.*?)' found")),
         [](const QStringList &contents) -> QString {
             return tr("No matching operator that takes the types '%1' "
                       "and '%2' found")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(QStringLiteral("^No matching symbol '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("No matching symbol '%1'").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Expression doesn't form a function call. '(.*?)' "
                            "evaluates to the non-function type '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Expression doesn't form a function call. '%1' "
                       "evaluates to the non-function type '%2'")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(QStringLiteral("^'(.*?)' is not declared")),
         [](const QStringList &contents) -> QString {
             return tr("'%1' is not declared").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral("^'(.*?)' is not initialized.")),
         [](const QStringList &contents) -> QString {
             return tr("'%1' is not initialized.").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^'(.*?)' is not a member of '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("'%1' is not a member of '%2'")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(QStringLiteral(
             "^Type '(.*?)' doesn't support the indexing operator")),
         [](const QStringList &contents) -> QString {
             return tr("Type '%1' doesn't support the indexing operator")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Parameter type can't be '(.*?)', because the type "
             "cannot be instantiated.")),
         [](const QStringList &contents) -> QString {
             return tr("Parameter type can't be '%1', because the type "
                       "cannot be instantiated.")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Previous error occurred while attempting to "
                            "compile initialization list for type '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Previous error occurred while attempting to "
                       "compile initialization list for type '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Illegal call to private method '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Illegal call to private method '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Illegal access to private property '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Illegal access to private property '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Illegal call to protected method '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Illegal call to protected method '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Illegal access to protected property '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Illegal access to protected property '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral("^Return type can't be '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Return type can't be '%1'").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Shared code cannot access non-shared global variable '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Shared code cannot access non-shared global variable "
                       "'%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Shared code cannot call non-shared function '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Shared code cannot call non-shared function '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Shared type cannot implement non-shared interface '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Shared type cannot implement non-shared interface '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Shared class cannot inherit from non-shared class '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Shared class cannot inherit from non-shared class '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Shared code cannot use non-shared type '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Shared code cannot use non-shared type '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Shared type '(.*?)' doesn't match the original "
                            "declaration in other module")),
         [](const QStringList &contents) -> QString {
             return tr("Shared type '%1' doesn't match the original "
                       "declaration in other module")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Template '(.*?)' expects (.*?) sub type\\(s\\)")),
         [](const QStringList &contents) -> QString {
             return tr("Template '%1' expects %2 sub type(s)")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(
             QStringLiteral("^Type '(.*?)' cannot be a reference")),
         [](const QStringList &contents) -> QString {
             return tr("Type '%1' cannot be a reference").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Type '(.*?)' is not available for this module")),
         [](const QStringList &contents) -> QString {
             return tr("Type '%1' is not available for this module")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Type '(.*?)' is not a template type")),
         [](const QStringList &contents) -> QString {
             return tr("Type '%1' is not a template type").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral("^Unexpected token '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Unexpected token '%1'").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Use of uninitialized global variable '(.*?)'.")),
         [](const QStringList &contents) -> QString {
             return tr("Use of uninitialized global variable '%1'.")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral("^Unknown scope '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Unknown scope '%1'").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral("^Where '(.*?)' is '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Where '%1' is '%2'")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(
             QStringLiteral("^Failed to initialize global variable '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Failed to initialize global variable '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral("^Exception '(.*?)' in '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Exception '%1' in '%2'")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(
             QStringLiteral("^Type '(.*?)' is missing behaviours")),
         [](const QStringList &contents) -> QString {
             return tr("Type '%1' is missing behaviours").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Can't pass type '(.*?)' by value unless the "
             "application type is informed in the registration")),
         [](const QStringList &contents) -> QString {
             return tr("Can't pass type '%1' by value unless the "
                       "application type is informed in the registration")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Can't return type '(.*?)' by value unless the "
             "application type is informed in the registration")),
         [](const QStringList &contents) -> QString {
             return tr("Can't return type '%1' by value unless the "
                       "application type is informed in the registration")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Don't support passing type '(.*?)' by value to application in "
             "native calling convention on this platform")),
         [](const QStringList &contents) -> QString {
             return tr("Don't support passing type '%1' by value to "
                       "application in native calling convention on this "
                       "platform")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Don't support returning type '(.*?)' by value "
                            "from application in "
                            "native calling convention on this platform")),
         [](const QStringList &contents) -> QString {
             return tr("Don't support returning type '%1' by value "
                       "from application in "
                       "native calling convention on this platform")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Object \\{(.*?)\\}. GC cannot destroy an object of "
             "type '(.*?)' as it "
             "doesn't know how many references to there are.")),
         [](const QStringList &contents) -> QString {
             return tr("Object {%1}. GC cannot destroy an object of "
                       "type '%2' as it "
                       "doesn't know how many references to there are.")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(QStringLiteral(
             "^Object \\{(.*?)\\}. GC cannot destroy an object of type '(.*?)' "
             "as it can't see all references. Current ref count is (.*?).")),
         [](const QStringList &contents) -> QString {
             return tr("Object {%1}. GC cannot destroy an object of type '%2' "
                       "as it can't see all references. Current ref count is "
                       "%3.")
                 .arg(contents.at(1), contents.at(2), contents.at(3));
         }},
        {QRegularExpression(
             QStringLiteral("^Object type '(.*?)' doesn't exist")),
         [](const QStringList &contents) -> QString {
             return tr("Object type '%1' doesn't exist").arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Cannot register. The template type instance '(.*?)' "
             "has already been generated.")),
         [](const QStringList &contents) -> QString {
             return tr("Cannot register. The template type instance '%1' "
                       "has already been generated.")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Template type '(.*?)' doesn't exist")),
         [](const QStringList &contents) -> QString {
             return tr("Template type '%1' doesn't exist").arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Template subtype '(.*?)' doesn't exist")),
         [](const QStringList &contents) -> QString {
             return tr("Template subtype '%1' doesn't exist")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Failed to read subtype of template type '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Failed to read subtype of template type '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^Failed in call to function '(.*?)' \\(Code: (.*?), (.*?)\\)")),
         [](const QStringList &contents) -> QString {
             return tr("Failed in call to function '%1' (Code: %2, %3)")
                 .arg(contents.at(1), contents.at(2), contents.at(3));
         }},
        {QRegularExpression(
             QStringLiteral("^Failed in call to function '(.*?)' "
                            "with '(.*?)' \\(Code: (.*?), (.*?)\\)")),
         [](const QStringList &contents) -> QString {
             return tr("Failed in call to function '%1' "
                       "with '%2' (Code: %3, %4)")
                 .arg(contents.at(1), contents.at(2), contents.at(3),
                      contents.at(4));
         }},
        {QRegularExpression(QStringLiteral(
             "^Failed in call to function '(.*?)' "
             "with '(.*?)' and '(.*?)' \\(Code: (.*?), (.*?)\\)")),
         [](const QStringList &contents) -> QString {
             return tr("Failed in call to function '%1' "
                       "with '%2' and '%3' (Code: %4, %5)")
                 .arg(contents.at(1), contents.at(2), contents.at(3),
                      contents.at(4), contents.at(5));
         }},
        {QRegularExpression(
             QStringLiteral("^Type '(.*?)' is still used by function '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("Type '%1' is still used by function '%2'")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(QStringLiteral(
             "^The builtin type in previous message is named '(.*?)'")),
         [](const QStringList &contents) -> QString {
             return tr("The builtin type in previous message is named '%1'")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^The function in previous message is named '(.*?)'. "
             "The func type is (.*?)")),
         [](const QStringList &contents) -> QString {
             return tr("The function in previous message is named '%1'. "
                       "The func type is %2")
                 .arg(contents.at(1), contents.at(2));
         }},
        {QRegularExpression(
             QStringLiteral("^The script object of type '(.*?)' is being "
                            "resurrected illegally during destruction")),
         [](const QStringList &contents) -> QString {
             return tr("The script object of type '%1' is being "
                       "resurrected illegally during destruction")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^LoadByteCode failed. The bytecode is invalid. "
                            "Number of bytes read from stream: (.*?)")),
         [](const QStringList &contents) -> QString {
             return tr("LoadByteCode failed. The bytecode is invalid. "
                       "Number of bytes read from stream: %1")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(
             QStringLiteral("^Function '(.*?)' appears to have been "
                            "compiled without JIT entry points")),
         [](const QStringList &contents) -> QString {
             return tr("Function '%1' appears to have been "
                       "compiled without JIT entry points")
                 .arg(contents.at(1));
         }},
        {QRegularExpression(QStringLiteral(
             "^There is an external reference to an object in "
             "module '(.*?)', preventing it from being deleted")),
         [](const QStringList &contents) -> QString {
             return tr("There is an external reference to an object in "
                       "module '%1', preventing it from being deleted")
                 .arg(contents.at(1));
         }}};

    QString con(content);
    for (auto exp = exps.keyValueBegin(); exp != exps.keyValueEnd(); ++exp) {
        auto m = exp->first.match(con);
        if (m.hasMatch()) {
            return exp->second(m.capturedTexts());
        }
    }

    return tr(content);
}

void ScriptMachine::translation() {
    // const strings from as_texts.h
    tr("Output argument expression is not assignable");
    tr("Auto is not allowed here");
    tr("Can't find unambiguous implicit conversion to make both expressions "
       "have the same type");
    tr("Both conditions must call constructor");
    tr("Base class doesn't have default constructor. Make explicit call to "
       "base constructor");
    tr("Base class doesn't have copy constructor or default constructor and "
       "assign operator. Make explicit call to base constructor");
    tr("Candidates are:");
    tr("Can't call a constructor in loops");
    tr("Can't call a constructor in switch");
    tr("Can't call a constructor multiple times");
    tr("Can't create delegate");
    tr("Can't create delegate for types that do not support handles");
    tr("Cannot flag function that will not be auto generated as deleted");
    tr("Conflict with explicit declaration of function and deleted function");
    tr("Can't implement itself, or another interface that implements this "
       "interface");
    tr("Can't inherit from multiple classes");
    tr("Can't inherit from itself, or another class that inherits from this "
       "class");
    tr("Can't pass class method as arg directly. Use a delegate object "
       "instead");
    tr("Unable to resolve auto type");
    tr("Can't return reference to local value.");
    tr("Can't return value when return type is 'void'");
    tr("Implicit conversion changed sign of value");
    tr("A class cannot be both abstract and final");
    tr("Compound assignments with property accessors on value types are not "
       "supported");
    tr("Compound assignments with indexed property accessors are not "
       "supported");
    tr("Compound assignments with property accessors require both get and set "
       "accessors");
    tr("Variables cannot be declared in switch cases, except inside statement "
       "blocks");
    tr("The default case must be the last one");
    tr("The type of the default argument expression doesn't match the function "
       "parameter type");
    tr("Deleted functions cannot have implementation");
    tr("The destructor must not have any parameters");
    tr("Value assignment on reference types is not allowed. Did you mean to do "
       "a handle assignment?");
    tr("Compound assignment on reference types is not allowed");
    tr("Duplicate named argument");
    tr("Duplicate switch case");
    tr("Else with empty statement");
    tr("Empty list element is not allowed");
    tr("Empty switch statement");
    tr("Expected constant");
    tr("Expected data type");
    tr("Expected expression value");
    tr("Expected identifier");
    tr("Expected a list enclosed by { } to match pattern");
    tr("Expected method or property");
    tr("Expected one of: ");
    tr("Expected operator");
    tr("Expected post operator");
    tr("Expected pre operator");
    tr("Expected string");
    tr("Expression doesn't evaluate to a function");
    tr("Previous error occurred while attempting to create a temporary copy of "
       "object");
    tr("Float value truncated in implicit conversion to integer");
    tr("Found multiple matching enum values");
    tr("A function with the same name and parameters already exists");
    tr("Global variables have been disabled by the application");
    tr("It is not allowed to perform a handle assignment on a non-handle "
       "property");
    tr("The operand is implicitly converted to handle in order to compare "
       "them");
    tr("Handle to handle is not allowed");
    tr("If with empty statement");
    tr("Illegal member type");
    tr("Illegal operation on this datatype");
    tr("Illegal target type for reference cast");
    tr("Interfaces can only implement other interfaces");
    tr("Invalid 'break'");
    tr("Invalid character literal");
    tr("Invalid 'continue'");
    tr("Invalid escape sequence");
    tr("Invalid expression: ambiguous name");
    tr("Invalid expression: stand-alone anonymous function");
    tr("Invalid operation on method");
    tr("Invalid reference. Property accessors cannot be used in combined "
       "read/write operations");
    tr("Invalid scope resolution");
    tr("Invalid signature for virtual property");
    tr("Invalid type");
    tr("Invalid unicode code point");
    tr("Invalid unicode sequence in source");
    tr("Invalid use of named arguments");
    tr("The method cannot be named with the class name");
    tr("Mixin classes cannot have constructors or destructors");
    tr("Mixin class cannot inherit from classes");
    tr("Mixin classes cannot have child types");
    tr("Found more than one matching operator");
    tr("Multiline strings are not allowed in this application");
    tr("Only objects have constructors");
    tr("Must return a value");
    tr("Detected named argument with old syntax");
    tr("No appropriate indexing operator found");
    tr("No appropriate opEquals method found");
    tr("The application doesn't support the default array type.");
    tr("Non-const method call on read-only object reference");
    tr("Non-terminated string literal");
    tr("Not all paths return a value");
    tr("Not enough values to match pattern");
    tr("Implicit conversion of value is not exact");
    tr("Expression is not an l-value");
    tr("Not a valid reference");
    tr("Not a valid lvalue");
    tr("Nothing was built in the module");
    tr("Object handle is not supported for this type");
    tr("Only object types that support object handles can use &inout. Use &in "
       "or &out instead");
    tr("A cast operator has one argument");
    tr("The code must contain one and only one function");
    tr("The code must contain one and only one global variable");
    tr("Both operands must be handles when comparing identity");
    tr("The overloaded functions are identical on initial parameters without "
       "default arguments");
    tr("Parameter already declared");
    tr("Positional arguments cannot be passed after named arguments");
    tr("Potentially matching non-const method is hidden on read-only object "
       "reference");
    tr("Property accessor with index must have 1 and only 1 index argument");
    tr("Property accessors have been disabled by the application");
    tr("Property accessor must be implemented");
    tr("Class properties cannot be declared as const");
    tr("The property has no get accessor");
    tr("The property has no set accessor");
    tr("Virtual property must have at least one get or set accessor");
    tr("Resulting reference cannot be returned. Returned references must not "
       "refer to local variables.");
    tr("Resulting reference cannot be returned. There are deferred arguments "
       "that may invalidate it.");
    tr("Resulting reference cannot be returned. The expression uses objects "
       "that during cleanup may invalidate it.");
    tr("Reference is read-only");
    tr("Reference is temporary");
    tr("Reference types cannot be passed by value in function parameters");
    tr("Reference types cannot be returned by value from functions");
    tr("The script section is empty");
    tr("Signed/Unsigned mismatch");
    tr("Strings are not recognized by the application");
    tr("Case expressions must be literal constants");
    tr("Switch expressions must be integral numbers");
    tr("The function has too many jump labels to handle. Split the function "
       "into smaller ones.");
    tr("Too many values to match pattern");
    tr("Unexpected end of file");
    tr("Unexpected variable declaration");
    tr("Unreachable code");
    tr("Virtual property contains unrecognized aspect");
    tr("Unused script node");
    tr("Value is too large for data type");
    tr("Void cannot be an operand in expressions");
    tr("Warnings are treated as errors by the application");
    tr("While parsing argument list");
    tr("While parsing expression");
    tr("While parsing initialization list");
    tr("While parsing namespace");
    tr("While parsing statement block");
    tr("Previous error occurred while including mixin");
    tr("Autohandles cannot be used with types that have been registered with "
       "NOCOUNT");
    tr("First parameter to template factory must be a reference. This will be "
       "used to pass the object type of the template");
    tr("Invalid configuration. Verify the registered application interface.");
    tr("A value type must be registered with a non-zero size");
    tr("The behaviour is not compatible with the type");
    tr("A garbage collected ref type must have the addref, release, and all gc "
       "behaviours");
    tr("A garbage collected value type must have the gc enum references "
       "behaviour");
    tr("A scoped reference type must have the release behaviour");
    tr("A reference type must have the addref and release behaviours");
    tr("A non-pod value type must have at least one constructor and the "
       "destructor behaviours");
    tr("Template list factory expects two reference parameters. The last is "
       "the pointer to the initialization buffer");
    tr("List factory expects only one reference parameter. The pointer to the "
       "initialization buffer will be passed in this parameter");
    tr("AddScriptObjectToGC called with null pointer");
    tr("An exception occurred in a nested call");
    tr("Uh oh! The engine's reference count is increasing while it is being "
       "destroyed. Make sure references needed for clean-up are immediately "
       "released");
    tr("The module is still in use and cannot be rebuilt. Discard it and "
       "request another module");
    tr("Property");
    tr("System function");
    tr("Variable declaration");
    tr("Stack overflow");
    tr("Null pointer access");
    tr("Divide by zero");
    tr("Overflow in integer division");
    tr("Overflow in exponent operation");
    tr("Unrecognized byte code");
    tr("Invalid calling convention");
    tr("Unbound function called");
    tr("Out of range");
    tr("Caught an exception from the application");
    tr("Mismatching types in value assignment");
    tr("Too many nested calls");
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

    engine->SetDefaultAccessMask(0x1);
    RegisterScriptFile(engine);
    registerExceptionRoutines(engine);
    registerEngineAssert(engine);
    registerEngineClipboard(engine);
    AsDirectPromise::Register(engine);
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
    QAsCodeParser parser(_engine);
    auto ccode = code.toUtf8();

    asIScriptFunction *func = nullptr;

    QList<QAsCodeParser::CodeSegment> ret;
    if (mode > 0) {
        ret = parser.parse(ccode);
    }

    // check whether there is any enum/class
    if (ret.isEmpty() ||
        std::any_of(ret.begin(), ret.end(),
                    [](const QAsCodeParser::CodeSegment &seg) {
                        switch (seg.type) {
                        case QAsCodeParser::SymbolType::Enum:
                        case QAsCodeParser::SymbolType::Class:
                        case QAsCodeParser::SymbolType::Function:
                        case QAsCodeParser::SymbolType::Interface:
                        case QAsCodeParser::SymbolType::Import:
                        case QAsCodeParser::SymbolType::Variable:
                            return false;
                        case QAsCodeParser::SymbolType::Invalid:
                        case QAsCodeParser::SymbolType::TypeDef:
                        case QAsCodeParser::SymbolType::FnDef:
                            return true;
                        }
                        return true;
                    })) {
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
        if (std::all_of(ret.begin(), ret.end(),
                        [](const QAsCodeParser::CodeSegment &seg) {
                            return seg.type ==
                                   QAsCodeParser::SymbolType::Variable;
                        })) {
            _curMsgMode = mode;

            for (auto &s : ret) {
                auto r = mod->CompileGlobalVar(nullptr, s.codes, 0);
                if (r < 0) {
                    MessageInfo info;
                    info.mode = mode;
                    info.message = tr("BadDecl:") + s.codes;
                    info.type = MessageType::Error;
                    outputMessage(info);
                }
            }

            if (mod->ResetGlobalVars() < 0) {
                MessageInfo info;
                info.mode = mode;
                info.message = tr("GlobalBadDecl");
                info.type = MessageType::Error;
                outputMessage(info);
            }
            return true;
        }

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
