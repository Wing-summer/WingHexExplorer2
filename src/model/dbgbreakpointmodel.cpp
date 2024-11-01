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
