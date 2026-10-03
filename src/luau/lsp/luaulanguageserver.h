#ifndef LUAULANGUAGESERVER_H
#define LUAULANGUAGESERVER_H

#include "lsp.h"
#include "semantictokensvisitor.h"

#include <functional>
#include <memory>
#include <unordered_map>

#include <QFileInfo>
#include <QHash>
#include <QStringList>
#include <QTextDocument>

#include "Luau/Autocomplete.h"
#include "Luau/Documentation.h"
#include "Luau/Frontend.h"
#include "Luau/TypeCheckLimits.h"

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
    };

    explicit LuauLanguageServer();
    static LuauLanguageServer &instance();

public:
    void onOpenDocument(const lsp::DocumentUri &uri, QTextDocument *document);
    void onSaveDocument(const lsp::DocumentUri &uri);
    void onCloseDocument(const lsp::DocumentUri &uri);
    void onUpdateDocument(const lsp::DocumentUri &uri, QTextDocument *document);

signals:
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

private:
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

    QStringList readAllowedCfgRoots;
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

    // Runs `Frontend::check` on the module and DISCARDS THE TYPE GRAPH.
    // Uses the diagnostic type checker, so strictness and DM awareness is not
    // enforced NOTE: do NOT use this if you later retrieve a ModulePtr (via
    // frontend.moduleResolver.getModule). Instead use `checkStrict` NOTE: use
    // `frontend.parse` if you do not care about typechecking
    Luau::CheckResult
    checkSimple(const Luau::ModuleName &moduleName,
                const LSPCancellationToken &cancellationToken);

    // Runs `Frontend::check` on the module whilst retaining the type graph.
    // Uses the autocomplete typechecker to enforce strictness and DM awareness.
    // NOTE: a disadvantage of the autocomplete typechecker is that it has a
    // timeout restriction that can often be hit
    Luau::CheckResult checkStrict(const Luau::ModuleName &moduleName,
                                  const LSPCancellationToken &cancellationToken,
                                  bool forAutocomplete = true);

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

    std::optional<QString> getDocumentationForType(const Luau::TypeId ty);

    std::optional<QString> getDocumentationForTypeReference(
        const Luau::ModuleName &moduleName, const Luau::ScopePtr &scope,
        const std::optional<Luau::AstName> &prefix, const Luau::Name &typeName,
        bool forAutocomplete);

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

    const Luau::ModulePtr getModule(const Luau::ModuleName &moduleName,
                                    bool forAutocomplete) const;

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
