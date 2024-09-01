#include "findresultmodel.h"

FindResultModel::FindResultModel(QObject *parent)
    : QAbstractTableModel(parent) {}

int FindResultModel::rowCount(const QModelIndex &parent) const {
    return m_results.size();
}

int FindResultModel::columnCount(const QModelIndex &parent) const { return 4; }

QVariant FindResultModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole: {
        auto r = m_results.at(index.row());
        switch (index.column()) {
        case 0: // line
            return r.line;
        case 1: // col
            return r.col;
        case 2: // offset
            return QStringLiteral("0x") +
                   QString::number(r.offset, 16).toUpper();
        case 3: // value
            return m_lastFindData;
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
            }
        } else {
            return section + 1;
        }
    }
    return QVariant();
}

QList<WingHex::FindResult> &FindResultModel::results() { return m_results; }

QString &FindResultModel::lastFindData() { return m_lastFindData; }

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
