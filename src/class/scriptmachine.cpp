#include "scriptmachine.h"

#include "AngelScript/add_on/datetime/datetime.h"
#include "AngelScript/add_on/scriptany/scriptany.h"
#include "AngelScript/add_on/scriptarray/scriptarray.h"
#include "AngelScript/add_on/scriptdictionary/scriptdictionary.h"
#include "AngelScript/add_on/scriptfile/scriptfilesystem.h"
#include "AngelScript/add_on/scriptgrid/scriptgrid.h"
#include "AngelScript/add_on/scripthandle/scripthandle.h"
#include "AngelScript/add_on/scripthelper/scripthelper.h"
#include "AngelScript/add_on/scriptmath/scriptmath.h"
#include "AngelScript/add_on/scriptmath/scriptmathcomplex.h"
#include "AngelScript/add_on/scriptstdstring/scriptstdstring.h"
#include "AngelScript/add_on/weakref/weakref.h"

#include "angelobjstring.h"

#include <QProcess>
#include <iostream>

ScriptMachine::~ScriptMachine() {
    if (_ctxMgr) {
        delete _ctxMgr;
    }

    _engine->ShutDownAndRelease();
}

bool ScriptMachine::inited() { return _engine != nullptr; }

bool ScriptMachine::isRunning() const { return _ctxMgr != nullptr; }

bool ScriptMachine::configureEngine(asIScriptEngine *engine) {
    if (engine == nullptr) {
        return false;
    }

    // The script compiler will send any compiler messages to the callback
    auto r = engine->SetMessageCallback(asFUNCTION(messageCallback), this,
                                        asCALL_CDECL);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    RegisterScriptArray(engine, false);
    RegisterStdString(engine);
    RegisterScriptMath(engine);
    RegisterScriptMathComplex(engine);
    RegisterScriptWeakRef(engine);
    RegisterScriptAny(engine);
    RegisterScriptDictionary(engine);
    RegisterScriptGrid(engine);
    RegisterScriptDateTime(engine);
    RegisterScriptFileSystem(engine);
    RegisterScriptHandle(engine);
    RegisterStdStringUtils(engine);
    RegisterExceptionRoutines(engine);

    _rtypes.resize(RegisteredType::tMAXCOUNT);
    _rtypes[RegisteredType::tString] =
        q_check_ptr(_engine->GetTypeInfoByName("string"));
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
    _rtypes[RegisteredType::tDateTime] =
        q_check_ptr(_engine->GetTypeInfoByName("datetime"));
    _rtypes[RegisteredType::tRef] =
        q_check_ptr(_engine->GetTypeInfoByName("ref"));

    // Register a couple of extra functions for the scripts
    _printFn = std::bind(&ScriptMachine::print, this, std::placeholders::_1,
                         std::placeholders::_2);
    r = engine->RegisterGlobalFunction("void print(? &in)",
                                       asMETHOD(decltype(_printFn), operator()),
                                       asCALL_THISCALL_ASGLOBAL, &_printFn);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = engine->RegisterGlobalFunction(
        "string getInput()", asMETHOD(decltype(_getInputFn), operator()),
        asCALL_THISCALL_ASGLOBAL, &_getInputFn);
    assert(r >= 0);
    if (r < 0) {
        return false;
    }

    r = engine->RegisterGlobalFunction(
        "int exec(const string &in, const string &in)",
        asFUNCTIONPR(execSystemCmd, (const std::string &, const std::string &),
                     int),
        asCALL_CDECL);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = engine->RegisterGlobalFunction(
        "int exec(const string &in, const string &in, string &out)",
        asFUNCTIONPR(execSystemCmd,
                     (const std::string &, const std::string &, std::string &),
                     int),
        asCALL_CDECL);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    // Setup the context manager and register the support for co-routines
    _ctxMgr = new CContextMgr();
    _ctxMgr->RegisterCoRoutineSupport(engine);

    // Tell the engine to use our context pool. This will also
    // allow us to debug internal script calls made by the engine
    r = engine->SetContextCallbacks(requestContextCallback,
                                    returnContextCallback, this);
    Q_ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    _debugger = new asDebugger(this);

    // Register the to-string callbacks so the user can see the contents of
    // strings
    _debugger->RegisterToStringCallback(_rtypes[RegisteredType::tString],
                                        &AngelObjString::stringToString);
    _debugger->RegisterToStringCallback(_rtypes[RegisteredType::tArray],
                                        &AngelObjString::arrayToString);
    _debugger->RegisterToStringCallback(_rtypes[RegisteredType::tDictionary],
                                        &AngelObjString::dictionaryToString);
    _debugger->RegisterToStringCallback(_rtypes[RegisteredType::tDateTime],
                                        &AngelObjString::dateTimeToString);

    return true;
}

