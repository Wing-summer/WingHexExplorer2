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

#ifndef LUAUINSPECTOR_H
#define LUAUINSPECTOR_H

#include <QByteArray>
#include <QHash>
#include <QSet>
#include <QString>
#include <QVector>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>

#include "lua.h"
#include "lualib.h"

class LuauInspector {
public:
    enum class StringMode {
        Quote, // "hello"
        Raw    // hello
    };

    enum class ValueContext { Root, TableValue, TableKey, Metatable };

    struct Options {
        Options() {}

        // < 0 == unlimited
        int maxDepth = -1;

        // < 0 == unlimited
        // 0 means the table is rendered as {...}
        int maxItems = -1;

        // < 0 == unlimited
        // Applied to the raw Lua string length in bytes.
        int maxStringLength = -1;

        QString newline = QStringLiteral("\n");
        QString indent = QStringLiteral("  ");

        // Only affects root strings.
        //
        // Quote:
        //     inspect("hello") -> "hello"
        //
        // Raw:
        //     print("hello") -> hello
        //
        // Strings inside tables are always quoted.
        StringMode stringMode = StringMode::Quote;
    };

    static QString inspect(lua_State *L, int index,
                           const Options &options = {});

private:
    struct ObjectKey {
        int type = LUA_TNONE;
        quintptr pointer = 0;

        friend bool operator==(const ObjectKey &a,
                               const ObjectKey &b) noexcept {
            return a.type == b.type && a.pointer == b.pointer;
        }

        friend size_t qHash(const ObjectKey &key, size_t seed = 0) noexcept {
            return qHashMulti(seed, key.type, key.pointer);
        }
    };

    struct KeyInfo {
        int registryRef = LUA_NOREF;

        int type = LUA_TNONE;

        QString stringValue;
        lua_Number numberValue = 0;

        quintptr objectValue = 0;
    };

    explicit LuauInspector(lua_State *L, Options options);

private:
    static ObjectKey objectKey(lua_State *L, int index);

private:
    void append(const QString &text);

    void append(QLatin1Char c);

    void tabify();

private:
    static bool isLuaIdentifier(const QString &str);

    static bool isLuaKeyword(const QString &str);

private:
    static QString escapeBytes(const QByteArray &bytes);

    static QString quote(const QString &escaped);

    static QString rawLuaString(lua_State *L, int index);

private:
    QString makeString(lua_State *L, int index, ValueContext context);

private:
    static int typeOrder(int type);

    static bool compareKeys(lua_State *L, const KeyInfo &a, const KeyInfo &b);

private:
    int sequenceLength(int index);

private:
    void countCycles(int index, int depth);

    void countCyclesImpl(int index, int depth);

    void countCyclesValue(int index, int depth);

private:
    QVector<KeyInfo> getKeys(int index, int sequenceLen);

private:
    int getId(int index);

private:
    QString objectTypeName(int type) const;

private:
    void putObject(int index);

private:
    void putValue(int index, ValueContext context);

private:
    void putTable(int index);

private:
    lua_State *m_L = nullptr;

    Options m_options;

    QString m_output;

    int m_level = 0;

    /*
     * How many times a table appeared during the
     * pre-scan.
     */
    QHash<ObjectKey, int> m_cycles;

    /*
     * Tables already recursively expanded during
     * cycle counting.
     */
    QSet<ObjectKey> m_cycleExpanded;

    /*
     * Object -> inspect ID.
     */
    QHash<ObjectKey, int> m_ids;

    /*
     * Per Lua type ID counter.
     *
     * inspect.lua does:
     *
     *   id = (ids[tv] or 0) + 1
     */
    QHash<int, int> m_typeIds;
};

#endif // LUAUINSPECTOR_H
