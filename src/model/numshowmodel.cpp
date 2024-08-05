#include "numshowmodel.h"

static auto UNKNOWN_NUM = QStringLiteral("-");

static QStringList _numHeader{"byte",   "char",  "ushort", "short",
                              "uint32", "int32", "uint64", "int64"};

NumShowModel::NumShowModel(QObject *parent) : QAbstractTableModel(parent) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_num.fill(QString(), _numHeader.size());
#else
    m_num.reserve(_numHeader.size());
    std::fill_n(std::back_inserter(m_num), _numHeader.size(), QString());
#endif
}

void NumShowModel::setNumData(NumTableIndex index, const QString &data) {
    if (data.isEmpty()) {
        m_num[index] = UNKNOWN_NUM;
    } else {
        m_num[index] = data;
    }
    emit dataChanged(this->index(index, 0), this->index(index, 0));
}

QString NumShowModel::numData(NumTableIndex index) const {
    return m_num.at(index);
}

void NumShowModel::clear() {
    beginResetModel();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_num.fill(QString());
#else
    std::fill_n(std::back_inserter(m_num), _numHeader.size(), QString());
#endif
    endResetModel();
}

int NumShowModel::rowCount(const QModelIndex &parent) const {
    return _numHeader.size();
}

int NumShowModel::columnCount(const QModelIndex &parent) const { return 1; }

QVariant NumShowModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole: {
        if (index.column() == 0) {
            auto r = m_num.at(index.row());
            return r.isEmpty() ? UNKNOWN_NUM : r;
        }
        return QVariant();
    }
    case Qt::TextAlignmentRole:
        return int(Qt::AlignCenter);
    }
    return QVariant();
}

QVariant NumShowModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            return tr("Value");
        } else {
            return _numHeader.at(section);
        }
    }
    return QVariant();
}
