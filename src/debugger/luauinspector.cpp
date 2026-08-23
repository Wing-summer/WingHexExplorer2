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

#include "luauinspector.h"

#include <lualib.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <qstringview.h>
#include <unordered_map>
#include <vector>

#include <LuaBridge/LuaBridge.h>

namespace {

// ============================================================================
// Helper functions
// ============================================================================

int typeOrder(int type) {
    switch (type) {
    case LUA_TNUMBER:
        return 1;
    case LUA_TBOOLEAN:
        return 2;
    case LUA_TSTRING:
        return 3;
    case LUA_TTABLE:
        return 4;
    case LUA_TFUNCTION:
        return 5;
    case LUA_TUSERDATA:
        return 6;
    case LUA_TTHREAD:
        return 7;
    default:
        return 100;
    }
}

bool isIdentifier(const char *s, size_t len) {
    if (!s || len == 0)
        return false;
    // first char must be underscore or letter
    if (!(std::isalpha(s[0]) || s[0] == '_'))
        return false;
    for (size_t i = 1; i < len; ++i) {
        char c = s[i];
        if (!(std::isalnum(c) || c == '_'))
            return false;
    }
    // Lua keywords are not identifiers
    static const char *keywords[] = {
        "and",      "break",  "do",   "else", "elseif", "end",  "false", "for",
        "function", "goto",   "if",   "in",   "local",  "nil",  "not",   "or",
        "repeat",   "return", "then", "true", "until",  "while"};

    for (auto keyword : keywords) {
        if (strlen(keyword) == len && strncmp(s, keyword, len) == 0) {
            return false;
        }
    }
    return true;
}

QByteArray escapeString(const char *s, size_t len) {
    QByteArray out;
    out.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        auto c = s[i];
        if (c == '\\') {
            out.append('\\');
        } else if (c < 32 || c == 127) {
            // Check if next character is a digit; if so use long escape
            bool nextIsDigit = (i + 1 < len) && std::isdigit(s[i + 1]);
            if (nextIsDigit) {
                out += QByteArrayLiteral(R"(\)") +
                       QByteArray::number(c).rightJustified(3, '0');
            } else {
                switch (c) {
                case '\a':
                    out.append(QByteArrayLiteral(R"(\a)"));
                    break;
                case '\b':
                    out.append(QByteArrayLiteral(R"(\b)"));
                    break;
                case '\f':
                    out.append(QByteArrayLiteral(R"(\f)"));
                    break;
                case '\n':
                    out.append(QByteArrayLiteral(R"(\n)"));
                    break;
                case '\r':
                    out.append(QByteArrayLiteral(R"(\r)"));
                    break;
                case '\t':
                    out.append(QByteArrayLiteral(R"(\t)"));
                    break;
                case '\v':
                    out.append(QByteArrayLiteral(R"(\v)"));
                    break;
                case 127:
                    out.append(QByteArrayLiteral(R"(\127)"));
                    break;
                default: {
                    out += QByteArrayLiteral(R"(\)") +
                           QByteArray::number(c).rightJustified(3, '0');
                    break;
                }
                }
            }
        } else {
            out.append(c);
        }
    }
    return out;
}

QByteArray smartQuote(const QByteArray &escaped) {
    if (escaped.contains('"') && !escaped.contains('\'')) {
        return '\'' + escaped + '\'';
    } else {
        QByteArray result(1, '"');
        for (char c : escaped) {
            if (c == '"')
                result += QByteArrayLiteral(R"(\")");
            else
                result += c;
        }
        result += QByteArrayLiteral(R"(")");
        return result;
    }
}

// Compare two Lua keys that are on the stack at absolute indices idxA and idxB.
// Returns true if keyA < keyB according to inspect.lua's sortKeys.
bool compareLuaKeys(lua_State *L, int idxA, int idxB) {
    int ta = lua_type(L, idxA);
    int tb = lua_type(L, idxB);
    if (ta == tb && (ta == LUA_TSTRING || ta == LUA_TNUMBER)) {
        if (ta == LUA_TNUMBER) {
            return lua_tonumber(L, idxA) < lua_tonumber(L, idxB);
        } else {
            size_t la, lb;
            const char *sa = lua_tolstring(L, idxA, &la);
            const char *sb = lua_tolstring(L, idxB, &lb);
            int cmp = std::strncmp(sa, sb, std::min(la, lb));
            if (cmp != 0)
                return cmp < 0;
            return la < lb;
        }
    }
    int oa = typeOrder(ta);
    int ob = typeOrder(tb);
    if (oa == ob) {
        return std::strcmp(lua_typename(L, ta), lua_typename(L, tb)) < 0;
    }
    return oa < ob;
}

