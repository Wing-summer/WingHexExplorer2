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

#ifndef DBGCALLSTACKMODEL_H
#define DBGCALLSTACKMODEL_H

#include <QAbstractTableModel>

#include "class/asdebugger.h"

class DbgCallStackModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit DbgCallStackModel(QObject *parent = nullptr);

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

public slots:
    void updateData(const QList<asDebugger::CallStackItem> &callstack);

private:
    QList<asDebugger::CallStackItem> _stack;
};

#endif // DBGCALLSTACKMODEL_H
