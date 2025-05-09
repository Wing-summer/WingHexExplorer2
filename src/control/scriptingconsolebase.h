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

#ifndef SCRIPTINGCONSOLEBASE_H
#define SCRIPTINGCONSOLEBASE_H

#include "QConsoleWidget/QConsoleWidget.h"

class ScriptingConsoleBase : public QConsoleWidget {
    Q_OBJECT
public:
    explicit ScriptingConsoleBase(QWidget *parent = nullptr);

public:
    QTextStream &consoleStream();

    bool lastCommandPrompt() const;

public slots:
    void stdOut(const QString &str);
    void stdErr(const QString &str);
    void stdWarn(const QString &str);
    void newLine();
    void flush();

    void initOutput();

    //! Appends a newline and command prompt at the end of the document.
    void appendCommandPrompt(bool storeOnly = false);

protected:
    void dontHighlightLastLine(bool followTheme);

    void dontHighlightLastOffset(int offset, bool followTheme = true);

private:
    QTextStream _s;
    QTextCharFormat _warnCharFmt;

    bool _lastCommandPrompt = false;
};

#endif // SCRIPTINGCONSOLEBASE_H
