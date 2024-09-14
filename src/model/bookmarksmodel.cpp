#include "bookmarksmodel.h"

BookMarksModel::BookMarksModel(QHexDocument *doc, QObject *parent)
    : QAbstractTableModel(parent), _doc(doc) {}

int BookMarksModel::rowCount(const QModelIndex &parent) const {
    return _doc ? _doc->bookMarksCount() : 0;
}

int BookMarksModel::columnCount(const QModelIndex &parent) const { return 2; }

QVariant BookMarksModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole: {
        auto r = index.row();
        auto b = _doc->bookMarkByIndex(r);
        switch (index.column()) {
        case 0: // offset
            return b.pos;
        case 1: // comment
            return b.comment;
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
    _doc = newDoc;
    connect(newDoc, &QHexDocument::bookMarkChanged, this,
            [=] { emit this->layoutChanged(); });
}
