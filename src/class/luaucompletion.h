/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
** FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
** for more details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#ifndef LUAUCOMPLETION_H
#define LUAUCOMPLETION_H

#include "WingCodeEdit/wingcompleter.h"
#include "class/lspeditorinterface.h"
#include "class/resettabletimer.h"

class LuauCompletion : public WingCompleter {
    Q_OBJECT

public:
    explicit LuauCompletion(LspEditorInterace *inter, WingCodeEdit *parent);
    ~LuauCompletion() override = default;

public:
    virtual QString wordSeperators() const override;
    void clearFunctionTip();

protected:
    bool processTrigger(const QString &trigger,
                        const QString &content) override;

public:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onCodeComplete();
    void onActivatedCodeComplete(const QModelIndex &index);

private:
    ResettableTimer *_timer = nullptr;
    LspEditorInterace *_inter = nullptr;
    bool _ok = true;
};

#endif // LUAUCOMPLETION_H
