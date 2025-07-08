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

#include "clangformatmanager.h"
#include "settings/settings.h"

#include "dbghelper.h"

#include <QDir>
#include <QProcess>
#include <QProcessEnvironment>

Q_GLOBAL_STATIC_WITH_ARGS(QString, CLANG_ENABLE_FMT, ("clang.enabled"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CLANG_AUTO_FMT, ("clang.auto"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CLANG_STYLE, ("clang.style"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, CLANG_CUSTOM_STYLE, ("clang.customStyle"))

Q_GLOBAL_STATIC_WITH_ARGS(QString, CLANG_DEFAULT_CUSTOM,
                          ("BasedOnStyle: llvm, IndentWidth: 4"))

ClangFormatManager::ClangFormatManager() : QObject() {
    ASSERT_SINGLETON;

    // load config
    HANDLE_CONFIG;

    READ_CONFIG_BOOL(m_enabled, CLANG_ENABLE_FMT, true);
    READ_CONFIG_BOOL(m_autoFmt, CLANG_AUTO_FMT, true);

    auto styles = supportedStyles();
    READ_CONFIG_STRING(m_clangStyle, CLANG_STYLE, styles.first());

    QByteArray buffer;
    buffer = READ_CONFIG(CLANG_CUSTOM_STYLE, QByteArray()).toByteArray();
    if (buffer.isEmpty()) {
        m_customStyleString = *CLANG_DEFAULT_CUSTOM;
    } else {
        auto data = qUncompress(buffer);
        if (buffer.isEmpty()) {
            m_customStyleString = QString::fromUtf8(data);
        } else {
            m_customStyleString = *CLANG_DEFAULT_CUSTOM;
        }
    }

    // ok find
    refind();
}

ClangFormatManager &ClangFormatManager::instance() {
    static ClangFormatManager ins;
    return ins;
}

bool ClangFormatManager::exists() { return !m_clangPath.isEmpty(); }

bool ClangFormatManager::refind() {
    m_clangPath.clear();

    // Get the PATH environment variable
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString pathEnv = env.value("PATH");

    // Split the PATH variable into individual directories
    QStringList paths = pathEnv.split(QDir::listSeparator());

    auto name = getProgramName();
    for (auto &path : paths) {
        QDir dir(path);

        auto pp = dir.absoluteFilePath(name);
        QFileInfo fileInfo(pp);

        // Check if the file exists and is executable
        if (fileInfo.exists() && fileInfo.isExecutable()) {
            m_clangPath = pp;
            bool ok;
            auto ret = runClangFormat({"--version"}, QString(), ok);
            if (ok) {
                auto vstr = QStringLiteral("clang-format version ");
                if (ret.startsWith(vstr)) {
                    m_clangVersion = ret.mid(vstr.length()).simplified();

                    if (!checkClangCustomStyle(m_customStyleString)) {
                        m_customStyleString = *CLANG_DEFAULT_CUSTOM;
                    }

                    return true;
                } else {
                    m_clangPath.clear();
                }
            }
        }
    }
    return false;
}

QString ClangFormatManager::getProgramName() {
#ifdef Q_OS_WIN
    return QStringLiteral("clang-format.exe");
#else
    return QStringLiteral("clang-format");
#endif
}

QStringList ClangFormatManager::supportedStyles() {
    static QStringList styles{"LLVM",      "GNU",     "Google", "Chromium",
                              "Microsoft", "Mozilla", "WebKit", "Custom"};
    return styles;
}

QString ClangFormatManager::formatCode(const QString &codes, bool &ok) {
    auto styles = supportedStyles();
    auto fmtind = styles.indexOf(m_clangStyle);
    Q_ASSERT(fmtind >= 0);
    if (fmtind != styles.size() - 1) {
        auto style =
            QStringLiteral("--style={BasedOnStyle: %1, IndentWidth: %2}")
                .arg(m_clangStyle)
                .arg(m_identWidth);
        auto ret = runClangFormat(
            {style, QStringLiteral("--assume-filename=wing.cs")}, codes, ok);
        return ret;
    } else {
        return runClangFormat({}, codes, ok);
    }
}

QString ClangFormatManager::runClangFormat(const QStringList &params,
                                           const QString &stdinput, bool &ok) {
    if (!exists()) {
        ok = false;
        return {};
    }

    QProcess process;
    process.setProgram(m_clangPath);
    process.setArguments(params);

    process.start();
    if (!process.waitForStarted()) {
        process.kill();            // Kill the process if it timed out
        process.waitForFinished(); // Ensure process has terminated
        ok = false;
        return {};
    }

    process.write(stdinput.toUtf8());
    process.closeWriteChannel();

    bool finished = process.waitForFinished(-1);
    if (finished) {
        auto data = process.readAllStandardOutput();
        auto error = process.readAllStandardError();
        ok = error.isEmpty();
        if (ok) {
            return QString::fromUtf8(data);
        } else {
            return {};
        }
    } else {
        process.kill();            // Kill the process if it timed out
        process.waitForFinished(); // Ensure process has terminated
        ok = false;
        return {};
    }
}

bool ClangFormatManager::autoFormat() const { return m_autoFmt; }

void ClangFormatManager::setAutoFormat(bool newAutoFmt) {
    m_autoFmt = newAutoFmt;
}

bool ClangFormatManager::checkClangCustomStyle(const QString &styles) {
    bool ok = false;
    auto style = QStringLiteral("--style={%1}").arg(styles.simplified());
    runClangFormat({style, QStringLiteral("--assume-filename=wing.cpp")},
                   QStringLiteral("void main(){}"), ok);
    return ok;
}
quint32 ClangFormatManager::identWidth() const { return m_identWidth; }

void ClangFormatManager::setIdentWidth(quint32 newIdentWidth) {
    m_identWidth = newIdentWidth;
}

void ClangFormatManager::save() {
    HANDLE_CONFIG;
    WRITE_CONFIG(CLANG_ENABLE_FMT, m_enabled);
    WRITE_CONFIG(CLANG_AUTO_FMT, m_autoFmt);
    WRITE_CONFIG(CLANG_STYLE, m_clangStyle);
    WRITE_CONFIG(CLANG_CUSTOM_STYLE, qCompress(m_customStyleString.toUtf8()));
}

void ClangFormatManager::reset() {
    m_enabled = true;
    m_autoFmt = false;
    auto styles = supportedStyles();
    m_clangPath = styles.first();
    m_customStyleString = *CLANG_DEFAULT_CUSTOM;
    save();
}

QString ClangFormatManager::clangStyle() const { return m_clangStyle; }

bool ClangFormatManager::setClangStyle(const QString &newClangStyle) {
    if (m_clangStyle != newClangStyle) {
        if (checkClangCustomStyle(newClangStyle)) {
            m_clangStyle = newClangStyle;
            return true;
        }
        return false;
    }
    return true;
}

int ClangFormatManager::clangCurrentStyleIndex() const {
    return supportedStyles().indexOf(m_clangStyle);
}

bool ClangFormatManager::enabled() const { return m_enabled; }

void ClangFormatManager::setEnabled(bool newEnabled) { m_enabled = newEnabled; }

QString ClangFormatManager::path() const { return m_clangPath; }

QString ClangFormatManager::customStyleString() const {
    return m_customStyleString;
}

void ClangFormatManager::setCustomStyleString(
    const QString &newCustomStyleString) {
    m_customStyleString = newCustomStyleString;
}

QString ClangFormatManager::version() const { return m_clangVersion; }

int ClangFormatManager::runningTimeout() const { return m_timeout; }

void ClangFormatManager::setRunningTimeout(int msecs) { m_timeout = msecs; }
