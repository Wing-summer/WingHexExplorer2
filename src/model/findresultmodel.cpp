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
#include "utilities.h"

FindResultModel::FindResultModel(QObject *parent)
    : QAbstractTableModel(parent), m_encoding(QStringLiteral("ASCII")) {}

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
        case 3: {
            // range
            auto data = m_findData.at(row);
            QString buffer =
                data.cheader.toHex(' ').toUpper() % QStringLiteral(" <b>");
            if (!data.hbuffer.isEmpty()) {
                buffer += data.hbuffer.toHex(' ').toUpper();
                if (!data.tbuffer.isEmpty()) {
                    buffer += QStringLiteral(" .. ");
                }
            }

            buffer += data.tbuffer.toHex(' ').toUpper() %
                      QStringLiteral("</b> ") %
                      data.ctailer.toHex(' ').toUpper();

            return buffer;
        }
        case 4: { // decoding
            auto data = m_findData.at(row);
            QString buffer =
                Utilities::decodingString(data.cheader, m_encoding) %
                QStringLiteral(" <b>");
            if (!data.hbuffer.isEmpty()) {
                buffer += Utilities::decodingString(data.hbuffer);
                if (!data.tbuffer.isEmpty()) {
                    buffer += QStringLiteral(" ... ");
                }
            }

            buffer += Utilities::decodingString(data.tbuffer) %
                      QStringLiteral("</b> ") %
                      Utilities::decodingString(data.ctailer);

            return buffer;
        }
        }
        return QVariant();
    }
    case Qt::TextAlignmentRole:
        switch (index.column()) {
        case 3:
        case 4:
            return Qt::AlignLeft;
        default:
            return Qt::AlignCenter;
        }
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

QString FindResultModel::encoding() const { return m_encoding; }

void FindResultModel::setEncoding(const QString &newEncoding) {
    if (m_encoding != newEncoding) {
        m_encoding = newEncoding;
        emit dataChanged(index(0, 4), index(rowCount(QModelIndex()), 4));
    }
}

QList<WingHex::FindResult> &FindResultModel::results() { return m_results; }

QList<FindResultModel::FindInfo> &FindResultModel::findData() {
    return m_findData;
}

QByteArray &FindResultModel::lastFindData() { return m_lastFindData; }

void FindResultModel::beginUpdate() { this->beginResetModel(); }

void FindResultModel::endUpdate() { this->endResetModel(); }

WingHex::FindResult FindResultModel::resultAt(qsizetype index) const {
    return m_results.at(index);
}

void FindResultModel::clear() {
    m_results.clear();
    m_findData.clear();
}

QList<WingHex::FindResult>::size_type FindResultModel::size() const {
    return m_results.size();
}

QString FindResultModel::copyContent(const QModelIndex &index) const {
    if (index.isValid()) {
        auto row = index.row();
        auto r = m_results.at(row);
        switch (index.column()) {
        case 0: // line
            return QString::number(r.line);
        case 1: // col
            return QString::number(r.col);
        case 2: // offset
            return QStringLiteral("0x") +
                   QString::number(r.offset, 16).toUpper();
        case 3: {
            // range
            auto data = m_findData.at(row);
            QString buffer = data.cheader.toHex(' ').toUpper();
            if (!data.hbuffer.isEmpty()) {
                buffer += data.hbuffer.toHex(' ').toUpper();
                if (!data.tbuffer.isEmpty()) {
                    buffer += QStringLiteral(" .. ");
                }
            }

            buffer += data.tbuffer.toHex(' ').toUpper() %
                      data.ctailer.toHex(' ').toUpper();

            return buffer;
        }
        case 4: { // decoding
            auto data = m_findData.at(row);
            QString buffer =
                Utilities::decodingString(data.cheader, m_encoding);
            if (!data.hbuffer.isEmpty()) {
                buffer += Utilities::decodingString(data.hbuffer);
                if (!data.tbuffer.isEmpty()) {
                    buffer += QStringLiteral(" ... ");
                }
            }

            buffer += Utilities::decodingString(data.tbuffer) %
                      Utilities::decodingString(data.ctailer);

            return buffer;
        }
        }
    }
    return {};
}
