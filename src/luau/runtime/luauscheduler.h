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
#include <QTimer>

#include "lua.h"

#include "qtluau/callback.hpp"

struct LuauThread;
class LuauDebugger;
class ScriptExecution;

struct LuauThreadData {
    LuauThread *parent = nullptr;
    quint64 startTime = 0;
    quint64 lastInteruptTime = 0;
    LuauDebugger *debugger = nullptr;
    int mode = 0;
    ScriptExecution *execution = nullptr;
};

struct LuauThread {
    lua_State *state = nullptr;
    LuauThreadData *data = nullptr;
    int refID = LUA_REFNIL;
    bool isRunning = false;
    bool requestStop = false;
    bool clearContextOnReset = true;

    inline void reset() {
        if (clearContextOnReset) {
            qtluau::clearLuaContext(state);
        }
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

class LuauExecutor {
public:
    inline virtual ~LuauExecutor() = default;
    virtual bool isExecuting() const = 0;
    virtual int resumeThread(LuauThread *thread, int arguments) = 0;
};

class LuauScheduler final : public QObject {
    Q_OBJECT
public:
    explicit LuauScheduler(LuauExecutor &executor, LuauThread *thread,
                           QObject *parent = nullptr);
    virtual ~LuauScheduler() override;

public:
    void start(int arguments = 0);
    void stop();

    QString executeError() const;

signals:
    void sigOnFinished(int status);

private Q_SLOTS:
    void step();

private:
    void finish(int status);

private:
    LuauExecutor &m_executor;
    LuauThread *m_th;
    QTimer m_timer;
    QString m_executeError;
    int m_arguments = 0;
};

#endif // LUAUSCHEDULER_H