bool sequenceKey(lua_State *L, int idx, lua_Integer &value) {
    if (lua_type(L, idx) != LUA_TNUMBER)
        return false;

    auto n = lua_tonumber(L, idx);
    if (!std::isfinite(n) || std::floor(n) != n || n < 1)
        return false;

    if (n > lua_Number(std::numeric_limits<int>::max()))
        return false;

    value = lua_Integer(n);
    return lua_Number(value) == n;
}

// ============================================================================
// Cycle counting
// ============================================================================
void countCyclesRec(lua_State *L, int absIdx, int depth,
                    std::unordered_map<const void *, int> &cycles) {
    if (lua_type(L, absIdx) != LUA_TTABLE)
        return;

    const void *ptr = lua_topointer(L, absIdx);

    auto it = cycles.find(ptr);
    if (it != cycles.end()) {
        ++it->second;
        return;
    }

    cycles[ptr] = 1;

    if (depth <= 0)
        return;

    const int baseTop = lua_gettop(L);

    for (int iter = 0; (iter = lua_rawiter(L, absIdx, iter)) != -1;) {
        // key at -2, value at -1
        countCyclesRec(L, lua_absindex(L, -2), depth - 1, cycles);
        countCyclesRec(L, lua_absindex(L, -1), depth - 1, cycles);
        lua_pop(L, 2);
    }

    // Metatable
    if (lua_getmetatable(L, absIdx)) {
        countCyclesRec(L, lua_absindex(L, -1), depth - 1, cycles);
        lua_pop(L, 1);
    }

    Q_ASSERT(lua_gettop(L) == baseTop);
}

// ============================================================================
// Inspector class
// ============================================================================

class Inspector {
public:
    Inspector(lua_State *L, const InspectOptions &opts)
        : L(L), options(opts), level(0), nextIdByType{}, items(0),
          pretty(opts.getMode() == InspectMode::Pretty) {}

    void putValue(int absIdx, bool quoteString) {
        if (truncated)
            return;

        if (items++ >= options.maxItems) {
            buf += QByteArrayLiteral("...");
            truncated = true;
            return;
        }

        int type = lua_type(L, absIdx);

        switch (type) {
        case LUA_TSTRING: {
            size_t len = 0;
            const char *s = lua_tolstring(L, absIdx, &len);

            if (quoteString) {
                buf += smartQuote(escapeString(s, len));
            } else {
                buf += QByteArray(s, len);
            }
            break;
        }

        case LUA_TNUMBER:
        case LUA_TINTEGER: {
            size_t len = 0;
            const char *s = luaL_tolstring(L, absIdx, &len);

            if (s) {
                buf += QByteArray(s, len);
            }

            lua_pop(L, 1);
            break;
        }

        case LUA_TBOOLEAN:
            buf += lua_toboolean(L, absIdx) ? QByteArrayLiteral("true")
                                            : QByteArrayLiteral("false");
            break;

        case LUA_TNIL:
            buf += QByteArrayLiteral("nil");
            break;

        case LUA_TVECTOR: {
            const float *v = lua_tovector(L, absIdx);

            if (v) {
                if (LUA_VECTOR_SIZE == 4) {
                    buf += QByteArrayLiteral("(");
                    buf += QByteArray::number(v[0]);
                    buf += QByteArrayLiteral(", ");
                    buf += QByteArray::number(v[1]);
                    buf += QByteArrayLiteral(", ");
                    buf += QByteArray::number(v[2]);
                    buf += QByteArrayLiteral(", ");
                    buf += QByteArray::number(v[3]);
                    buf += QByteArrayLiteral(")");
                } else {
                    buf += QByteArrayLiteral("(");
                    buf += QByteArray::number(v[0]);
                    buf += QByteArrayLiteral(", ");
                    buf += QByteArray::number(v[1]);
                    buf += QByteArrayLiteral(", ");
                    buf += QByteArray::number(v[2]);
                    buf += QByteArrayLiteral(")");
                }
            } else {
                buf += QByteArrayLiteral("<vector>");
            }
            break;
        }

        case LUA_TBUFFER: {
            size_t len = 0;
            lua_tobuffer(L, absIdx, &len);
            buf += QByteArrayLiteral("<buffer ") + QByteArray::number(len) +
                   QByteArrayLiteral(">");
            break;
        }

        case LUA_TLIGHTUSERDATA:
            buf +=
                QByteArrayLiteral("<") + QByteArray(luaL_typename(L, absIdx)) +
                QByteArrayLiteral(" ") +
                QByteArray::number(
                    reinterpret_cast<quintptr>(lua_touserdata(L, absIdx)), 16) +
                QByteArrayLiteral(">");
            break;

        case LUA_TFUNCTION:
        case LUA_TUSERDATA:
        case LUA_TTHREAD:
        case LUA_TCLASS:
        case LUA_TOBJECT:
            buf += QByteArrayLiteral("<") +
                   QByteArray(luaL_typename(L, absIdx)) +
                   QByteArrayLiteral(" ") +
                   QByteArray::number(getId(type, lua_topointer(L, absIdx))) +
                   QByteArrayLiteral(">");
            break;

        case LUA_TTABLE:
            putTable(absIdx);
            break;

        default:
            // Unknown type, use tostring if available
            {
                size_t len = 0;
                const char *s = luaL_tolstring(L, absIdx, &len);

                if (s) {
                    buf += QByteArray(s, len);
                } else {
                    buf += QByteArrayLiteral("<") +
                           QByteArray(luaL_typename(L, absIdx)) +
                           QByteArrayLiteral(">");
                }

                lua_pop(L, 1);
            }
            break;
        }
    }

public:
    const QByteArray &output() const { return buf; }

private:
    struct Entry {
        luabridge::LuaRef key;
        luabridge::LuaRef value;
    };

