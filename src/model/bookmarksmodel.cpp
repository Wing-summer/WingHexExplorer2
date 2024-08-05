#include "bookmarksmodel.h"

BookMarksModel::BookMarksModel(QList<BookMarkStruct> &db, QObject *parent)
    : QAbstractTableModel(parent), _bookmarks(db) {}

void BookMarksModel::clear() {
    beginResetModel();
    _bookmarks.clear();
    endResetModel();
}

void BookMarksModel::updateAll() {
    this->beginResetModel();
    this->endResetModel();
}

int BookMarksModel::rowCount(const QModelIndex &parent) const {
    return _bookmarks.size();
}

int BookMarksModel::columnCount(const QModelIndex &parent) const { return 3; }

QVariant BookMarksModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole: {
        auto b = _bookmarks.at(index.row());
        switch (index.column()) {
        case 0: // offset
            return b.pos;
        case 1: // comment
            return b.comment;
        }
        return QVariant();
    }
    case Qt::ToolTipPropertyRole:
        break;
    case Qt::BackgroundRole:
        break;
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
            case 1:
                return tr("offset");
            case 2:
                return tr("comment");
            }
        } else {
            return section + 1;
        }
    }
    return QVariant();
}
