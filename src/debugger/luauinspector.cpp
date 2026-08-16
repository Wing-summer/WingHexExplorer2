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

QString LuauInspector::inspect(lua_State *L, int index,
                               const Options &options) {
    if (!L) {
        return {};
    }

    LuauInspector inspector(L, options);
    index = lua_absindex(L, index);
    inspector.countCycles(index, options.maxDepth < 0
                                     ? std::numeric_limits<int>::max()
                                     : options.maxDepth);
    inspector.putValue(index, ValueContext::Root);
    return inspector.m_output;
}

LuauInspector::LuauInspector(lua_State *L, Options options)
    : m_L(L), m_options(std::move(options)) {}

LuauInspector::ObjectKey LuauInspector::objectKey(lua_State *L, int index) {
    index = lua_absindex(L, index);
    const int type = lua_type(L, index);

    switch (type) {
    case LUA_TTABLE:
    case LUA_TFUNCTION:
    case LUA_TUSERDATA:
    case LUA_TTHREAD:
    case LUA_TBUFFER:
    case LUA_TCLASS:
    case LUA_TOBJECT:
#if defined(LUA_TVECTOR)
    case LUA_TVECTOR:
#endif
        return {type, reinterpret_cast<quintptr>(lua_topointer(L, index))};

    default:
        return {type, 0};
    }
}

void LuauInspector::append(const QString &text) { m_output += text; }

void LuauInspector::append(QLatin1Char c) { m_output += c; }

void LuauInspector::tabify() {
    append(m_options.newline);
    for (int i = 0; i < m_level; ++i) {
        append(m_options.indent);
    }
}

bool LuauInspector::isLuaKeyword(const QString &str) {
    static const QSet<QString> keywords = {
        QStringLiteral("and"),      QStringLiteral("break"),
        QStringLiteral("do"),       QStringLiteral("else"),
        QStringLiteral("elseif"),   QStringLiteral("end"),
        QStringLiteral("false"),    QStringLiteral("for"),
        QStringLiteral("function"), QStringLiteral("goto"),
        QStringLiteral("if"),       QStringLiteral("in"),
        QStringLiteral("local"),    QStringLiteral("nil"),
        QStringLiteral("not"),      QStringLiteral("or"),
        QStringLiteral("repeat"),   QStringLiteral("return"),
        QStringLiteral("then"),     QStringLiteral("true"),
        QStringLiteral("until"),    QStringLiteral("while")};

    return keywords.contains(str);
}

bool LuauInspector::isLuaIdentifier(const QString &str) {
    if (str.isEmpty()) {
        return false;
    }

    auto isAlpha = [](QChar c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    };
    auto isDigit = [](QChar c) { return c >= '0' && c <= '9'; };
    auto isIdentChar = [&](QChar c) {
        return c == '_' || isAlpha(c) || isDigit(c);
    };

    const QChar first = str.at(0);
    if (!(first == '_' || isAlpha(first))) {
        return false;
    }
    for (qsizetype i = 1; i < str.size(); ++i) {
        if (!isIdentChar(str.at(i))) {
            return false;
        }
    }

    return !isLuaKeyword(str);
}

QString LuauInspector::escapeBytes(const QByteArray &bytes) {
    QString result;
    result.reserve(bytes.size() * 2);

    for (int i = 0; i < bytes.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(bytes.at(i));

        switch (c) {
        case '\\':
            result += QStringLiteral("\\\\");
            break;

        case '\a':
            result += QStringLiteral("\\a");
            break;

        case '\b':
            result += QStringLiteral("\\b");
            break;

        case '\f':
            result += QStringLiteral("\\f");
            break;

        case '\n':
            result += QStringLiteral("\\n");
            break;

        case '\r':
            result += QStringLiteral("\\r");
            break;

        case '\t':
            result += QStringLiteral("\\t");
            break;

        case '\v':
            result += QStringLiteral("\\v");
            break;

        case 127:
            result += QStringLiteral("\\127");
            break;

        default:
            if (c < 32) {
                result += QStringLiteral("\\%1").arg(static_cast<int>(c), 3, 10,
                                                     QLatin1Char('0'));
            } else {
                result += QChar::fromLatin1(static_cast<char>(c));
            }
            break;
        }
    }

    return result;
}

