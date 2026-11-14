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

#include "applicationmode.h"

#include <QCommandLineParser>
#include <QHostAddress>

StartupOptions ApplicationMode::m_options;
bool ApplicationMode::m_initialized = false;

void StartupOptions::configureParser(QCommandLineParser &parser) {
    parser.setApplicationDescription(tr("Binary editor and Luau tools"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({"headless", tr("Run one script without a window")});
    parser.addOption({"live", tr("Keep the headless script service running")});
    parser.addOption(
        {"address", tr("Live service listen address"), "address", "127.0.0.1"});
    parser.addOption({"port", tr("Live service listen port"), "port", "18888"});
    parser.addOption({"cert", tr("Live service TLS certificate"), "file"});
    parser.addOption({"key", tr("Live service TLS private key"), "file"});
    parser.addOption({"diff", tr("Open only the comparison window")});
    parser.addOption({"script", tr("Open only the script editor")});
    parser.addPositionalArgument(
        "files", tr("Files to open or script to execute"), "[files...]");
}

void StartupOptions::configureBootstrapParser(QCommandLineParser &parser) {
    configureParser(parser);
    // QApplication consumes these before the authoritative parse. Recognizing
    // them here prevents a valid Qt option from selecting the error backend.
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    const QStringList valueOptions = {"platform",
                                      "platformpluginpath",
                                      "platformtheme",
                                      "plugin",
                                      "style",
                                      "stylesheet",
                                      "session",
                                      "display",
                                      "geometry",
                                      "font",
                                      "name",
                                      "title",
                                      "visual",
                                      "ncols",
                                      "im",
                                      "inputstyle",
                                      "qwindowgeometry",
                                      "qwindowicon",
                                      "qwindowtitle",
                                      "qmljsdebugger"};
    for (const auto &name : valueOptions) {
        parser.addOption({name, QString(), "value"});
    }
    for (const auto &name :
         {QStringLiteral("reverse"), QStringLiteral("widgetcount"),
          QStringLiteral("testability"), QStringLiteral("nograb"),
          QStringLiteral("dograb"), QStringLiteral("cmap")}) {
        parser.addOption({name, QString()});
    }
}

StartupOptions StartupOptions::parse(QCommandLineParser &parser,
                                     const QStringList &arguments) {
    StartupOptions result;
    const bool valid = parser.parse(arguments);
    const bool headless = parser.isSet(QStringLiteral("headless"));
    const bool diff = parser.isSet(QStringLiteral("diff"));
    const bool script = parser.isSet(QStringLiteral("script"));
    if (headless) {
        result.m_mode = StartupMode::Headless;
    } else if (diff) {
        result.m_mode = StartupMode::Diff;
    } else if (script) {
        result.m_mode = StartupMode::Script;
    }
    result.m_live = parser.isSet(QStringLiteral("live"));
    result.m_informationRequested = parser.isSet(QStringLiteral("help")) ||
                                    parser.isSet(QStringLiteral("version"));
    result.m_files = parser.positionalArguments();
    result.m_address = parser.value(QStringLiteral("address"));
    result.m_certificate = parser.value(QStringLiteral("cert"));
    result.m_privateKey = parser.value(QStringLiteral("key"));
    bool portOk = false;
    const auto port = parser.value(QStringLiteral("port")).toUInt(&portOk);
    result.m_port = quint16(port);
    const QHostAddress address(result.m_address);
    const bool serviceOptions = parser.isSet(QStringLiteral("address")) ||
                                parser.isSet(QStringLiteral("port")) ||
                                parser.isSet(QStringLiteral("cert")) ||
                                parser.isSet(QStringLiteral("key"));
    if (!valid) {
        result.m_error = parser.errorText();
    } else if (int(headless) + int(diff) + int(script) > 1) {
        result.m_error =
            tr("--headless, --diff and --script are mutually exclusive");
    } else if (result.m_live && !headless) {
        result.m_error = tr("--live requires --headless");
    } else if (!result.informationRequested()) {
        if (serviceOptions && !result.m_live) {
            result.m_error = tr("--address, --port, --cert and --key require "
                                "--headless --live");
        } else if (result.m_live && !result.m_files.isEmpty()) {
            result.m_error =
                tr("--headless --live does not accept script arguments");
        } else if (result.m_live &&
                   (!portOk || port > 65535 || address.isNull())) {
            result.m_error = tr("Invalid live service address or port");
        } else if (result.m_live && (result.m_certificate.isEmpty() !=
                                     result.m_privateKey.isEmpty())) {
            result.m_error = tr("--cert and --key must be provided together");
        } else if (result.m_live && !address.isLoopback() &&
                   result.m_certificate.isEmpty()) {
            result.m_error =
                tr("Non-loopback service addresses require --cert and --key");
        } else if (headless && !result.m_live && result.m_files.size() != 1) {
            result.m_error = tr("--headless requires exactly one script");
        } else if (diff && result.m_files.size() > 2) {
            result.m_error = tr("--diff accepts at most two files");
        }
    }
    return result;
}

void ApplicationMode::initialize(const StartupOptions &options) {
    Q_ASSERT(!m_initialized);
    m_options = options;
    m_initialized = true;
}

bool ApplicationMode::headless() {
    return m_options.mode() == StartupMode::Headless;
}

bool ApplicationMode::live() { return m_options.live(); }

bool ApplicationMode::hasMainWindow() {
    return m_options.mode() == StartupMode::Full;
}
