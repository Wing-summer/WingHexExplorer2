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

#include "Luau/Common.h"
#include "luau/lsp/luaulanguageserver.h"
#include "luau/wingluaurequire.h"
LUAU_FASTFLAG(LuauIntegerType2)

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include <iostream>
#include <stdexcept>

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
    FFlag::LuauIntegerType2.value = true;
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
    QObject::connect(&server, &LuauLanguageServer::onPublishDiagnostic, &server,
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
    QObject::connect(&server, &LuauLanguageServer::onPublishDiagnostic, &server,
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
        "local fs_file = filesystem.file(\"fixture.bin\")\n"
        "local fs_info = filesystem.fileinfo(\"fixture.bin\")\n"
        "local fs_dir = filesystem.dir(\".\")\n"
        "local fs_size: integer = fs_info:size()\n"
        "local fs_opened = "
        "fs_file:open(filesystem.file.OpenModeFlag.ReadOnly)\n"
        "local fs_written: integer = fs_file:write(bytearray(\"data\"))\n"
        "local fs_raw_written: integer = fs_file:write(\"data\")\n"
        "local fs_entries = fs_dir:entry_info_list({\"*.bin\"})\n"
        "print(fs_size, fs_opened, fs_written, fs_raw_written, fs_entries, "
        "fs_dir:exists())\n"
        "local net = require(\"@network\")\n"
        "local socket = net.tcp_socket()\n"
        "socket:connect(\"state_changed\", function(state: number) "
        "print(state) end)\n"
        "local net_written: integer = socket:write(\"data\")\n"
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
            "socket hover exposes internal types");
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

int main(int argc, char **argv) {
    QGuiApplication application(argc, argv);
    try {
        const auto scenario = application.arguments().value(1);
        if (scenario == QLatin1String("bindings")) {
            testGeneratedBindings();
        }
        if (scenario.isEmpty() || scenario == QLatin1String("features")) {
            testLanguageFeatures();
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
            testBuiltinResolutionBoundaries();
        }
        std::cout << "Luau language server tests passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
