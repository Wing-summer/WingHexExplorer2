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

#include "scriptconsolemachine.h"

#include <QRegularExpression>
#include <QTextStream>

ScriptConsoleMachine::ScriptConsoleMachine(
    std::function<QString(void)> &getInputFn, QObject *parent)
    : ScriptMachine(getInputFn, parent) {
    if (!ScriptConsoleMachine::configureEngine(engine())) {
        destoryMachine();
    }
    if (engine()) {
        _model = new ScriptObjModel(engine(), debugger(), this);
    }
}

ScriptConsoleMachine::~ScriptConsoleMachine() {}

bool ScriptConsoleMachine::executeCode(const QString &code) {
    return execString(engine(), code);
}

bool ScriptConsoleMachine::configureEngine(asIScriptEngine *engine) {
    _clsfn = std::bind(&ScriptConsoleMachine::onClearConsole, this);
    auto r = engine->RegisterGlobalFunction(
        "void clear()", asMETHOD(decltype(_clsfn), operator()),
        asCALL_THISCALL_ASGLOBAL, &_clsfn);
    Q_ASSERT(r >= 0);
    return r >= 0;
}

bool ScriptConsoleMachine::execString(asIScriptEngine *engine,
                                      const QString &code) {
    auto mod = engine->GetModule("Console", asGM_CREATE_IF_NOT_EXISTS);
    if (code.startsWith(QStringLiteral("addvar "))) {
        auto varcmd = code.mid(7) + QStringLiteral(";");
        auto ret = mod->CompileGlobalVar("addvar", varcmd.toUtf8(), 0) >= 0;
        _model->updateData();
        return ret;
    } else if (code.startsWith(QStringLiteral("addfn "))) {
        auto fncmd = code.mid(6);
        asIScriptFunction *func = nullptr;
        int r = mod->CompileFunction("addfn", fncmd.toUtf8(), 0,
                                     asCOMP_ADD_TO_MODULE, &func);
        if (r < 0) {
            return false;
        } else {
            // The script engine supports function overloads, but to simplify
            // the console we'll disallow multiple functions with the same name.
            // We know the function was added, so if GetFunctionByName() fails
            // it is because there already was another function with the same
            // name.
            if (mod->GetFunctionByName(func->GetName()) == 0) {
                mod->RemoveFunction(func);
                return false;
            }
        }

        // We must release the function object
        if (func) {
            func->Release();
        }

        return true;
    } else if (code.startsWith(QStringLiteral("rmvar "))) {
        if (mod == nullptr || mod->GetGlobalVarCount() == 0) {
            return false;
        }

        // remove the tailing semi-colons
        static QRegularExpression re(QStringLiteral(";+$"));

        auto varcmd = code.mid(6).remove(re);
        for (auto &var : varcmd.split(' ', Qt::SkipEmptyParts)) {
            int index = mod->GetGlobalVarIndexByName(var.toUtf8());
            if (index >= 0) {
                mod->RemoveGlobalVar(index);
            }
        }
        _model->updateData();
        return true;
    } else if (code.startsWith(QStringLiteral("rmfn "))) {
        if (mod == nullptr || mod->GetFunctionCount() == 0) {
            return false;
        }

        // remove the tailing semi-colons
        static QRegularExpression re(QStringLiteral(";+$"));

        auto fncmd = code.mid(5).remove(re);
        for (auto &fn : fncmd.split(' ', Qt::SkipEmptyParts)) {
            asIScriptFunction *func = mod->GetFunctionByName(fn.toUtf8());
            if (func) {
                mod->RemoveFunction(func);
            }
        }
        engine->GarbageCollect();

        return true;
    } else if (code.startsWith(QStringLiteral("lsfn"))) {
        if (code.trimmed().length() != 4) {
            return false;
        }

        QString str;
        QTextStream s(&str);
        asUINT n;

        // List the application registered functions
        s << tr("Application functions:") << Qt::endl;
        for (n = 0; n < engine->GetGlobalFunctionCount(); n++) {
            asIScriptFunction *func = engine->GetGlobalFunctionByIndex(n);

            // Skip the functions that start with _ as these are not meant to be
            // called explicitly by the user
            if (func->GetName()[0] != '_')
                s << QStringLiteral(" ") << func->GetDeclaration() << Qt::endl;
        }

        // List the user functions in the module
        if (mod) {
            s << Qt::endl;
            s << tr("User functions:") << Qt::endl;
            for (n = 0; n < mod->GetFunctionCount(); n++) {
                asIScriptFunction *func = mod->GetFunctionByIndex(n);
                s << QStringLiteral(" ") << func->GetDeclaration() << Qt::endl;
            }
        }

        MessageInfo info;
        info.message = str;
        emit onOutput(MessageType::Info, info);
        return true;
    } else if (code.startsWith(QStringLiteral("lsvar"))) {
        if (code.trimmed().length() != 5) {
            return false;
        }

        QString str;
        QTextStream s(&str);
        asUINT n;

        // List the application registered variables
        s << tr("Application variables:") << Qt::endl;
        for (n = 0; n < engine->GetGlobalPropertyCount(); n++) {
            const char *name;
            int typeId;
            bool isConst;
            engine->GetGlobalPropertyByIndex(n, &name, 0, &typeId, &isConst);
            auto decl =
                isConst ? QStringLiteral(" const ") : QStringLiteral(" ");
            decl += engine->GetTypeDeclaration(typeId);
            decl += QStringLiteral(" ");
            decl += name;
            s << decl << Qt::endl;
        }

        // List the user variables in the module
        if (mod) {
            s << Qt::endl;
            s << tr("User variables:") << Qt::endl;
            for (n = 0; n < (asUINT)mod->GetGlobalVarCount(); n++) {
                s << QStringLiteral(" ") << mod->GetGlobalVarDeclaration(n)
                  << Qt::endl;
            }
        }

        MessageInfo info;
        info.message = str;
        emit onOutput(MessageType::Info, info);
        return true;
    } else {
        return ExecuteString(engine, code.toUtf8(), mod, immediateContext()) >=
               0;
    }
}

