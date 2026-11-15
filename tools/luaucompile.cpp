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

#include <cstdio>
#include <string>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>

#include "Luau/Compiler.h"

int main(int argc, char **argv) {
    QCoreApplication application(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("wing-luau-compile"));
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate(
        "LuauCompile", "Compile source to host-compatible Luau bytecode"));
    parser.addHelpOption();
    parser.addOption({QStringLiteral("binary"),
                      QCoreApplication::translate("LuauCompile",
                                                  "Write raw Luau bytecode")});
    parser.addOption(
        {QStringLiteral("O"),
         QCoreApplication::translate("LuauCompile", "Optimization level (0-2)"),
         QStringLiteral("level"), QStringLiteral("1")});
    parser.addOption({QStringLiteral("g"),
                      QCoreApplication::translate(
                          "LuauCompile", "Debug information level (0-2)"),
                      QStringLiteral("level"), QStringLiteral("1")});
    parser.addPositionalArgument(
        QStringLiteral("file"),
        QCoreApplication::translate("LuauCompile", "Source file"));
    parser.process(application);
    QTextStream errors(stderr);
    bool optimizationValid = false, debugValid = false;
    const auto optimization =
        parser.value(QStringLiteral("O")).toInt(&optimizationValid);
    const auto debug = parser.value(QStringLiteral("g")).toInt(&debugValid);
    const auto files = parser.positionalArguments();
    if (files.size() != 1 || !optimizationValid || !debugValid ||
        optimization < 0 || optimization > 2 || debug < 0 || debug > 2) {
        errors << QCoreApplication::translate(
                      "LuauCompile",
                      "Expected one source file and levels between 0 and 2")
               << Qt::endl;
        return 2;
    }
    QFile input(files.first());
    if (!input.open(QIODevice::ReadOnly)) {
        errors << input.errorString() << Qt::endl;
        return 1;
    }
    const auto source = input.readAll();
    if (input.error() != QFile::NoError) {
        errors << input.errorString() << Qt::endl;
        return 1;
    }
    Luau::CompileOptions options;
    options.optimizationLevel = optimization;
    options.debugLevel = debug;
    const auto bytecode = Luau::compile(
        std::string(source.constData(), size_t(source.size())), options);
    if (bytecode.empty() || bytecode.front() == 0) {
        errors << (bytecode.empty()
                       ? QCoreApplication::translate("LuauCompile",
                                                     "Compilation failed")
                       : QString::fromUtf8(bytecode.data() + 1,
                                           qsizetype(bytecode.size() - 1)))
               << Qt::endl;
        return 1;
    }
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#endif
    QFile output;
    if (!output.open(stdout, QIODevice::WriteOnly)) {
        return 1;
    }
    return output.write(bytecode.data(), qint64(bytecode.size())) ==
                       qint64(bytecode.size()) &&
                   output.flush()
               ? 0
               : 1;
}
