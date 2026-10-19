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

#ifndef LUAUSCHEDULER_H
#define LUAUSCHEDULER_H

#include <QObject>

#include "lua.h"

struct LuauThread;
class LuauDebugger;

struct LuauThreadData {
    LuauThread *parent = nullptr;
    quint64 startTime = 0;
    quint64 lastInteruptTime = 0;
    quint64 timeOutTime = 0;
    LuauDebugger *debugger = nullptr;
    int mode = 0;
};

struct LuauThread {
    lua_State *state = nullptr;
    LuauThreadData *data = nullptr;
    int refID = LUA_REFNIL;
    bool isRunning = false;
    bool requestStop = false;

    inline void reset() {
        lua_resetthread(state);
        isRunning = false;
        requestStop = false;
        Q_ASSERT(data);
        data->startTime = 0;
        data->lastInteruptTime = 0;
        data->debugger = nullptr;
    }

    inline void destory() {
        if (refID != LUA_REFNIL) {
            reset();
            lua_unref(lua_mainthread(state), refID);
            state = nullptr;
            refID = LUA_REFNIL;
        }
    }
};

class LuauScheduler final : public QObject {
    Q_OBJECT
public:
    explicit LuauScheduler(LuauThread *th);
    virtual ~LuauScheduler();

public:
    void start();
    void requestStop();

    QString executeError() const;

signals:
    void finished(int status);

private Q_SLOTS:
    void step();

private:
    LuauThread *m_th;
    QString m_executeError;
};

#endif // LUAUSCHEDULER_H
