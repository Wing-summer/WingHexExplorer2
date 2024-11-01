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

#ifndef PLUGINWINDOW_H
#define PLUGINWINDOW_H

#include "framelessdialogbase.h"
#include "plugin/pluginsystem.h"

#include <QListWidget>
#include <QTextBrowser>

class PluginWindow : public FramelessDialogBase {
    Q_OBJECT
public:
    PluginWindow(QWidget *parent = nullptr);
    void setPluginSystem(PluginSystem *pluginsys);
    virtual ~PluginWindow();

private:
    PluginSystem *m_pluginsys;
    QListWidget *plglist;
    QTextBrowser *txtb;
    QTextBrowser *txtc;

    void on_list_selchanged();
};

#endif // PLUGINWINDOW_H
