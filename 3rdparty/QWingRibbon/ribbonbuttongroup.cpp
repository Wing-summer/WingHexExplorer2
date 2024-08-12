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
    auto font = qApp->font();
    QFontMetrics fm(font);
    auto height = fm.height() * 1.5;
    ui->label->setMinimumHeight(int(height));
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

    button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    auto font = button->font();
    QFontMetrics fm(font);
    auto txtw = fm.horizontalAdvance(button->text());
    auto txth = fm.height();
    button->setIconSize(QSize(32, 32));
    auto height = txth + 32 + 8;
    button->setMinimumSize(std::max(txtw + 8, height), height);
    button->setAutoRaise(true);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

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
