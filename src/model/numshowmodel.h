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

#ifndef NUMSHOWMODEL_H
#define NUMSHOWMODEL_H

#include <QAbstractTableModel>

class NumShowModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum NumTableIndex {
        Byte,
        Char,
        Ushort,
        Short,
        Uint32,
        Int32,
        Uint64,
        Int64,
        Float32,
        Double64
    };

public:
    explicit NumShowModel(QObject *parent = nullptr);

    void setNumData(NumTableIndex index, const QString &data);
    QString numData(NumTableIndex index) const;
    void clear();

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

private:
    QStringList m_num;
};

#endif // NUMSHOWMODEL_H
