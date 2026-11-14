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

#ifndef SCRIPTSERVICETRAY_H
#define SCRIPTSERVICETRAY_H

#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>

#include "scriptservice.h"

class ScriptServiceTray final : public QObject {
    Q_OBJECT
public:
    explicit ScriptServiceTray(ScriptService &service,
                               QObject *parent = nullptr);
    virtual ~ScriptServiceTray() override;

private:
    void rebuildMenu();
    void updateToolTip();
    static QString stateText(ScriptService::TaskState state);

private:
    ScriptService &m_service;
    QSystemTrayIcon *m_icon = nullptr;
    QMenu *m_menu = nullptr;
};
#endif // SCRIPTSERVICETRAY_H
