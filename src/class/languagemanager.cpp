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

#include "class/settingmanager.h"
#include "define.h"
#include "wingmessagebox.h"

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
                 {"zh_TW", QStringLiteral("繁體中文")}};

    auto langPath =
        qApp->applicationDirPath() + QDir::separator() + QStringLiteral("lang");

    QDir langDir(langPath);
    Q_ASSERT(langDir.exists());

    QHash<QString, QLocale> localeMap;
    auto langFiles = langDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto &langinfo : langFiles) {
        auto lang = langinfo.fileName();
        QLocale locale(lang);
        if (locale == QLocale::c()) {
            continue;
        }
        m_langs << lang;
        localeMap.insert(lang, locale);
    }

    auto lang = SettingManager::instance().defaultLang();
    if (lang.isEmpty()) {
        _defaultLocale = QLocale::system();
    } else {
        QLocale locale(lang);
        if (locale == QLocale::c()) {
            _defaultLocale = QLocale::system();
        } else {
            _defaultLocale = locale;
        }
    }

    if (m_langs.isEmpty()) {
        abortAndExit();
    }

    bool found = false;
    for (auto p = localeMap.begin(); p != localeMap.end(); ++p) {
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        if (p->territory() == _defaultLocale.territory() &&
#else
        if (p->country() == _defaultLocale.country() &&
#endif
            p->language() == _defaultLocale.language() &&
            p->script() == _defaultLocale.script()) {
            found = true;
            break;
        }
    }
    if (!found) {
        _defaultLocale = localeMap.value(m_langs.first());
    }

    m_langs = localeMap.keys();

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

    if (unpackTr(langPath + QStringLiteral("/") + _defaultLocale.name() +
                     QStringLiteral("/") + QStringLiteral(LANG_PAK_NAME),
                 _defaultLocale)) {
        auto translator = new QTranslator(qApp);
        if (translator->load(
                reinterpret_cast<const uchar *>(_data.trFiles.data()),
                _data.trFiles.size())) {
            qApp->installTranslator(translator);
        }
    } else {
        abortAndExit();
    }
}

bool LanguageManager::unpackTr(const QString &filename, const QLocale &locale) {
    if (!QFile::exists(filename)) {
        return false;
    }

    QZipReader reader(filename);

    if (reader.count() != 3) {
        return false;
    }

    _data = {};
    for (auto &file : reader.fileInfoList()) {
        if (file.isValid() && file.isFile) {
            if (file.filePath == QStringLiteral("winghex_") + locale.name() +
                                     QStringLiteral(".qm")) {
                _data.trFiles = reader.fileData(file.filePath);
            } else if (file.filePath == QStringLiteral("about.md")) {
                _data.about = reader.fileData(file.filePath);
            } else if (file.filePath == QStringLiteral("devs.md")) {
                _data.dev = reader.fileData(file.filePath);
            }
        }
    }
    reader.close();

    return !_data.trFiles.isEmpty() && !_data.about.isEmpty() &&
           !_data.dev.isEmpty();
}

void LanguageManager::abortAndExit() {
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    if (QLocale::China == _defaultLocale.territory()
#else
    if (QLocale::China == _defaultLocale.country()
#endif
    ) {
        WingMessageBox::critical(
            nullptr, QStringLiteral("程序损坏"),
            QStringLiteral("语言文件已损坏，请尝试重装软件以解决这个问题。"));
    } else {
        WingMessageBox::critical(
            nullptr, QStringLiteral("Corruption"),
            QStringLiteral("The language file has been damaged. "
                           "Please try reinstalling the software to "
                           "solve the problem."));
    }

    throw CrashCode::LanguageFile;
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
