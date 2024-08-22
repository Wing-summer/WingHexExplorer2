#include "metadatamodel.h"

MetaDataModel::MetaDataModel(QHexDocument *doc, QObject *parent)
    : QAbstractTableModel(parent), _doc(doc) {}

int MetaDataModel::rowCount(const QModelIndex &parent) const {
    return _doc ? _doc->metadata()->size() : 0;
}

int MetaDataModel::columnCount(const QModelIndex &parent) const { return 5; }

QVariant MetaDataModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole: {
        auto r = index.row();
        const auto &b = _doc->metadata()->getallMetasPtr();
        auto d = b.at(r);
        switch (index.column()) {
        case 0: // begin
            return QString::number(d.begin) + QStringLiteral(" - 0x") +
                   QString::number(d.begin, 16).toUpper();
        case 1: // end
            return QString::number(d.end) + QStringLiteral(" - 0x") +
                   QString::number(d.end, 16).toUpper();
        case 2: {
            if (d.foreground.alpha() == 0) {
                return QStringLiteral("-");
            }
            return QVariant();
            break;
        }
        case 3:
            if (d.background.alpha() == 0) {
                return QStringLiteral("-");
            }
            return QVariant();
            break;
        case 4:
            return d.comment;
        }
    }
    case Qt::ToolTipRole: {
        auto r = index.row();
        const auto &b = _doc->metadata()->getallMetasPtr();
        auto d = b.at(r);
        switch (index.column()) {
        case 2: {
            if (d.foreground.alpha() == 0) {
                return QStringLiteral("-");
            } else {
                return d.foreground.name();
            }
            break;
        }
        case 3:
            if (d.background.alpha() == 0) {
                return QStringLiteral("-");
            } else {
                return d.background.name();
            }
            break;
        case 4:
            return d.comment;
        }
    } break;
    case Qt::BackgroundRole: {
        auto r = index.row();
        const auto &b = _doc->metadata()->getallMetasPtr();
        auto d = b.at(r);
        switch (index.column()) {
        case 2:
            return d.foreground;
        case 3:
            return d.background;
        default:
            break;
        }
    } break;
    case Qt::TextAlignmentRole:
        return int(Qt::AlignCenter);
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
