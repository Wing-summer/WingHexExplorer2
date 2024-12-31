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

#include "findresultmodel.h"

FindResultModel::FindResultModel(QObject *parent)
    : QAbstractTableModel(parent) {}

int FindResultModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_results.size();
}

int FindResultModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 5;
}

QVariant FindResultModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole: {
        auto row = index.row();
        auto r = m_results.at(row);
        switch (index.column()) {
        case 0: // line
            return r.line;
        case 1: // col
            return r.col;
        case 2: // offset
            return QStringLiteral("0x") +
                   QString::number(r.offset, 16).toUpper();
        case 3: // range
            return m_lastFindData.at(row).findRange;
        case 4: // decoding
            return m_lastFindData.at(row).decoding;
        }
        return QVariant();
    }
    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
    }
    return QVariant();
}

QVariant FindResultModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return tr("line");
            case 1:
                return tr("col");
            case 2:
                return tr("offset");
            case 3:
                return tr("value");
            case 4:
                return tr("encoding");
            }
        } else {
            return section + 1;
        }
    }
    return QVariant();
}

QList<WingHex::FindResult> &FindResultModel::results() { return m_results; }

QList<FindResultModel::FindInfo> &FindResultModel::lastFindData() {
    return m_lastFindData;
}

void FindResultModel::beginUpdate() { this->beginResetModel(); }

void FindResultModel::endUpdate() { this->endResetModel(); }

WingHex::FindResult FindResultModel::resultAt(qsizetype index) const {
    return m_results.at(index);
}

void FindResultModel::clear() {
    m_results.clear();
    m_lastFindData.clear();
}

QList<WingHex::FindResult>::size_type FindResultModel::size() const {
    return m_results.size();
}
