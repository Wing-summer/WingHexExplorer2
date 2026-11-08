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

#include "diffresultmodel.h"

DiffResultModel::DiffResultModel(QObject *parent)
    : QAbstractTableModel(parent) {}

int DiffResultModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_count;
}

int DiffResultModel::columnCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : ColumnCount;
}

QString DiffResultModel::field(const WingDiff::Range &range, int column) {
    switch (column) {
    case Kind:
        switch (range.type) {
        case WingDiff::Type::Equal:
            return tr("Equal");
        case WingDiff::Type::Insert:
            return tr("Insert");
        case WingDiff::Type::Delete:
            return tr("Delete");
        case WingDiff::Type::Replace:
            return tr("Replace");
        }
        break;
    case LeftOffset:
        return QStringLiteral("0x") +
               QString::number(range.leftOffset, 16).toUpper();
    case RightOffset:
        return QStringLiteral("0x") +
               QString::number(range.rightOffset, 16).toUpper();
    case LeftLength:
        return QString::number(range.leftLength);
    case RightLength:
        return QString::number(range.rightLength);
    case Precision:
        return range.coarse ? tr("Coarse") : tr("Exact");
    default:
        break;
    }
    return {};
}

QVariant DiffResultModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_count || role != Qt::DisplayRole) {
        return {};
    }
    return field(range(index.row()), index.column());
}

QVariant DiffResultModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const {
    if (role != Qt::DisplayRole) {
        return {};
    }
    if (orientation == Qt::Vertical) {
        return globalRow(section) + 1;
    }
    switch (section) {
    case Kind:
        return tr("Type");
    case LeftOffset:
        return tr("Left offset");
    case LeftLength:
        return tr("Left length");
    case RightOffset:
        return tr("Right offset");
    case RightLength:
        return tr("Right length");
    case Precision:
        return tr("Precision");
    default:
        return {};
    }
}

void DiffResultModel::setStore(std::shared_ptr<WingDiff::ResultStore> store) {
    beginResetModel();
    m_store = std::move(store);
    m_count = m_store ? int(m_store->publishedDifferences()) : 0;
    endResetModel();
}

quint64 DiffResultModel::globalRow(int row) const { return quint64(row); }

WingDiff::Range DiffResultModel::range(int row) const {
    WingDiff::Range result;
    if (m_store && row >= 0 && row < m_count) {
        m_store->difference(quint64(row), &result);
    }
    return result;
}

void DiffResultModel::refresh() {
    const int count = m_store ? int(m_store->publishedDifferences()) : 0;
    if (count > m_count) {
        beginInsertRows({}, m_count, count - 1);
        m_count = count;
        endInsertRows();
    }
}
