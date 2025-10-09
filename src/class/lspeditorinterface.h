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

#ifndef LSPEDITORINTERFACE_H
#define LSPEDITORINTERFACE_H

#include <QString>
#include <QTextCursor>

#include "WingCodeEdit/wingcodeedit.h"
#include "WingCodeEdit/wingsignaturetooltip.h"

class LspEditorInterace {
public:
    struct CursorPos {
        int blockNumber = -1;
        int positionInBlock = -1;
    };

public:
    virtual const WingCodeEdit *editorPtr() const = 0;

    virtual QString lspFileNameURL() const = 0;
    virtual bool isContentLspUpdated() const = 0;
    virtual CursorPos currentPosition() const = 0;

    virtual void sendDocChange() = 0;

    virtual void
    showFunctionTip(const QList<WingSignatureTooltip::Signature> &sigs) = 0;
    virtual void clearFunctionTip() = 0;
};

#endif // LSPEDITORINTERFACE_H
