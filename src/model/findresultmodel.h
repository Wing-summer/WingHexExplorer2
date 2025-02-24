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

#ifndef FINDRESULTMODEL_H
#define FINDRESULTMODEL_H

#include "plugin/iwingplugin.h"

#include <QAbstractTableModel>

class FindResultModel : public QAbstractTableModel {
    Q_OBJECT

public:
    struct FindInfo {
        QByteArray cheader;
        QByteArray hbuffer;
        QByteArray tbuffer;
        QByteArray ctailer;
    };

public:
    explicit FindResultModel(QObject *parent = nullptr);

    QList<WingHex::FindResult> &results();
    QList<FindInfo> &findData();
    QString &lastFindData();

    void beginUpdate();
    void endUpdate();

    WingHex::FindResult resultAt(qsizetype index) const;
    void clear();
    QList<WingHex::FindResult>::size_type size() const;

    QString copyContent(const QModelIndex &index) const;

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

    QString encoding() const;
    void setEncoding(const QString &newEncoding);

private:
    QList<WingHex::FindResult> m_results;
    QList<FindInfo> m_findData;
    QString m_lastFindData;

    QString m_encoding;
};

#endif // FINDRESULTMODEL_H
