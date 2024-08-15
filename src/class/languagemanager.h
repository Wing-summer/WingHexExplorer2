#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QHash>
#include <QLocale>
#include <QObject>
#include <QStringList>

class LanguageManager : public QObject {
    Q_OBJECT

public:
    struct LanguageData {
        QByteArray trFiles;
        QString about;
        QString component;
        QString dev;
        QString credit;
        QString trans;
    };

public:
    static LanguageManager &instance();

    QStringList langsDisplay() const;

    LanguageData data() const;

    QLocale defaultLocale() const;

private:
    LanguageManager();

    Q_DISABLE_COPY(LanguageManager)

private:
    bool unpackTr(const QString &filename);

private:
    LanguageData _data;

    QLocale _defaultLocale;

    QStringList m_langs;
    QStringList m_langsDisplay;
    QHash<QString, QString> m_langMap;
    QHash<QString, QLocale> m_localeMap;
};

#endif // LANGUAGEMANAGER_H
