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

#ifndef ASIDBTREEVIEW_H
#define ASIDBTREEVIEW_H

#include "as-debugger/as_debugger.h"
#include "model/asidbtreemodel.h"

#include <QTreeView>

class asIDBTreeView : public QTreeView {
    Q_OBJECT
public:
    explicit asIDBTreeView(QWidget *parent = nullptr);
    explicit asIDBTreeView(AsIDBTreeModel *model, QWidget *parent = nullptr);

public:
    void refreshWithNewRoots(const QVector<asIDBVariable::Ptr> &newRoots);
    void refreshWithNewRoot(const asIDBVariable::Ptr &newRoot);

public slots:
    void setModel(QAbstractItemModel *model) override;

private:
    void saveExpansionState();

    void saveExpansionStateRecursive(const QModelIndex &parent);

    void restoreExpansionState();

    void restoreExpansionStateRecursive(const QModelIndex &parent);

    QString getIndexIdentifier(const QModelIndex &index) const;

private:
    QSet<QString> m_expansionState;

    asIDBVariable::Ptr _newRoot; // hold the reference
};

#endif // ASIDBTREEVIEW_H
