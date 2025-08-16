#ifndef LSP_H
#define LSP_H

#include <QList>
#include <QString>

namespace LSP {

enum class ErrorCode {
    // Defined by JSON RPC.
    ParseError = -32700,
    InvalidRequest = -32600,
    MethodNotFound = -32601,
    InvalidParams = -32602,
    InternalError = -32603,
    ServerNotInitialized = -32002,
    UnknownErrorCode = -32001,
    // Defined by the protocol.
    RequestCancelled = -32800,
};

enum class TraceLevel {
    Off = 0,
    Messages = 1,
    Verbose = 2,
};

enum class TextDocumentSyncKind {
    /// Documents should not be synced at all.
    None = 0,

    /// Documents are synced by always sending the full content of the document.
    Full = 1,

    /// Documents are synced by sending the full content on open.  After that
    /// only incremental updates to the document are send.
    Incremental = 2,
};

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

enum class SymbolKind {
    File = 1,
    Module = 2,
    Namespace = 3,
    Package = 4,
    Class = 5,
    Method = 6,
    Property = 7,
    Field = 8,
    Constructor = 9,
    Enum = 10,
    Interface = 11,
    Function = 12,
    Variable = 13,
    Constant = 14,
    String = 15,
    Number = 16,
    Boolean = 17,
    Array = 18,
    Object = 19,
    Key = 20,
    Null = 21,
    EnumMember = 22,
    Struct = 23,
    Event = 24,
    Operator = 25,
    TypeParameter = 26
};

enum class MarkupKind {
    PlainText,
    Markdown,
};

enum class ResourceOperationKind { Create, Rename, Delete };

enum class FailureHandlingKind {
    Abort,
    Transactional,
    Undo,
    TextOnlyTransactional
};

enum class MessageType {
    /// An error message.
    Error = 1,
    /// A warning message.
    Warning = 2,
    /// An information message.
    Info = 3,
    /// A log message.
    Log = 4,
};

enum class FileChangeType {
    /// The file got created.
    Created = 1,
    /// The file got changed.
    Changed = 2,
    /// The file got deleted.
    Deleted = 3
};

enum class FoldingRangeKind {
    Comment,
    Imports,
    Region,
};

enum class DiagnosticSeverity {
    None = 0,
    Error = 1,       // Reports an error
    Warning = 2,     // Reports a warning
    Information = 3, // Reports an information
    Hint = 4         // Reports a hint
};

enum class CompletionTriggerKind {
    /// Completion was triggered by typing an identifier (24x7 code
    /// complete), manual invocation (e.g Ctrl+Space) or via API.
    Invoked = 1,
    /// Completion was triggered by a trigger character specified by
    /// the `triggerCharacters` properties of the
    /// `CompletionRegistrationOptions`.
    TriggerCharacter = 2,
    /// Completion was re-triggered as the current completion list is
    /// incomplete.
    TriggerTriggerForIncompleteCompletions = 3
};

enum class InsertTextFormat {
    Missing = 0,
    /// The primary text to be inserted is treated as a plain string.
    PlainText = 1,
    /// The primary text to be inserted is treated as a snippet.
    ///
    /// A snippet can define tab stops and placeholders with `$1`, `$2`
    /// and `${3:foo}`. `$0` defines the final tab stop, it defaults to the end
    /// of the snippet. Placeholders with equal identifiers are linked, that is
    /// typing in one will update others too.
    ///
    /// See also:
    /// https//github.com/Microsoft/vscode/blob/master/src/vs/editor/contrib/snippet/common/snippet.md
    Snippet = 2,
};

struct Location {
    int line = -1;
    int character = -1;
};

struct Range {
    Location start;
    Location end;
};

struct Diagnostics {
    QString message;
    Range range;
    DiagnosticSeverity severity = DiagnosticSeverity::None;
    // QString source; = "AngelScript"
};

struct CompletionItem {
    CompletionItemKind kind = CompletionItemKind::Missing;
    QString label;
    int data;
};

struct ResolvedCompletionItem : public CompletionItem {
    QString detail;
    QString insertText;
};

struct TextDocumentContentChangeEvent {
    /// The range of the document that changed.
    Range range;
    /// The length of the range that got replaced.
    int rangeLength;
    /// The new text of the range/document.
    QString text;
};

} // namespace LSP

#endif // LSP_H
