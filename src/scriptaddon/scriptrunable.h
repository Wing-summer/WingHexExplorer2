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

#ifndef SCRIPTRUNABLE_H
#define SCRIPTRUNABLE_H

#include "scriptaddon/contextmgr.h"

#include <QObject>

class ScriptRunable : public QObject {
    Q_OBJECT
public:
    ScriptRunable(CContextMgr *mgr, int owner);
    virtual ~ScriptRunable();

public:
    void start();

private slots:
    void step();

private:
    CContextMgr *_mgr;
    int _owner;
};

#endif // SCRIPTRUNABLE_H
