#include "dbgbreakpointmodel.h"

DbgBreakpointModel::DbgBreakpointModel(QObject *parent)
    : QAbstractTableModel(parent) {}

int DbgBreakpointModel::rowCount(const QModelIndex &parent) const {
    return _bps.size();
}

int DbgBreakpointModel::columnCount(const QModelIndex &parent) const {
    return 3;
}

QVariant DbgBreakpointModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole: {
        auto r = index.row();
        auto d = _bps.at(r);
        switch (index.column()) {
        case 0: // line
            return d.lineNbr;
        case 1: // file
            return d.name;
        case 2: // decl
            return d.func;
        }
    }
    case Qt::TextAlignmentRole:
        return int(Qt::AlignCenter);
    }
    return QVariant();
}

QVariant DbgBreakpointModel::headerData(int section,
                                        Qt::Orientation orientation,
                                        int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return tr("line");
            case 1:
                return tr("name");
            case 2:
                return tr("func");
            }
        } else {
            return section + 1;
        }
    }
    return QVariant();
}