bool ScriptMachine::compileScript(const QString &script) {

    // We will only initialize the global variables once we're
    // ready to execute, so disable the automatic initialization
    _engine->SetEngineProperty(asEP_INIT_GLOBAL_VARS_AFTER_BUILD, false);

    CScriptBuilder builder;

    // Set the pragma callback so we can detect if the script needs debugging
    builder.SetPragmaCallback(pragmaCallback, 0);

    // Compile the script
    auto r = builder.StartNewModule(_engine, "script");
    if (r < 0) {
        return false;
    }

    auto scriptFile = script.toUtf8();

    r = builder.AddSectionFromFile(scriptFile);
    if (r < 0) {
        return false;
    }

    r = builder.BuildModule();
    if (r < 0) {
        _engine->WriteMessage(scriptFile, 0, 0, asMSGTYPE_ERROR,
                              "Script failed to build");
        return false;
    }

    return true;
}

void ScriptMachine::print(void *ref, int typeId) {
    MessageInfo info;
    auto str = _debugger->ToString(ref, typeId, 3, _engine);
    info.message = QString::fromStdString(str);
    emit onOutput(MessageType::Info, info);
}

std::string ScriptMachine::getInput() {
    Q_ASSERT(_getInputFn);
    return _getInputFn();
}

bool ScriptMachine::isType(asITypeInfo *tinfo, RegisteredType type) {
    Q_ASSERT(type < RegisteredType::tMAXCOUNT);
    auto t = _rtypes.at(type);
    return tinfo->DerivesFrom(t) || tinfo->Implements(t);
}

int ScriptMachine::execSystemCmd(const std::string &exe,
                                 const std::string &params, std::string &out) {
    QProcess ps;
    ps.setProgram(QString::fromStdString(exe));
    ps.setArguments(QProcess::splitCommand(QString::fromStdString(params)));
    ps.start();
    ps.waitForFinished(-1);
    auto r = ps.readAllStandardOutput();
    out = r.toStdString();
    return ps.exitCode();
}

int ScriptMachine::execSystemCmd(const std::string &exe,
                                 const std::string &params) {
    return QProcess::execute(
        QString::fromStdString(exe),
        QProcess::splitCommand(QString::fromStdString(params)));
}

bool ScriptMachine::executeScript(const QString &script, bool isInDebug) {
    asIScriptModule *mod = _engine->GetModule("script", asGM_ONLY_IF_EXISTS);
    if (!mod) {
        return false;
    }

    // Find the main function
    asIScriptFunction *func = mod->GetFunctionByDecl("int main()");
    if (func == nullptr) {
        // Try again with "void main()"
        func = mod->GetFunctionByDecl("void main()");
    }

    if (func == nullptr) {
        _engine->WriteMessage(
            script.toUtf8(), 0, 0, asMSGTYPE_ERROR,
            tr("Cannot find 'int main()' or 'void main()'").toUtf8());
        return false;
    }

    _ctxMgr = new CContextMgr;
    _ctxMgr->RegisterCoRoutineSupport(_engine);
    Q_ASSERT(_engine->SetContextCallbacks(
                 &ScriptMachine::requestContextCallback,
                 &ScriptMachine::returnContextCallback, this) >= 0);

    if (isInDebug) {
        // Let the debugger hold an engine pointer that can be used by the
        // callbacks
        _debugger->SetEngine(_engine);

        // Allow the user to initialize the debugging before moving on
        MessageInfo info;
        info.message =
            tr("Debugging, waiting for commands. Type 'h' for help.");
        emit onOutput(MessageType::Info, info);
        _debugger->TakeCommands(0);
    }

    // Once we have the main function, we first need to initialize the global
    // variables Since we've set up the request context callback we will be able
    // to debug the initialization without passing in a pre-created context
    int r = mod->ResetGlobalVars(0);
    if (r < 0) {
        _engine->WriteMessage(
            script.toUtf8(), 0, 0, asMSGTYPE_ERROR,
            tr("Failed while initializing global variables").toUtf8());
        return false;
    }

    // Set up a context to execute the script
    // The context manager will request the context from the
    // pool, which will automatically attach the debugger
    asIScriptContext *ctx = _ctxMgr->AddContext(_engine, func, true);

    // Execute the script until completion
    // The script may create co-routines. These will automatically
    // be managed by the context manager
    while (_ctxMgr->ExecuteScripts())
        ;

    // Check if the main script finished normally
    r = ctx->GetState();
    if (r != asEXECUTION_FINISHED) {
        if (r == asEXECUTION_EXCEPTION) {
            std::cout << "The script failed with an exception" << std::endl;
            std::cout << GetExceptionInfo(ctx, true);
            r = -1;
        } else if (r == asEXECUTION_ABORTED) {
            std::cout << "The script was aborted" << std::endl;
            r = -1;
        } else {
            std::cout << "The script terminated unexpectedly (" << r << ")"
                      << std::endl;
            r = -1;
        }
    } else {
        // Get the return value from the script
        if (func->GetReturnTypeId() == asTYPEID_INT32) {
            r = *(int *)ctx->GetAddressOfReturnValue();
        } else
            r = 0;
    }

    // Return the context after retrieving the return value
    _ctxMgr->DoneWithContext(ctx);

    // Destroy the context manager
    if (_ctxMgr) {
        delete _ctxMgr;
        _ctxMgr = nullptr;
    }

    // Before leaving, allow the engine to clean up remaining objects by
    // discarding the module and doing a full garbage collection so that
    // this can also be debugged if desired
    mod->Discard();
    _engine->GarbageCollect();

    // Release all contexts that have been allocated
    for (auto ctx : _ctxPool) {
        ctx->Release();
    }

    _ctxPool.clear();

    // Detach debugger
    Q_ASSERT(_debugger);
    _debugger->SetEngine(nullptr);

    return r >= 0;
}