int ScriptConsoleMachine::ExecuteString(asIScriptEngine *engine,
                                        const char *code, asIScriptModule *mod,
                                        asIScriptContext *ctx) {
    return ExecuteString(engine, code, 0, asTYPEID_VOID, mod, ctx);
}

ScriptObjModel *ScriptConsoleMachine::model() const { return _model; }

int ScriptConsoleMachine::ExecuteString(asIScriptEngine *engine,
                                        const char *code, void *ref,
                                        int refTypeId, asIScriptModule *mod,
                                        asIScriptContext *ctx) {
    // Wrap the code in a function so that it can be compiled and executed
    std::string funcCode = " ExecuteString() {\n";
    funcCode += code;
    funcCode += "\n}";

    // Determine the return type based on the type of the ref arg
    funcCode = engine->GetTypeDeclaration(refTypeId, true) + funcCode;

    // GetModule will free unused types, so to be on the safe side we'll
    // hold on to a reference to the type
    asITypeInfo *type = 0;
    if (refTypeId & asTYPEID_MASK_OBJECT) {
        type = engine->GetTypeInfoById(refTypeId);
        if (type)
            type->AddRef();
    }

    // If no module was provided, get a dummy from the engine
    asIScriptModule *execMod =
        mod ? mod : engine->GetModule("ExecuteString", asGM_ALWAYS_CREATE);

    // Now it's ok to release the type
    if (type)
        type->Release();

    // Compile the function that can be executed
    asIScriptFunction *func = 0;
    int r = execMod->CompileFunction("ExecuteString", funcCode.c_str(), -1, 0,
                                     &func);
    if (r < 0)
        return r;

    // If no context was provided, request a new one from the engine
    asIScriptContext *execCtx = ctx ? ctx : engine->RequestContext();
    r = execCtx->Prepare(func);
    if (r >= 0) {
        // Execute the function
        r = execCtx->Execute();

        // Unless the provided type was void retrieve it's value
        if (ref != 0 && refTypeId != asTYPEID_VOID) {
            if (refTypeId & asTYPEID_OBJHANDLE) {
                // Expect the pointer to be null to start with
                assert(*reinterpret_cast<void **>(ref) == 0);
                *reinterpret_cast<void **>(ref) = *reinterpret_cast<void **>(
                    execCtx->GetAddressOfReturnValue());
                engine->AddRefScriptObject(*reinterpret_cast<void **>(ref),
                                           engine->GetTypeInfoById(refTypeId));
            } else if (refTypeId & asTYPEID_MASK_OBJECT) {
                // Use the registered assignment operator to do a value
                // assign. This assumes that the ref is pointing to a valid
                // object instance.
                engine->AssignScriptObject(ref,
                                           execCtx->GetAddressOfReturnValue(),
                                           engine->GetTypeInfoById(refTypeId));
            } else {
                // Copy the primitive value
                memcpy(ref, execCtx->GetAddressOfReturnValue(),
                       engine->GetSizeOfPrimitiveType(refTypeId));
            }
        }
    }

    // Clean up
    func->Release();
    if (!ctx)
        engine->ReturnContext(execCtx);

    return r;
}
