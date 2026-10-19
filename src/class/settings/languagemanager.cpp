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

#include "languagemanager.h"

#include "class/settings/settingmanager.h"
#include <QDebug>
#include <QFile>
#include <QMap>
#include <utility>

#include <QApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QTranslator>

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 2)
#include <QtCore/private/qzipreader_p.h>
#else
#include <QtGui/private/qzipreader_p.h>
#endif

LanguageManager &LanguageManager::instance() {
    static LanguageManager ins;
    return ins;
}

QString LanguageManager::langDisplay(const QString &lang) const {
    return m_langMap.value(lang, lang);
}

LanguageManager::LanguageManager() {
    m_langMap = {{"zh_CN", QStringLiteral("简体中文")},
                 {"zh_TW", QStringLiteral("繁體中文")},
                 {"en_US", QStringLiteral("English")}};

    // English is built in and must work without an external language directory.
    const QLocale english(QStringLiteral("en_US"));
    _defaultLocale = english;
    const auto readResource = [](const QString &path) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            return QString::fromUtf8(file.readAll());
        }
        qWarning("Cannot read built-in language resource: %s",
                 qUtf8Printable(path));
        return QString();
    };
    _data.about = readResource(QStringLiteral(":/i18n/en/about.md"));
    _data.dev = readResource(QStringLiteral(":/i18n/en/devs.md"));

    const QString langPath =
        qApp->applicationDirPath() + QStringLiteral("/lang");
    QMap<QString, QLocale> localeMap;
    const auto langFiles =
        QDir(langPath).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const auto &langinfo : langFiles) {
        const auto lang = langinfo.fileName();
        const QLocale locale(lang);
        if (locale.language() == QLocale::C ||
            locale.language() == QLocale::English) {
            continue;
        }
        if (QFile::exists(langinfo.absoluteFilePath() + QStringLiteral("/") +
                          QStringLiteral(LANG_PAK_NAME))) {
            localeMap.insert(lang, locale);
        }
    }
    m_langs = localeMap.keys();
    m_langs.prepend(QStringLiteral("en_US"));

    const auto configured = SettingManager::instance().defaultLang();
    const QLocale requested =
        configured.isEmpty() ? QLocale::system() : QLocale(configured);
    if (requested.language() != QLocale::English &&
        requested.language() != QLocale::C) {
        const auto key = requested.name();
        const auto available = localeMap.value(key, english);
        if (localeMap.contains(key) &&
            available.language() == requested.language() &&
            available.script() == requested.script() &&
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            available.territory() == requested.territory()) {
#else
            available.country() == requested.country()) {
#endif
            LanguageData candidate;
            const auto package = langPath + QStringLiteral("/") + key +
                                 QStringLiteral("/") +
                                 QStringLiteral(LANG_PAK_NAME);
            auto translator = new QTranslator(qApp);
            if (unpackTr(package, requested, candidate) &&
                translator->load(reinterpret_cast<const uchar *>(
                                     candidate.trFiles.constData()),
                                 candidate.trFiles.size()) &&
                qApp->installTranslator(translator)) {
                _data = std::move(candidate);
                _defaultLocale = requested;
            } else {
                qWarning("Cannot load language package; using English: %s",
                         qUtf8Printable(package));
                delete translator;
            }
        } else {
            qWarning("Unsupported UI language; using English: %s",
                     qUtf8Printable(requested.name()));
        }
    } else if (!configured.isEmpty() && requested.language() == QLocale::C) {
        qWarning("Invalid UI language setting; using English: %s",
                 qUtf8Printable(configured));
    }

    auto qtPath =
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
        QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif

    auto qtTranslator = new QTranslator(qApp);
    if (qtTranslator->load(_defaultLocale, QStringLiteral("qtbase"),
                           QStringLiteral("_"), qtPath)) {

        qApp->installTranslator(qtTranslator);
    }

    qtTranslator = new QTranslator(qApp);
    if (qtTranslator->load(_defaultLocale, QStringLiteral("qt"),
                           QStringLiteral("_"), qtPath)) {

        qApp->installTranslator(qtTranslator);
    }
}

bool LanguageManager::unpackTr(const QString &filename, const QLocale &locale,
                               LanguageData &data) {
    if (!QFile::exists(filename)) {
        return false;
    }

    QZipReader reader(filename);

    if (reader.count() != 3) {
        return false;
    }

    data = {};
    const auto finfos = reader.fileInfoList();
    for (const auto &file : finfos) {
        if (file.isValid() && file.isFile) {
            if (file.filePath == QStringLiteral("winghex_") + locale.name() +
                                     QStringLiteral(".qm")) {
                data.trFiles = reader.fileData(file.filePath);
            } else if (file.filePath == QStringLiteral("about.md")) {
                data.about = QString::fromUtf8(reader.fileData(file.filePath));
            } else if (file.filePath == QStringLiteral("devs.md")) {
                data.dev = QString::fromUtf8(reader.fileData(file.filePath));
            }
        }
    }
    reader.close();

    return reader.status() == QZipReader::NoError && !data.trFiles.isEmpty() &&
           !data.about.isEmpty() && !data.dev.isEmpty();
}

QStringList LanguageManager::langs() const { return m_langs; }

QLocale LanguageManager::defaultLocale() const { return _defaultLocale; }

QTranslator *LanguageManager::try2LoadPluginLang(const QString &plgID) {
    QDir langDir(QStringLiteral(":/PLGLANG"));
    if (!langDir.cd(plgID)) {
        return nullptr;
    }
    auto translator = new QTranslator(qApp);
    if (translator->load(_defaultLocale, plgID, QStringLiteral("_"),
                         langDir.absolutePath())) {
        if (qApp->installTranslator(translator)) {
            return translator;
        }
    }

    translator->deleteLater();
    return nullptr;
}

LanguageManager::LanguageData LanguageManager::data() const { return _data; }
