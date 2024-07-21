#include "languagemanager.h"

#include <QApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

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
        m_langs << lang;
        m_localeMap.insert(lang, locale);
    }

    auto defaultLocale = QLocale::system();
    bool found = false;
    for (auto p = m_localeMap.begin(); p != m_localeMap.end(); ++p) {
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        if (p->territory() == defaultLocale.territory() &&
#else
        if (p->country() == defaultLocale.country() &&
#endif
            p->language() == p->language() &&
            p->script() == defaultLocale.script()) {
            found = true;
            break;
        }
    }
    if (!found) {
        defaultLocale = m_localeMap.value(m_langs.first());
    }

    auto qtPath =
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
        QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif

    auto qtTranslator = new QTranslator(this);
    if (qtTranslator->load(defaultLocale, QStringLiteral("qtbase"),
                           QStringLiteral("_"), qtPath)) {

        qApp->installTranslator(qtTranslator);
    }

    qtTranslator = new QTranslator(this);
    if (qtTranslator->load(defaultLocale, QStringLiteral("qt"),
                           QStringLiteral("_"), qtPath)) {

        qApp->installTranslator(qtTranslator);
    }

    auto translator = new QTranslator(this);
    if (translator->load(defaultLocale, QStringLiteral("ws"),
                         QStringLiteral("_"), langPath)) {
        qApp->installTranslator(translator);
    }

    m_langMap = {{"zh_CN", tr("Chinese(Simplified)")}};

    for (auto &lang : m_langs) {
        m_langsDisplay << m_langMap.value(lang, lang);
    }
}

QStringList LanguageManager::langsDisplay() const { return m_langsDisplay; }
