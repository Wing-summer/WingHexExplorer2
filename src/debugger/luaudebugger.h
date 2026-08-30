/*==============================================================================
 ** Copyright (C) 2026-2029 WingSummer
 **
 ** This program is free software: you can redistribute it and/or modify it
 ** under the terms of the GNU Affero General Public License as published by the
 ** Free Software Foundation, version 3.
 **
 ** This program is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 ** for more details.
 **
 ** You should have received a copy of the GNU Affero General Public License
 ** along with this program. If not, see <https://www.gnu.org/licenses/>.
 ** =============================================================================
 */

#ifndef LUAUDEBUGGER_H
#define LUAUDEBUGGER_H

#include <QDir>
#include <QHash>
#include <QStack>

#include "debugger/breakpoint.h"
#include "debugger/luaufile.h"
#include "debugger/luauvariableregistry.h"

#include "lua.h"

struct LuauStackFrame {
public:
    int id = 0;
    QString name;
    QString source;
    int line = 0;
};

class LuauDebugger {
public:
    enum class BreakReason { Step, BreakPoint, Entry, Pause };

public:
    LuauDebugger();

public:
    void attach(lua_State *L);
    void detach();

public:
    // Called from **lua runtime** after lua file is loaded
    // Assume that the top closure from file is already on
    // the stack
    void onLuaFileLoaded(lua_State *L, const QString &path, bool is_entry);

    // Called from **lua runtime** when debug break encountered
    void onDebugBreak(lua_State *L, lua_Debug *ar, BreakReason reason);

public:
    bool isDebugBreak();

    // step to next line
    void stepOver();

    // step into function
    void stepIn();

    // step out of function
    void stepOut();

public:
    QVector<LuauStackFrame> updateStackFrames(lua_State *L);

    LuauFileContext findLoadedLuauFile(const QString &path);

private:
    int getStackDepth(lua_State *L) const;

    BreakContext getBreakContext(lua_State *L) const;

    void enableDebugStep(bool enable);

    void resumeInternal();

public:
    QVector<lua_State *> getThreadAncestors(lua_State *L);

    void pushThreadStack(lua_State *state);
    void popThreadStack();

private:
    lua_State *getParent(lua_State *L) const;

private:
    lua_State *L_ = nullptr;

    QHash<QString, LuauFileContext> files_;
    QStack<lua_State *> thread_stack_;
    LuauVariableRegistry variable_registry_;
};

#endif // LUAUDEBUGGER_H
