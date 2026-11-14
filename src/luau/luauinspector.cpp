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

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <unordered_map>
#include <vector>

#include <qstringview.h>

#include <lualib.h>

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
    if (!s || len == 0) {
        return false;
    }
    // first char must be underscore or letter
    if (!(std::isalpha(s[0]) || s[0] == '_')) {
        return false;
    }
    for (size_t i = 1; i < len; ++i) {
        char c = s[i];
        if (!(std::isalnum(c) || c == '_')) {
            return false;
        }
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
            if (c == '"') {
                result += QByteArrayLiteral(R"(\")");
            } else {
                result += c;
            }
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
            if (cmp != 0) {
                return cmp < 0;
            }
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
    if (lua_type(L, idx) != LUA_TNUMBER) {
        return false;
    }

    auto n = lua_tonumber(L, idx);
    if (!std::isfinite(n) || std::floor(n) != n || n < 1) {
        return false;
    }

    if (n > lua_Number(std::numeric_limits<int>::max())) {
        return false;
    }

    value = lua_Integer(n);
    return lua_Number(value) == n;
}

// ============================================================================
// Cycle counting
// ============================================================================
void countCyclesRec(lua_State *L, int absIdx, int depth,
                    std::unordered_map<const void *, int> &cycles) {
    if (lua_type(L, absIdx) != LUA_TTABLE) {
        return;
    }

    const void *ptr = lua_topointer(L, absIdx);

    auto it = cycles.find(ptr);
    if (it != cycles.end()) {
        ++it->second;
        return;
    }

    cycles[ptr] = 1;

    if (depth <= 0) {
        return;
    }

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
        : m_l(L), m_options(opts), m_level(0), m_nextIdByType{}, m_items(0),
          m_pretty(opts.getMode() == InspectMode::Pretty) {}

    void putValue(int absIdx, bool quoteString) {
        if (m_truncated) {
            return;
        }

        if (m_items++ >= m_options.maxItems) {
            m_buf += QByteArrayLiteral("...");
            m_truncated = true;
            return;
        }

        int type = lua_type(m_l, absIdx);

        switch (type) {
        case LUA_TSTRING: {
            size_t len = 0;
            const char *s = lua_tolstring(m_l, absIdx, &len);

            if (quoteString) {
                m_buf += smartQuote(escapeString(s, len));
            } else {
                m_buf += QByteArray(s, len);
            }
            break;
        }

        case LUA_TNUMBER: {
            size_t len = 0;
            const char *s = luaL_tolstring(m_l, absIdx, &len);

            if (s) {
                m_buf += QByteArray(s, len);
            }

            lua_pop(m_l, 1);
            break;
        }

        case LUA_TBOOLEAN:
            m_buf += lua_toboolean(m_l, absIdx) ? QByteArrayLiteral("true")
                                                : QByteArrayLiteral("false");
            break;

        case LUA_TNIL:
            m_buf += QByteArrayLiteral("nil");
            break;

        case LUA_TVECTOR: {
            const float *v = lua_tovector(m_l, absIdx);

            if (v) {
                if (LUA_VECTOR_SIZE == 4) {
                    m_buf += QByteArrayLiteral("(");
                    m_buf += QByteArray::number(v[0]);
                    m_buf += QByteArrayLiteral(", ");
                    m_buf += QByteArray::number(v[1]);
                    m_buf += QByteArrayLiteral(", ");
                    m_buf += QByteArray::number(v[2]);
                    m_buf += QByteArrayLiteral(", ");
                    m_buf += QByteArray::number(v[3]);
                    m_buf += QByteArrayLiteral(")");
                } else {
                    m_buf += QByteArrayLiteral("(");
                    m_buf += QByteArray::number(v[0]);
                    m_buf += QByteArrayLiteral(", ");
                    m_buf += QByteArray::number(v[1]);
                    m_buf += QByteArrayLiteral(", ");
                    m_buf += QByteArray::number(v[2]);
                    m_buf += QByteArrayLiteral(")");
                }
            } else {
                m_buf += QByteArrayLiteral("<vector>");
            }
            break;
        }

        case LUA_TBUFFER: {
            size_t len = 0;
            lua_tobuffer(m_l, absIdx, &len);
            m_buf += QByteArrayLiteral("<buffer ") + QByteArray::number(len) +
                     QByteArrayLiteral(">");
            break;
        }

        case LUA_TLIGHTUSERDATA:
            m_buf += QByteArrayLiteral("<") +
                     QByteArray(luaL_typename(m_l, absIdx)) +
                     QByteArrayLiteral(" ") +
                     QByteArray::number(reinterpret_cast<quintptr>(
                                            lua_touserdata(m_l, absIdx)),
                                        16) +
                     QByteArrayLiteral(">");
            break;

        case LUA_TUSERDATA:
            if (luaL_getmetafield(m_l, absIdx, "__tostring")) {
                lua_pushvalue(m_l, absIdx);
                if (lua_pcall(m_l, 1, 1, 0) == LUA_OK &&
                    lua_type(m_l, -1) == LUA_TSTRING) {
                    size_t length = 0;
                    const char *text = lua_tolstring(m_l, -1, &length);
                    m_buf += quoteString
                                 ? smartQuote(escapeString(text, length))
                                 : QByteArray(text, length);
                    lua_pop(m_l, 1);
                    break;
                }
                lua_pop(m_l, 1);
            }
            Q_FALLTHROUGH();
        case LUA_TFUNCTION:
        case LUA_TTHREAD:
        case LUA_TCLASS:
        case LUA_TOBJECT:
            m_buf +=
                QByteArrayLiteral("<") +
                QByteArray(luaL_typename(m_l, absIdx)) +
                QByteArrayLiteral(" ") +
                QByteArray::number(getId(type, lua_topointer(m_l, absIdx))) +
                QByteArrayLiteral(">");
            break;

        case LUA_TTABLE:
            putTable(absIdx);
            break;

        default:
            // Unknown type, use tostring if available
            {
                size_t len = 0;
                const char *s = luaL_tolstring(m_l, absIdx, &len);

                if (s) {
                    m_buf += QByteArray(s, len);
                } else {
                    m_buf += QByteArrayLiteral("<") +
                             QByteArray(luaL_typename(m_l, absIdx)) +
                             QByteArrayLiteral(">");
                }

                lua_pop(m_l, 1);
            }
            break;
        }
    }

public:
    const QByteArray &output() const { return m_buf; }

private:
    struct Entry {
        luabridge::LuaRef key;
        luabridge::LuaRef value;
    };

    lua_State *m_l;
    const InspectOptions &m_options;
    QByteArray m_buf;
    int m_level;
    std::unordered_map<int, int> m_nextIdByType;
    std::unordered_map<int, std::unordered_map<const void *, int>> m_idsByType;
    int m_items;
    bool m_pretty;
    bool m_truncated = false;

    int getId(int type, const void *ptr) {
        auto &typeIds = m_idsByType[type];
        auto it = typeIds.find(ptr);
        if (it != typeIds.end()) {
            return it->second;
        }
        int id = ++m_nextIdByType[type];
        typeIds[ptr] = id;
        return id;
    }

    void tabify() {
        m_buf += m_options.getNewline();
        for (int i = 0; i < m_level; ++i) {
            m_buf += m_options.getIndent();
        }
    }

    void putTable(int absIdx) {
        const void *ptr = lua_topointer(m_l, absIdx);

        // Check if this table has already been output (by id)
        auto &typeIds = m_idsByType[LUA_TTABLE];
        auto existing = typeIds.find(ptr);
        if (existing != typeIds.end()) {
            m_buf += QByteArrayLiteral("<table ") +
                     QByteArray::number(existing->second) +
                     QByteArrayLiteral(">");
            return;
        }

        // Depth limit
        if (m_level >= m_options.depth) {
            m_buf += QByteArrayLiteral("{...}");
            return;
        }

        // Assign id (needed for cycle prefix)
        int id = getId(LUA_TTABLE, ptr);
        // If cycle count > 1, output prefix <id>
        auto cycleIt = cycles.find(ptr);
        if (cycleIt != cycles.end() && cycleIt->second > 1) {
            m_buf += QByteArrayLiteral("<") + QByteArray::number(id) +
                     QByteArrayLiteral(">");
        }

        std::vector<Entry> entries;
        std::unordered_map<lua_Integer, size_t> integerKeys;

        const int initialTop = lua_gettop(m_l);

        for (int iter = 0; (iter = lua_rawiter(m_l, absIdx, iter)) != -1;) {
            // key at -2, value at -1
            const size_t entryIndex = entries.size();

            entries.push_back({luabridge::LuaRef::fromStack(m_l, -2),
                               luabridge::LuaRef::fromStack(m_l, -1)});

            lua_Integer sequenceIndex = 0;
            if (sequenceKey(m_l, lua_absindex(m_l, -2), sequenceIndex)) {
                integerKeys.emplace(sequenceIndex, entryIndex);
            }

            lua_pop(m_l, 2);
        }

        Q_ASSERT(lua_gettop(m_l) == initialTop);

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
        std::sort(mapEntries.begin(), mapEntries.end(),
                  [&](size_t a, size_t b) {
                      entries[a].key.push(m_l);
                      entries[b].key.push(m_l);
                      bool less = compareLuaKeys(m_l, lua_absindex(m_l, -2),
                                                 lua_absindex(m_l, -1));
                      lua_pop(m_l, 2);
                      return less;
                  });

        m_buf += QByteArrayLiteral("{");
        ++m_level;

        bool first = true;
        auto beginElement = [&]() {
            if (first) {
                tabify();
                first = false;
            } else {
                m_buf += QByteArrayLiteral(",");

                if (m_pretty) {
                    tabify();
                } else {
                    m_buf += QByteArrayLiteral(" ");
                }
            }
        };

        // Output sequence part (array-like items)
        for (size_t entryIndex : sequenceEntries) {
            if (m_truncated) {
                break;
            }

            beginElement();
            entries[entryIndex].value.push(m_l);
            putValue(lua_absindex(m_l, -1), true);
            lua_pop(m_l, 1);

            if (m_truncated) {
                break;
            }
        }

        // Output key-value pairs (non-sequence keys)
        for (size_t entryIndex : mapEntries) {
            if (m_truncated) {
                break;
            }

            beginElement();
            entries[entryIndex].key.push(m_l);
            const int keyAbs = lua_absindex(m_l, -1);

            // Output key
            if (lua_type(m_l, keyAbs) == LUA_TSTRING) {
                size_t len;
                const char *s = lua_tolstring(m_l, keyAbs, &len);

                if (isIdentifier(s, len)) {
                    m_buf += QByteArray(s, static_cast<int>(len));
                } else {
                    m_buf += QByteArrayLiteral("[");
                    lua_pushvalue(m_l, keyAbs);
                    putValue(lua_absindex(m_l, -1), true);
                    lua_pop(m_l, 1);
                    m_buf += QByteArrayLiteral("]");
                }
            } else {
                m_buf += QByteArrayLiteral("[");
                lua_pushvalue(m_l, keyAbs);
                putValue(lua_absindex(m_l, -1), true);
                lua_pop(m_l, 1);
                m_buf += QByteArrayLiteral("]");
            }

            if (m_truncated) {
                lua_pop(m_l, 1);
                break;
            }

            m_buf += QByteArrayLiteral(" = ");

            entries[entryIndex].value.push(m_l);
            putValue(lua_absindex(m_l, -1), true);
            lua_pop(m_l, 1);

            lua_pop(m_l, 1);

            if (m_truncated) {
                break;
            }
        }

        // Metatable handling
        if (!m_truncated && lua_getmetatable(m_l, absIdx)) {
            if (lua_type(m_l, -1) == LUA_TTABLE) {
                beginElement();
                m_buf += QByteArrayLiteral("<metatable> = ");
                putValue(lua_absindex(m_l, -1), true);
            }

            lua_pop(m_l, 1);
        }

        --m_level;

        // If Pretty mode and there are elements, add newline and indent before
        // closing '}'
        if (!first) {
            tabify();
        }

        m_buf += QByteArrayLiteral("}");
        Q_ASSERT(lua_gettop(m_l) == initialTop);
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
