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

#include "QConsoleWidget/QConsoleWidget.h"
#include "class/scriptconsolemachine.h"

#include <QMutex>

class ScriptingConsole : public QConsoleWidget {
    Q_OBJECT

public:
    explicit ScriptingConsole(QWidget *parent = nullptr);

    virtual ~ScriptingConsole();

    ScriptMachine *machine() const;
    ScriptConsoleMachine *consoleMachine() const;

    //! Appends a newline and command prompt at the end of the document.
    void appendCommandPrompt(bool storeOnly = false);

public slots:
    void newLine();

    void init();

    void initOutput();

    void clearConsole();

    void pushInputCmd(const QString &cmd);

    void processKeyEvent(QKeyEvent *e);

private:
    void runConsoleCommand(const QString &code);

    QString getInput();

    QString packUpLoggingStr(const QString &message);

protected:
    void keyPressEvent(QKeyEvent *e) override;

private:
    ScriptConsoleMachine *_sp = nullptr;
    QTextStream _s;

    bool _lastCommandPrompt = false;
    QString _codes;

    QStringList _cmdQueue;
    QMutex _queueLocker;
    bool _waitforRead = false;

    QTextCharFormat _warnCharFmt;
    std::function<QString(void)> _getInputFn;
};

#endif
