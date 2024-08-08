#include "checksummodel.h"
#include "src/utilities.h"

CheckSumModel::CheckSumModel(QObject *parent) : QAbstractTableModel(parent) {
    for (auto &cs : Utilities::supportedHashAlgorithms()) {
        _checkSumData.insert(cs, QString());
    }
}

void CheckSumModel::setCheckSumData(QCryptographicHash::Algorithm algorithm,
                                    const QString &data) {
    if (_checkSumData.contains(algorithm)) {
        _checkSumData[algorithm] = data;
    }
}

QString
CheckSumModel::checkSumData(QCryptographicHash::Algorithm algorithm) const {
    return _checkSumData.value(algorithm);
}

void CheckSumModel::clearData() {
    for (auto p = _checkSumData.begin(); p != _checkSumData.end(); ++p) {
        p->clear();
    }
}

int CheckSumModel::rowCount(const QModelIndex &parent) const {
    return _checkSumData.size();
}

int CheckSumModel::columnCount(const QModelIndex &parent) const { return 1; }

QVariant CheckSumModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole: {
        auto r =
            _checkSumData.value(QCryptographicHash::Algorithm(index.row()));
        if (index.column() == 0) {
            return r.isEmpty() ? QStringLiteral("-") : r;
        }
    }
    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
    }
    return QVariant();
}

QVariant CheckSumModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return tr("Checksum");
            }
        } else {
            return Utilities::supportedHashAlgorithmStringList().at(section);
        }
    }
    return QVariant();
}
