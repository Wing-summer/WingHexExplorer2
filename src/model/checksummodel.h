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

#ifndef CHECKSUMMODEL_H
#define CHECKSUMMODEL_H

#include <QAbstractTableModel>

#include "class/cryptographichash.h"

class CheckSumModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit CheckSumModel(QObject *parent = nullptr);

    QString checkSumData(CryptographicHash::Algorithm algorithm) const;

    void
    updateCheckSumData(const QMap<CryptographicHash::Algorithm, QString> &data);

    void clearData();

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

private:
    QMap<CryptographicHash::Algorithm, QString> _checkSumData;
};

#endif // CHECKSUMMODEL_H
