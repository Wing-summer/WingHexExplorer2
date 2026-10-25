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

#include "luaulanguageserver.h"

#include "Luau/ToString.h"
#include "Luau/TypeUtils.h"
#include "typehelpers.h"
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>

static void collectSymbolProperties(Luau::TypeId type,
                                    Luau::TableType::Props &properties,
                                    QSet<Luau::TypeId> &visited) {
    type = Luau::follow(type);
    if (visited.contains(type)) {
        return;
    }
    visited.insert(type);
    if (auto table = Luau::get<Luau::TableType>(type)) {
        properties.insert(table->props.begin(), table->props.end());
    } else if (auto external = Luau::get<Luau::ExternType>(type)) {
        properties.insert(external->props.begin(), external->props.end());
        if (external->parent) {
            collectSymbolProperties(*external->parent, properties, visited);
        }
    } else if (auto meta = Luau::get<Luau::MetatableType>(type)) {
        collectSymbolProperties(meta->table, properties, visited);
        auto index =
            LuauLsp::lookupProp(Luau::follow(meta->metatable), "__index");
        if (!index.empty() && index.front().property.readTy) {
            collectSymbolProperties(*index.front().property.readTy, properties,
                                    visited);
        }
    }
}

QString
LuauLanguageServer::symbolTypeAnnotation(Luau::TypeId type,
                                         Luau::ToStringOptions &options) {
    type = Luau::follow(type);
    if (auto table = Luau::get<Luau::TableType>(type)) {
        if (table->name) {
            return QString::fromStdString(*table->name);
        }
        return table->props.empty() && !table->indexer
                   ? QStringLiteral("{}")
                   : QStringLiteral("{ ... }");
    }
    if (Luau::get<Luau::MetatableType>(type)) {
        return QStringLiteral("{ ... }");
    }
    return QString::fromStdString(Luau::toString(type, options));
}

static QString symbolFunctionSignature(const QString &name,
                                       const Luau::FunctionType &function,
                                       Luau::ToStringOptions &options) {
    auto signature = QString::fromStdString(
        Luau::toStringNamedFunction(name.toStdString(), function, options));
    signature.replace(QLatin1String("_: "), QLatin1String());
    return signature;
}

void LuauLanguageServer::formatSymbolDeclaration(
    LuauLanguageServer::Symbol &symbol, Luau::TypeId type,
    Luau::ToStringOptions options) {
    const auto &name = symbol.item.label;
    options.hideFunctionSelfArgument =
        symbol.item.kind == lsp::CompletionItemKind::Method;
    if (symbol.item.kind == lsp::CompletionItemKind::Function ||
        symbol.item.kind == lsp::CompletionItemKind::Method) {
        QStringList signatures;
        if (auto function = Luau::get<Luau::FunctionType>(type)) {
            signatures.append(
                symbolFunctionSignature(name, *function, options));
        } else if (auto intersection =
                       Luau::get<Luau::IntersectionType>(type)) {
            for (auto part : intersection->parts) {
                if (auto function =
                        Luau::get<Luau::FunctionType>(Luau::follow(part))) {
                    signatures.append(
                        symbolFunctionSignature(name, *function, options));
                }
            }
        }
        if (!signatures.isEmpty()) {
            symbol.overloads = signatures;
            symbol.item.detail = signatures.first();
            if (signatures.size() > 1) {
                *symbol.item.detail += LuauLanguageServer::tr(
                    " (+%n overload(s))", nullptr, signatures.size() - 1);
            }
            symbol.fullDeclaration = signatures.join('\n');
            return;
        }
    }

    symbol.item.detail = name + ": " + symbolTypeAnnotation(type, options);
    options.maxTableLength = 1000;
    options.maxTypeLength = 2000;
    symbol.fullDeclaration =
        name + ": " + QString::fromStdString(Luau::toString(type, options));
}

