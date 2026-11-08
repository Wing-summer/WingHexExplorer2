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
    return parent.isValid() ? 0 : m_frames.size();
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
        if (r < 0 || r >= m_frames.size()) {
            return {};
        }
        const auto &frame = m_frames.at(r);
        switch (index.column()) {
        case 0:
            return frame.line;
        case 1:
            return role == Qt::ToolTipRole ? frame.source
                                           : QFileInfo(frame.source).fileName();
        case 2:
            return frame.name;
        }
        return {};
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
                return tr("Line");
            case 1:
                return tr("File");
            case 2:
                return tr("Declaration");
            }
        } else {
            return section + 1;
        }
    }
    return QVariant();
}

void DbgCallStackModel::attachDebugger(LuauDebugger *debugger) {
    if (m_debugger != debugger) {
        if (m_debugger) {
            m_debugger->disconnect(this, nullptr);
        }
        m_debugger = debugger;
        if (m_debugger) {
            connect(m_debugger, &LuauDebugger::sigOnPullCallStack, this,
                    [this]() {
                        beginResetModel();
                        m_frames = m_debugger->stackFrames();
                        endResetModel();
                    });
        }
    }
    beginResetModel();
    m_frames =
        m_debugger ? m_debugger->stackFrames() : QVector<LuauStackFrame>{};
    endResetModel();
}
