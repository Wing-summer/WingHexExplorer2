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
**
** The original License is MIT from Dax89/QHexView. I have modified a lot so I
** decide to change the Open Source License. You can use the original library
** under MIT. Thanks for Dax89's efforts.
** =============================================================================
*/

#include "qhexundostack.h"

QHexUndoStack::QHexUndoStack(QObject *parent) : QUndoStack(parent) {
    connect(this, &QUndoStack::indexChanged, this,
            &QHexUndoStack::recomputeUnsaved);
    connect(this, &QUndoStack::cleanChanged, this,
            &QHexUndoStack::recomputeUnsaved);
}

QHexUndoStack::~QHexUndoStack() { blockSignals(true); }

bool QHexUndoStack::hasUnsavedId(int id) const {
    return m_unsavedCounts.contains(id);
}

void QHexUndoStack::recomputeUnsaved() {
    QSet<int> newCounts;

    int clean = this->cleanIndex();
    int idx = this->index();

    if (clean < 0) {
        clean = 0;
    }

    int lo = qMin(clean, idx);
    int hi = qMax(clean, idx) - 1;

    if (hi >= lo) {
        for (int i = lo; i <= hi; ++i) {
            const QUndoCommand *cmd = this->command(i);
            if (!cmd) {
                continue;
            }
            collectRecursive(cmd, newCounts);
        }
    }

    if (newCounts != m_unsavedCounts) {
        m_unsavedCounts = newCounts;
    }
}

void QHexUndoStack::collectRecursive(const QUndoCommand *cmd,
                                     QSet<int> &out) const {
    if (!cmd)
        return;

    int cmdId = cmd->id();
    if (cmdId >= 0) {
        out.insert(cmdId);
    }

    for (int i = 0; i < cmd->childCount(); ++i) {
        collectRecursive(cmd->child(i), out);
    }
}
