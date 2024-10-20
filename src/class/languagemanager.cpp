#include "languagemanager.h"

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

LanguageManager::LanguageManager() {
    auto langPath =
        qApp->applicationDirPath() + QDir::separator() + QStringLiteral("lang");

    QDir langDir(langPath);
    Q_ASSERT(langDir.exists());

    auto langFiles = langDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto &langinfo : langFiles) {
        auto lang = langinfo.fileName();
        QLocale locale(lang);
        if (locale == QLocale::C) {
            continue;
        }
        m_langs << lang;
        m_localeMap.insert(lang, locale);
    }

    _defaultLocale = QLocale::system();

    if (m_langs.isEmpty()) {
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        if (QLocale::China == _defaultLocale.territory()
#else
        if (QLocale::China == _defaultLocale.country()
#endif
        ) {
            QMessageBox::critical(
                nullptr, QStringLiteral("程序损坏"),
                QStringLiteral(
                    "语言文件已损坏，请尝试重装软件以解决这个问题。"));
        } else {
            QMessageBox::critical(
                nullptr, QStringLiteral("Corruption"),
                QStringLiteral("The language file has been damaged. "
                               "Please try reinstalling the software to "
                               "solve the problem."));
        }
        qApp->exit(-1);
    }

    bool found = false;
    for (auto p = m_localeMap.begin(); p != m_localeMap.end(); ++p) {
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
        _defaultLocale = m_localeMap.value(m_langs.first());
    }

    auto qtPath =
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
        QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif

    auto qtTranslator = new QTranslator(this);
    if (qtTranslator->load(_defaultLocale, QStringLiteral("qtbase"),
                           QStringLiteral("_"), qtPath)) {

        qApp->installTranslator(qtTranslator);
    }

    qtTranslator = new QTranslator(this);
    if (qtTranslator->load(_defaultLocale, QStringLiteral("qt"),
                           QStringLiteral("_"), qtPath)) {

        qApp->installTranslator(qtTranslator);
    }

    if (unpackTr(langPath + QStringLiteral("/") + _defaultLocale.name() +
                 QStringLiteral("/") + QStringLiteral(LANG_PAK_NAME))) {
        auto translator = new QTranslator(this);
        if (translator->load(
                reinterpret_cast<const uchar *>(_data.trFiles.data()),
                _data.trFiles.size())) {
            qApp->installTranslator(translator);
        }
    } else {
        WingMessageBox::critical(
            nullptr, qAppName(),
            "Translation files loading error! Please try to reinstall.");
    }

    m_langMap = {{"zh_CN", tr("Chinese(Simplified)")}};

    for (auto &lang : m_langs) {
        m_langsDisplay << m_langMap.value(lang, lang);
    }
}

bool LanguageManager::unpackTr(const QString &filename) {
    if (!QFile::exists(filename)) {
        return false;
    }

    QZipReader reader(filename);
    _data = {};
    for (auto &file : reader.fileInfoList()) {
        if (file.isValid() && file.isFile) {
            if (file.filePath.endsWith(QStringLiteral("winghex.qm"))) {
                _data.trFiles = reader.fileData(file.filePath);
            } else if (file.filePath.endsWith(QStringLiteral("about.md"))) {
                _data.about = reader.fileData(file.filePath);
            } else if (file.filePath.endsWith(
                           QStringLiteral("components.md"))) {
                _data.component = reader.fileData(file.filePath);
            } else if (file.filePath.endsWith(QStringLiteral("credits.md"))) {
                _data.credit = reader.fileData(file.filePath);
            } else if (file.filePath.endsWith(QStringLiteral("devs.md"))) {
                _data.dev = reader.fileData(file.filePath);
            } else if (file.filePath.endsWith(QStringLiteral("trans.md"))) {
                _data.trans = reader.fileData(file.filePath);
            }
        }
    }
    reader.close();

    return !_data.trFiles.isEmpty() && !_data.about.isEmpty() &&
           !_data.component.isEmpty() && !_data.credit.isEmpty() &&
           !_data.dev.isEmpty() && !_data.trans.isEmpty();
}

QLocale LanguageManager::defaultLocale() const { return _defaultLocale; }

LanguageManager::LanguageData LanguageManager::data() const { return _data; }

QStringList LanguageManager::langsDisplay() const { return m_langsDisplay; }