QString LuauInspector::quote(const QString &escaped) {
    // Same strategy as inspect.lua:
    //
    // if string contains " but not '
    //     use single quotes
    //
    // otherwise use double quotes.

    if (escaped.contains('"') && !escaped.contains('\'')) {
        return '\'' + escaped + '\'';
    }

    QString result;
    result.reserve(escaped.size() + 2);
    result += '"';
    for (const QChar c : escaped) {
        if (c == '"')
            result += QStringLiteral("\\\"");
        else
            result += c;
    }
    result += '"';

    return result;
}

QString LuauInspector::rawLuaString(lua_State *L, int index) {
    size_t length = 0;

    const char *data = lua_tolstring(L, index, &length);

    if (!data)
        return {};

    return QString::fromUtf8(data, qsizetype(length));
}

QString LuauInspector::makeString(lua_State *L, int index,
                                  ValueContext context) {
    size_t length = 0;

    const char *data = lua_tolstring(L, index, &length);

    if (!data)
        return {};

    qsizetype usableLength = qsizetype(length);

    bool truncated = false;

    if (m_options.maxStringLength >= 0 &&
        usableLength > m_options.maxStringLength) {
        usableLength = m_options.maxStringLength;

        truncated = true;
    }

    QByteArray bytes(data, usableLength);

    /*
     * Root + Raw:
     *
     * print("hello")
     *     -> hello
     *
     * Do not escape here either. This is intentionally
     * closer to print() semantics.
     */
    if (context == ValueContext::Root &&
        m_options.stringMode == StringMode::Raw) {
        QString result = QString::fromUtf8(bytes.constData(), bytes.size());

        if (truncated)
            result += QStringLiteral("...");

        return result;
    }

    QString escaped = escapeBytes(bytes);

    if (truncated)
        escaped += QStringLiteral("...");

    return quote(escaped);
}

