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

#include "codeedit.h"
#include "model/codecompletionmodel.h"

#include <QModelIndex>
#include <QShortcut>

CodeEdit::CodeEdit(QWidget *parent) : WingCodeEdit(parent) {
    setAutoIndent(true);
    setAutoCloseChar(true);
    setMatchBraces(true);
    setShowIndentGuides(true);
    setShowLineNumbers(true);
    setShowFolding(true);
    setShowSymbolMark(true);

    connect(this->document(), &QTextDocument::modificationChanged, this,
            &CodeEdit::contentModified);

    addMoveLineShortCut();
}

void CodeEdit::onCompletion(const QModelIndex &index) {
    WingCodeEdit::onCompletion(index);
    auto selfdata = index.data(Qt::SelfDataRole).value<CodeInfoTip>();
    if (selfdata.type == CodeInfoTip::Type::Function ||
        selfdata.type == CodeInfoTip::Type::ClsFunction) {
        auto args = selfdata.addinfo.value(CodeInfoTip::Args);
        auto cursor = textCursor();
        cursor.insertText(QStringLiteral("()"));
        if (!args.isEmpty()) {
            cursor.movePosition(QTextCursor::Left);
            setTextCursor(cursor);
        }
    }
}

void CodeEdit::addMoveLineShortCut() {
    auto upLines = new QShortcut(
        QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_Up), this);
    upLines->setContext(Qt::WidgetShortcut);
    connect(upLines, &QShortcut::activated, this,
            [this]() { moveLines(QTextCursor::PreviousBlock); });
    auto downLines = new QShortcut(
        QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_Down),
        this);
    downLines->setContext(Qt::WidgetShortcut);
    connect(downLines, &QShortcut::activated, this,
            [this]() { moveLines(QTextCursor::NextBlock); });
}
