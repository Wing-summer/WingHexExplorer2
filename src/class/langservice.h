#ifndef LANGSERVICE_H
#define LANGSERVICE_H

#include "class/ascompletion.h"
#include "qlanguagefactory.h"

#include "angelscript.h"
#include "qsnippetmanager.h"

#include <QObject>

class LangService : QObject {
    Q_OBJECT

public:
    static LangService &instance();

    void init(asIScriptEngine *engine);

    QLanguageFactory *languageFactory() const;

    void applyLanguageSerivce(QEditor *editor);

    QSnippetManager *snippetManager() const;

    static void addAdditionalFormat(QFormatScheme *scheme);

    QHash<QString, QFormatScheme *> formatSchemes() const;

    const QString defaultSchemeName() const;

    void saveSnippets();

private:
    LangService();
    ~LangService();

private:
    void initAdditionalFormatScheme();

private:
    QLanguageFactory *m_language = nullptr;
    QSnippetManager *m_snippetManager = nullptr;

    AsCompletion *_completion = nullptr;

    QHash<QString, QFormatScheme *> _formatSchemes;

    Q_DISABLE_COPY_MOVE(LangService)
};

#endif // LANGSERVICE_H
