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
#include "class/editorinfo.h"
#include "class/lspeditorinterface.h"
#include "class/resettabletimer.h"
#include "control/codeedit.h"
#include "utilities.h"

#include <QFileSystemWatcher>

class asIScriptEngine;

class ScriptEditor : public ads::CDockWidget,
                     public LspEditorInterace,
                     public EditorInfo {
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr);
    virtual ~ScriptEditor();

    CodeEdit *editor() const;

    bool formatCode();
    bool isModified() const;

    quint64 getVersion() const;

    QString fileName() const;

public:
    static const LinkedList<ScriptEditor *> &instances();

    virtual const WingCodeEdit *editorPtr() const override;
    virtual QString lspFileNameURL() const override;
    virtual bool isContentLspUpdated() const override;
    virtual CursorPos currentPosition() const override;
    virtual void showFunctionTip(
        const QList<WingSignatureTooltip::Signature> &sigs) override;
    virtual void clearFunctionTip() override;
    virtual void sendDocChange() override;

    virtual void saveState(QXmlStreamWriter &Stream) const override;

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

    void onReconnectLsp();
    void setCompleterEnabled(bool b);

private slots:
    void onSendFullTextChangeCompleted();

private:
    void processTitle();

    bool increaseVersion();

public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    inline static LinkedList<ScriptEditor *> m_instances;

    CodeEdit *m_editor = nullptr;
    quint64 version = 1;

    ResettableTimer *_timer;
    bool _ok = true;
    bool _lastSent = true;

    QFileSystemWatcher _watcher;

    // EditorInfo interface
public:
    virtual QIcon editorIcon() const override;
    virtual QString infoFileName() const override;
    virtual QString infoTooltip() const override;
};

#endif // SCRIPTEDITOR_H
