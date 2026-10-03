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

#ifndef LUAU_LSP_TYPES_H
#define LUAU_LSP_TYPES_H

#include <optional>
#include <variant>
#include <vector>

#include <QHash>
#include <QUrl>

namespace lsp {

using DocumentUri = QUrl;

Q_NAMESPACE
enum class CompletionItemKind {
    Missing = 0,
    Text = 1,
    Method = 2,
    Function = 3,
    Constructor = 4,
    Field = 5,
    Variable = 6,
    Class = 7,
    Interface = 8,
    Module = 9,
    Property = 10,
    Unit = 11,
    Value = 12,
    Enum = 13,
    Keyword = 14,
    Snippet = 15,
    Color = 16,
    File = 17,
    Reference = 18,
    Folder = 19,
    EnumMember = 20,
    Constant = 21,
    Struct = 22,
    Event = 23,
    Operator = 24,
    TypeParameter = 25,
};
Q_ENUM_NS(CompletionItemKind)

enum class InsertTextFormat {
    PlainText = 1,
    Snippet = 2,
};

enum class MarkupKind {
    PlainText,
    Markdown,
};

enum class CompletionTriggerKind {
    Invoked = 1,
    TriggerCharacter = 2,
    TriggerForIncompleteCompletions = 3,
};

enum class DiagnosticSeverity {
    Error = 1,
    Warning = 2,
    Information = 3,
    Hint = 4,
};

enum class DiagnosticTag {
    Unnecessary = 1,
    Deprecated = 2,
};

enum class DocumentDiagnosticReportKind {
    Full,
    Unchanged,
};

enum class ErrorCode {
    RequestCancelled = -32800,
};

enum class MessageType {
    Error = 1,
    Warning = 2,
    Info = 3,
    Log = 4,
};

enum class SemanticTokenTypes {
    Namespace,
    Type,
    Class,
    Enum,
    EnumMember,
    Variable,
    Property,
    Function,
    Method,
    Parameter,
    Event,
};

enum class SemanticTokenModifiers : int {
    None = 0,
    DefaultLibrary = 1 << 0,
    Readonly = 1 << 1,
};

constexpr SemanticTokenModifiers operator|(SemanticTokenModifiers left,
                                           SemanticTokenModifiers right) {
    return SemanticTokenModifiers(int(left) | int(right));
}

constexpr SemanticTokenModifiers operator&(SemanticTokenModifiers left,
                                           SemanticTokenModifiers right) {
    return SemanticTokenModifiers(int(left) & int(right));
}

struct Position {
    uint line = 0;
    uint character = 0;
    bool operator==(const Position &) const = default;
};

struct CodeDescription {
    QUrl href;
};

struct Range {
    Position start;
    Position end;
    bool operator==(const Range &) const = default;
};

struct Location {
    DocumentUri uri;
    Range range;
    bool operator==(const Location &) const = default;
};

struct TextEdit {
    Range range;
    QString newText;
};

struct MarkupContent {
    MarkupKind kind = MarkupKind::PlainText;
    QString value;
};

struct CompletionParams {
    DocumentUri textDocument;
    Position position;
    struct Context {
        CompletionTriggerKind triggerKind = CompletionTriggerKind::Invoked;
        std::optional<QString> triggerCharacter;
    };
    std::optional<Context> context;
};

struct CompletionLabelDetails {
    std::optional<QString> detail;
    std::optional<QString> description;
};

struct CompletionItem {
    QString label;
    std::optional<CompletionItemKind> kind;
    std::optional<CompletionLabelDetails> labelDetails;
    std::optional<QString> detail;
    MarkupContent documentation;
    bool deprecated = false;
    std::optional<QString> sortText;
    std::optional<QString> filterText;
    std::optional<QString> insertText;
    InsertTextFormat insertTextFormat = InsertTextFormat::PlainText;
    std::optional<TextEdit> textEdit;
    std::vector<TextEdit> additionalTextEdits;
};

struct SignatureHelpParams {
    DocumentUri textDocument;
    Position position;
};

struct ParameterInformation {
    std::variant<QString, std::vector<qsizetype>> label;
    MarkupContent documentation;
};

struct SignatureInformation {
    QString label;
    MarkupContent documentation;
    std::vector<ParameterInformation> parameters;
    std::optional<uint> activeParameter;
};

struct SignatureHelp {
    std::vector<SignatureInformation> signatures;
    std::optional<uint> activeSignature;
    std::optional<uint> activeParameter;
};

struct HoverParams {
    DocumentUri textDocument;
    Position position;
};

struct Hover {
    MarkupContent contents;
    std::optional<Range> range;
};

struct SemanticTokensParams {
    DocumentUri textDocument;
};

struct DefinitionParams {
    DocumentUri textDocument;
    Position position;
};

using DefinitionResult = std::vector<Location>;

struct TypeDefinitionParams {
    DocumentUri textDocument;
    Position position;
};

struct Diagnostic {
    Range range;
    DiagnosticSeverity severity = DiagnosticSeverity::Error;
    std::optional<std::variant<int, QString>> code;
    QString source;
    QString message;
    std::vector<DiagnosticTag> tags;
    std::optional<CodeDescription> codeDescription;
};

struct DocumentDiagnosticParams {
    DocumentUri textDocument;
};

struct DocumentDiagnosticReport {
    DocumentDiagnosticReportKind kind = DocumentDiagnosticReportKind::Full;
    QVector<Diagnostic> items;
    QHash<DocumentUri, DocumentDiagnosticReport> relatedDocuments;
};

struct WorkspaceDocumentDiagnosticReport {
    DocumentUri uri;
    DocumentDiagnosticReportKind kind = DocumentDiagnosticReportKind::Full;
    std::vector<Diagnostic> items;
};

struct WorkspaceDiagnosticReportPartialResult {
    std::vector<WorkspaceDocumentDiagnosticReport> items;
};

struct SemanticToken {
    int line = 0;          // 0-based
    int character = 0;     // 0-based
    int length = 0;        // UTF-16 code units / document positions
    QString tokenType;     // e.g. "class", "function", "typeParameter"
    QStringList modifiers; // e.g. "declaration", "readonly"
};

} // namespace lsp

#endif // LUAU_LSP_TYPES_H
