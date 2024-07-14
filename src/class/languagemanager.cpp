#include "languagemanager.h"

#include <QApplication>
#include <QDir>
#include <QLocale>

LanguageManager &LanguageManager::instance() {
    static LanguageManager ins;
    return ins;
}

LanguageManager::LanguageManager() {
    auto langPath =
        qApp->applicationDirPath() + QDir::separator() + QStringLiteral("lang");

    QDir langDir(langPath);
    Q_ASSERT(langDir.exists());

    auto langFiles =
        langDir.entryInfoList({"ws_*.qm"}, QDir::Files | QDir::NoDotAndDotDot);
    for (auto &langinfo : langFiles) {
        auto langName = langinfo.fileName();
        auto lang = langName.mid(3, langName.length() - 3 - 3);
        QLocale locale(lang);
        if (locale == QLocale::C) {
            continue;
        }
        // TODO
        m_langs << QLocale::countryToString(locale.country());
    }
}
