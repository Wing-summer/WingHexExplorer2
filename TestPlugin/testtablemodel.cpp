#include "testtablemodel.h"

#include "iwingplugin.h"

TestTableModel::TestTableModel(QObject *parent) : QAbstractTableModel(parent) {}

int TestTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return WingHex::SDKVERSION;
}

int TestTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 5;
}

QVariant TestTableModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole: {
        return QStringLiteral("(%1, %2)").arg(index.row()).arg(index.column());
    }
    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
    }
    return QVariant();
}

QVariant TestTableModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const {
    Q_UNUSED(orientation);
    if (role == Qt::DisplayRole) {
        return section + 1;
    }
    return QVariant();
}
