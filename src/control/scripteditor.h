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

signals:
    void onToggleMark(int line);
    void onFunctionTip(const QString &tip);

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
    void processContentsChange(const LSP::TextDocumentContentChangeEvent &e);

private:
    void processTitle();

private:
    CodeEdit *m_editor = nullptr;
    QString m_fileName;
    quint64 version = 1;

    QFileSystemWatcher _watcher;
};

#endif // SCRIPTEDITOR_H
