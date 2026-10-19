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

#include "luau/lsp/luaulanguageserver.h"

#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QTemporaryDir>

#include <iostream>
#include <stdexcept>

static void require(bool condition, const char *message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

static lsp::Position positionOf(const QString &source, const QString &text,
                                int offset = 0) {
    const auto index = source.indexOf(text);
    require(index >= 0, "test position not found");
    const auto prefix = source.first(index + offset);
    const auto newline = prefix.lastIndexOf('\n');
    return {static_cast<uint>(prefix.count('\n')),
            static_cast<uint>(prefix.size() - newline - 1)};
}

static bool hasCompletion(const QVector<lsp::CompletionItem> &items,
                          const QString &label) {
    for (const auto &item : items) {
        if (item.label == label) {
            return true;
        }
    }
    return false;
}

static void testLanguageFeatures() {
    LuauLanguageServer server;
    server.initialize({});
    QTemporaryDir directory;
    require(directory.isValid(), "temporary directory unavailable");
    const auto uri = QUrl::fromLocalFile(directory.filePath("main.luau"));
    QVector<lsp::Diagnostic> diagnostics;
    QObject::connect(&server, &LuauLanguageServer::onPublishDiagnostic, &server,
                     [&](const lsp::DocumentUri &document,
                         const QVector<lsp::Diagnostic> &items) {
                         if (document == uri) {
                             diagnostics = items;
                         }
                     });

    const QString source =
        QStringLiteral("--!strict\n"
                       "type Point = { x: number, y: number }\n"
                       "local point: Point = { x = 1, y = 2 }\n"
                       "local function twice(value: number): number\n"
                       "    return value * 2\n"
                       "end\n"
                       "local result = twice(2)\n"
                       "print(point.x, result)\n");
    QTextDocument document(source);
    server.onOpenDocument(uri, &document);
    require(diagnostics.isEmpty(), "valid strict document reported errors");
    const auto hover =
        server.hover({uri, positionOf(source, "point.x", 1)}, nullptr);
    require(hover && hover->contents.value.contains("number"),
            "hover lost the diagnostic type graph");
    const auto signature =
        server.signatureHelp({uri, positionOf(source, "twice(2)", 6)}, nullptr);
    require(signature && !signature->signatures.empty(),
            "signature help is empty");
    require(signature->signatures.front().label.contains("number"),
            "signature help lost parameter types");
    const auto definition = server.gotoDefinition(
        {uri, positionOf(source, "twice(2)", 1)}, nullptr);
    require(!definition.empty() && definition.front().range.start.line == 3,
            "function definition lookup failed");
    const auto typeDefinition = server.gotoTypeDefinition(
        {uri, positionOf(source, "point.x", 1)}, nullptr);
    require(typeDefinition && typeDefinition->range.start.line == 1,
            "type definition lookup failed");
    require(!server.semanticTokens({uri}, nullptr).isEmpty(),
            "semantic tokens are empty");

    QString edited = source + QStringLiteral("point.\n");
    document.setPlainText(edited);
    server.onUpdateDocument(uri, &document);
    auto completion = server.completion(
        {uri, positionOf(edited, "point.\n", 6), std::nullopt}, nullptr);
    require(hasCompletion(completion, "x") && hasCompletion(completion, "y"),
            "member completion failed after diagnostics");

    edited.replace(QStringLiteral("x: number, y: number"),
                   QStringLiteral("x: number, z: number"));
    edited.replace(QStringLiteral("x = 1, y = 2"),
                   QStringLiteral("x = 1, z = 2"));
    document.setPlainText(edited);
    server.onUpdateDocument(uri, &document);
    completion = server.completion(
        {uri, positionOf(edited, "point.\n", 6), std::nullopt}, nullptr);
    require(hasCompletion(completion, "z") && !hasCompletion(completion, "y"),
            "completion reused a stale module");

    document.setPlainText(
        QStringLiteral("--!strict\nlocal n: number = 'wrong'\n"));
    server.onUpdateDocument(uri, &document);
    require(!diagnostics.isEmpty(), "strict diagnostics disappeared");
    server.onCloseDocument(uri);
    require(diagnostics.isEmpty(),
            "closing the document did not clear diagnostics");
}

static void testTypeFunctionsAndNonstrictInference() {
    LuauLanguageServer server;
    server.initialize({});
    const auto uri = QUrl::fromLocalFile("/tmp/wing-type-functions.luau");
    QVector<lsp::Diagnostic> diagnostics;
    QObject::connect(&server, &LuauLanguageServer::onPublishDiagnostic, &server,
                     [&](const lsp::DocumentUri &document,
                         const QVector<lsp::Diagnostic> &items) {
                         if (document == uri) {
                             diagnostics = items;
                         }
                     });
    const QString source =
        QStringLiteral("--!strict\n"
                       "type function identity(ty) return ty end\n"
                       "type Number = identity<number>\n"
                       "local value: Number = 42\n"
                       "print(value)\n");
    QTextDocument document(source);
    server.onOpenDocument(uri, &document);
    require(diagnostics.isEmpty(), "new solver type functions were rejected");
    const auto hover =
        server.hover({uri, positionOf(source, "print(value)", 7)}, nullptr);
    require(hover && hover->contents.value.contains("number"),
            "type function result was not available to hover");

    const QString nonstrict = QStringLiteral("local value = { count = 1 }\n"
                                             "print(value.count)\n"
                                             "value.\n");
    document.setPlainText(nonstrict);
    server.onUpdateDocument(uri, &document);
    const auto items = server.completion(
        {uri, positionOf(nonstrict, "value.\n", 6), std::nullopt}, nullptr);
    require(hasCompletion(items, "count"),
            "nonstrict inference lost table members");
    server.onCloseDocument(uri);
}

static void testDefinitionsAndOverloads() {
    QTemporaryDir directory;
    require(directory.isValid(), "temporary directory unavailable");
    QFile definitions(directory.filePath("host.d.luau"));
    require(definitions.open(QIODevice::WriteOnly),
            "cannot create definitions");
    definitions.write("declare choose: ((value: number) -> number) & "
                      "((value: string) -> string)\n");
    definitions.write(
        "declare pair: ((value: number, other: number) -> number) & "
        "((value: string, other: string) -> string)\n");
    definitions.write("declare generic: ((value: number) -> number) & "
                      "(<T>(value: T) -> T)\n");
    definitions.close();
    LuauLanguageServer server;
    LuauLanguageServer::InitializationOptions options;
    options.definitionsFiles.insert(QStringLiteral("host"),
                                    definitions.fileName());
    server.initialize(options);
    const auto symbols = server.globalSymbols();
    bool foundChoose = false;
    bool foundMath = false;
    for (const auto &symbol : symbols) {
        if (symbol.item.label == QLatin1String("choose")) {
            foundChoose = symbol.fullDeclaration.contains("number") &&
                          symbol.fullDeclaration.contains("string");
        } else if (symbol.item.label == QLatin1String("math")) {
            foundMath = !symbol.children.isEmpty();
        }
    }
    require(foundChoose && foundMath, "registered symbols are missing");

    const QString source = QStringLiteral(
        "--!strict\nlocal result = choose(\"text\")\nprint(result)\n");
    QTextDocument document(source);
    const auto uri = QUrl::fromLocalFile(directory.filePath("overload.luau"));
    server.onOpenDocument(uri, &document);
    const auto signature = server.signatureHelp(
        {uri, positionOf(source, "choose(\"text\")", 9)}, nullptr);
    require(signature && signature->signatures.size() == 2,
            "definition overloads were lost");
    require(signature->activeSignature &&
                signature->signatures.at(*signature->activeSignature)
                    .label.contains("string"),
            "new solver selected the wrong overload");
    const QString incomplete =
        QStringLiteral("--!strict\nlocal result = pair(\"text\", ");
    document.setPlainText(incomplete);
    server.onUpdateDocument(uri, &document);
    const auto partial = server.signatureHelp(
        {uri, positionOf(incomplete, "pair(\"text\", ", 13)}, nullptr);
    require(partial && partial->activeSignature &&
                partial->signatures.at(*partial->activeSignature)
                    .label.contains("string"),
            "incomplete call selected the wrong overload");

    const QString generic = QStringLiteral(
        "--!strict\nlocal result = generic(\"text\")\nprint(result)\n");
    document.setPlainText(generic);
    server.onUpdateDocument(uri, &document);
    const auto genericSignature = server.signatureHelp(
        {uri, positionOf(generic, "generic(\"text\")", 10)}, nullptr);
    require(
        genericSignature && genericSignature->activeSignature &&
            genericSignature->signatures.at(*genericSignature->activeSignature)
                .label.contains("<T>"),
        "generic overload matching failed");

    const QString methods =
        QStringLiteral("--!strict\n"
                       "type Object = { select: ((Object, number) -> number) & "
                       "((Object, string) -> string) }\n"
                       "local object: Object = nil :: any\n"
                       "local result = object:select(\"text\")\n"
                       "print(result)\n");
    document.setPlainText(methods);
    server.onUpdateDocument(uri, &document);
    const auto methodSignature = server.signatureHelp(
        {uri, positionOf(methods, "select(\"text\")", 9)}, nullptr);
    require(
        methodSignature && methodSignature->activeSignature &&
            methodSignature->signatures.at(*methodSignature->activeSignature)
                .label.contains("string"),
        "method overload did not use the receiver type");
    server.onCloseDocument(uri);
}

static void testRequiredModuleUpdates() {
    QTemporaryDir directory(QDir::tempPath() +
                            QStringLiteral("/wing luau-XXXXXX"));
    require(directory.isValid(), "temporary directory unavailable");
    QFile moduleFile(directory.filePath("module.luau"));
    require(moduleFile.open(QIODevice::WriteOnly),
            "cannot create required module");
    moduleFile.write("export type Record = { count: number }\n"
                     "return { count = 1 }\n");
    moduleFile.close();
    const auto moduleUri = QUrl::fromLocalFile(moduleFile.fileName());
    const auto mainUri = QUrl::fromLocalFile(directory.filePath("main.luau"));
    QTextDocument moduleDocument(QStringLiteral(
        "export type Record = { count: number }\nreturn { count = 1 }\n"));
    const QString source =
        QStringLiteral("--!strict\nlocal mod = "
                       "require(\"./module\")\nprint(mod.count)\nmod.\n");
    QTextDocument mainDocument(source);
    LuauLanguageServer server;
    server.initialize({});
    server.onOpenDocument(mainUri, &mainDocument);
    auto items = server.completion(
        {mainUri, positionOf(source, "mod.\n", 4), std::nullopt}, nullptr);
    require(hasCompletion(items, "count"), "required module completion failed");
    const auto definition = server.gotoDefinition(
        {mainUri, positionOf(source, "mod.count", 5)}, nullptr);
    require(!definition.empty() && definition.front().uri == moduleUri,
            "cross-module definition lookup failed");

    server.onOpenDocument(moduleUri, &moduleDocument);
    moduleDocument.setPlainText(
        QStringLiteral("return { title = 'changed' }\n"));
    server.onUpdateDocument(moduleUri, &moduleDocument);
    items = server.completion(
        {mainUri, positionOf(source, "mod.\n", 4), std::nullopt}, nullptr);
    require(hasCompletion(items, "title") && !hasCompletion(items, "count"),
            "dependency update reused stale type information");
    server.onCloseDocument(mainUri);
    server.onCloseDocument(moduleUri);
}

int main(int argc, char **argv) {
    QGuiApplication application(argc, argv);
    try {
        const auto scenario = application.arguments().value(1);
        if (scenario.isEmpty() || scenario == QLatin1String("features")) {
            testLanguageFeatures();
        }
        if (scenario.isEmpty() || scenario == QLatin1String("types")) {
            testTypeFunctionsAndNonstrictInference();
        }
        if (scenario.isEmpty() || scenario == QLatin1String("overloads")) {
            testDefinitionsAndOverloads();
        }
        if (scenario.isEmpty() || scenario == QLatin1String("modules")) {
            testRequiredModuleUpdates();
        }
        std::cout << "Luau language server tests passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
