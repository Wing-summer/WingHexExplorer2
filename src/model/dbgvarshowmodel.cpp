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

#include "dbgvarshowmodel.h"

DbgVarShowModel::DbgVarShowModel(QObject *parent)
    : QAbstractTableModel(parent) {}

int DbgVarShowModel::rowCount(const QModelIndex &parent) const {
    return _vars.size();
}

int DbgVarShowModel::columnCount(const QModelIndex &parent) const { return 2; }

QVariant DbgVarShowModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole: {
        auto r = index.row();
        auto d = _vars.at(r);
        switch (index.column()) {
        case 0: // name
            return d.name;
        case 1: // value
            return d.value;
        }
    }
    case Qt::TextAlignmentRole:
        return int(Qt::AlignCenter);
    }
    return QVariant();
}

QVariant DbgVarShowModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return tr("name");
            case 1:
                return tr("value");
            }
        } else {
            return section + 1;
        }
    }
    return QVariant();
}

void DbgVarShowModel::updateData(
    const QVector<asDebugger::VariablesInfo> &varinfos) {
    _vars = varinfos;
    emit layoutChanged();
}
