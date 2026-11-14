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

#include <iostream>
#include <stdexcept>

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include "Luau/Common.h"

#include "luau/lsp/luaulanguageserver.h"
#include "luau/wingluaurequire.h"

static void require(bool condition, const char *message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

static void testTypeDisplayIdentity() {
    Luau::TypeArena arena;
    Luau::TableType table;
    table.name = "Hidden";
    auto hidden = arena.addType(std::move(table));
    Luau::TableType otherTable;
    otherTable.name = "Hidden";
    auto other = arena.addType(std::move(otherTable));
    auto literal =
        arena.addType(Luau::SingletonType{Luau::StringSingleton{"Hidden"}});
    LuauLsp::TypeDisplayOptions options;
    options.typeNameOverride =
        [hidden](Luau::TypeId type) -> std::optional<std::string> {
        return Luau::follow(type) == hidden
                   ? std::optional<std::string>{"public.Hidden"}
                   : std::nullopt;
    };
    require(LuauLsp::displayType(hidden, options) == "public.Hidden",
            "public type name missing");
    require(LuauLsp::displayType(other, options) == "Hidden",
            "same-name user type was rewritten");
    require(LuauLsp::displayType(literal, options) == "\"Hidden\"",
            "string singleton was rewritten");
    Luau::FunctionType function(arena.addTypePack({hidden, literal, other}),
                                arena.addTypePack({hidden, literal}));
    const auto signature =
        LuauLsp::displayNamedFunction("identity", function, options);
    require(signature == "identity(_: public.Hidden, _: \"Hidden\", "
                         "_: Hidden): (public.Hidden, \"Hidden\")",
            "function display lost type identities or return pack layout");
    auto functionType = arena.addType(function);
    const auto nested = LuauLsp::displayType(functionType, options);
    require(nested.find("public.Hidden") != std::string::npos &&
                nested.find("\"Hidden\"") != std::string::npos,
            "nested function type spans lost public names or literals");
    require(Luau::toString(hidden) == "Hidden",
            "display changed the analysis type graph");
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

static void testColonCompletion() {
    QTemporaryDir directory;
    require(directory.isValid(), "temporary directory unavailable");
    const auto uri = QUrl::fromLocalFile(directory.filePath("main.luau"));
    const QString source =
        QStringLiteral("local object = { method = function(self) end }\n"
                       "object:\n");
    QTextDocument document(source);
    LuauLanguageServer server;
    server.initialize({});
    server.onOpenDocument(uri, &document);
    const auto items =
        server.completion({uri, positionOf(source, "object:\n", 7),
                           lsp::CompletionParams::Context{
                               lsp::CompletionTriggerKind::TriggerCharacter,
                               QStringLiteral(":")}},
                          nullptr);
    require(hasCompletion(items, "method"),
            "colon-triggered method completion failed");
    server.onCloseDocument(uri);
}

static void testLanguageFeatures() {
    LuauLanguageServer server;
    server.initialize({});
    QTemporaryDir directory;
    require(directory.isValid(), "temporary directory unavailable");
    const auto uri = QUrl::fromLocalFile(directory.filePath("main.luau"));
    QVector<lsp::Diagnostic> diagnostics;
    QObject::connect(&server, &LuauLanguageServer::sigOnPublishDiagnostic,
                     &server,
                     [&](const lsp::DocumentUri &document,
                         const QVector<lsp::Diagnostic> &items) {
                         if (document == uri) {
                             diagnostics = items;
                         }
                     });

    const QString source = QStringLiteral(
        "--!strict\n"
        "type Point = { x: number, y: number }\n"
        "local point: Point = { x = 1, y = 2 }\n"
        "local function twice(value: number): number\n"
        "    return value * 2\n"
        "end\n"
        "local result = twice(2)\n"
        "print(point.x, result)\n"
        "local instance = { new = function(self: any) return 1 end }\n"
        "local created = instance:new()\nprint(created)\n");
    QTextDocument document(source);
    server.onOpenDocument(uri, &document);
    require(diagnostics.isEmpty(), "valid strict document reported errors");
    const auto hover =
        server.hover({uri, positionOf(source, "point.x", 1)}, nullptr);
    require(hover && hover->contents.value.contains("number"),
            "hover lost the diagnostic type graph");
    const auto methodHover =
        server.hover({uri, positionOf(source, "instance:new()", 10)}, nullptr);
    require(methodHover && methodHover->contents.value.contains("function ") &&
                !methodHover->contents.value.contains("functioninstance") &&
                methodHover->contents.value.contains(":new("),
            "method hover omitted the space after function");
    const auto functionHover =
        server.hover({uri, positionOf(source, "twice(2)", 2)}, nullptr);
    require(functionHover &&
                functionHover->contents.value.contains("function twice("),
            "local function hover omitted the space after function");
    const auto signature =
        server.signatureHelp({uri, positionOf(source, "twice(2)", 6)}, nullptr);
    require(signature && !signature->signatures.empty(),
            "signature help is empty");
    require(signature->signatures.front().label.startsWith("function twice("),
            "local function signature omitted the space after function");
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
    QObject::connect(&server, &LuauLanguageServer::sigOnPublishDiagnostic,
                     &server,
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
    QFile pluginDefinitions(directory.filePath("aaa_plugin.d.luau"));
    require(pluginDefinitions.open(QIODevice::WriteOnly),
            "cannot create plugin definitions");
    pluginDefinitions.write("declare PluginProbe: { member: typeof(choose), "
                            "nested: { enabled: boolean } }\n");
    pluginDefinitions.close();
    LuauLanguageServer server;
    LuauLanguageServer::InitializationOptions options;
    options.definitionsFiles.insert(QStringLiteral("host"),
                                    definitions.fileName());
    options.pluginDefinitionFiles.append(pluginDefinitions.fileName());
    using NativeSymbol = WingScriptBind::ScriptSymbol;
    options.pluginSymbols = {{"opaque-plugin-root",
                              {},
                              "PluginProbe",
                              NativeSymbol::Kind::Module,
                              {}},
                             {"opaque-plugin-member",
                              "opaque-plugin-root",
                              "member",
                              NativeSymbol::Kind::Function,
                              {}}};
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
    bool browsedMath = false;
    bool browsedPlugin = false;
    for (auto symbol : server.builtinSymbols()) {
        if (symbol.item.label == QLatin1String("math")) {
            symbol.id = QStringLiteral("opaque-global-id");
            browsedMath =
                symbol.source == LuauLanguageServer::SymbolSource::Global &&
                !server.symbolChildren(symbol).isEmpty();
        } else if (symbol.item.label == QLatin1String("PluginProbe")) {
            const auto children = server.symbolChildren(symbol);
            browsedPlugin =
                symbol.source == LuauLanguageServer::SymbolSource::Plugin &&
                symbol.category ==
                    LuauLanguageServer::SymbolCategory::Plugins &&
                children.size() == 2;
            for (const auto &child : children) {
                if (child.item.label == QLatin1String("member")) {
                    require(child.id ==
                                    QStringLiteral("opaque-plugin-member") &&
                                child.overloads.size() == 2,
                            "plugin definitions lost overloads or native IDs");
                } else if (child.item.label == QLatin1String("nested")) {
                    const auto nested = server.symbolChildren(child);
                    require(nested.size() == 1 && nested.front().item.label ==
                                                      QLatin1String("enabled"),
                            "definition-only plugin tables cannot be expanded");
                }
            }
        }
    }
    require(browsedMath && browsedPlugin,
            "symbol browsing inferred source from string IDs");

    const QString source = QStringLiteral(
        "--!strict\nlocal result = choose(\"text\")\nprint(result)\n");
    QTextDocument document(source);
    const auto uri = QUrl::fromLocalFile(directory.filePath("overload.luau"));
    server.onOpenDocument(uri, &document);
    const auto signature = server.signatureHelp(
        {uri, positionOf(source, "choose(\"text\")", 9)}, nullptr);
    require(signature && signature->signatures.size() == 2,
            "definition overloads were lost");
    require(signature->signatures.front().label.startsWith("function choose("),
            "global function signature omitted the space after function");
    require(signature->activeSignature &&
                signature->signatures.at(*signature->activeSignature)
                    .label.contains("string"),
            "new solver selected the wrong overload");
    const auto callText = QStringLiteral("choose(\"text\")");
    const auto afterCall = server.signatureHelp(
        {uri, positionOf(source, callText, int(callText.size()))}, nullptr);
    require(!afterCall, "signature help remained active after the call");
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

static void testRequirePathCompletion() {
    QTemporaryDir directory;
    require(directory.isValid(), "temporary directory unavailable");
    const QDir root(directory.path());
    require(root.mkpath("folder"), "cannot create require subdirectory");

    QFile moduleFile(root.filePath("module.luau"));
    require(moduleFile.open(QIODevice::WriteOnly),
            "cannot create require module");
    moduleFile.write("return {}\n");
    moduleFile.close();

    QFile nestedFile(root.filePath("folder/child.luau"));
    require(nestedFile.open(QIODevice::WriteOnly),
            "cannot create nested require module");
    nestedFile.write("return {}\n");
    nestedFile.close();

    const auto uri = QUrl::fromLocalFile(root.filePath("main.luau"));
    const QString source =
        QStringLiteral("local loaded = require(\"./mod\")\n"
                       "local nested = require(\"./folder/ch\")\n");
    QTextDocument document(source);
    LuauLanguageServer server;
    server.initialize({});
    server.onOpenDocument(uri, &document);

    const auto moduleItems = server.completion(
        {uri, positionOf(source, "./mod", int(QStringLiteral("./mod").size())),
         std::nullopt},
        nullptr);
    const auto moduleItem =
        std::find_if(moduleItems.cbegin(), moduleItems.cend(),
                     [](const lsp::CompletionItem &item) {
                         return item.label == QLatin1String("module");
                     });
    require(moduleItem != moduleItems.cend() && moduleItem->textEdit &&
                moduleItem->textEdit->newText == QLatin1String("./module"),
            "relative require path completion failed");

    const auto nestedItems = server.completion(
        {uri,
         positionOf(source, "./folder/ch",
                    int(QStringLiteral("./folder/ch").size())),
         std::nullopt},
        nullptr);
    require(hasCompletion(nestedItems, "child"),
            "nested require path completion failed");
    server.onCloseDocument(uri);
}

static void testBuiltinRequirePathCompletion() {
    QTemporaryDir directory;
    require(directory.isValid(), "temporary builtin directory unavailable");
    const auto writeModule = [&directory](const QString &path) {
        const auto filePath = directory.filePath(path);
        require(QDir().mkpath(QFileInfo(filePath).path()),
                "cannot create builtin module directory");
        QFile file(filePath);
        require(file.open(QIODevice::WriteOnly),
                "cannot create builtin module fixture");
        file.write("return {}\n");
        file.close();
        return file.fileName();
    };
    const auto rootModule = writeModule(QStringLiteral("core.luau"));
    const auto nestedModule = writeModule(QStringLiteral("core/net.luau"));

    LuauLanguageServer::InitializationOptions options;
    options.builtinModuleFiles.insert(QStringLiteral("core"), rootModule);
    options.builtinModuleFiles.insert(QStringLiteral("core/net"), nestedModule);
    LuauLanguageServer server;
    server.initialize(options);

    const auto uri = QUrl::fromLocalFile(directory.filePath("main.luau"));
    const QString source =
        QStringLiteral("local root = require(\"@b/\")\n"
                       "local nested = require(\"@b/core/ne\")\n");
    QTextDocument document(source);
    server.onOpenDocument(uri, &document);

    const auto rootItems = server.completion(
        {uri, positionOf(source, "@b/", 3), std::nullopt}, nullptr);
    require(hasCompletion(rootItems, "core"),
            "explicit builtin root completion failed");

    const auto nestedItems =
        server.completion({uri,
                           positionOf(source, "@b/core/ne",
                                      int(QStringLiteral("@b/core/ne").size())),
                           std::nullopt},
                          nullptr);
    require(hasCompletion(nestedItems, "net"),
            "nested explicit builtin completion failed");
    server.onCloseDocument(uri);
}

static void testRequireAliasDirectoryCompletion() {
    QTemporaryDir directory;
    require(directory.isValid(), "temporary package directory unavailable");
    const QDir root(directory.path());
    require(root.mkpath(QStringLiteral("user/pkg/nested")),
            "cannot create user package");
    const auto writeFile = [&root](const QString &path,
                                   const QByteArray &data) {
        QFile file(root.filePath(path));
        require(file.open(QIODevice::WriteOnly), "cannot write module fixture");
        require(file.write(data) == data.size(), "incomplete module fixture");
    };
    writeFile(QStringLiteral("user/pkg/init.luau"), "return {}\n");
    writeFile(QStringLiteral("user/pkg/nested/child.luau"), "return {}\n");
    writeFile(QStringLiteral("user/single.luau"), "return {}\n");
    writeFile(QStringLiteral(".luaurc"),
              R"({"aliases":{"project":"./user/pkg/nested"}})");

    WingLuauRequire::Options options;
    options.userRoot = root.filePath(QStringLiteral("user"));
    WingLuauRequire::ResolutionSession resolver(options, {});
    const auto suggester = WingLuauRequire::createPathSuggester(
        {}, options.userRoot, {}, &resolver);
    const auto context = root.filePath(QStringLiteral("main.luau"));
    const auto hasSuggestion = [&](const std::string &path,
                                   const std::string &fullPath) {
        const auto items =
            suggester->getRequireSuggestions(context.toStdString(), path);
        return items && std::any_of(items->begin(), items->end(),
                                    [&](const Luau::RequireSuggestion &item) {
                                        return item.fullPath == fullPath;
                                    });
    };
    require(hasSuggestion("@u/", "@u/pkg"), "package store root lost children");
    require(hasSuggestion("@u/p", "@u/pkg"), "partial package lost children");
    require(hasSuggestion("@u/pkg/", "@u/pkg/nested"),
            "package init module lost children");
    require(hasSuggestion("@pkg/nested/", "@pkg/nested/child"),
            "package namespace lost children");
    require(hasSuggestion("@project/", "@project/child"),
            "configuration alias namespace lost children");
    require(hasSuggestion("", "@single"), "file package alias missing");
    using Kind = WingLuauRequire::FileModuleResolution::Kind;
    require(resolver.resolve(context, QStringLiteral("@project")).kind ==
                Kind::NotFound,
            "completion made namespace directories loadable");
    require(resolver.resolve(context, QStringLiteral("@u/pkg")).kind ==
                Kind::File,
            "completion query leaked into module resolution");
}

static void testBuiltinResolutionBoundaries() {
    QTemporaryDir directory;
    require(directory.isValid(), "temporary package directory unavailable");
    const QDir root(directory.path());
    require(root.mkpath("user/qt"), "cannot create package fixture");
    QFile module(root.filePath("user/qt/init.luau"));
    require(module.open(QIODevice::WriteOnly),
            "cannot create installed package");
    module.write("return { installed = true }\n");
    module.close();
    WingLuauRequire::Options options;
    options.userRoot = root.filePath("user");
    const QSet<QString> builtins{QStringLiteral("qt")};
    WingLuauRequire::ResolutionSession session(options, builtins);
    using Kind = WingLuauRequire::FileModuleResolution::Kind;
    require(WingLuauRequire::resolveModule(directory.filePath("main.luau"),
                                           "@qt", options, builtins)
                    .kind == Kind::Ambiguous,
            "builtin shorthand ignored installed-package ambiguity");
    require(WingLuauRequire::resolveModule(directory.filePath("main.luau"),
                                           "@b/qt", options, builtins)
                    .kind == Kind::Builtin,
            "explicit builtin did not bypass installed package");
    require(WingLuauRequire::resolveModule(directory.filePath("main.luau"),
                                           "@u/qt", options, builtins)
                    .path == QFileInfo(module).canonicalFilePath(),
            "explicit user package failed to resolve");
    require(WingLuauRequire::resolveModule(directory.filePath("main.luau"),
                                           "@b/missing", options, builtins)
                    .kind == Kind::NotFound,
            "missing builtin was resolved");
    QFile alias(root.filePath(".luaurc"));
    require(alias.open(QIODevice::WriteOnly),
            "cannot create alias configuration");
    alias.write("{\"aliases\":{\"qt\":\"./override\"}}");
    alias.close();
    QFile target(root.filePath("override.luau"));
    require(target.open(QIODevice::WriteOnly),
            "cannot create alias destination");
    target.write("error('analysis must not execute this module')\n");
    target.close();
    require(
        session.resolve(directory.filePath("main.luau"), "@b/missing").kind ==
            Kind::NotFound,
        "reused resolver did not report missing module");
    require(session.resolve(directory.filePath("main.luau"), "@qt").path ==
                QFileInfo(target).canonicalFilePath(),
            "reused resolver retained a prior failure or stale alias "
            "configuration");
    require(session.resolve(directory.filePath("main.luau"), "@qt").path ==
                QFileInfo(target).canonicalFilePath(),
            "native require cache skipped a repeated resolution query");
    require(WingLuauRequire::resolveModule(directory.filePath("main.luau"),
                                           "@qt", options, builtins)
                    .path == QFileInfo(target).canonicalFilePath(),
            "runtime configuration aliases lost precedence");
    require(WingLuauRequire::resolveModule(directory.filePath("main.luau"),
                                           "@qt", options, builtins,
                                           {root.filePath("untrusted")})
                    .kind == Kind::Ambiguous,
            "analysis read configuration outside its allowed roots");
    QElapsedTimer timer;
    timer.start();
    for (int i = 0; i < 100; ++i) {
        require(WingLuauRequire::resolveModule(directory.filePath("main.luau"),
                                               "@b/qt", options, builtins)
                        .kind == Kind::Builtin,
                "fresh resolver query failed");
    }
    const auto fresh = timer.nsecsElapsed() / 1000;
    timer.restart();
    for (int i = 0; i < 100; ++i) {
        require(
            session.resolve(directory.filePath("main.luau"), "@b/qt").kind ==
                Kind::Builtin,
            "session resolver query failed");
    }
    std::cout << "100 require queries: fresh VM " << fresh << "us; reused VM "
              << timer.nsecsElapsed() / 1000 << "us\n";
}

static void testGeneratedBindings() {
    const QDir output(qEnvironmentVariable("WING_BINDINGS_DIR"));
    QFile manifest(output.filePath("manifest.json"));
    require(manifest.open(QIODevice::ReadOnly),
            "generated manifest unavailable");
    const auto model = QJsonDocument::fromJson(manifest.readAll()).object();
    const QDir definitions(output.filePath("definitions"));
    LuauLanguageServer::InitializationOptions options;
    int index = 0;
    for (const auto &name : model.value("definition_preload").toArray()) {
        options.definitionsFiles.insert(
            QString::number(index++).rightJustified(5, '0'),
            definitions.filePath(name.toString()));
    }
    for (const QString &name : {QStringLiteral("qt"), QStringLiteral("cffi"),
                                QStringLiteral("network")}) {
        options.builtinModuleFiles.insert(
            name,
            definitions.filePath(
                model.value("modules").toObject().value(name).toString()));
    }
    options.symbolCatalogFile =
        definitions.filePath(model.value("symbol_catalog").toString());
    LuauLanguageServer server;
    QElapsedTimer timer;
    timer.start();
    bool definitionErrors = false;
    QObject::connect(&server, &LuauLanguageServer::sigOnPublishDiagnostic,
                     &server,
                     [&](const lsp::DocumentUri &uri,
                         const QVector<lsp::Diagnostic> &items) {
                         definitionErrors |= !items.isEmpty();
                         for (const auto &item : items) {
                             std::cerr << uri.toString().toStdString() << ":"
                                       << item.range.start.line << ":"
                                       << item.range.start.character << ": "
                                       << item.message.toStdString() << '\n';
                         }
                     });
    server.initialize(options);
    std::cout << "Generated definitions initialization: " << timer.elapsed()
              << "ms\n";
    require(!definitionErrors, "generated ambient definitions failed");
    timer.restart();
    const auto roots = server.builtinSymbols();
    bool foundQt = false;
    bool foundFfiInstance = false;
    for (const auto &root : roots) {
        if (root.item.label == QLatin1String("cffi")) {
            for (const auto &child : server.symbolChildren(root)) {
                if (child.item.label == QLatin1String("Instance")) {
                    foundFfiInstance = !server.symbolChildren(child).isEmpty();
                }
            }
        }
        if (root.item.label == QLatin1String("qt")) {
            foundQt =
                root.requiresImport && root.moduleRoot &&
                root.children.isEmpty() &&
                root.source == LuauLanguageServer::SymbolSource::Catalog &&
                root.category == LuauLanguageServer::SymbolCategory::Qt;
            const auto children = server.symbolChildren(root);
            bool foundDialog = false;
            bool foundLoader = false;
            for (const auto &child : children) {
                if (child.item.label == QLatin1String("load_ui_widget_file")) {
                    foundLoader =
                        child.fullDeclaration.contains("qt.QWidget") &&
                        !child.fullDeclaration.contains("_qt_");
                }
                require(child.item.label != QLatin1String("to_std_string"),
                        "instance method leaked into Qt root");
                require(child.item.label !=
                            QLatin1String("QProcessEnvironment"),
                        "excluded type in catalog");
                foundDialog |= child.item.label == QLatin1String("QDialog");
            }
            require(foundDialog, "Qt class absent from lazy catalog");
            require(foundLoader, "catalog exposes internal Qt type names");
        }
    }
    require(foundQt, "required Qt module root missing");
    require(foundFfiInstance, "FFI instance type absent from lazy catalog");
    std::cout << "Catalog roots and Qt branch: " << timer.elapsed() << "ms\n";
    QTemporaryDir directory;
    const auto uri = QUrl::fromLocalFile(directory.filePath("main.luau"));
    QVector<lsp::Diagnostic> diagnostics;
    QObject::connect(&server, &LuauLanguageServer::sigOnPublishDiagnostic,
                     &server,
                     [&](const lsp::DocumentUri &document,
                         const QVector<lsp::Diagnostic> &items) {
                         if (document == uri) {
                             diagnostics = items;
                         }
                     });
    const QString source = QStringLiteral(
        "--!strict\nlocal qt = require(\"@qt\")\nlocal text = "
        "ustring(\"中文\")\nlocal calendar = "
        "qt.QCalendar(qt.QCalendar.System.Gregorian)\nlocal valid = "
        "calendar:is_gregorian()\nlocal dialog = qt.QDialog()\nlocal "
        "ffi = require(\"@cffi\").new()\nlocal size = "
        "ffi.sizeof(\"int\")\nprint(text, dialog, size, valid)\n"
        "local literal: \"_qt_QWidget\" = \"_qt_QWidget\"\nprint(literal)\n"
        "local widget = "
        "qt.load_ui_widget_file(\"fixture.ui\")\nprint(widget)\n"
        "local wide = int64(\"9223372036854775806\") + int64(1)\n"
        "local bits = uint64(\"0xffffffffffffffff\"):band(uint64(255))\n"
        "local precise: string = wide:to_string()\nprint(bits, precise)\n"
        "local numeric_left = 2 + uint64(\"9007199254740993\")\n"
        "local compared = int64(-1) < uint64(\"18446744073709551615\")\n"
        "print(numeric_left, compared)\n"
        "local fs_file = filesystem.file(\"fixture.bin\")\n"
        "local fs_info = filesystem.fileinfo(\"fixture.bin\")\n"
        "local fs_dir = filesystem.dir(\".\")\n"
        "local fs_size: number | typeof(int64(0)) = fs_info:size()\n"
        "local fs_opened = "
        "fs_file:open(filesystem.file.OpenModeFlag.ReadOnly)\n"
        "local fs_written: number | typeof(int64(0)) = "
        "fs_file:write(bytearray(\"data\"))\n"
        "local fs_raw_written: number | typeof(int64(0)) = "
        "fs_file:write(\"data\")\n"
        "local fs_entries = fs_dir:entry_info_list({\"*.bin\"})\n"
        "print(fs_size, fs_opened, fs_written, fs_raw_written, fs_entries, "
        "fs_dir:exists())\n"
        "local net = require(\"@network\")\n"
        "local socket = net.tcp_socket()\n"
        "socket:connect(\"state_changed\", function(state: number) "
        "print(state) end)\n"
        "local net_written: number | typeof(int64(0)) = "
        "socket:write(\"data\")\n"
        "local address = net.host_address(\"127.0.0.1\")\n"
        "local ssl_server = net.ssl_server()\n"
        "local secure_peer = ssl_server:next_pending_connection()\n"
        "if secure_peer then print(secure_peer:is_encrypted()) end\n"
        "print(socket, address, net_written)\n");
    QTextDocument document(source);
    timer.restart();
    server.onOpenDocument(uri, &document);
    for (const auto &diagnostic : diagnostics) {
        std::cerr << diagnostic.message.toStdString() << '\n';
    }
    require(diagnostics.isEmpty(), "generated bindings produced diagnostics");
    const auto socketHover = server.hover(
        {uri, positionOf(source, "print(socket, address", 7)}, nullptr);
    require(socketHover &&
                socketHover->contents.value.contains("network.tcp_socket") &&
                !socketHover->contents.value.contains("_qt_"),
            qPrintable(QStringLiteral("socket hover exposes internal types: %1")
                           .arg(socketHover ? socketHover->contents.value
                                            : QStringLiteral("missing"))));
    const auto dialogHover = server.hover(
        {uri, positionOf(source, "print(text, dialog", 13)}, nullptr);
    require(dialogHover && dialogHover->contents.value.contains("qt.QDialog") &&
                !dialogHover->contents.value.contains("_qt_") &&
                !dialogHover->contents.value.contains("__WingLuauQt"),
            "hover exposes internal Qt type names");
    const auto ffiHover =
        server.hover({uri, positionOf(source, "ffi.sizeof", 1)}, nullptr);
    require(ffiHover && ffiHover->contents.value.contains("cffi.Instance") &&
                !ffiHover->contents.value.contains("__WingLuauQt"),
            "FFI hover lost the public instance type");
    const auto factoryHover = server.hover(
        {uri, positionOf(source, "\"@cffi\").new()", 10)}, nullptr);
    require(factoryHover &&
                factoryHover->contents.value.contains("function cffi.new("),
            "factory hover omitted its public qualifier or function spacing");
    const auto literalHover =
        server.hover({uri, positionOf(source, "print(literal)", 7)}, nullptr);
    require(literalHover &&
                literalHover->contents.value.contains("\"_qt_QWidget\""),
            "type rendering rewrote a string singleton");
    const auto textHover = server.hover(
        {uri, positionOf(source, "print(text, dialog", 7)}, nullptr);
    require(textHover && textHover->contents.value.contains("ustring") &&
                !textHover->contents.value.contains("__WingLuauQt"),
            "global text type lost its public name");
    const auto uiSignature = server.signatureHelp(
        {uri, positionOf(source, "qt.load_ui_widget_file(", 23)}, nullptr);
    require(uiSignature && !uiSignature->signatures.empty() &&
                uiSignature->signatures.front().label.contains("qt.QWidget") &&
                !uiSignature->signatures.front().label.contains("_qt_"),
            "signature help exposes internal type names");
    std::cout << "Required Qt and cffi analysis: " << timer.elapsed() << "ms\n";
    document.setPlainText(source + "\ndialog.\n");
    server.onUpdateDocument(uri, &document);
    const auto completion = server.completion(
        {uri, positionOf(document.toPlainText(), "dialog.\n", 7), std::nullopt},
        nullptr);
    require(hasCompletion(completion, "set_window_title"),
            "required Qt type lost inherited methods");
    document.setPlainText(source + "\ndialog:missing_binding_member()\n");
    server.onUpdateDocument(uri, &document);
    require(!diagnostics.isEmpty(), "required Qt type degraded to any");
    server.onCloseDocument(uri);
}

static void testHexdocModule() {
    const QDir output(qEnvironmentVariable("WING_BINDINGS_DIR"));
    QFile manifest(output.filePath("manifest.json"));
    require(manifest.open(QIODevice::ReadOnly),
            "generated manifest unavailable");
    const auto model = QJsonDocument::fromJson(manifest.readAll()).object();
    LuauLanguageServer::InitializationOptions options;
    const QDir definitions(output.filePath("definitions"));
    int index = 0;
    for (const auto &name : model.value("definition_preload").toArray()) {
        options.definitionsFiles.insert(
            QString::number(index++).rightJustified(5, '0'),
            definitions.filePath(name.toString()));
    }
    QTemporaryDir modules;
    require(modules.isValid(), "temporary directory unavailable");
    const QDir source(qEnvironmentVariable("WING_SOURCE_DIR"));
    require(QFile::copy(source.filePath("src/luau/modules/hexdoc.luau"),
                        modules.filePath("hexdoc.luau")),
            "cannot copy module");
    require(QFile::copy(output.filePath("generated/hexdoc_api.luau"),
                        modules.filePath("hexdoc_api.luau")),
            "cannot copy operations");
    options.systemModuleRoot = modules.path();
    options.symbolCatalogFile =
        definitions.filePath(model.value("symbol_catalog").toString());
    LuauLanguageServer server;
    QVector<lsp::Diagnostic> diagnostics;
    QObject::connect(&server, &LuauLanguageServer::sigOnPublishDiagnostic,
                     &server,
                     [&](const lsp::DocumentUri &diagnosticUri,
                         const QVector<lsp::Diagnostic> &items) {
                         diagnostics = items;
                         for (const auto &item : items) {
                             std::cerr << diagnosticUri.toString().toStdString()
                                       << ":" << item.range.start.line << ": "
                                       << item.message.toStdString() << '\n';
                         }
                     });
    server.initialize(options);
    require(diagnostics.isEmpty(), "hexdoc definitions failed");
    bool foundModule = false;
    bool foundDocument = false;
    bool foundOperation = false;
    for (const auto &root : server.builtinSymbols()) {
        if (root.item.label != QLatin1String("hexdoc")) {
            continue;
        }
        foundModule =
            root.moduleRoot && root.requiresImport &&
            root.importPath == QLatin1String("@s/hexdoc") &&
            root.category == LuauLanguageServer::SymbolCategory::Document;
        for (const auto &child : server.symbolChildren(root)) {
            if (child.item.label != QLatin1String("Document")) {
                continue;
            }
            foundDocument = child.hasChildren;
            for (const auto &method : server.symbolChildren(child)) {
                if (method.item.label == QLatin1String("read_bytes")) {
                    foundOperation = !method.fullDeclaration.isEmpty();
                }
            }
        }
    }
    require(foundModule && foundDocument && foundOperation,
            "hexdoc module or exported type missing from the object catalog");
    const auto uri = QUrl::fromLocalFile(modules.filePath("probe.luau"));
    const QString valid = QStringLiteral(
        "--!strict\nlocal hexdoc = require(\"@s/hexdoc\")\n"
        "local doc = hexdoc.new()\nlocal code: number = "
        "doc:open_file(\"test.bin\")\n"
        "local closed: number = doc:close()\n"
        "local isValid: boolean = doc:is_valid()\n"
        "local own = reader.opened_files()\n"
        "local all = reader.all_opened_files()\n"
        "local rawValid: boolean = reader.is_file_opened(0)\n"
        "for _, url in own do local scheme: string = url:scheme(); "
        "print(scheme) end\n"
        "print(isValid, rawValid, all)\n"
        "local bytes: string = doc:use(function(d: hexdoc.Document)\n"
        "    d:move_to(1)\n    d:move_to(0, 1, 0, true)\n"
        "    d:select(0, 1)\n    return d:read_bytes(0, 16)\nend)\n"
        "local first: string, missing: nil, last: boolean = doc:use(\n"
        "    function(d: hexdoc.Document, value: string): (string, nil, "
        "boolean)\n"
        "        d:use(function(inner: hexdoc.Document) inner:select(0, 1) "
        "end)\n"
        "        return value, nil, true\n    end, \"value\")\n"
        "print(code, closed, bytes, first, missing, last)\n");
    QTextDocument document(valid);
    server.onOpenDocument(uri, &document);
    require(diagnostics.isEmpty(), "valid hexdoc program failed type checking");
    document.setPlainText(valid + "\ndoc.\n");
    server.onUpdateDocument(uri, &document);
    const auto completion = server.completion(
        {uri, positionOf(document.toPlainText(), "doc.\n", 4), std::nullopt},
        nullptr);
    require(hasCompletion(completion, "read_bytes") &&
                hasCompletion(completion, "close_handle") &&
                hasCompletion(completion, "is_valid"),
            "hexdoc require lost typed completion");
    const QString callbackSource = QStringLiteral(
        "--!strict\nlocal hexdoc = require(\"@s/hexdoc\")\n"
        "local doc = hexdoc.new()\ndoc:use(function(d: hexdoc.Document)\n"
        "    d.\nend)\n");
    document.setPlainText(callbackSource);
    server.onUpdateDocument(uri, &document);
    const auto callbackCompletion = server.completion(
        {uri, positionOf(callbackSource, "d.\n", 2), std::nullopt}, nullptr);
    require(hasCompletion(callbackCompletion, "select") &&
                hasCompletion(callbackCompletion, "metadata"),
            "hexdoc callback lost operation completion");
    document.setPlainText(valid + "\ndoc:move_to(0, 1)\n");
    server.onUpdateDocument(uri, &document);
    const auto signatures = server.signatureHelp(
        {uri, positionOf(document.toPlainText(), "doc:move_to(0, 1)", 15)},
        nullptr);
    require(signatures && signatures->signatures.size() >= 2,
            "hexdoc operation overloads were lost");
    for (const QString &invalid :
         {QStringLiteral("reader.is_file_opened(\"bad\")"),
          QStringLiteral("local invalid: boolean = reader.opened_files()[1]"),
          QStringLiteral("doc:is_valid(1)"),
          QStringLiteral("doc:read_bytes(\"bad\", 1)"),
          QStringLiteral("local invalid: boolean = doc:read_bytes(0, 1)"),
          QStringLiteral("doc:missing_member()"),
          QStringLiteral("doc:move_to()")}) {
        document.setPlainText(valid + invalid + '\n');
        server.onUpdateDocument(uri, &document);
        require(!diagnostics.isEmpty(),
                "hexdoc invalid API use escaped type checking");
    }
    server.onCloseDocument(uri);
    QFile moduleSource(modules.filePath("hexdoc.luau"));
    require(moduleSource.open(QIODevice::ReadOnly),
            "cannot read module source");
    QTextDocument moduleDocument(QString::fromUtf8(moduleSource.readAll()));
    const auto moduleUri = QUrl::fromLocalFile(moduleSource.fileName());
    server.onOpenDocument(moduleUri, &moduleDocument);
    require(diagnostics.isEmpty(),
            "hexdoc source module failed strict checking");
    server.onCloseDocument(moduleUri);
}

int main(int argc, char **argv) {
    QGuiApplication application(argc, argv);
    try {
        const auto scenario = application.arguments().value(1);
        if (scenario == QLatin1String("hexdoc")) {
            testHexdocModule();
        }
        if (scenario == QLatin1String("bindings")) {
            testGeneratedBindings();
        }
        if (scenario.isEmpty() || scenario == QLatin1String("features")) {
            testLanguageFeatures();
            testColonCompletion();
        }
        if (scenario.isEmpty() || scenario == QLatin1String("types")) {
            testTypeDisplayIdentity();
            testTypeFunctionsAndNonstrictInference();
        }
        if (scenario.isEmpty() || scenario == QLatin1String("overloads")) {
            testDefinitionsAndOverloads();
        }
        if (scenario.isEmpty() || scenario == QLatin1String("modules")) {
            testRequiredModuleUpdates();
            testRequirePathCompletion();
            testBuiltinRequirePathCompletion();
            testBuiltinResolutionBoundaries();
            testRequireAliasDirectoryCompletion();
        }
        std::cout << "Luau language server tests passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
