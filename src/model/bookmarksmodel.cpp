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

#include "bookmarksmodel.h"

BookMarksModel::BookMarksModel(QHexDocument *doc, QObject *parent)
    : QAbstractTableModel(parent), _doc(doc) {}

int BookMarksModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return _doc ? _doc->bookMarksCount() : 0;
}

int BookMarksModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 2;
}

QVariant BookMarksModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole: {
        auto r = index.row();
        auto offset = _doc->bookMarkPos(r);
        switch (index.column()) {
        case 0: // offset
            return offset;
        case 1: // comment
            return _doc->bookMark(offset);
        }
    }
    case Qt::TextAlignmentRole:
        return int(Qt::AlignCenter);
    }
    return QVariant();
}

QVariant BookMarksModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return tr("offset");
            case 1:
                return tr("comment");
            }
        } else {
            return section + 1;
        }
    }
    return QVariant();
}

void BookMarksModel::setDocument(QHexDocument *newDoc) {
    beginResetModel();
    if (_doc) {
        disconnect(_doc);
    }
    _doc = newDoc;
    if (_doc) {
        connect(_doc, &QHexDocument::bookMarkChanged, this,
                [=] { Q_EMIT this->layoutChanged(); });
    }
    endResetModel();
}
