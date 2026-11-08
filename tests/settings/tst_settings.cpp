// SPDX-License-Identifier: AGPL-3.0-or-later
#include "settings/settings.h"

#include <QGlobalStatic>
#include <QTemporaryDir>
#include <QtTest>
#include <memory>
#include <type_traits>
#include <utility>

Q_GLOBAL_STATIC_WITH_ARGS(QString, StaticConfigKey,
                          (QStringLiteral("static.key")))

template <typename T, typename = void>
struct IsConfigKey : std::false_type {};

template <typename T>
struct IsConfigKey<T, std::void_t<decltype(getRealContent(std::declval<T>()))>>
    : std::true_type {};

static_assert(IsConfigKey<QString &>::value);
static_assert(IsConfigKey<const QString &>::value);
static_assert(IsConfigKey<QString &&>::value);
static_assert(IsConfigKey<QString *&>::value);
static_assert(IsConfigKey<const QString *const &>::value);
static_assert(IsConfigKey<decltype(StaticConfigKey) &>::value);
static_assert(IsConfigKey<const std::unique_ptr<QString> &>::value);
static_assert(!IsConfigKey<int>::value);
static_assert(!IsConfigKey<int *>::value);
static_assert(!IsConfigKey<std::unique_ptr<int>>::value);

class SettingsKeyTests : public QObject {
    Q_OBJECT
private slots:
    void supportedKeyTypes() {
        QString key = QStringLiteral("mutable.key");
        const QString fixed = QStringLiteral("const.key");
        auto pointer = &key;
        const auto constPointer = &fixed;
        const auto owned =
            std::make_unique<QString>(QStringLiteral("owned.key"));
        QCOMPARE(getRealContent(key), key);
        QCOMPARE(getRealContent(fixed), fixed);
        QCOMPARE(getRealContent(QStringLiteral("temporary.key")),
                 QStringLiteral("temporary.key"));
        QCOMPARE(getRealContent(pointer), key);
        QCOMPARE(getRealContent(constPointer), fixed);
        QCOMPARE(getRealContent(StaticConfigKey), QStringLiteral("static.key"));
        QCOMPARE(getRealContent(owned), QStringLiteral("owned.key"));
    }

    void settingsMacroRoundTrip() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto file = directory.filePath(QStringLiteral("settings.ini"));
        QString historyKey = QStringLiteral("startup.splash.1.0.1.1.0");
        const QString fixedKey = QStringLiteral("fixed.key");
        const QVariantList history = {qint64(100), qint64(200), qint64(150)};
        {
            QSettings set(file, QSettings::IniFormat);
            WRITE_CONFIG(historyKey, history);
            WRITE_CONFIG(fixedKey, 7);
            WRITE_CONFIG(StaticConfigKey, true);
            WRITE_CONFIG(QStringLiteral("temporary.key"), 9);
            set.sync();
            QCOMPARE(set.status(), QSettings::NoError);
        }
        {
            QSettings set(file, QSettings::IniFormat);
            QCOMPARE(READ_CONFIG(historyKey, {}).toList(), history);
            int number = 0;
            READ_CONFIG_INT(number, fixedKey, -1);
            QCOMPARE(number, 7);
            bool flag = false;
            READ_CONFIG_BOOL(flag, StaticConfigKey, false);
            QVERIFY(flag);
            QCOMPARE(READ_CONFIG(QStringLiteral("temporary.key"), 0).toInt(),
                     9);
            QCOMPARE(READ_CONFIG(QStringLiteral("missing.key"), 42).toInt(),
                     42);
        }
    }
};

QTEST_GUILESS_MAIN(SettingsKeyTests)
#include "tst_settings.moc"
