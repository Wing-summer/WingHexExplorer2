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

#ifndef DRIVERSELECTORDIALOG_H
#define DRIVERSELECTORDIALOG_H

#include "framelessdialogbase.h"

#include <QDialog>
#include <QList>
#include <QListWidget>
#include <QMainWindow>
#include <QObject>
#include <QStorageInfo>
#include <QTextBrowser>

class DriverSelectorDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    DriverSelectorDialog(QWidget *parent = nullptr);
    QStorageInfo GetResult();

private:
    QListWidget *drivers;
    QTextBrowser *infob;
    QList<QStorageInfo> m_infos;
    QStorageInfo m_si;

    void on_list_selectionChanged();

    void on_accepted();
    void on_rejected();
};

#endif // DRIVERSELECTORDIALOG_H
