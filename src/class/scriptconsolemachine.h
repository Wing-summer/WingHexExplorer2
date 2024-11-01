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

#ifndef SCRIPTCONSOLEMACHINE_H
#define SCRIPTCONSOLEMACHINE_H

#include "model/scriptobjmodel.h"
#include "scriptmachine.h"

class ScriptConsoleMachine : public ScriptMachine {
    Q_OBJECT
public:
    explicit ScriptConsoleMachine(std::function<QString()> &getInputFn,
                                  QObject *parent = nullptr);
    virtual ~ScriptConsoleMachine();

    virtual bool executeCode(const QString &code) override;

    ScriptObjModel *model() const;

signals:
    void onClearConsole();

protected:
    virtual bool configureEngine(asIScriptEngine *engine) override;

private:
    bool execString(asIScriptEngine *engine, const QString &code);

private:
    ScriptObjModel *_model = nullptr;

    std::function<void(void)> _clsfn;
};

#endif // SCRIPTCONSOLEMACHINE_H
