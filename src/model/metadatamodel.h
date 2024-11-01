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

#ifndef METADATAMODEL_H
#define METADATAMODEL_H

#include "QHexView/document/qhexdocument.h"
#include <QAbstractTableModel>
#include <QObject>

class MetaDataModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit MetaDataModel(QHexDocument *doc, QObject *parent = nullptr);

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

public slots:
    void setDocument(QHexDocument *newDoc);

private:
    QHexDocument *_doc = nullptr;
};

#endif // METADATAMODEL_H