    lua_State *L;
    const InspectOptions &options;
    QByteArray buf;
    int level;
    std::unordered_map<int, int> nextIdByType;
    std::unordered_map<int, std::unordered_map<const void *, int>> idsByType;
    int items;
    bool pretty;
    bool truncated = false;

    int getId(int type, const void *ptr) {
        auto &typeIds = idsByType[type];
        auto it = typeIds.find(ptr);
        if (it != typeIds.end())
            return it->second;
        int id = ++nextIdByType[type];
        typeIds[ptr] = id;
        return id;
    }

    void tabify() {
        buf += options.getNewline();
        for (int i = 0; i < level; ++i) {
            buf += options.getIndent();
        }
    }

    void putTable(int absIdx) {
        const void *ptr = lua_topointer(L, absIdx);

        // Check if this table has already been output (by id)
        auto &typeIds = idsByType[LUA_TTABLE];
        auto existing = typeIds.find(ptr);
        if (existing != typeIds.end()) {
            buf += QByteArrayLiteral("<table ") +
                   QByteArray::number(existing->second) +
                   QByteArrayLiteral(">");
            return;
        }

        // Depth limit
        if (level >= options.depth) {
            buf += QByteArrayLiteral("{...}");
            return;
        }

        // Assign id (needed for cycle prefix)
        int id = getId(LUA_TTABLE, ptr);
        // If cycle count > 1, output prefix <id>
        auto cycleIt = cycles.find(ptr);
        if (cycleIt != cycles.end() && cycleIt->second > 1) {
            buf += QByteArrayLiteral("<") + QByteArray::number(id) +
                   QByteArrayLiteral(">");
        }

        std::vector<Entry> entries;
        std::unordered_map<lua_Integer, size_t> integerKeys;

        const int initialTop = lua_gettop(L);

        for (int iter = 0; (iter = lua_rawiter(L, absIdx, iter)) != -1;) {
            // key at -2, value at -1
            const size_t entryIndex = entries.size();

            entries.push_back({luabridge::LuaRef::fromStack(L, -2),
                               luabridge::LuaRef::fromStack(L, -1)});

            lua_Integer sequenceIndex = 0;
            if (sequenceKey(L, lua_absindex(L, -2), sequenceIndex)) {
                integerKeys.emplace(sequenceIndex, entryIndex);
            }

            lua_pop(L, 2);
        }

        Q_ASSERT(lua_gettop(L) == initialTop);

        std::vector<size_t> sequenceEntries;
        std::vector<size_t> mapEntries;

        for (lua_Integer i = 1;; ++i) {
            auto it = integerKeys.find(i);
            if (it == integerKeys.end()) {
                break;
            }

            sequenceEntries.push_back(it->second);
        }

        std::vector<bool> isSequence(entries.size(), false);

        for (size_t entryIndex : sequenceEntries) {
            isSequence[entryIndex] = true;
        }

        for (size_t i = 0; i < entries.size(); ++i) {
            if (!isSequence[i]) {
                mapEntries.push_back(i);
            }
        }

        // Sort keys
        std::sort(
            mapEntries.begin(), mapEntries.end(), [&](size_t a, size_t b) {
                entries[a].key.push(L);
                entries[b].key.push(L);
                bool less =
                    compareLuaKeys(L, lua_absindex(L, -2), lua_absindex(L, -1));
                lua_pop(L, 2);
                return less;
            });

        buf += QByteArrayLiteral("{");
        ++level;

        bool first = true;
        auto beginElement = [&]() {
            if (first) {
                tabify();
                first = false;
            } else {
                buf += QByteArrayLiteral(",");

                if (pretty) {
                    tabify();
                } else {
                    buf += QByteArrayLiteral(" ");
                }
            }
        };

        // Output sequence part (array-like items)
        for (size_t entryIndex : sequenceEntries) {
            if (truncated) {
                break;
            }

            beginElement();
            entries[entryIndex].value.push(L);
            putValue(lua_absindex(L, -1), true);
            lua_pop(L, 1);

            if (truncated) {
                break;
            }
        }

        // Output key-value pairs (non-sequence keys)
        for (size_t entryIndex : mapEntries) {
            if (truncated) {
                break;
            }

            beginElement();
            entries[entryIndex].key.push(L);
            const int keyAbs = lua_absindex(L, -1);

            // Output key
            if (lua_type(L, keyAbs) == LUA_TSTRING) {
                size_t len;
                const char *s = lua_tolstring(L, keyAbs, &len);

                if (isIdentifier(s, len)) {
                    buf += QByteArray(s, static_cast<int>(len));
                } else {
                    buf += QByteArrayLiteral("[");
                    lua_pushvalue(L, keyAbs);
                    putValue(lua_absindex(L, -1), true);
                    lua_pop(L, 1);
                    buf += QByteArrayLiteral("]");
                }
            } else {
                buf += QByteArrayLiteral("[");
                lua_pushvalue(L, keyAbs);
                putValue(lua_absindex(L, -1), true);
                lua_pop(L, 1);
                buf += QByteArrayLiteral("]");
            }

            if (truncated) {
                lua_pop(L, 1);
                break;
            }

            buf += QByteArrayLiteral(" = ");

            entries[entryIndex].value.push(L);
            putValue(lua_absindex(L, -1), true);
            lua_pop(L, 1);

            lua_pop(L, 1);

            if (truncated) {
                break;
            }
        }

        // Metatable handling
        if (!truncated && lua_getmetatable(L, absIdx)) {
            if (lua_type(L, -1) == LUA_TTABLE) {
                beginElement();
                buf += QByteArrayLiteral("<metatable> = ");
                putValue(lua_absindex(L, -1), true);
            }

            lua_pop(L, 1);
        }

        --level;

        // If Pretty mode and there are elements, add newline and indent before
        // closing '}'
        if (!first) {
            tabify();
        }

        buf += QByteArrayLiteral("}");
        Q_ASSERT(lua_gettop(L) == initialTop);
    }

public:
    std::unordered_map<const void *, int> cycles;
};

} // namespace

// ============================================================================
// Public API
// ============================================================================

QString LuauInspector::inspect(lua_State *L, int index,
                               const InspectOptions &options) {
    const int baseTop = lua_gettop(L);
    const int absIndex = lua_absindex(L, index);

    std::unordered_map<const void *, int> cycles;
    countCyclesRec(L, absIndex, options.depth, cycles);

    Inspector inspector(L, options);
    inspector.cycles = std::move(cycles);
    inspector.putValue(absIndex, options.quoteString);

    const QByteArray result = inspector.output();
    Q_ASSERT(lua_gettop(L) == baseTop);
    lua_settop(L, baseTop);
    return QString::fromUtf8(result);
}

QString LuauInspector::inspect(lua_State *L, int index, InspectMode mode) {
    InspectOptions options(mode);
    return inspect(L, index, options);
}
