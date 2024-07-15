#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QHash>
#include <QObject>
#include <QStringList>

class LanguageManager : public QObject {
    Q_OBJECT
public:
    static LanguageManager &instance();

private:
    LanguageManager();

private:
    QStringList m_langs;
    QHash<QString, QString> m_langMap;
    QHash<QString, QLocale> m_localeMap;
};

#endif // LANGUAGEMANAGER_H
