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

#ifndef ScriptingConsole_H
#define ScriptingConsole_H

#include "class/lspeditorinterface.h"
#include "class/scriptmachine.h"
#include "scriptingconsolebase.h"

#include <QMutex>
#include <QTextDocument>

class ScriptingConsole : public ScriptingConsoleBase, public LspEditorInterace {
    Q_OBJECT

public:
    explicit ScriptingConsole(QWidget *parent = nullptr);

    virtual ~ScriptingConsole();

    QString currentCodes() const;

public:
    void enableLSP();

public:
    QString getInput();

    bool isTerminal() const;
    void setIsTerminal(bool newIsTerminal);

    QList<QTextBlock> visibleTextBlocks() const;

public:
    virtual const WingCodeEdit *editorPtr() const override;
    virtual lsp::DocumentUri lspFileNameURL() const override;
    virtual CursorPos currentPosition() const override;
    virtual CursorPos cursorPosition(const QTextCursor &cursor) const override;
    virtual void showFunctionTip(
        const QList<WingSignatureTooltip::Signature> &sigs) override;
    virtual void clearFunctionTip() override;
    virtual void syncDocChange() override;

    virtual void syncSemanticTokens() override;

protected:
    virtual QVector<lsp::SemanticToken> parseSemanticTokens() override;

private:
    static lsp::DocumentUri lspURL();

    void setEditMode(ConsoleMode mode);

public slots:
    void init();

    void clearConsole();

    void clear();

    void processKeyEvent(QKeyEvent *e);

    void onOutput(const ScriptMachine::MessageInfo &message);

    void abortCurrentCode();

private slots:
    void applyScriptSettings();

signals:
    void consoleScriptRunFinished();

private:
    void runConsoleCommand(const QString &code);

    QString packUpLoggingStr(const QString &message);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void handleReturnKey(Qt::KeyboardModifiers mod) override;
    virtual void keyPressEvent(QKeyEvent *e) override;
    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void wheelEvent(QWheelEvent *e) override;
    virtual bool event(QEvent *event) override;

protected slots:
    virtual void onCompletion(const QModelIndex &index) override;
    virtual void paste() override;

private:
    QStringList _codes;
    QTextDocument _lspDocument;

    bool _isTerminal = true;
    bool _isWaitingRead = false;
    ScriptMachine::MessageType _lastOutputType =
        ScriptMachine::MessageType::Unknown;
    std::function<QString(void)> _getInputFn;
};

#endif
