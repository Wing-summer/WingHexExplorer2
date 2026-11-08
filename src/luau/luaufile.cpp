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

#include "luaufile.h"

#include <unordered_set>

#include <QVector>
#include <qpair.h>

#include "Luau/RegisterX64.h"

#include "luauutil.h"

LuaFileRef::LuaFileRef(lua_State *L) {
    state = L;
    func = LuauUtil::getLuaFunction(L, -1);
    lua_checkstack(L, 1);

    // Save the lua_State associated with file
    lua_pushthread(L);
    threadRef = lua_ref(L, -1);
    lua_pop(L, 1);

    // Save the function return by luau_load
    fileRef = lua_ref(L, -1);
}

LuaFileRef::~LuaFileRef() { release(); }

LuaFileRef::LuaFileRef(const LuaFileRef &other) { copyFrom(other); }

LuaFileRef &LuaFileRef::operator=(const LuaFileRef &other) {
    if (this == &other) {
        return *this;
    }

    copyFrom(other);
    return *this;
}

bool LuaFileRef::operator==(const LuaFileRef &other) const {
    return state == other.state && func == other.func;
}

void LuaFileRef::release() {
    if (state == nullptr) {
        return;
    }
    if (threadRef != LUA_REFNIL) {
        lua_unref(state, threadRef);
    }
    if (fileRef != LUA_REFNIL) {
        lua_unref(state, fileRef);
    }
    threadRef = LUA_REFNIL;
    fileRef = LUA_REFNIL;
}

void LuaFileRef::copyFrom(const LuaFileRef &other) {
    state = other.state;
    lua_checkstack(state, 1);

    // Copy the lua_State reference
    lua_getref(state, other.threadRef);
    threadRef = lua_ref(state, -1);
    lua_pop(state, 1);

    // Copy the function reference
    lua_getref(state, other.fileRef);
    fileRef = lua_ref(state, -1);
    func = LuauUtil::getLuaFunction(state, -1);
    lua_pop(state, 1);
}

LuauFile::LuauFile(const QString &path) { setPath(path); }

void LuauFile::setPath(const QString &path) {
    if (m_path == path) {
        return;
    }
    clearBreakPoints();
    m_path = path;
}

QString LuauFile::path() const { return m_path; }

QString LuauFile::source() const { return m_src; }

void LuauFile::setSource(const QString &source) { m_src = source; }

void LuauFile::setBreakPoints(
    const std::unordered_map<int, BreakPoint> &breakpoints) {
    std::unordered_set<int> settled;
    for (const auto &[_, bp] : breakpoints) {
        addBreakPoint(bp);
        settled.insert(bp.line());
    }
    removeBreakPointsIf([&settled](const BreakPoint &bp) {
        return settled.find(bp.line()) == settled.end();
    });
}

QVector<QPair<int, int>> LuauFile::addRef(LuaFileRef ref) {
    QVector<QPair<int, int>> adjustedLines;
    if (std::find(m_refs.begin(), m_refs.end(), ref) != m_refs.end()) {
        return adjustedLines;
    }

    for (auto &[_, bp] : m_breakpoints) {
        const auto originLine = bp.enable(ref.state, ref.fileRef, true);
        if (originLine >= 0) {
            if (originLine != bp.line()) {
                adjustedLines.append(qMakePair(originLine, bp.line()));
            }
        } else {
            adjustedLines.append(qMakePair(originLine, -1));
        }
    }
    m_refs.emplace_back(std::move(ref));
    return adjustedLines;
}

void LuauFile::removeRef(lua_State *L) {
    auto it = std::remove_if(
        m_refs.begin(), m_refs.end(),
        [L](const LuaFileRef &ref) { return lua_mainthread(ref.state) == L; });
    m_refs.erase(it, m_refs.end());
}

bool LuauFile::enableBreakPoint(BreakPoint &bp, bool enable) {
    bool ok = true;
    for (auto &ref : m_refs) {
        ok &= bp.enable(ref.state, ref.fileRef, enable) >= 0;
    }
    return ok;
}

int LuauFile::addBreakPoint(int line) {
    return addBreakPoint(BreakPoint::create(line));
}

int LuauFile::addBreakPoint(const BreakPoint &bp) {
    auto line = bp.line();
    auto it = m_breakpoints.find(line);
    if (it == m_breakpoints.end()) {
        auto nbp = bp;
        // invalid breakpoint
        if (!enableBreakPoint(nbp, true)) {
            enableBreakPoint(nbp, false);
            return -1;
        }
        // if adjusted line is different from the original line,
        // check if the new line already exists
        auto nline = nbp.line();
        if (nline != line) {
            auto it = m_breakpoints.find(nline);
            if (it != m_breakpoints.end()) {
                return -1;
            }
        }
        it = m_breakpoints.emplace(nline, nbp).first;
    } else {
        it->second = bp;
    }
    return it->second.line();
}

void LuauFile::removeBreakPoint(int line) {
    auto it = m_breakpoints.find(line);
    if (it != m_breakpoints.end()) {
        enableBreakPoint(it->second, false);
        m_breakpoints.erase(it);
    }
}

void LuauFile::clearBreakPoints() {
    for (auto &[line, bp] : m_breakpoints) {
        enableBreakPoint(bp, false);
    }
    m_breakpoints.clear();
}

BreakPoint *LuauFile::findBreakPoint(int line) {
    auto it = m_breakpoints.find(line);
    if (it == m_breakpoints.end()) {
        return nullptr;
    }
    return &it->second;
}
