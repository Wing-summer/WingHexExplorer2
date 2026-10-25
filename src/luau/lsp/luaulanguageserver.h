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

#ifndef LUAULANGUAGESERVER_H
#define LUAULANGUAGESERVER_H

#include "lsp.h"
#include "luau/scriptsymbol.h"
#include "semantictokensvisitor.h"

#include <functional>
#include <memory>
#include <unordered_map>

#include <QFileInfo>
#include <QHash>
#include <QSet>
#include <QStringList>
#include <QTextDocument>

#include "Luau/Autocomplete.h"
#include "Luau/Documentation.h"
#include "Luau/Frontend.h"
#include "Luau/TypeCheckLimits.h"
#include "luau/wingluaurequire.h"

namespace Luau {
struct ToStringOptions;
}

class LuauLanguageServer : public QObject,
                           Luau::FileResolver,
                           Luau::ConfigResolver {
    Q_OBJECT

    using LSPCancellationToken =
        std::shared_ptr<Luau::FrontendCancellationToken>;

public:
    struct InitializationOptions {
        Luau::Mode defaultMode = Luau::Mode::Nonstrict;
        QStringList readCfgRoots;
        QHash<QString, QString> definitionsFiles;
        QHash<QString, QString> builtinModuleFiles;
        QString symbolCatalogFile;
        QString userModuleRoot;
        QString systemModuleRoot;
        WingScriptBind::ScriptSymbolCatalog pluginSymbols;
    };

    explicit LuauLanguageServer();
    static LuauLanguageServer &instance();

public:
    void onOpenDocument(const lsp::DocumentUri &uri, QTextDocument *document);
    void onSaveDocument(const lsp::DocumentUri &uri);
    void onCloseDocument(const lsp::DocumentUri &uri);
    void onUpdateDocument(const lsp::DocumentUri &uri, QTextDocument *document);

signals:
    void initialized();
    void onPublishDiagnostic(const lsp::DocumentUri &,
                             const QVector<lsp::Diagnostic> &);

public:
    QVector<lsp::CompletionItem>
    completion(const lsp::CompletionParams &params,
               const LSPCancellationToken &cancellationToken);

    std::optional<lsp::SignatureHelp>
    signatureHelp(const lsp::SignatureHelpParams &params,
                  const LSPCancellationToken &cancellationToken);

    std::optional<lsp::Hover>
    hover(const lsp::HoverParams &params,
          const LSPCancellationToken &cancellationToken);

    QVector<SemanticToken>
    semanticTokens(const lsp::SemanticTokensParams &params,
                   const LSPCancellationToken &cancellationToken);

    lsp::DefinitionResult
    gotoDefinition(const lsp::DefinitionParams &params,
                   const LSPCancellationToken &cancellationToken);

    std::optional<lsp::Location>
    gotoTypeDefinition(const lsp::TypeDefinitionParams &params,
                       const LSPCancellationToken &cancellationToken);

public:
    void initialize(const InitializationOptions &options);

    enum class SymbolSource { Catalog, Global, Plugin };
    enum class SymbolCategory {
        Standard,
        Document,
        Logging,
        Values,
        Data,
        Services,
        Qt,
        Ffi,
        Plugins
    };
    struct Symbol {
        lsp::CompletionItem item;
        QString fullDeclaration;
        QStringList overloads;
        QList<Symbol> children;
        QString id;
        SymbolSource source = SymbolSource::Catalog;
        SymbolCategory category = SymbolCategory::Data;
        QStringList path;
        bool moduleRoot = false;
        QString membersFile;
        bool requiresImport = false;
        bool hasChildren = false;
    };

    // A value snapshot of registered globals and types, independent of the
    // frontend type graph. Empty until initialization completes.
    QList<Symbol> globalSymbols();
    QList<Symbol> builtinSymbols();
    QList<Symbol> symbolChildren(const Symbol &symbol);

private:
    Symbol createGlobalSymbol(const QString &name, Luau::TypeId type,
                              lsp::CompletionItemKind kind,
                              Luau::ToStringOptions &options,
                              QSet<Luau::TypeId> &ancestors, int depth = 32);

    void clearConfigCache();

private:
    struct DocumentHandle {
        lsp::DocumentUri uri;
        QTextDocument *document = nullptr;
        std::unique_ptr<QTextDocument> ownedDocument;

        DocumentHandle() = default;
        DocumentHandle(lsp::DocumentUri uri, QTextDocument *document);
        DocumentHandle(lsp::DocumentUri uri,
                       std::unique_ptr<QTextDocument> document);
        explicit operator bool() const { return document != nullptr; }
    };

    QTextDocument *getTextDocument(const Luau::ModuleName &moduleName);
    DocumentHandle
    getOrCreateTextDocumentFromModuleName(const Luau::ModuleName &name);

private:
    struct DefinitionsFileState {
        std::unique_ptr<QTextDocument> textDocument;
        Luau::SourceModule sourceModule;
        /// The checked module must be kept alive so that weak_ptr references
        /// in UserDefinedFunctionData::owner (used by type functions) remain
        /// valid.
        Luau::ModulePtr checkedModule;
    };

    /// Mapping between a definitions package name to its loaded state.
    /// Used for documentation comment lookup within definition files and
    /// to keep checked modules alive for type function lifetime.
    std::unordered_map<Luau::ModuleName, DefinitionsFileState>
        definitionsFileState{};

private:
    Luau::Frontend frontend;
    Luau::TypeCheckLimits limits;
    Luau::Config defaultConfig;

    QHash<QString, QString> builtinModuleFiles;
    QString symbolCatalogFile;
    QHash<QString, QList<Symbol>> symbolPages;
    QList<Symbol> pluginRoots;
    QHash<QString, QList<Symbol>> pluginPages;
    QList<Symbol> catalogRoots;
    std::optional<QList<Symbol>> builtinRoots;
    QStringList readAllowedCfgRoots;
    std::unique_ptr<WingLuauRequire::ResolutionSession> requireResolver;
    /// A registered definitions file passed by the client
    std::unordered_map<std::string, QString> definitionsFiles;
    /// A registered documentation file passed by the client
    QStringList documentationFiles;
    /// Parsed documentation database
    Luau::DocumentationDatabase documentation{};
    /// Whether this workspace folder has completed an initial set up process.
    /// Workspaces are initialized lazily on demand.
    /// When a new request comes in and the workspace is not ready, we will
    /// prepare it then.
    bool isReady = false;

    // Currently opened files where content is managed by client
    mutable std::unordered_map<Luau::ModuleName, QTextDocument *>
        managedFiles{};
    mutable std::unordered_map<QString, Luau::Config> configCache{};

    /// Whether the first-time configuration (platform, global types) have been
    /// applied for this folder. First-time configuration is only applied once,
    /// and changes require a language server restart
    bool appliedFirstTimeConfiguration = false;

    // FileResolver interface
private:
    virtual std::optional<Luau::SourceCode>
    readSource(const Luau::ModuleName &name) override;
    virtual std::optional<Luau::ModuleInfo>
    resolveModule(const Luau::ModuleInfo *context, Luau::AstExpr *expr,
                  const Luau::TypeCheckLimits &limits) override;
    virtual std::string
    getHumanReadableModuleName(const Luau::ModuleName &name) const override;
    virtual std::optional<std::string>
    getEnvironmentForModule(const Luau::ModuleName &name) const override;
    virtual const Luau::Config &
    getConfig(const Luau::ModuleName &name,
              const Luau::TypeCheckLimits &limits) const override;

private:
    void documentDiagnostics(const Luau::ModuleName &moduleName);

    /// Compute a document diagnostics report for a single file (and potentially
    /// related files) By default, this is called by the client for an open
    /// document. Hence we can expect that files are managed However, we
    /// sometimes call this as part of reverse-dependency updates (see
    /// updateTextDocument), where the file may be unmanaged In the default
    /// cause, we don't want to bother opening the file unnecessarily if it was
    /// closed.
    lsp::DocumentDiagnosticReport
    documentDiagnostics(const lsp::DocumentDiagnosticParams &params,
                        const LSPCancellationToken &cancellationToken,
                        bool allowUnmanagedFiles = false);

    // Check dependency diagnostics without retaining their internal type
    // graphs.
    Luau::CheckResult
    checkSimple(const Luau::ModuleName &moduleName,
                const LSPCancellationToken &cancellationToken);

    // Check using the document's configured mode and retain its type graph
    // for language features. Diagnostics and completion share one module.
    Luau::CheckResult
    checkStrict(const Luau::ModuleName &moduleName,
                const LSPCancellationToken &cancellationToken);

    void publishDiagnostics(const lsp::DocumentUri &uri,
                            const QVector<lsp::Diagnostic> &diagnostic) const;

private:
    std::optional<QString> getDocumentationForAutocompleteEntry(
        const std::string &name, const Luau::AutocompleteEntry &entry,
        const std::vector<Luau::AstNode *> &ancestry,
        const Luau::ModulePtr &localModule, const Luau::Position &position);

    /// Get all moonwave-style documentation comments
    /// Performs transformations so that the comments are normalised to lines
    /// inside of it (i.e., trimming whitespace, removing comment start/end)
    void extracted(QStringList &lines, int &indentation);
    QStringList getComments(const Luau::ModuleName &moduleName,
                            const Luau::Location &node);

    void
    initializeSymbolCatalog(const WingScriptBind::ScriptSymbolCatalog &plugins);
    static void formatSymbolDeclaration(Symbol &symbol, Luau::TypeId type,
                                        Luau::ToStringOptions options);
    static QString symbolTypeAnnotation(Luau::TypeId type,
                                        Luau::ToStringOptions &options);
    std::optional<QString> getDocumentationForType(const Luau::TypeId ty);

    std::optional<QString> getDocumentationForTypeReference(
        const Luau::ModuleName &moduleName, const Luau::ScopePtr &scope,
        const std::optional<Luau::AstName> &prefix, const Luau::Name &typeName);

    std::optional<QString>
    getDocumentationForAstNode(const Luau::ModuleName &moduleName,
                               const Luau::AstNode *node,
                               const Luau::ScopePtr scope);

private:
    void registerTypes();

    std::optional<Luau::ModuleInfo>
    resolveStringRequire(const Luau::ModuleInfo *context,
                         const std::string_view requiredString,
                         const Luau::TypeCheckLimits &limits);

    Luau::LoadDefinitionFileResult
    registerDefinitions(Luau::Frontend &frontend, Luau::GlobalTypes &globals,
                        const std::string &packageName,
                        const std::string &definitions);
    Luau::LoadDefinitionFileResult
    loadDefinitionFile(const std::string &packageName,
                       const std::string &source);

    const Luau::ModulePtr getModule(const Luau::ModuleName &moduleName) const;

    Luau::SourceCode::Type
    sourceCodeTypeFromPath(const std::string &path) const;

    const Luau::Config &
    readConfigRec(const QString &path,
                  const Luau::TypeCheckLimits &limits) const;

    void loadConfig(
        const Luau::Config &cfg, const QString &filePath,
        const std::function<std::optional<std::string>(const std::string &)>
            &parse) const;

    void clearDiagnosticsForFile(const lsp::DocumentUri &uri) const;

private:
    bool isIgnoredFile(const Luau::ModuleName &moduleName) const;
    bool isDefinitionFile(const Luau::ModuleName &moduleName) const;
};

#endif // LUAULANGUAGESERVER_H
