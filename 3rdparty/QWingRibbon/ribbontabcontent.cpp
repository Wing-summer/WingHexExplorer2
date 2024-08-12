/*
 * Copyright (C) Martijn Koopman
 * All Rights Reserved
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 */

#include "ribbontabcontent.h"
#include "ribbonbuttongroup.h"
#include "ui_ribbontabcontent.h"

#include "../src/class/eventfilter.h"

#include <QScrollBar>
#include <QWheelEvent>

RibbonTabContent::RibbonTabContent(QWidget *parent)
    : QWidget(parent), ui(new Ui::RibbonTabContent) {
    ui->setupUi(this);
    setMouseTracking(true);

    ui->ribbonTabScrollArea->verticalScrollBar()->setEnabled(false);
    auto hb = ui->ribbonTabScrollArea->horizontalScrollBar();
    auto view = ui->ribbonTabScrollArea->viewport();
    auto we = new EventFilter(QEvent::Wheel, this);
    connect(we, &EventFilter::eventTriggered, this,
            [hb, view](QObject *obj, QEvent *event) {
                Q_ASSERT(obj == view);
                auto e = static_cast<QWheelEvent *>(event);
                bool horizontal =
                    qAbs(e->angleDelta().x()) > qAbs(e->angleDelta().y());

                if (!horizontal && e->angleDelta().x() != 0) {
                    return;
                }

                int delta = e->angleDelta().y();
                hb->setValue(hb->value() - delta);
            });
    view->installEventFilter(we);
}

RibbonTabContent::~RibbonTabContent() { delete ui; }

RibbonButtonGroup *RibbonTabContent::addGroup(const QString &groupName) {
    RibbonButtonGroup *ribbonButtonGroup = new RibbonButtonGroup;
    ribbonButtonGroup->setTitle(groupName);

    ui->ribbonHorizontalLayout->addWidget(ribbonButtonGroup);

    return ribbonButtonGroup;
}

void RibbonTabContent::removeGroup(const QString &groupName) {
    // Find ribbon group
    for (int i = 0; i < ui->ribbonHorizontalLayout->count(); i++) {
        auto group = static_cast<RibbonButtonGroup *>(
            ui->ribbonHorizontalLayout->itemAt(i)->widget());
        if (group->title() == groupName) {
            ui->ribbonHorizontalLayout->removeWidget(group);
            delete group;
            break;
        }
    }
}

void RibbonTabContent::removeGroup(RibbonButtonGroup *group) {
    auto index = ui->ribbonHorizontalLayout->indexOf(group);
    if (index >= 0) {
        ui->ribbonHorizontalLayout->removeWidget(group);
        delete group;
    }
}

int RibbonTabContent::groupCount() const {
    return ui->ribbonHorizontalLayout->count();
}
