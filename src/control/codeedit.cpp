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
#include "qlinemarkpanel.h"
#include "qlinenumberpanel.h"
#include "qpanellayout.h"

#include "qeditor.h"

CodeEdit::CodeEdit(bool haslineMark, QWidget *parent, bool actions)
    : QObject(parent), QCodeEdit(actions, parent) {
    auto l = this->panelLayout();
    l->setSizeConstraint(QLayout::SetMinimumSize);

    auto editor = this->editor();
    if (haslineMark) {
        auto lineMark = new QLineMarkPanel(editor);
        this->addPanel(lineMark, QCodeEdit::West);
        connect(lineMark, &QLineMarkPanel::onToggleMark, this,
                &CodeEdit::onToggleMark);
    }

    auto lineNum = new QLineNumberPanel(editor);
    lineNum->setVerboseMode(true);
    this->addPanel(lineNum, QCodeEdit::West, true);
    connect(editor, &QEditor::cursorPositionChanged, lineNum,
            QOverload<>::of(&QLineMarkPanel::update));

    this->addPanel(QStringLiteral("Fold Panel"), QCodeEdit::West, true);
    this->addPanel(QStringLiteral("Line Change Panel"), QCodeEdit::West, true);
    this->addPanel(QStringLiteral("Goto Line Panel"), QCodeEdit::South);
    this->addPanel(QStringLiteral("Search Replace Panel"), QCodeEdit::South);
}