LuauLanguageServer::Symbol LuauLanguageServer::createGlobalSymbol(
    const QString &name, Luau::TypeId type, lsp::CompletionItemKind kind,
    Luau::ToStringOptions &options, QSet<Luau::TypeId> &ancestors, int depth) {
    type = Luau::follow(type);
    Symbol symbol;
    symbol.item.label = name;
    if (kind != lsp::CompletionItemKind::Interface &&
        (Luau::get<Luau::FunctionType>(type) ||
         Luau::isOverloadedFunction(type))) {
        kind = kind == lsp::CompletionItemKind::Property
                   ? lsp::CompletionItemKind::Method
                   : lsp::CompletionItemKind::Function;
    } else if (Luau::get<Luau::ExternType>(type)) {
        kind = lsp::CompletionItemKind::Class;
    } else if ((Luau::get<Luau::TableType>(type) ||
                Luau::get<Luau::MetatableType>(type)) &&
               kind == lsp::CompletionItemKind::Variable) {
        kind = lsp::CompletionItemKind::Module;
    }
    symbol.item.kind = kind;
    formatSymbolDeclaration(symbol, type, options);
    if (auto docs = getDocumentationForType(type)) {
        symbol.item.documentation = {lsp::MarkupKind::Markdown, *docs};
    }

    // Keep recursive references visible, but do not expand them again.
    if (ancestors.contains(type) || ancestors.size() >= 32) {
        return symbol;
    }
    ancestors.insert(type);
    Luau::TableType::Props properties;
    QSet<Luau::TypeId> visited;
    collectSymbolProperties(type, properties, visited);
    symbol.hasChildren = !properties.empty();
    if (depth == 0) {
        ancestors.remove(type);
        return symbol;
    }
    symbol.children.reserve(properties.size());
    for (const auto &[propertyName, property] : properties) {
        const auto propertyType =
            property.readTy ? property.readTy : property.writeTy;
        if (propertyType) {
            symbol.children.append(createGlobalSymbol(
                QString::fromStdString(propertyName), *propertyType,
                lsp::CompletionItemKind::Property, options, ancestors,
                depth - 1));
        }
    }
    ancestors.remove(type);
    return symbol;
}

namespace {
LuauLanguageServer::Symbol catalogSymbol(const QJsonObject &object) {
    LuauLanguageServer::Symbol symbol;
    symbol.id = object.value(QStringLiteral("id")).toString();
    using Category = LuauLanguageServer::SymbolCategory;
    static const QHash<QString, Category> categories = {
        {QStringLiteral("standard"), Category::Standard},
        {QStringLiteral("document"), Category::Document},
        {QStringLiteral("logging"), Category::Logging},
        {QStringLiteral("values"), Category::Values},
        {QStringLiteral("data"), Category::Data},
        {QStringLiteral("services"), Category::Services},
        {QStringLiteral("qt"), Category::Qt},
        {QStringLiteral("ffi"), Category::Ffi},
        {QStringLiteral("plugins"), Category::Plugins},
    };
    symbol.category = categories.value(
        object.value(QStringLiteral("category")).toString(), Category::Data);
    symbol.moduleRoot = object.value(QStringLiteral("module_root")).toBool();
    symbol.membersFile = object.value(QStringLiteral("members")).toString();
    symbol.requiresImport = object.value(QStringLiteral("require")).toBool();
    symbol.hasChildren = !symbol.membersFile.isEmpty();
    symbol.item.label = object.value(QStringLiteral("label")).toString();
    symbol.item.detail = object.value(QStringLiteral("detail")).toString();
    symbol.fullDeclaration =
        object.value(QStringLiteral("declaration")).toString();
    const auto overloads = object.value(QStringLiteral("overloads")).toArray();
    for (const auto &signature : overloads) {
        symbol.overloads.append(signature.toString());
    }
    const auto kind = object.value(QStringLiteral("kind")).toString();
    symbol.item.kind =
        kind == QStringLiteral("interface") ? lsp::CompletionItemKind::Interface
        : kind == QStringLiteral("constant") ? lsp::CompletionItemKind::Constant
        : kind == QStringLiteral("property") ? lsp::CompletionItemKind::Property
        : kind == QStringLiteral("class")    ? lsp::CompletionItemKind::Class
        : kind == QStringLiteral("module") ? lsp::CompletionItemKind::Module
                                           : lsp::CompletionItemKind::Function;
    return symbol;
}
QJsonObject readSymbolPage(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Cannot read symbol catalog: %s", qPrintable(path));
        return {};
    }
    return QJsonDocument::fromJson(file.readAll()).object();
}
} // namespace

void LuauLanguageServer::initializeSymbolCatalog(
    const WingScriptBind::ScriptSymbolCatalog &plugins) {
    symbolPages.clear();
    builtinRoots.reset();
    catalogRoots.clear();
    pluginRoots.clear();
    pluginPages.clear();
    if (!symbolCatalogFile.isEmpty()) {
        const auto roots = readSymbolPage(symbolCatalogFile)
                               .value(QStringLiteral("roots"))
                               .toArray();
        for (const auto &entry : roots) {
            catalogRoots.append(catalogSymbol(entry.toObject()));
        }
    }
    QSet<QString> parents;
    for (const auto &entry : plugins) {
        parents.insert(entry.parent);
    }
    for (const auto &entry : plugins) {
        Symbol symbol;
        symbol.id = entry.id;
        symbol.source = SymbolSource::Plugin;
        symbol.category = SymbolCategory::Plugins;
        symbol.moduleRoot = entry.parent.isEmpty();
        symbol.item.label = entry.label;
        symbol.item.detail = entry.detail;
        using Kind = WingScriptBind::ScriptSymbol::Kind;
        switch (entry.kind) {
        case Kind::Module:
            symbol.item.kind = lsp::CompletionItemKind::Module;
            break;
        case Kind::Class:
            symbol.item.kind = lsp::CompletionItemKind::Class;
            break;
        case Kind::Function:
            symbol.item.kind = lsp::CompletionItemKind::Function;
            break;
        case Kind::Property:
            symbol.item.kind = lsp::CompletionItemKind::Property;
            break;
        case Kind::Constant:
            symbol.item.kind = lsp::CompletionItemKind::Constant;
            break;
        }
        symbol.hasChildren = parents.contains(entry.id);
        if (symbol.moduleRoot) {
            pluginRoots.append(std::move(symbol));
        } else {
            pluginPages[entry.parent].append(std::move(symbol));
        }
    }
}

