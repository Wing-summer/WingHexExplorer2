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

#include "checksummodel.h"
#include "utilities.h"

CheckSumModel::CheckSumModel(QObject *parent) : QAbstractTableModel(parent) {
    for (auto &cs : Utilities::supportedHashAlgorithms()) {
        _checkSumData.insert(cs, QString());
    }
}

void CheckSumModel::setCheckSumData(QCryptographicHash::Algorithm algorithm,
                                    const QString &data) {
    if (_checkSumData.contains(algorithm)) {
        _checkSumData[algorithm] = data;
        auto index = this->index(
            Utilities::supportedHashAlgorithms().indexOf(algorithm), 0);
        Q_EMIT dataChanged(index, index);
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
    Q_EMIT dataChanged(
        this->index(0, 0),
        this->index(Utilities::supportedHashAlgorithms().size() - 1, 0));
}

int CheckSumModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return _checkSumData.size();
}

int CheckSumModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}

QVariant CheckSumModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole: {
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
