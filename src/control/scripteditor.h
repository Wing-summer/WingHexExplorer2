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

#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include "Qt-Advanced-Docking-System/src/DockWidget.h"
#include "class/lsp.h"
#include "class/resettabletimer.h"
#include "control/codeedit.h"

#include <QFileSystemWatcher>

class asIScriptEngine;

class ScriptEditor : public ads::CDockWidget {
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr);
    virtual ~ScriptEditor();

    QString fileName() const;

    CodeEdit *editor() const;

    bool formatCode();

    quint64 getVersion() const;

    bool isContentLspUpdated() const;

signals:
    void onToggleMark(int line);
    void need2Reload();

public slots:
    void setReadOnly(bool b);
    bool openFile(const QString &filename);

    bool save(const QString &path = QString());
    bool reload();

    void find();
    void replace();
    void gotoLine();

private slots:
    void onFunctionTip(LSP::CompletionItemKind kind, const QString &content);

    void onSendFullTextChangeCompleted();

private:
    void processTitle();

    bool increaseVersion();
    void sendDocChange();

private:
    CodeEdit *m_editor = nullptr;
    quint64 version = 1;

    ResettableTimer *_timer;
    bool _ok = true;
    bool _lastSent = true;

    QFileSystemWatcher _watcher;
};

#endif // SCRIPTEDITOR_H