QList<LuauLanguageServer::Symbol> LuauLanguageServer::builtinSymbols() {
    if (!isReady) {
        return {};
    }
    if (builtinRoots) {
        return *builtinRoots;
    }
    QList<Symbol> result;
    QSet<QString> names;
    for (const auto &symbol : std::as_const(catalogRoots)) {
        names.insert(symbol.item.label);
        result.append(symbol);
    }
    for (const auto &symbol : std::as_const(pluginRoots)) {
        names.insert(symbol.item.label);
        result.append(symbol);
    }
    const auto scope = frontend.globals.globalScope;
    Luau::ToStringOptions options;
    options.scope = scope;
    options.hideTableAliasExpansions = true;
    options.maxTableLength = 48;
    options.maxTypeLength = 200;
    QSet<Luau::TypeId> ancestors;
    for (const auto &[name, binding] : scope->bindings) {
        const auto label = QString::fromUtf8(name.c_str());
        if (names.contains(label) || label.startsWith('_')) {
            continue;
        }
        auto symbol = createGlobalSymbol(label, binding.typeId,
                                         lsp::CompletionItemKind::Variable,
                                         options, ancestors, 0);
        symbol.id = QStringLiteral("global:") + label;
        symbol.source = SymbolSource::Global;
        symbol.category = SymbolCategory::Standard;
        symbol.path = {label};
        symbol.moduleRoot = symbol.item.kind == lsp::CompletionItemKind::Module;
        result.append(std::move(symbol));
    }
    builtinRoots = result;
    return result;
}

QList<LuauLanguageServer::Symbol>
LuauLanguageServer::symbolChildren(const Symbol &symbol) {
    if (symbolPages.contains(symbol.id)) {
        return symbolPages.value(symbol.id);
    }
    QList<Symbol> result;
    if (!symbol.membersFile.isEmpty()) {
        if (QFileInfo(symbol.membersFile).fileName() != symbol.membersFile) {
            return {};
        }
        const auto page = readSymbolPage(
            QFileInfo(symbolCatalogFile).dir().filePath(symbol.membersFile));
        const auto children = page.value(QStringLiteral("children")).toArray();
        for (const auto &entry : children) {
            result.append(catalogSymbol(entry.toObject()));
        }
    } else if (symbol.source == SymbolSource::Plugin) {
        return pluginPages.value(symbol.id);
    } else if (symbol.source == SymbolSource::Global) {
        const auto &path = symbol.path;
        if (path.isEmpty()) {
            return {};
        }
        const auto scope = frontend.globals.globalScope;
        const auto binding =
            scope->lookup(Luau::AstName(path.front().toUtf8().constData()));
        if (binding) {
            auto type = *binding;
            for (qsizetype i = 1; i < path.size(); ++i) {
                const auto properties = LuauLsp::lookupProp(
                    Luau::follow(type), path[i].toStdString());
                if (properties.empty() || !properties.front().property.readTy) {
                    return {};
                }
                type = *properties.front().property.readTy;
            }
            Luau::ToStringOptions options;
            options.scope = scope;
            options.maxTableLength = 48;
            options.maxTypeLength = 200;
            QSet<Luau::TypeId> ancestors;
            auto parent = createGlobalSymbol(symbol.item.label, type,
                                             lsp::CompletionItemKind::Module,
                                             options, ancestors, 1);
            result = std::move(parent.children);
            for (auto &child : result) {
                child.id = symbol.id + '/' + child.item.label;
                child.source = SymbolSource::Global;
                child.category = symbol.category;
                child.path = path;
                child.path.append(child.item.label);
            }
        }
    }
    if (symbol.category == SymbolCategory::Standard &&
        symbol.source == SymbolSource::Catalog && symbol.moduleRoot) {
        Symbol standard = symbol;
        standard.id = QStringLiteral("global:") + symbol.item.label;
        standard.membersFile.clear();
        standard.source = SymbolSource::Global;
        standard.path = {symbol.item.label};
        standard.moduleRoot = false;
        for (auto &child : symbolChildren(standard)) {
            const auto found = std::find_if(
                result.cbegin(), result.cend(), [&child](const Symbol &item) {
                    return item.item.label == child.item.label;
                });
            if (found == result.cend()) {
                result.append(child);
            }
        }
    }
    symbolPages.insert(symbol.id, result);
    return result;
}
