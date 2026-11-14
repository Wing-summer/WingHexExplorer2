/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include "scriptservicetray.h"

#include <QApplication>

#include "scriptservice.h"

ScriptServiceTray::ScriptServiceTray(ScriptService &service, QObject *parent)
    : QObject(parent), m_service(service) {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }
    m_icon = new QSystemTrayIcon(
        QIcon(
            QStringLiteral(":/com.wingsummer.winghex/images/application.svg")),
        this);
    m_menu = new QMenu;
    m_icon->setContextMenu(m_menu);
    connect(&service, &ScriptService::sigOnTasksChanged, this,
            &ScriptServiceTray::updateToolTip);
    updateToolTip();
    connect(m_menu, &QMenu::aboutToShow, this, &ScriptServiceTray::rebuildMenu);
    rebuildMenu();
    m_icon->show();
}

ScriptServiceTray::~ScriptServiceTray() {
    delete m_icon;
    delete m_menu;
}

void ScriptServiceTray::rebuildMenu() {
    m_menu->clear();
    m_menu->addAction(m_service.endpoint())->setEnabled(false);
    m_menu->addSeparator();
    const auto tasks = m_service.taskSummaries();
    if (tasks.isEmpty()) {
        m_menu->addAction(tr("No script tasks"))->setEnabled(false);
    }
    for (const auto &task : tasks) {
        const auto label = tr("%1 — %2").arg(task.id, stateText(task.state));
        auto submenu = m_menu->addMenu(label);
        submenu->addAction(task.path)->setEnabled(false);
        auto stop = submenu->addAction(tr("Stop task"));
        using TaskState = ScriptService::TaskState;
        stop->setEnabled(task.state != TaskState::Stopped &&
                         task.state != TaskState::Failed &&
                         task.state != TaskState::Stopping);
        connect(stop, &QAction::triggered, this,
                [this, id = task.id] { m_service.stopTask(id); });
    }
    m_menu->addSeparator();
    connect(m_menu->addAction(tr("Force shutdown")), &QAction::triggered,
            &m_service, &ScriptService::shutdown);
}

QString ScriptServiceTray::stateText(ScriptService::TaskState state) {
    using TaskState = ScriptService::TaskState;
    switch (state) {
    case TaskState::Queued:
        return tr("Queued");
    case TaskState::Running:
        return tr("Running");
    case TaskState::Resident:
        return tr("Resident");
    case TaskState::Stopping:
        return tr("Stopping");
    case TaskState::Stopped:
        return tr("Stopped");
    case TaskState::Failed:
        return tr("Failed");
    }
    Q_UNREACHABLE();
}

void ScriptServiceTray::updateToolTip() {
    if (!m_icon) {
        return;
    }
    m_icon->setToolTip(tr("%1\n%2\nActive tasks: %3")
                           .arg(qAppName(), m_service.endpoint())
                           .arg(m_service.activeTasks()));
}
