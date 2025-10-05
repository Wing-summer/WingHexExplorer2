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
#include "class/lspeditorinterface.h"
#include "class/scriptmachine.h"
#include "scriptingconsolebase.h"

#include <QMutex>

class ScriptingConsole : public ScriptingConsoleBase, public LspEditorInterace {
    Q_OBJECT

public:
    explicit ScriptingConsole(QWidget *parent = nullptr);

    virtual ~ScriptingConsole();

    QString currentCodes() const;

public:
    void enableLSP();
    quint64 getVersion() const;

public:
    QString getInput();

    bool isTerminal() const;
    void setIsTerminal(bool newIsTerminal);

public:
    virtual QString lspFileNameURL() const override;
    virtual bool isContentLspUpdated() const override;
    virtual CursorPos currentPosition() const override;
    virtual void showFunctionTip(
        const QList<WingSignatureTooltip::Signature> &sigs) override;
    virtual void clearFunctionTip() override;
    virtual void sendDocChange() override;

private:
    static QString lspURL();

    void setEditMode(ConsoleMode mode);

public slots:
    void init();

    void clearConsole();

    void processKeyEvent(QKeyEvent *e);

    void onOutput(const ScriptMachine::MessageInfo &message);

    void abortCurrentCode();

private slots:
    void applyScriptSettings();
    void onSendFullTextChangeCompleted();

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

protected slots:
    virtual void onCompletion(const QModelIndex &index) override;
    virtual void paste() override;

private:
    bool increaseVersion();

private:
    QString _codes;

    quint64 version = 1;

    ResettableTimer *_timer;
    bool _ok = true;
    bool _lastSent = true;

    bool _isTerminal = true;
    bool _isWaitingRead = false;
    std::function<QString(void)> _getInputFn;
};

#endif
