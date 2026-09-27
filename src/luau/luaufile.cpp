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

#include "Luau/RegisterX64.h"
#include "luauutil.h"

#include <QVector>
#include <qpair.h>
#include <unordered_set>

LuaFileRef::LuaFileRef(lua_State *L) {
    L_ = L;
    func_ = LuauUtil::getLuaFunction(L, -1);
    lua_checkstack(L, 1);

    // Save the lua_State associated with file
    lua_pushthread(L);
    thread_ref_ = lua_ref(L, -1);
    lua_pop(L, 1);

    // Save the function return by luau_load
    file_ref_ = lua_ref(L, -1);
}

LuaFileRef::~LuaFileRef() { release(); }

LuaFileRef::LuaFileRef(const LuaFileRef &other) { copyFrom(other); }

LuaFileRef &LuaFileRef::operator=(const LuaFileRef &other) {
    if (this == &other)
        return *this;

    copyFrom(other);
    return *this;
}

bool LuaFileRef::operator==(const LuaFileRef &other) const {
    return L_ == other.L_ && func_ == other.func_;
}

void LuaFileRef::release() {
    if (L_ == nullptr) {
        return;
    }
    if (thread_ref_ != LUA_REFNIL) {
        lua_unref(L_, thread_ref_);
    }
    if (file_ref_ != LUA_REFNIL) {
        lua_unref(L_, file_ref_);
    }
    thread_ref_ = LUA_REFNIL;
    file_ref_ = LUA_REFNIL;
}

void LuaFileRef::copyFrom(const LuaFileRef &other) {
    L_ = other.L_;
    lua_checkstack(L_, 1);

    // Copy the lua_State reference
    lua_getref(L_, other.thread_ref_);
    thread_ref_ = lua_ref(L_, -1);
    lua_pop(L_, 1);

    // Copy the function reference
    lua_getref(L_, other.file_ref_);
    file_ref_ = lua_ref(L_, -1);
    func_ = LuauUtil::getLuaFunction(L_, -1);
    lua_pop(L_, 1);
}

LuauFile::LuauFile(const QString &path) { setPath(path); }

void LuauFile::setPath(const QString &path) {
    if (path_ == path) {
        return;
    }
    clearBreakPoints();
    path_ = path;
}

QString LuauFile::path() const { return path_; }

QString LuauFile::source() const { return src_; }

void LuauFile::setSource(const QString &source) { src_ = source; }

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
    if (std::find(refs_.begin(), refs_.end(), ref) != refs_.end()) {
        return adjustedLines;
    }

    for (auto &[_, bp] : breakpoints_) {
        const auto originLine = bp.enable(ref.L_, ref.file_ref_, true);
        if (originLine >= 0) {
            if (originLine != bp.line()) {
                adjustedLines.append(qMakePair(originLine, bp.line()));
            }
        } else {
            adjustedLines.append(qMakePair(originLine, -1));
        }
    }
    refs_.emplace_back(std::move(ref));
    return adjustedLines;
}

void LuauFile::removeRef(lua_State *L) {
    auto it =
        std::remove_if(refs_.begin(), refs_.end(), [L](const LuaFileRef &ref) {
            return lua_mainthread(ref.L_) == L;
        });
    refs_.erase(it, refs_.end());
}

bool LuauFile::enableBreakPoint(BreakPoint &bp, bool enable) {
    bool ok = true;
    for (auto &ref : refs_) {
        ok &= bp.enable(ref.L_, ref.file_ref_, enable) >= 0;
    }
    return ok;
}

int LuauFile::addBreakPoint(int line) {
    return addBreakPoint(BreakPoint::create(line));
}

int LuauFile::addBreakPoint(const BreakPoint &bp) {
    auto line = bp.line();
    auto it = breakpoints_.find(line);
    if (it == breakpoints_.end()) {
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
            auto it = breakpoints_.find(nline);
            if (it != breakpoints_.end()) {
                return -1;
            }
        }
        it = breakpoints_.emplace(nline, nbp).first;
    } else {
        it->second = bp;
    }
    return it->second.line();
}

void LuauFile::removeBreakPoint(int line) {
    auto it = breakpoints_.find(line);
    if (it != breakpoints_.end()) {
        enableBreakPoint(it->second, false);
        breakpoints_.erase(it);
    }
}

void LuauFile::clearBreakPoints() {
    for (auto &[line, bp] : breakpoints_) {
        enableBreakPoint(bp, false);
    }
    breakpoints_.clear();
}

BreakPoint *LuauFile::findBreakPoint(int line) {
    auto it = breakpoints_.find(line);
    if (it == breakpoints_.end()) {
        return nullptr;
    }
    return &it->second;
}