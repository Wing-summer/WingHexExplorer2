#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QStringList>

class LanguageManager {
public:
    static LanguageManager &instance();

private:
    LanguageManager();

private:
    QStringList m_langs;
};

#endif // LANGUAGEMANAGER_H
