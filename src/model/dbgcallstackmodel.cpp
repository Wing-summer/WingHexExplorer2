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
    if (_debugger) {
        if (_debugger->cache) {
            _debugger->cache->CacheCallstack();
            return _debugger->cache->call_stack.size();
        }
    }
    return 0;
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
        auto &cache = _debugger->cache;
        if (!cache) {
            return {};
        }

        auto &d = cache->call_stack.at(r);
        switch (index.column()) {
        case 0: // line
            return d.row;
        case 1: // file
        {
            auto section = d.section;
            auto file = QString::fromUtf8(section.data(), section.length());
            if (role == Qt::ToolTipRole) {
                return file;
            } else {
                return QFileInfo(file).fileName();
            }
        }
        case 2: // decl
            return QString::fromStdString(d.declaration);
        }
    }
    case Qt::TextAlignmentRole:
        return int(Qt::AlignCenter);
    }
    return {};
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

void DbgCallStackModel::attachDebugger(asDebugger *debugger) {
    if (_debugger != debugger) {
        if (_debugger) {
            _debugger->disconnect(this, nullptr);
        }
        _debugger = debugger;
        connect(_debugger, &asDebugger::onPullCallStack, this,
                [this]() { Q_EMIT layoutChanged(); });
        Q_EMIT layoutChanged();
    }
}
