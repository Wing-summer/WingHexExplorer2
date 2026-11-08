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

#ifndef LUAUFILE_H
#define LUAUFILE_H

#include <QSharedPointer>
#include <QString>

#include "luau/VM/src/lobject.h"

#include "breakpoint.h"

// Reference to a loaded Lua file
struct LuaFileRef final {
    LuaFileRef(lua_State *L);
    ~LuaFileRef();

    LuaFileRef(const LuaFileRef &other);
    LuaFileRef &operator=(const LuaFileRef &other);
    bool operator==(const LuaFileRef &other) const;

    void release();
    void copyFrom(const LuaFileRef &other);

    lua_State *state = nullptr;
    Closure *func = nullptr;
    int fileRef = LUA_REFNIL;
    int threadRef = LUA_REFNIL;
};

class LuauFile {
    Q_DISABLE_COPY(LuauFile)
public:
    LuauFile() = default;
    LuauFile(const QString &path);

    void setPath(const QString &path);
    QString path() const;

    QString source() const;
    void setSource(const QString &source);

    void setBreakPoints(const std::unordered_map<int, BreakPoint> &breakpoints);
    QVector<QPair<int, int>> addRef(LuaFileRef ref);
    void removeRef(lua_State *L);

    int addBreakPoint(const BreakPoint &bp);
    int addBreakPoint(int line);
    void removeBreakPoint(int line);
    void clearBreakPoints();

    template <class Predicate>
    void removeBreakPointsIf(Predicate pred);

    BreakPoint *findBreakPoint(int line);

private:
    bool enableBreakPoint(BreakPoint &bp, bool enable);

private:
    QString m_path;
    QString m_src;
    std::unordered_map<int, BreakPoint> m_breakpoints;
    std::vector<LuaFileRef> m_refs;
};

using LuauFileContext = QSharedPointer<LuauFile>;

template <class Predicate>
void LuauFile::removeBreakPointsIf(Predicate pred) {
    for (auto it = m_breakpoints.begin(); it != m_breakpoints.end();) {
        if (pred(it->second)) {
            enableBreakPoint(it->second, false);
            it = m_breakpoints.erase(it);
        } else {
            ++it;
        }
    }
}

#endif // LUAUFILE_H