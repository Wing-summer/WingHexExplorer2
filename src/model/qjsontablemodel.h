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

#ifndef QJSONTABLEMODEL_H
#define QJSONTABLEMODEL_H

#include <QAbstractTableModel>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QVector>

class QJsonTableModel : public QAbstractTableModel {
public:
    typedef QHash<QString, QString> Heading;
    typedef QVector<Heading> Header;
    QJsonTableModel(const Header &header, QObject *parent = 0);

    bool setJson(const QJsonDocument &json);
    bool setJson(const QJsonArray &array);

    virtual QJsonObject getJsonObject(const QModelIndex &index) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index,
                          int role = Qt::DisplayRole) const;

private:
    Header m_header;
    QJsonArray m_json;
};

#endif // QJSONTABLEMODEL_H
