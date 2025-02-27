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

#include "qjsontablemodel.h"
#include <QJsonObject>

QJsonTableModel::QJsonTableModel(const QJsonTableModel::Header &header,
                                 QObject *parent)
    : QAbstractTableModel(parent), m_header(header) {}

bool QJsonTableModel::setJson(const QJsonDocument &json) {
    return setJson(json.array());
}

bool QJsonTableModel::setJson(const QJsonArray &array) {
    beginResetModel();
    m_json = array;
    endResetModel();
    return true;
}

QVariant QJsonTableModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (orientation) {
    case Qt::Horizontal:
        return m_header[section]["title"];
    case Qt::Vertical:
        // return section + 1;
        return QVariant();
    default:
        return QVariant();
    }
}

int QJsonTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_json.size();
}

int QJsonTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_header.size();
}

QJsonObject QJsonTableModel::getJsonObject(const QModelIndex &index) const {
    const QJsonValue &value = m_json[index.row()];
    return value.toObject();
}

QVariant QJsonTableModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole: {
        QJsonObject obj = getJsonObject(index);
        const QString &key = m_header[index.column()]["index"];
        if (obj.contains(key)) {
            QJsonValue v = obj[key];

            if (v.isString()) {
                return v.toString();
            } else if (v.isDouble()) {
                return QString::number(v.toDouble());
            } else {
                return QVariant();
            }
        } else {
            return QVariant();
        }
    }
    case Qt::ToolTipRole:
        return QVariant();
    default:
        return QVariant();
    }
}
