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

#include "asdebugger.h"
#include "class/appmanager.h"
#include "class/aswingcache.h"
#include "define.h"

asDebugger::asDebugger(asIDBWorkspace *workspace)
    : QObject(), asIDBDebugger(workspace) {
    this->onLineCallBack = [this](asIScriptContext *ctx) -> bool {
        // This should never happen, but it doesn't hurt to validate it
        if (ctx == nullptr)
            return false;

        // By default we ignore callbacks when the context is not active.
        // An application might override this to for example disconnect the
        // debugger as the execution finished.
        if (ctx->GetState() != asEXECUTION_ACTIVE)
            return false;

        auto now = AppManager::instance()->currentMSecsSinceEpoch();
        auto timer = reinterpret_cast<asPWORD>(
            ctx->GetUserData(AsUserDataType::UserData_Timer));
        auto timeOutTime = reinterpret_cast<asPWORD>(
            ctx->GetUserData(AsUserDataType::UserData_TimeOut));
        auto mode = reinterpret_cast<asPWORD>(
            ctx->GetUserData(AsUserDataType::UserData_ContextMode));

        auto isDbg = reinterpret_cast<asPWORD>(
            ctx->GetUserData(AsUserDataType::UserData_isDbg));

        if (!isDbg) {
            bool timeOut = false;
            if (timer < 0) {
                timeOut = true;
            } else {
                if (mode == 0) {
                    timeOut = (now - timer) > 3000; // 3 s
                } else {
                    if (timeOutTime) {
                        timeOut = (now - timer) > timeOutTime; // 10 min
                    }
                }
            }

            if (timeOut && mode) {
                auto timeOut = tr("ScriptTimedOut");
                ScriptMachine::MessageInfo info;
                info.message = timeOut;
                info.mode = ScriptMachine::ConsoleMode(mode);
                info.type = ScriptMachine::MessageType::Error;
                ScriptMachine::instance().outputMessage(info);
                ctx->Abort();
                return false;
            }
        }

        return isDbg;
    };

    this->onLineCallBackExec = [this](int row, int col,
                                      const char *section) -> void {
        Q_EMIT onRunCurrentLine(QString::fromUtf8(section), row);
        Q_EMIT onDebugActionExec();
    };

    this->onDebugBreak = [this]() {
        Q_EMIT onPullCallStack();
        Q_EMIT onPullVariables();
        Q_EMIT onDebugActionExec();
    };

    this->onAdjustBreakPoint = [this](int old, int line, const char *section) {
        Q_EMIT onAdjustBreakPointLine(QString::fromUtf8(section), old, line);
    };
}

asDebugger::~asDebugger() {}

void asDebugger::addFileBreakPoint(const QString &file, int lineNbr) {
    auto id = file.toStdString();

    asIDBBreakpoint bp;
    bp.line = lineNbr;
    bp.needAdjust = true;

    auto r = breakpoints.find(id);
    if (r == breakpoints.end()) {
        breakpoints[id].push_back(bp);
    } else {
        auto &bps = r->second;
        auto r = std::find_if(bps.begin(), bps.end(),
                              [lineNbr](const asIDBBreakpoint &bp) {
                                  return bp.line == lineNbr;
                              });
        if (r == bps.end()) {
            bps.push_back(bp);
        }
    }
}

void asDebugger::removeFileBreakPoint(const QString &file, int lineNbr) {
    auto id = file.toStdString();

    auto r = breakpoints.find(id);
    if (r != breakpoints.end()) {
        auto &bps = r->second;
        auto r = std::find_if(bps.begin(), bps.end(),
                              [lineNbr](const asIDBBreakpoint &bp) {
                                  return bp.line == lineNbr;
                              });
        bps.erase(r);
    }
}

void asDebugger::clearBreakPoint() { breakpoints.clear(); }

void asDebugger::reset() {
    action = asIDBAction::None;
    stack_size = 0;
    internal_execution = false;
    breakpoints.clear();
    function_breakpoints.clear();
    frame_offset = 0;
    cache->call_stack.clear();
    cache->globals.reset();
    cache->variables.clear();
    cache->variable_refs.clear();
    _lastFunction = nullptr;
}

void asDebugger::Suspend() {
    resume = false;
    while (!resume) {
        QApplication::processEvents();
    }
}

void asDebugger::Resume() { resume = true; }

std::unique_ptr<asIDBCache> asDebugger::CreateCache(asIScriptContext *ctx) {
    return std::make_unique<asWingCache>(*this, ctx);
}
