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

#ifndef ScriptingConsole_H
#define ScriptingConsole_H

#include "class/asconsolecompletion.h"
#include "class/scriptmachine.h"
#include "scriptingconsolebase.h"

#include <QMutex>

class ScriptingConsole : public ScriptingConsoleBase {
    Q_OBJECT

public:
    explicit ScriptingConsole(QWidget *parent = nullptr);

    virtual ~ScriptingConsole();

    QString currentCodes() const;

signals:
    void onFunctionTip(const QString &tip);

public:
    QString getInput();

public slots:
    void init();

    void clearConsole();

    void processKeyEvent(QKeyEvent *e);

    void onOutput(const ScriptMachine::MessageInfo &message);

private slots:
    void applyScriptSettings();

private:
    void runConsoleCommand(const QString &code);

    QString packUpLoggingStr(const QString &message);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    void handleReturnKey() override;
    void keyPressEvent(QKeyEvent *e) override;

protected slots:
    virtual void onCompletion(const QModelIndex &index) override;

private:
    QString _codes;

    std::function<QString(void)> _getInputFn;
};

#endif
