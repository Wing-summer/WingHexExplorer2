/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include "scriptrunable.h"

#include <QTimer>

ScriptRunable::ScriptRunable(CContextMgr *mgr, int owner)
    : _mgr(mgr), _owner(owner) {
    Q_ASSERT(mgr && owner);
}

ScriptRunable::~ScriptRunable() { delete _mgr; }

void ScriptRunable::start() {
    QTimer::singleShot(0, this, &ScriptRunable::step);
}

void ScriptRunable::step() {
    if (_mgr->ExecuteScripts()) {
        QTimer::singleShot(0, this, &ScriptRunable::step);
    } else {
        deleteLater();
    }
}
