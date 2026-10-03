/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include <QFileSystemWatcher>

class ScriptEditor final : public ads::CDockWidget,
                           public LspEditorInterace,
                           public EditorInfo {
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr);
    virtual ~ScriptEditor();

    CodeEdit *editor() const;

    bool formatCode();
    bool isModified() const;

    QString fileName() const;

public:
    static const QList<ScriptEditor *> &instances();
    static bool isAllClosed();

    void scrollView(const QPoint &p);
    QPoint scrollViewValue() const;

    void setCursorPos(const QPair<qsizetype, qsizetype> &p);
    QPair<qsizetype, qsizetype> cursorPosValue() const;

public:
    virtual const WingCodeEdit *editorPtr() const override;
    virtual lsp::DocumentUri lspFileNameURL() const override;
    virtual CursorPos currentPosition() const override;
    virtual CursorPos cursorPosition(const QTextCursor &cursor) const override;
    virtual void showFunctionTip(
        const QList<WingSignatureTooltip::Signature> &sigs) override;
    virtual void clearFunctionTip() override;
    virtual void syncDocChange() override;

    virtual void saveState(QXmlStreamWriter &Stream) const override;

    virtual void syncSemanticTokens() override;

protected:
    virtual QVector<lsp::SemanticToken> parseSemanticTokens() override;

signals:
    void onToggleMark(int line);
    void need2Reload();
    void navigateToLocation(const QString &path, int line, int character);

public slots:
    void setReadOnly(bool b);
    bool openFile(const QString &filename);

    bool save(const QString &path = QString());
    bool reload();

    void find();
    void replace();
    void gotoLine();

private:
    void processTitle();

public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    inline static QList<ScriptEditor *> m_instances;

    CodeEdit *m_editor = nullptr;
    ResettableTimer *_tokentimer;
    bool _reloadLater = false;

    QFileSystemWatcher _watcher;

    // EditorInfo interface
public:
    virtual QIcon editorIcon() const override;
    virtual QString infoFileName() const override;
    virtual QString infoTooltip() const override;
    bool reloadLater() const;
    void setReloadLater(bool newReloadLater);

    // QWidget interface
protected:
    virtual void focusInEvent(QFocusEvent *event) override;
};

#endif // SCRIPTEDITOR_H
