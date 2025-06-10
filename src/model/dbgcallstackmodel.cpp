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

#include "dbgcallstackmodel.h"

#include <QFileInfo>

DbgCallStackModel::DbgCallStackModel(QObject *parent)
    : QAbstractTableModel(parent) {}

int DbgCallStackModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return _stack.size();
}

int DbgCallStackModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 3;
}

QVariant DbgCallStackModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole: {
        auto r = index.row();
        auto d = _stack.at(r);
        switch (index.column()) {
        case 0: // line
            return d.lineNbr;
        case 1: // file
        {
            if (role == Qt::ToolTipRole) {
                return d.file;
            } else {
                return QFileInfo(d.file).fileName();
            }
        }
        case 2: // decl
            return d.decl;
        }
    }
    case Qt::TextAlignmentRole:
        return int(Qt::AlignCenter);
    }
    return QVariant();
}

QVariant DbgCallStackModel::headerData(int section, Qt::Orientation orientation,
                                       int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return tr("line");
            case 1:
                return tr("file");
            case 2:
                return tr("decl");
            }
        } else {
            return section + 1;
        }
    }
    return QVariant();
}

void DbgCallStackModel::updateData(
    const QList<asDebugger::CallStackItem> &callstack) {
    _stack = callstack;
    Q_EMIT layoutChanged();
}
