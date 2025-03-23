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

#include "gotolinewidget.h"

#include "class/skinmanager.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

GotoLineWidget::GotoLineWidget(QWidget *parent)
    : CodeEditControlWidget(parent) {
    auto &skin = SkinManager::instance();

    auto closeBtn = new QToolButton(this);
    closeBtn->setIcon(skin.themeIcon(QStringLiteral("dialog_close")));
    closeBtn->setToolTip(tr("Close"));
    closeBtn->setIconSize(QSize(12, 12));
    closeBtn->setStyleSheet(QStringLiteral("border:none;"));
    connect(closeBtn, &QToolButton::clicked, this, &GotoLineWidget::close);

    m_sbline = new QSpinBox(this);
    m_sbline->setRange(1, 1);
    m_sbline->setMinimumWidth(120);
    connect(m_sbline, &QSpinBox::valueChanged, this,
            &GotoLineWidget::onGotoLine);

    auto tbGoto = new QToolButton(this);
    tbGoto->setAutoRaise(true);
    tbGoto->setIcon(skin.themeIcon(QStringLiteral("right_arrow")));
    connect(tbGoto, &QToolButton::clicked, this, [this]() {
        emit onGotoLine(m_sbline->value());
        close();
    });

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);
    layout->addWidget(closeBtn);
    layout->addWidget(m_sbline, 1);
    layout->addWidget(tbGoto);

    setLayout(layout);
    setDefaultButton(tbGoto);
}

void GotoLineWidget::setTotalLines(int lineTotal) {
    m_sbline->setMaximum(lineTotal);
}

void GotoLineWidget::setOriginLine(int line) {
    m_sbline->setValue(line);
    m_line = m_sbline->value();
}

void GotoLineWidget::cancel() {
    emit onGotoLine(m_line);
    CodeEditControlWidget::cancel();
}
