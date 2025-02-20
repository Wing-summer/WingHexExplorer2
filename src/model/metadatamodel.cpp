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

#include "metadatamodel.h"

MetaDataModel::MetaDataModel(QHexDocument *doc, QObject *parent)
    : QAbstractTableModel(parent), _doc(doc) {}

int MetaDataModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return _doc ? _doc->metadata()->size() : 0;
}

int MetaDataModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 5;
}

QVariant MetaDataModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole: {
        auto r = index.row();
        const auto &b = _doc->metadata()->getAllMetadata();
        auto d = b.at(r);
        switch (index.column()) {
        case 0: // begin
            return QString::number(d.begin) + QStringLiteral(" - 0x") +
                   QString::number(d.begin, 16).toUpper();
        case 1: // end
            return QString::number(d.end) + QStringLiteral(" - 0x") +
                   QString::number(d.end, 16).toUpper();
        case 2: {
            if (!d.foreground.isValid()) {
                return QStringLiteral("-");
            }
            return d.foreground.name();
        }
        case 3:
            if (!d.background.isValid()) {
                return QStringLiteral("-");
            }
            return d.background.name();
        case 4:
            return d.comment;
        }
    } break;
    case Qt::DecorationRole: {
        auto r = index.row();
        const auto &b = _doc->metadata()->getAllMetadata();
        auto d = b.at(r);
        switch (index.column()) {
        case 2:
            if (d.foreground.isValid())
                return d.foreground;
            break;
        case 3:
            if (d.background.isValid())
                return d.background;
            break;
        default:
            break;
        }
    } break;
    case Qt::TextAlignmentRole:
        return int(Qt::AlignCenter);
    case Qt::UserRole:
        auto r = index.row();
        const auto &b = _doc->metadata()->getAllMetadata();
        auto d = b.at(r);
        switch (index.column()) {
        case 0: // begin
            return d.begin;
        case 1: // end
            return d.end;
        }
        break;
    }
    return QVariant();
}

QVariant MetaDataModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0: // begin
                return tr("Begin");
            case 1: // end
                return tr("End");
            case 2:
                return tr("Foreground");
            case 3:
                return tr("Background");
            case 4:
                return tr("Comment");
            }
        } else {
            return section + 1;
        }
    }
    return QVariant();
}

void MetaDataModel::setDocument(QHexDocument *newDoc) {
    _doc = newDoc;
    connect(newDoc, &QHexDocument::metaDataChanged, this,
            [=] { emit this->layoutChanged(); });
}
