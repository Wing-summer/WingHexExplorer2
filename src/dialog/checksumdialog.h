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

#ifndef CHECKSUMDIALOG_H
#define CHECKSUMDIALOG_H

#include "framelessdialogbase.h"

#include <QTreeWidget>

class CheckSumDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    explicit CheckSumDialog(QVector<int> &cs, QWidget *parent = nullptr);

    const QVector<int> &getResults();

private slots:
    void on_accept();
    void on_reject();

    void onItemChanged(QTreeWidgetItem *item, int column);
    void onItemClicked(QTreeWidgetItem *item, int /*column*/);

private:
    static void collectCheckedLeaves(QTreeWidgetItem *item, QList<int> &out);

    void populateData(QVector<int> &cs);

    void updateCheckStatus(QTreeWidgetItem *parent);

private:
    QTreeWidget *hashlist = nullptr;
    QTreeWidgetItem *m_pressedItem;
    QVector<int> _result;
    bool m_updating = false;
};

#endif // CHECKSUMDIALOG_H
