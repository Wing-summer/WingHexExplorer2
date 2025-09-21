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

DbgVarShowModel::DbgVarShowModel(bool isGlobal, QObject *parent)
    : QAbstractTableModel(parent), _isGlobal(isGlobal) {}

int DbgVarShowModel::rowCount(const QModelIndex &parent) const {
    if (_debugger) {
        if (_debugger->cache) {
            if (_isGlobal) {
                _debugger->cache->CacheGlobals();
                return _debugger->cache->globals->namedProps.size();
            } else {
                auto &cs = _debugger->cache->call_stack;
                if (!cs.empty()) {
                    _debugger->cache->CacheCallstack();
                    auto l = _debugger->cache->call_stack.at(0).scope.locals;
                    if (l) {
                        return l->namedProps.size();
                    }
                }
            }
        }
    }
    return 0;
}

int DbgVarShowModel::columnCount(const QModelIndex &parent) const { return 2; }

QVariant DbgVarShowModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole: {
        auto r = index.row();
        if (_isGlobal) {
            auto d =
                *std::next(_debugger->cache->globals->namedProps.begin(), r);
            d->Evaluate();
            switch (index.column()) {
            case 0: // name
                return QString::fromStdString(d->identifier.Combine());
            case 1: // value
                return QString::fromStdString(d->value);
            }
        } else {
            auto &cs = _debugger->cache->call_stack;
            if (!cs.empty()) {
                _debugger->cache->CacheCallstack();
                auto l = _debugger->cache->call_stack.at(0).scope.locals;
                if (l) {
                    auto d = *std::next(l->namedProps.begin(), r);
                    d->Evaluate();
                    switch (index.column()) {
                    case 0: // name
                        return QString::fromStdString(d->identifier.Combine());
                    case 1: // value
                        return QString::fromStdString(d->value);
                    }
                }
            }
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

void DbgVarShowModel::attachDebugger(asDebugger *debugger) {
    if (_debugger != debugger) {
        if (_debugger) {
            _debugger->disconnect(this, nullptr);
        }
        _debugger = debugger;
        connect(_debugger, &asDebugger::onPullVariables, this,
                [this]() { Q_EMIT layoutChanged(); });
        Q_EMIT layoutChanged();
    }
}