void ScriptMachine::messageCallback(const asSMessageInfo *msg, void *param) {
    MessageType t;
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
    info.row = msg->row;
    info.col = msg->col;
    info.section = msg->section;
    info.message = msg->message;
    emit ins->onOutput(t, info);
}

ScriptMachine::ScriptMachine(std::function<std::string()> &getInputFn,
                             QObject *parent)
    : QObject(parent), _getInputFn(getInputFn) {
    Q_ASSERT(getInputFn);
    qRegisterMetaType<MessageInfo>();

    _engine = asCreateScriptEngine();
    if (!configureEngine(_engine)) {
        _engine->ShutDownAndRelease();
        _engine = nullptr;
    }
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
    if (ctx && p->_debugger) {
        // Set the line callback for the debugging
        ctx->SetLineCallback(asMETHOD(CDebugger, LineCallback), p->_debugger,
                             asCALL_THISCALL);
    }

    return ctx;
}

void ScriptMachine::returnContextCallback(asIScriptEngine *engine,
                                          asIScriptContext *ctx, void *param) {
    // We can also check for possible script exceptions here if so desired

    // Unprepare the context to free any objects it may still hold (e.g. return
    // value) This must be done before making the context available for re-use,
    // as the clean up may trigger other script executions, e.g. if a destructor
    // needs to call a function.
    ctx->Unprepare();

    auto p = reinterpret_cast<ScriptMachine *>(param);
    Q_ASSERT(p);

    // Place the context into the pool for when it will be needed again
    p->_ctxPool.push_back(ctx);
}

int ScriptMachine::pragmaCallback(const std::string &pragmaText,
                                  CScriptBuilder &builder, void *userParam) {
    asIScriptEngine *engine = builder.GetEngine();

    // Filter the pragmaText so only what is of interest remains
    // With this the user can add comments and use different whitespaces
    // without affecting the result
    asUINT pos = 0;
    asUINT length = 0;
    std::string cleanText;
    while (pos < pragmaText.size()) {
        asETokenClass tokenClass =
            engine->ParseToken(pragmaText.c_str() + pos, 0, &length);
        if (tokenClass == asTC_IDENTIFIER || tokenClass == asTC_KEYWORD ||
            tokenClass == asTC_VALUE) {
            std::string token = pragmaText.substr(pos, length);
            cleanText += " " + token;
        }
        if (tokenClass == asTC_UNKNOWN)
            return -1;
        pos += length;
    }

    // Interpret the result
    if (cleanText == " debug") {
        // g_doDebug = true;
        return 0;
    }

    // The #pragma directive was not accepted
    return -1;
}
