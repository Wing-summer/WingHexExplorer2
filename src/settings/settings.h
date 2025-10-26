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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QApplication>
#include <QSettings>
#include <QString>

inline QString getRealContent(const QString &value) { return value; }

template <typename T>
inline QString getRealContent(T &value) {
    static_assert(std::is_same<QString &, decltype(*value)>());
    return *value;
}

#define HANDLE_CONFIG                                                          \
    Q_ASSERT(!QApplication::organizationName().isEmpty() &&                    \
             !QApplication::applicationName().isEmpty());                      \
    QSettings set(QStringLiteral(APP_ORG), QStringLiteral(APP_NAME))

#define CONFIG set

#define WRITE_CONFIG(config, dvalue)                                           \
    set.setValue(getRealContent(config), dvalue);

#define READ_CONFIG(config, dvalue) set.value(getRealContent(config), dvalue)

#define READ_CONFIG_SAFE(var, config, dvalue, func)                            \
    {                                                                          \
        auto b = false;                                                        \
        var = READ_CONFIG(config, dvalue).func(&b);                            \
        if (!b) {                                                              \
            var = dvalue;                                                      \
        }                                                                      \
    }

#define READ_CONFIG_INT(var, config, dvalue)                                   \
    READ_CONFIG_SAFE(var, config, dvalue, toInt)

#define READ_CONFIG_STRING(var, config, dvalue)                                \
    var = READ_CONFIG(config, dvalue).toString()

#define READ_CONFIG_BOOL(var, config, dvalue)                                  \
    var = READ_CONFIG(config, dvalue).toBool()

#define READ_CONFIG_QSIZETYPE(var, config, dvalue)                             \
    READ_CONFIG_SAFE(var, config, dvalue, toLongLong)

#define READ_CONFIG_INT_POSITIVE(var, config, dvalue)                          \
    {                                                                          \
        Q_ASSERT(dvalue > 0);                                                  \
        READ_CONFIG_SAFE(var, config, dvalue, toInt);                          \
        if (var <= 0) {                                                        \
            var = dvalue;                                                      \
        }                                                                      \
    }

#define READ_CONFIG_DOUBLE_POSITIVE(var, config, dvalue)                       \
    {                                                                          \
        Q_ASSERT(dvalue > 0);                                                  \
        READ_CONFIG_SAFE(var, config, dvalue, toDouble);                       \
        if (var <= 0) {                                                        \
            var = dvalue;                                                      \
        }                                                                      \
    }

#endif // SETTINGS_H
