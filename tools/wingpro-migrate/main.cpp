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

#include "legacyworkspace.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#ifdef Q_OS_UNIX
#include <sys/stat.h>
#elif defined(Q_OS_WIN)
#include <qt_windows.h>
#endif

namespace {
bool sameFile(const QFileInfo &input, const QFileInfo &output) {
    if (input.absoluteFilePath() == output.absoluteFilePath() ||
        (input.exists() && output.exists() &&
         input.canonicalFilePath() == output.canonicalFilePath())) {
        return true;
    }
#ifdef Q_OS_UNIX
    struct stat a = {};
    struct stat b = {};
    if (::stat(QFile::encodeName(input.absoluteFilePath()).constData(), &a) ==
            0 &&
        ::stat(QFile::encodeName(output.absoluteFilePath()).constData(), &b) ==
            0) {
        return a.st_dev == b.st_dev && a.st_ino == b.st_ino;
    }
#elif defined(Q_OS_WIN)
    const auto open = [](const QString &path) {
        return CreateFileW(
            reinterpret_cast<LPCWSTR>(path.utf16()), 0,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
            OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    };
    const auto inputHandle = open(input.absoluteFilePath());
    const auto outputHandle = open(output.absoluteFilePath());
    BY_HANDLE_FILE_INFORMATION a = {};
    BY_HANDLE_FILE_INFORMATION b = {};
    const bool same = inputHandle != INVALID_HANDLE_VALUE &&
                      outputHandle != INVALID_HANDLE_VALUE &&
                      GetFileInformationByHandle(inputHandle, &a) &&
                      GetFileInformationByHandle(outputHandle, &b) &&
                      a.dwVolumeSerialNumber == b.dwVolumeSerialNumber &&
                      a.nFileIndexHigh == b.nFileIndexHigh &&
                      a.nFileIndexLow == b.nFileIndexLow;
    if (inputHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(inputHandle);
    }
    if (outputHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(outputHandle);
    }
    return same;
#endif
    return false;
}
} // namespace

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("wingpro-migrate"));
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral(
        "Convert a legacy JSON workspace to binary WINGPRO format"));
    parser.addHelpOption();
    parser.addOption({QStringLiteral("force"),
                      QStringLiteral("Replace an existing output file")});
    parser.addPositionalArgument(QStringLiteral("INPUT"),
                                 QStringLiteral("Legacy JSON workspace"));
    parser.addPositionalArgument(
        QStringLiteral("OUTPUT"),
        QStringLiteral("Binary workspace destination"));
    parser.process(app);
    const auto args = parser.positionalArguments();
    if (args.size() != 2) {
        parser.showHelp(2);
    }
    QTextStream err(stderr);
    const QFileInfo input(args.at(0));
    const QFileInfo output(args.at(1));
    if (sameFile(input, output)) {
        err << "Input and output must be different files\n";
        return 2;
    }
    if ((output.exists() || output.isSymLink()) &&
        !parser.isSet(QStringLiteral("force"))) {
        err << "Output already exists; use --force to replace it\n";
        return 2;
    }
    WorkSpaceData data;
    QString error;
    if (!LegacyWorkspace::load(input.absoluteFilePath(), data, &error)) {
        err << error << '\n';
        return 1;
    }
    if (!WorkSpaceManager::saveWorkSpace(output.absoluteFilePath(), data,
                                         &error)) {
        err << error << '\n';
        return 1;
    }
    QTextStream(stdout) << "Migrated workspace: " << output.absoluteFilePath()
                        << '\n';
    return 0;
}
