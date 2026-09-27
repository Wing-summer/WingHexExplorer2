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
#include <QEventLoop>
#include <QHash>
#include <QStack>

#include <functional>

#include "luau/breakpoint.h"
#include "luau/luaufile.h"
#include "luau/luauvariableregistry.h"

#include "lua.h"

struct LuauStackFrame {
public:
    int id = 0;
    QString name;
    QString source;
    int line = 0;
};

class LuauDebugger : public QObject {
    Q_OBJECT
public:
    enum class BreakReason { Step, BreakPoint, Entry, Pause };

public:
    LuauDebugger();

Q_SIGNALS:
    void onAdjustBreakPointLine(const QString &file, int oldLineNbr,
                                int newLineNbr);
    void onPullVariables();
    void onPullCallStack();
    void onRunCurrentLine(const QString &file, int lineNr);
    void onDebugActionExec();

public:
    void attach(lua_State *L);
    void detach();

    void resume();
    void pause();
    void terminate();
    void setBreakPoints(const QString &path,
                        const std::unordered_map<int, BreakPoint> &breakpoints);
    void clearBreakPoints();

public:
    // Called from **lua runtime** after lua file is loaded
    // Assume that the top closure from file is already on
    // the stack
    void onLuaFileLoaded(lua_State *L, const QString &path,
                         const QByteArray &source = {});

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
    const QVector<LuauStackFrame> &stackFrames() const;
    LuauVariableRegistry *variableRegistry();
    LuauVariable::Ptr evaluateExpression(const QString &expression,
                                         int frameId = 0);
    LuauScope localScope(int frameId) const;
    LuauScope upvalueScope(int frameId) const;
    LuauScope globalScope() const;

    LuauFileContext findLoadedLuauFile(const QString &path);

private:
    using SingleStepProcessor = std::function<bool(lua_State *, lua_Debug *)>;

    static void debugBreak(lua_State *L, lua_Debug *ar);
    static void debugStep(lua_State *L, lua_Debug *ar);

    void processSingleStep(SingleStepProcessor processor);
    bool hitBreakPoint(lua_State *L) const;
    BreakPoint *findBreakPoint(lua_State *L) const;
    void waitForResume();

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
    lua_Callbacks *callbacks_ = nullptr;
    bool attached_ = false;
    bool resume_ = true;
    bool pauseRequested_ = false;
    bool terminateRequested_ = false;
    QEventLoop *breakLoop_ = nullptr;
    lua_State *breakVm_ = nullptr;
    BreakContext breakContext_;
    SingleStepProcessor singleStepProcessor_;
    QVector<LuauStackFrame> stackFrames_;
    QVector<lua_State *> frameStates_;
    QVector<int> frameLevels_;
    QVector<int> frameDepths_;

    QHash<QString, LuauFileContext> files_;
    QStack<lua_State *> thread_stack_;
    LuauVariableRegistry variable_registry_;
};

#endif // LUAUDEBUGGER_H
