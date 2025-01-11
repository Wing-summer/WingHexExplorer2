/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QHash>
#include <QLocale>
#include <QObject>
#include <QStringList>
#include <QTranslator>

class LanguageManager : public QObject {
    Q_OBJECT

public:
    struct LanguageData {
        QByteArray trFiles;
        QString about;
        QString dev;
    };

public:
    static LanguageManager &instance();

    QString langDisplay(const QString &lang) const;

    LanguageData data() const;

    QLocale defaultLocale() const;

    QTranslator *try2LoadPluginLang(const QString &plgID);

    QStringList langs() const;

private:
    LanguageManager();

    Q_DISABLE_COPY(LanguageManager)

private:
    bool unpackTr(const QString &filename, const QLocale &locale);

    void abortAndExit();

private:
    LanguageData _data;

    QLocale _defaultLocale;

    QStringList m_langs;
    QHash<QString, QString> m_langMap;
};

#endif // LANGUAGEMANAGER_H
