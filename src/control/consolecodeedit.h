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

#ifndef CONSOLECODEEDIT_H
#define CONSOLECODEEDIT_H

#include "class/lspeditorinterface.h"
#include "class/resettabletimer.h"
#include "codeedit.h"

class ConsoleCodeEdit : public CodeEdit, public LspEditorInterace {
    Q_OBJECT

public:
    explicit ConsoleCodeEdit(QWidget *parent = nullptr);
    virtual ~ConsoleCodeEdit();

    // LspEditorInterace interface
public:
    virtual const WingCodeEdit *editorPtr() const override;
    virtual QString lspFileNameURL() const override;
    virtual bool isContentLspUpdated() const override;
    virtual CursorPos currentPosition() const override;
    virtual void sendDocChange() override;
    virtual void showFunctionTip(
        const QList<WingSignatureTooltip::Signature> &sigs) override;
    virtual void clearFunctionTip() override;

signals:
    void onCloseEvent();

    // QWidget interface
protected:
    virtual void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void onSendFullTextChangeCompleted();

private:
    static QString lspURL();
    quint64 getVersion() const;

    bool increaseVersion();

private:
    quint64 version = 1;

    ResettableTimer *_timer;
    bool _ok = true;
    bool _lastSent = true;
};

#endif // CONSOLECODEEDIT_H
