#ifndef LANGSERVICE_H
#define LANGSERVICE_H

#include "class/ascompletion.h"
#include "control/scriptingconsole.h"
#include "qlanguagefactory.h"

#include <QObject>

class LangService : public QObject {
    Q_OBJECT

public:
    static LangService &instance();

    void init(ScriptingConsole *console);

    QLanguageFactory *languageFactory() const;

    void applyLanguageSerivce(QEditor *editor);

    static void addAdditionalFormat(QFormatScheme *scheme);

    QHash<QString, QFormatScheme *> formatSchemes() const;

    const QString defaultSchemeName() const;

signals:
    void onConsoleTip(const QString &tip);
    void onScriptEditorTip(const QString &tip);

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
