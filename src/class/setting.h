#ifndef SETTING_H
#define SETTING_H

#include <QApplication>
#include <QSettings>

#define HANDLE_CONFIG                                                          \
    QSettings set(QStringLiteral(APP_ORG), QStringLiteral(APP_NAME))

#define CONFIG set

#define WRITE_CONFIG(config, dvalue) set.setValue(config, dvalue)

#define READ_CONFIG(config, dvalue) set.value(config, dvalue)

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

#endif // SETTING_H
