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

#ifndef DOCKWIDGETTAB_H
#define DOCKWIDGETTAB_H

#include "Qt-Advanced-Docking-System/src/DockWidgetTab.h"

class DockWidgetTab : public ads::CDockWidgetTab {
    Q_OBJECT

public:
    DockWidgetTab(ads::CDockWidget *DockWidget, QWidget *parent = nullptr);

    // CDockAreaTitleBar interface
public:
    virtual QMenu *buildContextMenu(QMenu *menu) override;

private:
    void initMenuItems(QMenu *menu, const QUrl &path);
};

#endif // DOCKWIDGETTAB_H
