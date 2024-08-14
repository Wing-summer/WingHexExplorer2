/*
 * Copyright (C) Martijn Koopman
 * All Rights Reserved
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 */

#include "ribbon.h"
#include "ribbontabcontent.h"

#include <QApplication>
#include <QLayout>
#include <QStackedWidget>
#include <QStyleOption>
#include <QToolButton>

Ribbon::Ribbon(QWidget *parent) : QTabWidget(parent) {
    auto w = new QWidget(this);
    auto layout = new QHBoxLayout(w);
    setObjectName(QStringLiteral("RIBBON"));
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    auto hideBtn = new QToolButton(this);
    hideBtn->setObjectName(QStringLiteral("RIBBON_HIDE_BTN"));
    hideBtn->setCheckable(true);
    hideBtn->setChecked(false);

    layout->setSpacing(5);
    layout->addWidget(hideBtn);

    m_conerLayout = layout;

    connect(hideBtn, &QToolButton::toggled, this, [this](bool checked) {
        auto *tab_pane = this->findChild<QStackedWidget *>();
        Q_ASSERT(tab_pane);
        tab_pane->setVisible(!checked);
        if (checked) {
            this->setMaximumHeight(this->tabBar()->height());
        } else {
            this->setMaximumHeight(QWIDGETSIZE_MAX);
        }
    });

    this->setCornerWidget(w);
}

RibbonTabContent *Ribbon::addTab(const QString &tabName) {
    // Note: superclass QTabWidget also has a function addTab()
    RibbonTabContent *ribbonTabContent = new RibbonTabContent;
    QTabWidget::addTab(ribbonTabContent, tabName);
    return ribbonTabContent;
}

RibbonTabContent *Ribbon::addTab(const QIcon &tabIcon, const QString &tabName) {
    // Note: superclass QTabWidget also has a function addTab()
    RibbonTabContent *ribbonTabContent = new RibbonTabContent;
    QTabWidget::addTab(ribbonTabContent, tabIcon, tabName);
    return ribbonTabContent;
}

void Ribbon::removeTab(const QString &tabName) {
    // Find ribbon tab
    for (int i = 0; i < count(); i++) {
        if (tabText(i) == tabName) {
            // Remove tab
            QWidget *tab = QTabWidget::widget(i);
            QTabWidget::removeTab(i);
            delete tab;
            break;
        }
    }
}

void Ribbon::removeTab(RibbonTabContent *tabContent) {
    if (tabContent == nullptr) {
        return;
    }

    auto index = QTabWidget::indexOf(tabContent);
    if (index >= 0) {
        QTabWidget::removeTab(index);
        delete tabContent;
    }
}

QHBoxLayout *Ribbon::conerLayout() const { return m_conerLayout; }
