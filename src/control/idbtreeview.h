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

#ifndef IDBTREEVIEW_H
#define IDBTREEVIEW_H

#include "model/idbtreemodel.h"

#include <QTreeView>

class IDBTreeView : public QTreeView {
    Q_OBJECT
public:
    explicit IDBTreeView(QWidget *parent = nullptr);
    explicit IDBTreeView(IDBTreeModel *model, QWidget *parent = nullptr);

public:
    IDBTreeModel *dataModel() const;

public slots:
    void setModel(QAbstractItemModel *model) override;
    void refreshData();

private:
    void saveExpansionState();

    void saveExpansionStateRecursive(const QModelIndex &parent);

    void restoreExpansionState();

    void restoreExpansionStateRecursive(const QModelIndex &parent);

    QString getIndexIdentifier(const QModelIndex &index) const;

private:
    QSet<QString> m_expansionState;
};

#endif // IDBTREEVIEW_H
