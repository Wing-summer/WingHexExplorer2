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

ScriptObjModel *ScriptConsoleMachine::model() const { return _model; }

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
        if (varcmd.length() == 1) {
            MessageInfo info;
            info.message = tr("EmptyDecl");
            emit onOutput(MessageType::Error, info);
            return false;
        }
        auto ret = mod->CompileGlobalVar("addvar", varcmd.toUtf8(), 0) >= 0;
        _model->updateData();
        return ret;
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
        return ScriptMachine::executeCode(code);
    }
}
