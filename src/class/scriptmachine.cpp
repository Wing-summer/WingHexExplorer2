#include "scriptmachine.h"

//#include "../AngelScript/add_on/autowrapper/aswrappedcall.h"
#include "../AngelScript/add_on/datetime/datetime.h"
#include "../AngelScript/add_on/scriptany/scriptany.h"
#include "../AngelScript/add_on/scriptarray/scriptarray.h"
#include "../AngelScript/add_on/scriptdictionary/scriptdictionary.h"
//#include "../AngelScript/add_on/scriptfile/scriptfile.h"
#include "../AngelScript/add_on/scriptfile/scriptfilesystem.h"
#include "../AngelScript/add_on/scriptgrid/scriptgrid.h"
#include "../AngelScript/add_on/scripthandle/scripthandle.h"
#include "../AngelScript/add_on/scripthelper/scripthelper.h"
#include "../AngelScript/add_on/scriptmath/scriptmath.h"
#include "../AngelScript/add_on/scriptmath/scriptmathcomplex.h"
#include "../AngelScript/add_on/scriptstdstring/scriptstdstring.h"
//#include "../AngelScript/add_on/serializer/serializer.h"
#include "../AngelScript/add_on/weakref/weakref.h"

#include <iostream>

ScriptMachine::~ScriptMachine() {
    if (_debugger) {
        delete _debugger;
    }

    if (_ctxMgr) {
        delete _ctxMgr;
    }

    _engine->ShutDownAndRelease();
}

bool ScriptMachine::isRunning() const { return _ctxMgr != nullptr; }

bool ScriptMachine::runScript(const QString &script, bool isInDebug) {
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
        _engine->WriteMessage(script.toUtf8(), 0, 0, asMSGTYPE_ERROR,
                              "Cannot find 'int main()' or 'void main()'");
        return false;
    }

    _ctxMgr = new CContextMgr;
    _ctxMgr->RegisterCoRoutineSupport(_engine);
    _ctxMgr->RegisterThreadSupport(_engine);
    Q_ASSERT(_engine->SetContextCallbacks(
                 &ScriptMachine::requestContextCallback,
                 &ScriptMachine::returnContextCallback, this) >= 0);

    if (isInDebug) {
        _debugger = new CDebugger;

        // Let the debugger hold an engine pointer that can be used by the
        // callbacks
        _debugger->SetEngine(_engine);

        // Register the to-string callbacks so the user can see the contents of
        // strings
        _debugger->RegisterToStringCallback(
            _engine->GetTypeInfoByName("string"),
            &ScriptMachine::stringToString);
        _debugger->RegisterToStringCallback(_engine->GetTypeInfoByName("array"),
                                            &ScriptMachine::arrayToString);
        _debugger->RegisterToStringCallback(
            _engine->GetTypeInfoByName("dictionary"),
            &ScriptMachine::dictionaryToString);
        _debugger->RegisterToStringCallback(
            _engine->GetTypeInfoByName("datetime"),
            &ScriptMachine::dateTimeToString);

        // Allow the user to initialize the debugging before moving on
        std::cout << "Debugging, waiting for commands. Type 'h' for help."
                  << std::endl;
        _debugger->TakeCommands(0);
    }

    // Once we have the main function, we first need to initialize the global
    // variables Since we've set up the request context callback we will be able
    // to debug the initialization without passing in a pre-created context
    int r = mod->ResetGlobalVars(0);
    if (r < 0) {
        _engine->WriteMessage(script.toUtf8(), 0, 0, asMSGTYPE_ERROR,
                              "Failed while initializing global variables");
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

    // Destroy debugger
    if (_debugger) {
        delete _debugger;
        _debugger = nullptr;
    }

    return r >= 0;
}

