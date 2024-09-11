#include "scriptconsolemachine.h"

#include "AngelScript/sdk/add_on/scripthelper/scripthelper.h"
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

ScriptObjModel *ScriptConsoleMachine::model() const { return _model; }