int LuauInspector::typeOrder(int type) {
    switch (type) {
    case LUA_TNUMBER:
#ifdef LUA_TINTEGER
    case LUA_TINTEGER:
#endif
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

bool LuauInspector::compareKeys(lua_State *L, const KeyInfo &a,
                                const KeyInfo &b) {
    if (a.type == b.type) {
        if (a.type == LUA_TSTRING)
            return a.stringValue < b.stringValue;

        if (a.type == LUA_TNUMBER
#ifdef LUA_TINTEGER
            || a.type == LUA_TINTEGER
#endif
        ) {
            return a.numberValue < b.numberValue;
        }

        return a.objectValue < b.objectValue;
    }

    const int oa = typeOrder(a.type);
    const int ob = typeOrder(b.type);

    if (oa != ob)
        return oa < ob;

    const char *ta = lua_typename(L, a.type);

    const char *tb = lua_typename(L, b.type);

    return qstrcmp(ta, tb) < 0;
}

int LuauInspector::sequenceLength(int index) {
    index = lua_absindex(m_L, index);

    int sequenceLength = 0;

    while (true) {
        lua_rawgeti(m_L, index, sequenceLength + 1);

        const int type = lua_type(m_L, -1);

        lua_pop(m_L, 1);

        if (type == LUA_TNIL)
            break;

        ++sequenceLength;
    }

    return sequenceLength;
}

void LuauInspector::countCycles(int index, int depth) {
    index = lua_absindex(m_L, index);

    if (lua_type(m_L, index) != LUA_TTABLE)
        return;

    countCyclesImpl(index, depth);
}

void LuauInspector::countCyclesImpl(int index, int depth) {
    index = lua_absindex(m_L, index);

    const ObjectKey key = objectKey(m_L, index);

    /*
     * This exactly follows inspect.lua's important
     * semantic:
     *
     *   first occurrence:
     *       cycles[x] = 1
     *
     *   later occurrence:
     *       cycles[x]++
     *
     * and only the first occurrence is recursively
     * traversed.
     */
    auto it = m_cycleExpanded.find(key);

    if (it != m_cycleExpanded.end()) {
        m_cycles[key] = m_cycles.value(key) + 1;
        return;
    }

    m_cycleExpanded.insert(key);
    m_cycles[key] = 1;

    if (depth <= 0)
        return;

    const int top = lua_gettop(m_L);

    lua_pushnil(m_L);

    while (lua_next(m_L, index) != 0) {
        /*
         * stack:
         *
         *   ...
         *   key
         *   value
         */

        countCyclesValue(-2, depth - 1);

        countCyclesValue(-1, depth - 1);

        /*
         * Remove value.
         * Keep key for lua_next().
         */
        lua_pop(m_L, 1);
    }

    /*
     * Metatable.
     */
    if (lua_getmetatable(m_L, index)) {
        countCyclesValue(-1, depth - 1);

        lua_pop(m_L, 1);
    }

    Q_ASSERT(lua_gettop(m_L) == top);

    lua_settop(m_L, top);
}

void LuauInspector::countCyclesValue(int index, int depth) {
    if (lua_type(m_L, index) == LUA_TTABLE) {
        countCyclesImpl(index, depth);
    }
}

QVector<LuauInspector::KeyInfo> LuauInspector::getKeys(int index,
                                                       int sequenceLen) {
    index = lua_absindex(m_L, index);

    QVector<KeyInfo> keys;

    const int top = lua_gettop(m_L);

    lua_pushnil(m_L);

    while (lua_next(m_L, index) != 0) {
        // stack:
        //   ...
        //   key
        //   value

        const int keyIndex = lua_gettop(m_L) - 1;
        const int type = lua_type(m_L, keyIndex);

        bool isSequenceKey = false;

        if (type == LUA_TNUMBER) {
            const lua_Number number = lua_tonumber(m_L, keyIndex);

            isSequenceKey = std::isfinite(number) && number >= 1 &&
                            number <= sequenceLen &&
                            std::floor(number) == number;
        }

        if (!isSequenceKey) {
            KeyInfo info;
            info.type = type;

            switch (type) {
            case LUA_TSTRING:
                info.stringValue = rawLuaString(m_L, keyIndex);
                break;

            case LUA_TNUMBER:
                info.numberValue = lua_tonumber(m_L, keyIndex);
                break;

            default:
                info.objectValue =
                    reinterpret_cast<quintptr>(lua_topointer(m_L, keyIndex));
                break;
            }

            info.registryRef = lua_ref(m_L, keyIndex);
            keys.push_back(std::move(info));
        }

        lua_pop(m_L, 1);
    }

    Q_ASSERT(lua_gettop(m_L) == top);

    std::sort(keys.begin(), keys.end(),
              [this](const KeyInfo &a, const KeyInfo &b) {
                  return compareKeys(m_L, a, b);
              });

    return keys;
}

int LuauInspector::getId(int index) {
    index = lua_absindex(m_L, index);

    const ObjectKey key = objectKey(m_L, index);

    const auto it = m_ids.constFind(key);

    if (it != m_ids.constEnd())
        return it.value();

    const int type = lua_type(m_L, index);

    const int id = m_typeIds.value(type) + 1;

    m_typeIds[type] = id;
    m_ids.insert(key, id);

    return id;
}

QString LuauInspector::objectTypeName(int type) const {
    const char *name = lua_typename(m_L, type);

    if (!name)
        return QStringLiteral("unknown");

    return QString::fromLatin1(name);
}

void LuauInspector::putObject(int index) {
    const int id = getId(index);
    const QString typeName = objectTypeName(lua_type(m_L, index));
    append(QStringLiteral("<%1 %2>").arg(typeName).arg(id));
}

void LuauInspector::putValue(int index, ValueContext context) {
    index = lua_absindex(m_L, index);

    const int type = lua_type(m_L, index);

    switch (type) {
    case LUA_TNIL:
        append(QStringLiteral("nil"));
        return;

    case LUA_TBOOLEAN:
        append(lua_toboolean(m_L, index) ? QStringLiteral("true")
                                         : QStringLiteral("false"));
        return;

    case LUA_TNUMBER:
#ifdef LUA_TINTEGER
    case LUA_TINTEGER:
#endif
    {
        /*
         * lua_tolstring() gives us Luau's textual
         * representation instead of relying on
         * QString formatting.
         */
        size_t length = 0;
        const char *text = lua_tolstring(m_L, index, &length);

        if (text) {
            append(QString::fromUtf8(text, qsizetype(length)));
        }
        return;
    }

    case LUA_TSTRING:
        append(makeString(m_L, index, context));
        return;

    case LUA_TTABLE:
        putTable(index);
        return;

    default:
        putObject(index);
        return;
    }
}

void LuauInspector::putTable(int index) {
    index = lua_absindex(m_L, index);

    const ObjectKey key = objectKey(m_L, index);
    const auto existingId = m_ids.constFind(key);

    if (existingId != m_ids.constEnd()) {
        append(QStringLiteral("<table %1>").arg(existingId.value()));
        return;
    }

    if (m_options.maxDepth >= 0 && m_level >= m_options.maxDepth) {
        append(QStringLiteral("{...}"));
        return;
    }

    const int cycleCount = m_cycles.value(key);

    if (cycleCount > 1) {
        const int id = getId(index);

        append(QStringLiteral("<%1>").arg(id));
    }

    const int sequenceLen = sequenceLength(index);

    QVector<KeyInfo> keys = getKeys(index, sequenceLen);

    const int totalItems = sequenceLen + keys.size();

    if (m_options.maxItems == 0 && totalItems > 0) {
        append(QStringLiteral("{...}"));

        for (const KeyInfo &keyInfo : keys) {
            lua_unref(m_L, keyInfo.registryRef);
        }

        return;
    }

    append(QLatin1Char('{'));

    ++m_level;

    int itemLimit = totalItems;

    if (m_options.maxItems >= 0) {
        itemLimit = std::min(itemLimit, m_options.maxItems);
    }

    for (int i = 1; i <= itemLimit; ++i) {
        if (i > 1)
            append(QLatin1Char(','));

        if (i <= sequenceLen) {
            append(QLatin1Char(' '));
            lua_rawgeti(m_L, index, i);
            putValue(-1, ValueContext::TableValue);
            lua_pop(m_L, 1);
        } else {
            // map key/value
            const KeyInfo &keyInfo = keys.at(i - sequenceLen);
            tabify();
            lua_getref(m_L, keyInfo.registryRef);
            const int keyIndex = lua_gettop(m_L);

            const QString keyString = (keyInfo.type == LUA_TSTRING)
                                          ? rawLuaString(m_L, keyIndex)
                                          : QString();

            if (keyInfo.type == LUA_TSTRING && isLuaIdentifier(keyString)) {
                append(keyString);
            } else {
                append(QLatin1Char('['));
                putValue(keyIndex, ValueContext::TableKey);
                append(QLatin1Char(']'));
            }

            lua_pop(m_L, 1);
            append(QStringLiteral(" = "));
            lua_getref(m_L, keyInfo.registryRef);
            lua_rawget(m_L, index);
            putValue(-1, ValueContext::TableValue);

            // pop value + key
            lua_pop(m_L, 2);
        }
    }

    const bool truncated = itemLimit < totalItems;

    if (truncated) {
        if (itemLimit > 0)
            append(QLatin1Char(','));

        tabify();
        append(QStringLiteral("..."));
    }

    bool hasMetatable = false;

    if (!truncated && lua_getmetatable(m_L, index)) {
        hasMetatable = true;

        if (totalItems > 0)
            append(QLatin1Char(','));

        tabify();

        append(QStringLiteral("<metatable> = "));

        putValue(-1, ValueContext::Metatable);

        lua_pop(m_L, 1);
    }

    --m_level;

    if (truncated || !keys.isEmpty() || hasMetatable) {
        tabify();
    } else if (sequenceLen > 0) {
        append(QLatin1Char(' '));
    }

    append(QLatin1Char('}'));

    for (const KeyInfo &keyInfo : keys) {
        lua_unref(m_L, keyInfo.registryRef);
    }
}
