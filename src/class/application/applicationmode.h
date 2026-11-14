/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#ifndef APPLICATIONMODE_H
#define APPLICATIONMODE_H

#include <QCoreApplication>
#include <QStringList>

class QCommandLineParser;

enum class StartupMode { Full, Headless, Diff, Script };

class StartupOptions {
    Q_DECLARE_TR_FUNCTIONS(StartupOptions)

public:
    static void configureParser(QCommandLineParser &parser);
    static void configureBootstrapParser(QCommandLineParser &parser);
    static StartupOptions parse(QCommandLineParser &parser,
                                const QStringList &arguments);

    inline StartupMode mode() const { return m_mode; }

    inline bool live() const { return m_live; }

    inline const QString &address() const { return m_address; }

    inline quint16 port() const { return m_port; }

    inline const QString &certificate() const { return m_certificate; }

    inline const QString &privateKey() const { return m_privateKey; }

    inline bool informationRequested() const { return m_informationRequested; }

    inline const QStringList &files() const { return m_files; }

    inline const QString &error() const { return m_error; }

private:
    QString m_address = QStringLiteral("127.0.0.1");
    quint16 m_port = 18888;
    QString m_certificate;
    QString m_privateKey;
    StartupMode m_mode = StartupMode::Full;
    bool m_live = false;
    bool m_informationRequested = false;
    QStringList m_files;
    QString m_error;
};

// Process configuration is installed once, before initializing services.
class ApplicationMode {
public:
    static void initialize(const StartupOptions &options);
    static bool headless();
    static bool live();
    static bool hasMainWindow();

private:
    static StartupOptions m_options;
    static bool m_initialized;
};
#endif // APPLICATIONMODE_H