bool ScriptMachine::compileScript(const QString &script) {
    int r;

    // We will only initialize the global variables once we're
    // ready to execute, so disable the automatic initialization
    _engine->SetEngineProperty(asEP_INIT_GLOBAL_VARS_AFTER_BUILD, false);

    CScriptBuilder builder;

    // Set the pragma callback so we can detect if the script needs debugging
    builder.SetPragmaCallback(&ScriptMachine::pragmaCallback, 0);

    // Compile the script
    r = builder.StartNewModule(_engine, "script");
    if (r < 0)
        return false;

    r = builder.AddSectionFromFile(script.toUtf8());
    if (r < 0)
        return false;

    r = builder.BuildModule();
    if (r < 0) {
        _engine->WriteMessage(script.toUtf8(), 0, 0, asMSGTYPE_ERROR,
                              "Script failed to build");
        return false;
    }

    return true;
}

ScriptMachine::ScriptMachine(QObject *parent) : QObject(parent) {
    _engine = asCreateScriptEngine();

    RegisterStdString(_engine);
    RegisterStdStringUtils(_engine);
    RegisterScriptMath(_engine);
    RegisterScriptMathComplex(_engine);
    RegisterScriptFileSystem(_engine);
    RegisterScriptDateTime(_engine);
    RegisterScriptWeakRef(_engine);
    RegisterScriptAny(_engine);
    RegisterScriptArray(_engine, false);
    RegisterScriptDictionary(_engine);
    RegisterScriptGrid(_engine);
    RegisterScriptHandle(_engine);
    RegisterExceptionRoutines(_engine);
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

std::string ScriptMachine::stringToString(void *obj, int expandMembers,
                                          CDebugger *dbg) {
    // We know the received object is a string
    std::string *val = reinterpret_cast<std::string *>(obj);

    // Format the output string
    // TODO: Should convert non-readable characters to escape sequences
    std::stringstream s;
    s << "(len=" << val->length() << ") \"";
    if (val->length() < 20)
        s << *val << "\"";
    else
        s << val->substr(0, 20) << "...";

    return s.str();
}

std::string ScriptMachine::arrayToString(void *obj, int expandMembers,
                                         CDebugger *dbg) {
    CScriptArray *arr = reinterpret_cast<CScriptArray *>(obj);

    std::stringstream s;
    s << "(len=" << arr->GetSize() << ")";

    if (expandMembers > 0) {
        s << " [";
        for (asUINT n = 0; n < arr->GetSize(); n++) {
            s << dbg->ToString(arr->At(n), arr->GetElementTypeId(),
                               expandMembers - 1,
                               arr->GetArrayObjectType()->GetEngine());
            if (n < arr->GetSize() - 1)
                s << ", ";
        }
        s << "]";
    }

    return s.str();
}

std::string ScriptMachine::dictionaryToString(void *obj, int expandMembers,
                                              CDebugger *dbg) {
    CScriptDictionary *dic = reinterpret_cast<CScriptDictionary *>(obj);

    std::stringstream s;
    s << "(len=" << dic->GetSize() << ")";

    if (expandMembers > 0) {
        s << " [";
        asUINT n = 0;
        for (CScriptDictionary::CIterator it = dic->begin(); it != dic->end();
             it++, n++) {
            s << "[" << it.GetKey() << "] = ";

            // Get the type and address of the value
            const void *val = it.GetAddressOfValue();
            int typeId = it.GetTypeId();

            // Use the engine from the currently active context (if none is
            // active, the debugger will use the engine held inside it by
            // default, but in an environment where there multiple engines this
            // might not be the correct instance).
            asIScriptContext *ctx = asGetActiveContext();

            s << dbg->ToString(const_cast<void *>(val), typeId,
                               expandMembers - 1, ctx ? ctx->GetEngine() : 0);

            if (n < dic->GetSize() - 1)
                s << ", ";
        }
        s << "]";
    }

    return s.str();
}

std::string ScriptMachine::dateTimeToString(void *obj, int expandMembers,
                                            CDebugger *dbg) {
    CDateTime *dt = reinterpret_cast<CDateTime *>(obj);

    std::stringstream s;
    s << "{" << dt->getYear() << "-" << dt->getMonth() << "-" << dt->getDay()
      << " ";
    s << dt->getHour() << ":" << dt->getMinute() << ":" << dt->getSecond()
      << "}";

    return s.str();
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
