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

#ifndef DIFFRESULTMODEL_H
#define DIFFRESULTMODEL_H

#include "class/diff/binarydiff.h"

#include <QAbstractTableModel>

class DiffResultModel final : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        Kind,
        LeftOffset,
        LeftLength,
        RightOffset,
        RightLength,
        Precision,
        ColumnCount
    };

public:
    explicit DiffResultModel(QObject *parent = nullptr);

public:
    virtual int rowCount(const QModelIndex &parent = {}) const override;
    virtual int columnCount(const QModelIndex &parent = {}) const override;
    virtual QVariant data(const QModelIndex &index,
                          int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

public:
    void setStore(std::shared_ptr<WingDiff::ResultStore> store);
    quint64 globalRow(int row) const;
    WingDiff::Range range(int row) const;
    void refresh();
    static QString field(const WingDiff::Range &range, int column);

private:
    std::shared_ptr<WingDiff::ResultStore> m_store;
    int m_count = 0;
};
#endif
