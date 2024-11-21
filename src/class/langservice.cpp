#include "langservice.h"

#include "class/ascompletion.h"
#include "class/skinmanager.h"
#include "qdocument.h"
#include "qeditor.h"
#include "qformat.h"
#include "qformatscheme.h"
#include "qsnippetmanager.h"

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

    m_language = new QLanguageFactory(format, this);
    m_language->addDefinitionPath(QStringLiteral(":/qcodeedit"));
    _completion = new AsCompletion(engine, this);
    m_language->addCompletionEngine(_completion);

    m_snippetManager = new QSnippetManager(this);
    m_snipbind = new QSnippetBinding(m_snippetManager);
}

LangService::LangService() : QObject(nullptr) {}

LangService::~LangService() { delete m_snipbind; }

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
