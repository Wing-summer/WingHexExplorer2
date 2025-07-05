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

#include "crashhandler.h"
#include "angelscript.h"
#include "class/pluginsystem.h"
#include "dialog/crashreport.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QTextStream>

#include <cpptrace/cpptrace.hpp>

#include <cpptrace/formatting.hpp>
#include <csignal>

#ifdef Q_OS_WIN
#include <Windows.h>

#include <cpptrace/from_current.hpp>

LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS *) {
    CrashHandler::reportCrashAndExit();
    // Terminate the program
    return EXCEPTION_EXECUTE_HANDLER;
}

#else
#include <unistd.h>
#endif

void signalHandler(int) { CrashHandler::reportCrashAndExit(); }

CrashHandler &CrashHandler::instance() {
    static CrashHandler ins;
    return ins;
}

void CrashHandler::init() {
#ifdef Q_OS_WIN
    ::SetUnhandledExceptionFilter(ExceptionFilter);
    std::signal(SIGABRT, signalHandler);
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGILL, signalHandler);
    std::signal(SIGFPE, signalHandler);
#else
    ::signal(SIGSEGV, signalHandler);
    ::signal(SIGILL, signalHandler);
    ::signal(SIGABRT, signalHandler);
    ::signal(SIGFPE, signalHandler);
    ::signal(SIGBUS, signalHandler);
#endif
}

CrashHandler::CrashHandler() {}

void CrashHandler::reportCrashAndExit() {
    QString buffer;
    QTextStream ss(&buffer);

    ss << QStringLiteral("Basic Infomation:") << Qt::endl;
    ss << QStringLiteral("* Qt: ") << QLibraryInfo::build() << Qt::endl;
    ss << QStringLiteral("* OS: ") << QSysInfo::prettyProductName() << Qt::endl;
    ss << QStringLiteral("* Version: ") << WINGHEX_VERSION << Qt::endl;
    ss << QStringLiteral("* ScriptEngine: ") << ANGELSCRIPT_VERSION_STRING
       << Qt::endl;

    ss << Qt::endl;

    // we should keep this exception free
    auto &plgsys = PluginSystem::instance();
    ss << QStringLiteral("Plugin System:") << Qt::endl;
    ss << QStringLiteral("* Loading: ") << plgsys.currentLoadingPlugin()
       << Qt::endl;

    size_t i = 1;
    ss << QStringLiteral("* Plugins:") << Qt::endl;
    for (auto &plg : plgsys.plugins()) {
        auto info = plgsys.getPluginInfo(plg);

        auto header = QStringLiteral("  ");
        ss << header << QStringLiteral("#") << i << Qt::endl;

        ss << header << QStringLiteral("- ID: ") << info.id << Qt::endl;
        ss << header << QStringLiteral("- Name: ") << plg->pluginName()
           << Qt::endl;
        ss << header << QStringLiteral("- License: ") << info.license
           << Qt::endl;
        ss << header << QStringLiteral("- Author: ") << info.author << Qt::endl;
        ss << header << QStringLiteral("- Vendor: ") << info.vendor << Qt::endl;
        ss << header << QStringLiteral("- Version: ")
           << info.version.toString();
        ss << header << QStringLiteral("- Comment: ") << plg->pluginComment()
           << Qt::endl;
        ss << header << QStringLiteral("- URL: ") << info.url << Qt::endl;

        i++;

        ss << Qt::endl;
    }

    i = 1;
    ss << QStringLiteral("* Devices:") << Qt::endl;
    for (auto &plg : plgsys.devices()) {
        auto info = plgsys.getPluginInfo(plg);

        auto header = QStringLiteral("  ");
        ss << header << QStringLiteral("#") << i << Qt::endl;

        ss << header << QStringLiteral("- ID: ") << info.id << Qt::endl;
        ss << header << QStringLiteral("- Name: ") << plg->pluginName()
           << Qt::endl;
        ss << header << QStringLiteral("- License: ") << info.license
           << Qt::endl;
        ss << header << QStringLiteral("- Author: ") << info.author << Qt::endl;
        ss << header << QStringLiteral("- Vendor: ") << info.vendor << Qt::endl;
        ss << header << QStringLiteral("- Version: ")
           << info.version.toString();
        ss << header << QStringLiteral("- Comment: ") << plg->pluginComment()
           << Qt::endl;
        ss << header << QStringLiteral("- URL: ") << info.url << Qt::endl;

        i++;

        ss << Qt::endl;
    }

    auto formatter =
        cpptrace::formatter{}.paths(cpptrace::formatter::path_mode::basename);
    auto str = formatter.format(cpptrace::generate_trace());
    ss << QString::fromStdString(str) << Qt::endl;

    CrashReport r;
    r.setInfomation(buffer);
    r.exec();

    // because abort() will also trigger it
#ifdef Q_OS_WIN
    std::signal(SIGABRT, nullptr);
#else
    ::signal(SIGABRT, nullptr);
#endif

    abort();
}
