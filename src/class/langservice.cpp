#include "langservice.h"

#include "class/ascompletion.h"
#include "class/skinmanager.h"
#include "qdocument.h"
#include "qeditor.h"
#include "qformat.h"
#include "qformatscheme.h"
#include "qsnippetmanager.h"
#include "utilities.h"

LangService &LangService::instance() {
    static LangService ins;
    return ins;
}

void LangService::init(asIScriptEngine *engine) {
    QFormatScheme *format = nullptr;

    switch (SkinManager::instance().currentTheme()) {
    case SkinManager::Theme::Dark:
        format =
            new QFormatScheme(QStringLiteral(":/qcodeedit/as_dark.qxf"), this);
        break;
    case SkinManager::Theme::Light:
        format =
            new QFormatScheme(QStringLiteral(":/qcodeedit/as_light.qxf"), this);
        break;
    }

    addAdditionalFormat(format);
    QDocument::setDefaultFormatScheme(format);
    _formatSchemes.insert(defaultSchemeName(), format);

    m_language = new QLanguageFactory(format, this);
    m_language->addDefinitionPath(QStringLiteral(":/qcodeedit"));
    _completion = new AsCompletion(engine, this);
    m_language->addCompletionEngine(_completion);

    m_snippetManager = new QSnippetManager(this);
    QDir snippetDir(Utilities::getAppDataPath());
    const auto snippetName = QStringLiteral("snippets");
    if (!snippetDir.exists(snippetName)) {
        snippetDir.mkdir(snippetName);
    }
    if (snippetDir.cd(snippetName)) {
        m_snippetManager->loadSnippetsFromDirectory(snippetDir.absolutePath());
    }

    m_snipbind = new QSnippetBinding(m_snippetManager);

    initAdditionalFormatScheme();
}

LangService::LangService() : QObject(nullptr) {}

LangService::~LangService() {
    delete m_snipbind;
    m_snipbind = nullptr;
    qDeleteAll(_formatSchemes);
}

void LangService::initAdditionalFormatScheme() {
    QDir appDataDir(Utilities::getAppDataPath());
    const auto dirName = QStringLiteral("scheme");
    appDataDir.mkdir(dirName);
    if (appDataDir.exists(dirName)) {
        appDataDir.cd(dirName);
        for (auto &sch :
             appDataDir.entryInfoList({QStringLiteral("*.qxf")}, QDir::Files)) {
            auto fmt = new QFormatScheme(sch.absoluteFilePath());
            if (fmt->formatCount()) {
                addAdditionalFormat(fmt);
                _formatSchemes.insert(sch.baseName(), fmt);
            } else {
                delete fmt;
            }
        }
    }
}

QHash<QString, QFormatScheme *> LangService::formatSchemes() const {
    return _formatSchemes;
}

const QString LangService::defaultSchemeName() const { return tr("Default"); }

void LangService::saveSnippets() {
    QDir snippetDir(Utilities::getAppDataPath());
    const auto snippetName = QStringLiteral("snippets");
    if (!snippetDir.exists(snippetName)) {
        snippetDir.mkdir(snippetName);
    }
    if (snippetDir.cd(snippetName)) {
        m_snippetManager->saveSnippetsToDirectory(snippetDir.absolutePath());
    }
}

QSnippetManager *LangService::snippetManager() const {
    return m_snippetManager;
}

void LangService::addAdditionalFormat(QFormatScheme *scheme) {
    if (scheme) {
        QFormat fmt;
        fmt.foreground = Qt::red;
        scheme->setFormat(QStringLiteral("stderr"), fmt);
        fmt.foreground = QColorConstants::Svg::gold;
        scheme->setFormat(QStringLiteral("stdwarn"), fmt);
        fmt.foreground = Qt::green;
        scheme->setFormat(QStringLiteral("stdout"), fmt);
    }
}

QLanguageFactory *LangService::languageFactory() const { return m_language; }

void LangService::applyLanguageSerivce(QEditor *editor) {
    Q_ASSERT(editor);
    m_language->setLanguage(editor, QStringLiteral("AngelScript"));
    editor->addInputBinding(m_snipbind);
}

QSnippetBinding *LangService::snippetBinding() const { return m_snipbind; }
