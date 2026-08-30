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

#include "luauutil.h"

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
    if (thread_ref_ != LUA_REFNIL)
        lua_unref(L_, file_ref_);

    if (file_ref_ != LUA_REFNIL)
        lua_unref(L_, thread_ref_);
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

void LuauFile::setPath(const QString &path) { path_ = path; }

QString LuauFile::path() const { return path_; }

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

void LuauFile::addRef(LuaFileRef ref) {
    if (std::find(refs_.begin(), refs_.end(), ref) != refs_.end()) {
        return;
    }

    for (auto &[_, bp] : breakpoints_) {
        bp.enable(ref.L_, ref.file_ref_, true);
    }
    refs_.emplace_back(std::move(ref));
}

void LuauFile::removeRef(lua_State *L) {
    auto it =
        std::remove_if(refs_.begin(), refs_.end(), [L](const LuaFileRef &ref) {
            return lua_mainthread(ref.L_) == L;
        });
    refs_.erase(it, refs_.end());
}

void LuauFile::enableBreakPoint(BreakPoint &bp, bool enable) {
    for (auto &ref : refs_) {
        int target = bp.enable(ref.L_, ref.file_ref_, enable);
    }
}

void LuauFile::addBreakPoint(int line) {
    addBreakPoint(BreakPoint::create(line));
}

void LuauFile::addBreakPoint(const BreakPoint &bp) {
    auto it = breakpoints_.find(bp.line());
    if (it == breakpoints_.end()) {
        auto inserted = breakpoints_.emplace(bp.line(), bp).first;
        enableBreakPoint(inserted->second, true);
    } else {
        it->second = bp;
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