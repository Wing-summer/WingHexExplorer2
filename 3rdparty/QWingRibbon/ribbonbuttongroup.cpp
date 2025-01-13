/*
 * Copyright (C) Martijn Koopman
 * All Rights Reserved
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 */

#include "ribbonbuttongroup.h"
#include "ui_ribbonbuttongroup.h"

#include <QToolButton>

RibbonButtonGroup::RibbonButtonGroup(QWidget *parent)
    : QWidget(parent), ui(new Ui::RibbonButtonGroup), m_title() {
    ui->setupUi(this);
}

RibbonButtonGroup::~RibbonButtonGroup() {
    for (auto &btn : _buttons) {
        ui->horizontalLayout->removeWidget(btn);
        delete btn;
    }
    _buttons.clear();
    delete ui;
}

void RibbonButtonGroup::setTitle(const QString &title) {
    m_title = title;
    ui->label->setText(m_title);
}

QString RibbonButtonGroup::title() const { return m_title; }

int RibbonButtonGroup::buttonCount() const {
    return ui->horizontalLayout->count();
}

void RibbonButtonGroup::addButton(QToolButton *button) {
    if (_buttons.contains(button)) {
        return;
    }

    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    button->setIconSize(QSize(32, 32));
    button->setAutoRaise(true);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    auto txt = button->text();
    auto font = button->font();
    auto metr = QFontMetrics(font);
    auto txtlen = int(metr.horizontalAdvance(txt) + 15);

#if QT_VERSION > QT_VERSION_CHECK(6, 6, 0)
    // Icon size + padding = 32 + 30
    button->setFixedWidth(qMax(32 + 30, txtlen));
#else
    button->setFixedWidth(qMax(32 + 15, txtlen));
#endif

    _buttons << button;
    ui->horizontalLayout->addWidget(button);

    if (_buttons.size() == 1) {
        emit emptyStatusChanged(false);
    }
}

void RibbonButtonGroup::removeButton(QToolButton *button) {
    if (_buttons.contains(button)) {
        ui->horizontalLayout->removeWidget(button);
        _buttons.removeOne(button);
        if (_buttons.empty()) {
            emit emptyStatusChanged(true);
        }
    }
}
