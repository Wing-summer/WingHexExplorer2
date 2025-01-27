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

#include <QFileInfo>

DbgBreakpointModel::DbgBreakpointModel(QObject *parent)
    : QAbstractTableModel(parent) {}

int DbgBreakpointModel::rowCount(const QModelIndex &parent) const {
    return _dbg ? _dbg->breakPoints().size() : 0;
}

int DbgBreakpointModel::columnCount(const QModelIndex &parent) const {
    return 2;
}

QVariant DbgBreakpointModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole: {
        auto r = index.row();
        auto d = _dbg->breakPoints().at(r);
        switch (index.column()) {
        case 0: // line
            return d.lineNbr;
        case 1: // file
            return role == Qt::ToolTipRole ? d.name
                                           : QFileInfo(d.name).fileName();
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
                return tr("file");
            }
        } else {
            return section + 1;
        }
    }
    return QVariant();
}

void DbgBreakpointModel::setDebugger(asDebugger *dbg) {
    Q_ASSERT(dbg);
    connect(dbg, &asDebugger::breakPointChanged, this,
            [=]() { emit layoutChanged(); });
    _dbg = dbg;
}
