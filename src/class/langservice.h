#ifndef LANGSERVICE_H
#define LANGSERVICE_H

#include "class/ascompletion.h"
#include "qlanguagefactory.h"

#include "angelscript.h"

#include <QObject>

class LangService : QObject {
    Q_OBJECT

public:
    static LangService &instance();

    void init(asIScriptEngine *engine);

    QLanguageFactory *languageFactory() const;

    void applyLanguageSerivce(QEditor *editor);

    static void addAdditionalFormat(QFormatScheme *scheme);

    QHash<QString, QFormatScheme *> formatSchemes() const;

    const QString defaultSchemeName() const;

private:
    LangService();
    ~LangService();

private:
    void initAdditionalFormatScheme();

private:
    QLanguageFactory *m_language = nullptr;

    AsCompletion *_completion = nullptr;

    QHash<QString, QFormatScheme *> _formatSchemes;

    Q_DISABLE_COPY_MOVE(LangService)
};

#endif // LANGSERVICE_H
