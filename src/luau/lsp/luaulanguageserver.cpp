#include "luaulanguageserver.h"

#include "../wingluaurequire.h"
#include "Luau/AstQuery.h"
#include "Luau/BuiltinDefinitions.h"
#include "Luau/Config.h"
#include "Luau/FragmentAutocomplete.h"
#include "Luau/LuauConfig.h"
#include "Luau/Normalize.h"
#include "Luau/PrettyPrinter.h"
#include "Luau/TimeTrace.h"
#include "Luau/ToString.h"
#include "Luau/TxnLog.h"
#include "Luau/TypeFunctionRuntime.h"
#include "Luau/TypeUtils.h"
#include "Luau/Unifier.h"
#include "Luau/UnifierSharedState.h"
#include "attachcommentsvisitor.h"
#include "findnodetype.h"
#include "lua.h"
#include "semantictokensvisitor.h"
#include "typehelpers.h"

#include <QDebug>
#include <QDir>
#include <QRegularExpression>
#include <QSet>
#include <QUrl>

struct LuauConfigInterruptInfo {
    Luau::TypeCheckLimits limits;
    std::string module;
};

/// Defining sort text levels assigned to completion items
/// Note that sort text is lexicographically
namespace SortText {
using SortTextT = const char *;
static constexpr SortTextT PrioritisedSuggestion = "0";
static constexpr SortTextT TableProperties = "1";
static constexpr SortTextT CorrectTypeKind = "2";
static constexpr SortTextT CorrectFunctionResult = "3";
static constexpr SortTextT Default = "4";
static constexpr SortTextT WrongIndexType = "5";
static constexpr SortTextT MetatableIndex = "6";
static constexpr SortTextT AutoImports = "7";
static constexpr SortTextT AutoImportsAbsolute = "71";
static constexpr SortTextT Keywords = "8";
static constexpr SortTextT Deprioritized = "9";
} // namespace SortText

std::optional<QString> getTypeName(Luau::TypeId typeId) {
    std::optional<QString> name;
    auto ty = Luau::follow(typeId);

    if (auto typeName = Luau::getName(ty)) {
        name = QString::fromStdString(*typeName);
    } else if (auto mtv = Luau::get<Luau::MetatableType>(ty)) {
        if (auto mtvName = Luau::getName(mtv->metatable)) {
            name = QString::fromStdString(*mtvName);
        }
    } else if (auto parentClass = Luau::get<Luau::ExternType>(ty)) {
        name = QString::fromStdString(parentClass->name);
    }
    // strip synthetic typeof() for builtin tables
    if (name && name->sliced(0, 7) == QLatin1String("typeof(") &&
        name->back() == ')') {
        return name->sliced(7, name->length() - 8);
    } else {
        return name;
    }
}

static Luau::AstNode *findNodeOrTypeAtPosition(const Luau::SourceModule &source,
                                               Luau::Position pos) {
    const Luau::Position end = source.root->location.end;
    if (pos < source.root->location.begin) {
        return source.root;
    }
    if (pos > end) {
        pos = end;
    }
    FindNodeType findNode{pos, end, /* closed: */ false};
    findNode.visit(source.root);
    return findNode.best;
}

static Luau::AstNode *
findNodeOrTypeAtPositionClosed(const Luau::SourceModule &source,
                               Luau::Position pos) {
    const Luau::Position end = source.root->location.end;
    if (pos < source.root->location.begin) {
        return source.root;
    }

    if (pos > end) {
        pos = end;
    }

    FindNodeType findNode{pos, end, /* closed: */ true};
    findNode.visit(source.root);
    return findNode.best;
}

LUAU_FASTINT(LuauTypeInferRecursionLimit)
LUAU_FASTINT(LuauTypeInferIterationLimit)

// Taken from Luau/Autocomplete.cpp
static bool checkOverloadMatch(Luau::TypePackId subTp, Luau::TypePackId superTp,
                               Luau::NotNull<Luau::Scope> scope,
                               Luau::TypeArena *typeArena,
                               Luau::NotNull<Luau::BuiltinTypes> builtinTypes) {
    Luau::InternalErrorReporter iceReporter;
    Luau::UnifierSharedState unifierState(&iceReporter);
    Luau::Normalizer normalizer{typeArena, builtinTypes,
                                Luau::NotNull{&unifierState},
                                Luau::SolverMode::Old};
    Luau::Unifier unifier(Luau::NotNull<Luau::Normalizer>{&normalizer}, scope,
                          Luau::Location(), Luau::Variance::Covariant);

    unifier.normalize = false;
    unifier.checkInhabited = false;

    return unifier.canUnify(subTp, superTp).empty();
}

static std::optional<Luau::ModuleName>
lookupImportedModule(const Luau::Scope &deepScope, const Luau::Name &name) {
    const Luau::Scope *scope = &deepScope;
    while (true) {
        auto it = scope->importedModules.find(name);
        if (it != scope->importedModules.end()) {
            return it->second;
        }

        if (scope->parent) {
            scope = scope->parent.get();
        } else {
            return std::nullopt;
        }
    }
}

static std::optional<Luau::Location>
lookupTypeLocation(const Luau::Scope &deepScope, const Luau::Name &name) {
    const Luau::Scope *scope = &deepScope;
    while (true) {
        auto it = scope->typeAliasLocations.find(name);
        if (it != scope->typeAliasLocations.end()) {
            return it->second;
        }

        if (scope->parent) {
            scope = scope->parent.get();
        } else {
            return std::nullopt;
        }
    }
}

static QString printMoonwaveDocumentation(const QStringList &comments) {
    if (comments.empty()) {
        return {};
    }

    QString result;
    QStringList fields;
    QStringList params;
    QStringList returns;
    QStringList throws;

    for (auto &comment : comments) {
        if (comment.startsWith(QLatin1String("@param "))) {
            params.emplace_back(comment);
        } else if (comment.startsWith(QLatin1String("@return "))) {
            returns.emplace_back(comment);
        } else if (comment.startsWith(QLatin1String("@error "))) {
            throws.emplace_back(comment);
        } else if (comment.startsWith(QLatin1String("@field "))) {
            fields.emplace_back(comment);
        } else if (comment == QLatin1String("@private")) {
            result += QStringLiteral("**Private**\n");
        } else if (comment == QLatin1String("@yields")) {
            result += QStringLiteral("**Yields**\n");
        } else if (comment == QLatin1String("@unreleased")) {
            result += QStringLiteral("**Unreleased**\n");
        } else if (comment == QLatin1String("@plugin")) {
            result += QStringLiteral("**Plugin**\n");
        } else if (comment == QLatin1String("@readonly")) {
            result += QStringLiteral("**Read Only**\n");
        } else if (comment.startsWith(QLatin1String("@deprecated "))) {
            result += QStringLiteral("**Deprecated** ");

            auto description = comment.sliced(12);
            auto version = description;

            if (auto space = description.indexOf(' '); space >= 0) {
                version = description.sliced(0, space);
                description = description.sliced(space);
            }

            if (version == description) {
                result += '`' + version + '`' + '\n';
            } else {
                result += '`' + version + '`' + description + '\n';
            }
        } else if (comment.startsWith(QLatin1String("@since "))) {
            result += QStringLiteral("**Since** `") + comment.sliced(7) +
                      QStringLiteral("`\n");
        } else if (comment == QLatin1String("@ignore") ||
                   comment.startsWith(QLatin1String("@tag ")) ||
                   comment.startsWith(QLatin1String("@within ")) ||
                   comment.startsWith(QLatin1String("@class ")) ||
                   comment.startsWith(QLatin1String("@function ")) ||
                   comment.startsWith(QLatin1String("@method ")) ||
                   comment.startsWith(QLatin1String("@prop ")) ||
                   comment.startsWith(QLatin1String("@interface ")) ||
                   comment.startsWith(QLatin1String("@type ")) ||
                   comment.startsWith(QLatin1String("@__index ")) ||
                   comment.startsWith(QLatin1String("@external "))) {
            // Ignore
            continue;
        } else {
            result += comment + '\n';
        }
    }

    if (!fields.isEmpty()) {
        result += QLatin1String("\n\n**Fields**\n");
        for (auto &field : fields) {
            auto fieldText = field.sliced(7);

            // Parse name
            auto fieldName = fieldText;
            if (auto space = fieldText.indexOf(' '); space >= 0) {
                fieldName = fieldText.sliced(0, space);
                fieldText = fieldText.sliced(space);
            }

            if (fieldText == fieldName) {
                result += QStringLiteral("\n- `") + fieldName + '`';
            } else {
                result += QStringLiteral("\n- `") + fieldName + '`' + fieldText;
            }
        }
    }

    if (!params.isEmpty()) {
        result += QLatin1String("\n\n**Parameters**\n");
        for (auto &param : params) {
            auto paramText = param.sliced(7);

            // Parse name
            auto paramName = paramText;
            if (auto space = paramText.indexOf(' '); space >= 0) {
                paramName = paramText.sliced(0, space);
                paramText = paramText.sliced(space);
            }

            if (paramText == paramName) {
                result += "\n- `" + paramName + "`";
            } else {
                result += "\n- `" + paramName + "`" + paramText;
            }
        }
    }

    if (!returns.isEmpty()) {
        result += QLatin1String("\n\n**Returns**\n");
        for (auto &ret : returns) {
            auto returnText = ret.sliced(8);

            // Parse return type
            auto retType = returnText;
            if (auto delim = returnText.indexOf(QLatin1String(" --"));
                delim >= 0) {
                retType = returnText.sliced(0, delim);
                returnText = returnText.sliced(delim);
            }

            if (!retType.isEmpty() && retType != returnText) {
                result += QLatin1String("\n- `") + retType + '`' + returnText;
            } else {
                result += QLatin1String("\n- ") + returnText;
            }
        }
    }

    if (!throws.isEmpty()) {
        result += QLatin1String("\n\n**Throws**\n");
        for (auto &thr : throws) {
            auto throwText = thr.sliced(7);

            // Parse throw type
            auto throwType = throwText;
            if (auto delim = throwText.indexOf(QLatin1String(" --"));
                delim >= 0) {
                throwType = throwText.sliced(0, delim);
                throwText = throwText.sliced(delim);
            }

            if (!throwType.isEmpty() && throwType != throwText) {
                result += QStringLiteral("\n- `") + throwType + '`' + throwText;
            } else {
                result += QStringLiteral("\n- ") + throwText;
            }
        }
    }

    return result;
}

static std::vector<Luau::Comment>
getCommentLocations(const Luau::SourceModule *module,
                    const Luau::Location &node) {
    if (!module) {
        return {};
    }

    AttachCommentsVisitor visitor{node, module->commentLocations};
    visitor.visit(module->root);
    return visitor.attachComments();
}

static inline lsp::DocumentUri getUri(const Luau::ModuleName &moduleName) {
    return QUrl(QString::fromStdString(moduleName));
}

static inline Luau::ModuleName getModuleName(const lsp::DocumentUri &uri) {
    return uri.toString().toStdString();
}

static bool isPathSubdirectory(const QString &parentPath,
                               const QString &childPath) {
    const auto parent = QDir::cleanPath(parentPath);
    const auto child = QDir::cleanPath(childPath);
    if (parent == child) {
        return false;
    }

    const auto relative = QDir(parent).relativeFilePath(child);
    if (relative.isEmpty() || relative == QLatin1String(".")) {
        return false;
    }
    if (QDir::isAbsolutePath(relative)) {
        return false;
    }
    if (relative == QLatin1String("..") ||
        relative.startsWith(QLatin1String("../"))) {
        return false;
    }
    return true;
}

constexpr auto keywords = {
    "and", "break",    "do",     "else", "elseif", "end",   "false",
    "for", "function", "if",     "in",   "local",  "nil",   "not",
    "or",  "repeat",   "return", "then", "true",   "until", "while"};

static bool isKeyword(std::string_view s) {
    return std::find(keywords.begin(), keywords.end(), s) != keywords.end();
}

static bool isIdentifier(std::string_view s) {
    return Luau::isIdentifier(s) && !isKeyword(s);
}

static bool canSuggestType(Luau::TypeId ty) {
    ty = Luau::follow(ty);
    if (Luau::get<Luau::AnyType>(ty) || Luau::get<Luau::ErrorType>(ty) ||
        Luau::get<Luau::GenericType>(ty) || Luau::get<Luau::FreeType>(ty)) {
        return false;
    }
    if (Luau::get<Luau::MetatableType>(ty))
        return false;
    if (const Luau::TableType *ttv = Luau::get<Luau::TableType>(ty)) {
        if (ttv->name) {
            return true;
        }
        if (ttv->syntheticName) {
            return false;
        }
    }
    return true;
}

static bool canSuggestTypePack(Luau::TypePackId tp) {
    tp = Luau::follow(tp);
    if (Luau::get<Luau::ErrorTypePack>(tp) ||
        Luau::get<Luau::GenericTypePack>(tp) ||
        Luau::get<Luau::FreeTypePack>(tp)) {
        return false;
    }
    auto [head, tail] = Luau::flatten(tp);
    for (Luau::TypeId headTy : head) {
        if (!canSuggestType(headTy)) {
            return false;
        }
    }
    return true;
}

/// Construct the initial type description from a typeFun, i.e. Foo<T>
static QString toStringTypeFun(const std::string typeName,
                               const Luau::TypeFun &typeFun) {
    auto output = QString::fromStdString(typeName);
    if (!typeFun.typeParams.empty() || !typeFun.typePackParams.empty()) {
        output += '<';
        bool addComma = false;
        for (const auto &typeParam : typeFun.typeParams) {
            if (addComma) {
                output += QLatin1String(", ");
            }
            output += Luau::toString(Luau::follow(typeParam.ty));
            if (typeParam.defaultValue) {
                output += QLatin1String(" = ");
                output += Luau::toString(
                    Luau::follow(typeParam.defaultValue.value()));
            }
            addComma = true;
        }
        for (const auto &typePack : typeFun.typePackParams) {
            if (addComma) {
                output += QLatin1String(", ");
            }
            output += Luau::toString(Luau::follow(typePack.tp));
            if (typePack.defaultValue) {
                output += QLatin1String(" = ");
                output +=
                    Luau::toString(Luau::follow(typePack.defaultValue.value()));
            }
            addComma = true;
        }
        output += '>';
    }
    return output;
}

template <typename T>
static std::optional<QString> tryGetAnnotation(const Luau::ScopePtr &scope,
                                               T ty) {
    Luau::ToStringOptions opts;
    opts.useLineBreaks = false;
    opts.hideTableKind = true;
    opts.functionTypeArguments = true;
    opts.scope = scope;
    auto result = Luau::toStringDetailed(ty, opts);
    if (result.error || result.invalid || result.cycle || result.truncated) {
        return std::nullopt;
    }
    return QString::fromStdString(result.name);
}

static std::optional<QString> tryGetTypeAnnotation(const Luau::ScopePtr &scope,
                                                   Luau::TypeId ty) {
    if (!canSuggestType(ty)) {
        return std::nullopt;
    }
    return tryGetAnnotation(scope, ty);
}

static std::optional<QString>
tryGetTypePackAnnotation(const Luau::ScopePtr &scope, Luau::TypePackId tp) {
    if (!canSuggestTypePack(tp)) {
        return std::nullopt;
    }
    return tryGetAnnotation(scope, tp);
}

static QString buildGeneratedFunctionSnippet(const Luau::FunctionType *ftv,
                                             const Luau::ScopePtr &scope,
                                             bool addTypeAnnotations,
                                             bool addTabstopForParameters) {
    auto snippet = QStringLiteral("function(");
    auto [args, tail] = Luau::flatten(ftv->argTypes);

    bool first = true;
    size_t snippetIndex = 1;
    for (size_t argIdx = 0; argIdx < args.size(); ++argIdx) {
        if (!first) {
            snippet += QLatin1String(", ");
        } else {
            first = false;
        }
        QString name;
        if (argIdx < ftv->argNames.size() && ftv->argNames[argIdx]) {
            name = QString::fromStdString(ftv->argNames[argIdx]->name);
        } else {
            name = 'a' + QString::number(argIdx);
        }

        if (addTabstopForParameters) {
            snippet += QLatin1String("${") + QString::number(snippetIndex++) +
                       ':' + name + '}';
        } else {
            snippet += name;
        }

        if (addTypeAnnotations) {
            if (auto typeStr = tryGetTypeAnnotation(scope, args[argIdx])) {
                snippet += ": ";
                snippet += *typeStr;
            }
        }
    }

    if (tail && (Luau::isVariadic(*tail) ||
                 Luau::get<Luau::FreeTypePack>(Luau::follow(*tail)))) {
        if (!first) {
            snippet += QLatin1String(", ");
        }

        std::optional<QString> varArgType;
        if (addTypeAnnotations) {
            if (const auto *pack =
                    Luau::get<Luau::VariadicTypePack>(Luau::follow(*tail))) {
                varArgType = tryGetTypeAnnotation(scope, pack->ty);
            }
        }

        snippet += varArgType ? QString(QLatin1String("...: ") + *varArgType)
                              : QStringLiteral("...");
    }

    snippet += ')';

    if (addTypeAnnotations) {
        auto [rets, retTail] = Luau::flatten(ftv->retTypes);
        if (const size_t totalRetSize = rets.size() + (retTail ? 1 : 0);
            totalRetSize > 0) {
            if (auto returnTypes =
                    tryGetTypePackAnnotation(scope, ftv->retTypes)) {
                snippet += QLatin1String(": ");
                bool wrap = totalRetSize != 1;
                if (wrap) {
                    snippet += '(';
                }
                snippet += *returnTypes;
                if (wrap) {
                    snippet += ')';
                }
            }
        }
    }

    snippet += QLatin1String("\n\t$0\nend");
    return snippet;
}

static Luau::Position convertPosition(const lsp::Position &p) {
    return Luau::Position(p.line, p.character);
}

static lsp::Position convertPosition(const Luau::Position &p) {
    return {p.line, p.column};
}

static QPair<QString, QString>
computeLabelDetailsForFunction(const Luau::AutocompleteEntry &entry,
                               const Luau::FunctionType *ftv) {
    auto detail = QStringLiteral("(");
    auto parenthesesSnippet = QStringLiteral("(");

    bool comma = false;
    size_t argIndex = 0;
    size_t snippetIndex = 1;

    auto [minCount, _] =
        Luau::getParameterExtents(Luau::TxnLog::empty(), ftv->argTypes, true);

    // Include 'unknown' arguments as required types
    for (auto arg : ftv->argTypes) {
        if (Luau::get<Luau::UnknownType>(follow(arg))) {
            minCount += 1;
        }
    }
    auto it = Luau::begin(ftv->argTypes);
    for (; it != Luau::end(ftv->argTypes); ++it, ++argIndex) {
        auto argName = QStringLiteral("_");
        if (argIndex < ftv->argNames.size() && ftv->argNames.at(argIndex)) {
            argName = QString::fromStdString(ftv->argNames.at(argIndex)->name);
        }

        if (argIndex == 0 && entry.indexedWithSelf) {
            continue;
        }

        // If the rest of the arguments are optional, don't include in
        // filled call arguments
        bool includeParensSnippet = argIndex < minCount;

        if (comma) {
            detail += QLatin1String(", ");
            if (includeParensSnippet) {
                parenthesesSnippet += QLatin1String(", ");
            }
        }

        detail += argName;
        if (includeParensSnippet) {
            parenthesesSnippet += QLatin1String("${") +
                                  QString::number(snippetIndex) + ':' +
                                  argName + '}';
        }

        comma = true;
        snippetIndex++;
    }

    if (auto tail = it.tail(); tail && !Luau::isEmpty(*tail)) {
        tail = Luau::follow(tail);
        if (auto vtp = Luau::get<Luau::VariadicTypePack>(tail);
            !vtp || !vtp->hidden) {
            if (comma) {
                detail += QLatin1String(", ");
            }
            detail += Luau::toString(*tail);
        }
    }

    // If Luau recommended we put the cursor inside, but we haven't recorded
    // any arguments yet, then we are going to fail to do this. This can
    // happen when all the arguments to function are optional or any (e.g.,
    // wait or require) Let's force a tabstop inside if this happens
    if (entry.parens == Luau::ParenthesesRecommendation::CursorInside &&
        parenthesesSnippet == QLatin1String("(")) {
        parenthesesSnippet += QLatin1String("$1");
    }

    detail += ')';
    parenthesesSnippet += ')';

    return qMakePair(detail, parenthesesSnippet);
}

static bool isInitLuauFile(const QFileInfo &uri) {
    auto name = uri.fileName();
    return name == QLatin1String("init") ||
           name.startsWith(QLatin1String("init."));
}

inline static bool startsWith(std::string_view haystack,
                              std::string_view needle) {
    // ::starts_with is C++20
    return haystack.size() >= needle.size() &&
           haystack.substr(0, needle.size()) == needle;
}

static lsp::Range toLspRange(const Luau::Location &loc) {
    return {{loc.begin.line, loc.begin.column}, {loc.end.line, loc.end.column}};
}

static QString getParentPath(const QString &path) {
    QFileInfo finfo(path);
    if (!finfo.exists()) {
        return {};
    }
    return finfo.absolutePath();
}

static lsp::Diagnostic
createTypeErrorDiagnostic(const Luau::TypeError &error,
                          Luau::FileResolver *fileResolver) {
    QString message;
    if (const auto *syntaxError =
            Luau::get_if<Luau::SyntaxError>(&error.data)) {
        message = QLatin1String("SyntaxError: ");
        message += syntaxError->message;
    } else {
        message = QLatin1String("TypeError: ");
        message +=
            Luau::toString(error, Luau::TypeErrorToStringOptions{fileResolver});
    }

    lsp::Diagnostic diagnostic;
    diagnostic.source = QLatin1String("Luau");
    diagnostic.code = error.code();
    diagnostic.message = message;
    diagnostic.severity = lsp::DiagnosticSeverity::Error;
    diagnostic.range = toLspRange(error.location);
    diagnostic.codeDescription =
        lsp::CodeDescription{QUrl(QStringLiteral("https://luau.org/types"))};
    return diagnostic;
}

static lsp::Diagnostic createLintDiagnostic(const Luau::LintWarning &lint) {
    std::string lintName = Luau::LintWarning::getName(lint.code);
    lsp::Diagnostic diagnostic;
    diagnostic.source = QLatin1String("Luau");
    diagnostic.code = lint.code;
    diagnostic.message = QString::fromStdString(lintName) +
                         QLatin1String(": ") +
                         QString::fromStdString(lint.text);
    diagnostic.severity =
        lsp::DiagnosticSeverity::Warning; // Configuration can convert this
                                          // to an error
    diagnostic.range = toLspRange(lint.location);
    const QString href = QStringLiteral("https://luau.org/lint#") +
                         QString::fromStdString(lintName).toLower() + "-" +
                         QString::number(lint.code);
    diagnostic.codeDescription = lsp::CodeDescription{QUrl(href)};

    if (lint.code == Luau::LintWarning::Code::Code_LocalUnused ||
        lint.code == Luau::LintWarning::Code::Code_ImportUnused ||
        lint.code == Luau::LintWarning::Code::Code_FunctionUnused) {
        diagnostic.tags.emplace_back(lsp::DiagnosticTag::Unnecessary);
    } else if (lint.code == Luau::LintWarning::Code::Code_DeprecatedApi ||
               lint.code == Luau::LintWarning::Code::Code_DeprecatedGlobal) {
        diagnostic.tags.emplace_back(lsp::DiagnosticTag::Deprecated);
    }

    return diagnostic;
}

static lsp::Diagnostic
createParseErrorDiagnostic(const Luau::ParseError &error) {
    lsp::Diagnostic diagnostic;
    diagnostic.source = QLatin1String("Luau");
    diagnostic.code = QLatin1String("SyntaxError");
    diagnostic.message = QLatin1String("SyntaxError: ") +
                         QString::fromStdString(error.getMessage());
    diagnostic.severity = lsp::DiagnosticSeverity::Error;
    diagnostic.range = toLspRange(error.getLocation());
    diagnostic.codeDescription =
        lsp::CodeDescription{QUrl(QStringLiteral("https://luau.org/syntax"))};
    return diagnostic;
}

QString codeBlock(const char *language, const QString &code) {
    auto marker = QLatin1String("```");
    return marker + language + '\n' + code + '\n' + marker;
}

/// Returns a markdown string of the provided documentation
/// If we can't find any documentation for the given symbol, then we return
/// nullopt
static std::optional<QString>
printDocumentation(const Luau::DocumentationDatabase &database,
                   const Luau::DocumentationSymbol &symbol) {
    if (auto documentation = database.find(symbol)) {
        QString result;
        if (auto *basic = documentation->get_if<Luau::BasicDocumentation>()) {
            result = QString::fromStdString(basic->documentation);
            if (!basic->learnMoreLink.empty()) {
                result += QLatin1String("\n\n[Learn More](") +
                          QString::fromStdString(basic->learnMoreLink) + ')';
            }
            if (!basic->codeSample.empty()) {
                result += QLatin1String("\n\n") +
                          codeBlock("luau",
                                    QString::fromStdString(basic->codeSample));
            }
        } else if (auto *func =
                       documentation->get_if<Luau::FunctionDocumentation>()) {
            result = QString::fromStdString(func->documentation);
            if (!func->learnMoreLink.empty()) {
                result += QLatin1String("\n\n[Learn More](") +
                          QString::fromStdString(func->learnMoreLink) + ')';
            }
            if (!func->codeSample.empty()) {
                result +=
                    QLatin1String("\n\n") +
                    codeBlock("luau", QString::fromStdString(func->codeSample));
            }
        } else if (auto *overloaded =
                       documentation
                           ->get_if<Luau::OverloadedFunctionDocumentation>()) {
            if (overloaded->overloads.size() > 0) {
                // Use the first overload
                if (auto firstOverloadDocs = printDocumentation(
                        database, overloaded->overloads.begin()->second)) {
                    result = *firstOverloadDocs;
                }

                auto remainingOverloads = overloaded->overloads.size() - 1;
                result += QLatin1String("\n\n*+") +
                          QString::number(remainingOverloads) +
                          QLatin1String(" overload") +
                          QLatin1String(remainingOverloads == 1 ? "*" : "s*");
            }
        } else if (auto *tbl =
                       documentation->get_if<Luau::TableDocumentation>()) {
            result = QString::fromStdString(tbl->documentation);
            if (!tbl->learnMoreLink.empty()) {
                result += QLatin1String("\n\n[Learn More](") +
                          QString::fromStdString(tbl->learnMoreLink) + ')';
            }
            if (!tbl->codeSample.empty()) {
                result +=
                    QLatin1String("\n\n") +
                    codeBlock("luau", QString::fromStdString(tbl->codeSample));
            }
        }
        return result;
    }

    return std::nullopt;
}

static bool deprecated(const Luau::AutocompleteEntry &entry,
                       std::optional<lsp::MarkupContent> documentation) {
    if (entry.deprecated) {
        return true;
    }

    // Luau also exposes deprecation through function types for some builtins.
    if (entry.type) {
        const auto ty = Luau::follow(*entry.type);
        if (const auto ftv = Luau::get<Luau::FunctionType>(ty);
            ftv && ftv->isDeprecatedFunction) {
            return true;
        } else if (const auto itv = Luau::get<Luau::IntersectionType>(ty)) {
            const auto allDeprecated = std::all_of(
                itv->parts.begin(), itv->parts.end(), [](const auto &part) {
                    const auto partTy = Luau::follow(part);
                    const auto ftv = Luau::get<Luau::FunctionType>(partTy);
                    return ftv && ftv->isDeprecatedFunction;
                });
            if (allDeprecated) {
                return true;
            }
        }
    }

    if (documentation) {
        if (documentation->value.contains(QLatin1String("@deprecated")) ||
            documentation->value.contains(QLatin1String("**Deprecated**"))) {
            return true;
        }
    }

    return false;
}

static std::optional<lsp::CompletionItemKind>
entryKind(const QString &label, const Luau::AutocompleteEntry &entry) {
    if (entry.type.has_value() &&
        entry.kind != Luau::AutocompleteEntryKind::Type) {
        auto id = Luau::follow(entry.type.value());
        if (Luau::isOverloadedFunction(id)) {
            return lsp::CompletionItemKind::Function;
        }

        // Try to infer more type info about the entry to provide better
        // suggestion info
        if (Luau::get<Luau::FunctionType>(id)) {
            return lsp::CompletionItemKind::Function;
        }
    }

    if (std::find(entry.tags.begin(), entry.tags.end(), "Alias") !=
        entry.tags.end()) {
        return lsp::CompletionItemKind::Constant;
    } else if (std::find(entry.tags.begin(), entry.tags.end(), "File") !=
               entry.tags.end()) {
        return lsp::CompletionItemKind::File;
    } else if (std::find(entry.tags.begin(), entry.tags.end(), "Directory") !=
               entry.tags.end()) {
        return lsp::CompletionItemKind::Folder;
    }

    switch (entry.kind) {
    case Luau::AutocompleteEntryKind::Property:
        return lsp::CompletionItemKind::Field;
    case Luau::AutocompleteEntryKind::Binding:
        return lsp::CompletionItemKind::Variable;
    case Luau::AutocompleteEntryKind::Keyword:
        return lsp::CompletionItemKind::Keyword;
    case Luau::AutocompleteEntryKind::String:
        return lsp::CompletionItemKind::Constant;
    case Luau::AutocompleteEntryKind::Type:
        return lsp::CompletionItemKind::Interface;
    case Luau::AutocompleteEntryKind::Module:
        return lsp::CompletionItemKind::Module;
    case Luau::AutocompleteEntryKind::GeneratedFunction:
        return lsp::CompletionItemKind::Function;
    case Luau::AutocompleteEntryKind::RequirePath: {
        if (label == ".." || label == ".")
            return lsp::CompletionItemKind::Folder;
        return lsp::CompletionItemKind::File;
    }
    case Luau::AutocompleteEntryKind::HotComment:
        return lsp::CompletionItemKind::Snippet;
    }

    return std::nullopt;
}

static const char *sortText(const Luau::Frontend &frontend, const QString &name,
                            const lsp::CompletionItem &item,
                            const Luau::AutocompleteEntry &entry,
                            const std::unordered_set<std::string> &tags) {
    if (item.deprecated) {
        return SortText::Deprioritized;
    }
    // If it's a file or directory alias, de-prioritise it compared to
    // normal paths
    if (std::find(entry.tags.begin(), entry.tags.end(), "Alias") !=
        entry.tags.end()) {
        return SortText::AutoImports;
    }

    // If the entry is `loadstring`, deprioritise it
    auto &completionGlobals = frontend.globalsForAutocomplete;
    if (auto it = completionGlobals.globalScope->bindings.find(
            Luau::AstName("loadstring"));
        it != completionGlobals.globalScope->bindings.end()) {
        if (entry.type == it->second.typeId) {
            return SortText::Deprioritized;
        }
    }

    if (entry.wrongIndexType)
        return SortText::WrongIndexType;
    else if (entry.kind == Luau::AutocompleteEntryKind::Property &&
             LuauLsp::isMetamethod(name))
        return SortText::MetatableIndex;
    else if (entry.kind == Luau::AutocompleteEntryKind::Property)
        return SortText::TableProperties;
    else if (entry.kind == Luau::AutocompleteEntryKind::Keyword) {
        // These keywords are contextual and only show up when relevant -
        // they should be prioritised over other suggestions
        if (name == QLatin1String("else") || name == QLatin1String("elseif") ||
            name == QLatin1String("until") || name == QLatin1String("end")) {
            return SortText::PrioritisedSuggestion;
        }
        return SortText::Keywords;
    } else if (entry.typeCorrect == Luau::TypeCorrectKind::Correct)
        return SortText::CorrectTypeKind;
    else if (entry.typeCorrect == Luau::TypeCorrectKind::CorrectFunctionResult)
        return SortText::CorrectFunctionResult;

    return SortText::Default;
}

LuauLanguageServer::DocumentHandle::DocumentHandle(lsp::DocumentUri uri,
                                                   QTextDocument *document)
    : uri(std::move(uri)), document(document) {}

LuauLanguageServer::DocumentHandle::DocumentHandle(
    lsp::DocumentUri uri, std::unique_ptr<QTextDocument> document)
    : uri(std::move(uri)), document(document.get()),
      ownedDocument(std::move(document)) {}

LuauLanguageServer &LuauLanguageServer::instance() {
    static LuauLanguageServer server;
    return server;
}

LuauLanguageServer::LuauLanguageServer()
    : frontend(Luau::Frontend(this, this,
                              {/* retainFullTypeGraphs: */ true,
                               /* forAutocomplete: */ false,
                               /* runLintChecks: */ true})) {
    defaultConfig.mode = Luau::Mode::Nonstrict;
}

void LuauLanguageServer::onOpenDocument(const lsp::DocumentUri &uri,
                                        QTextDocument *document) {
    if (document == nullptr) {
        return;
    }
    const auto moduleName = getModuleName(uri);
    managedFiles.insert_or_assign(moduleName, document);
    frontend.markDirty(moduleName);
    documentDiagnostics(moduleName);
}

void LuauLanguageServer::onSaveDocument(const lsp::DocumentUri &uri) {
    documentDiagnostics(getModuleName(uri));
}

void LuauLanguageServer::onCloseDocument(const lsp::DocumentUri &uri) {
    auto moduleName = getModuleName(uri);
    managedFiles.erase(moduleName);
    frontend.markDirty(moduleName);

    clearDiagnosticsForFile(getUri(moduleName));
}

void LuauLanguageServer::clearConfigCache() { configCache.clear(); }

void LuauLanguageServer::onUpdateDocument(const lsp::DocumentUri &uri,
                                          QTextDocument *document) {
    if (document == nullptr) {
        return;
    }

    const auto moduleName = getModuleName(uri);
    managedFiles.insert_or_assign(moduleName, document);
    frontend.markDirty(moduleName);
    documentDiagnostics(moduleName);
}

QVector<lsp::CompletionItem>
LuauLanguageServer::completion(const lsp::CompletionParams &params,
                               const LSPCancellationToken &cancellationToken) {
    if (params.context &&
        params.context->triggerCharacter == QStringLiteral("\n")) {
        return {};
    }

    auto moduleName = getModuleName(params.textDocument);
    auto textDocument = getTextDocument(moduleName);
    if (!textDocument) {
        return {};
    }

    std::unordered_set<std::string> tags;

    auto stringCompletionCB = [&](const std::string &tag,
                                  std::optional<const Luau::ExternType *> ctx,
                                  std::optional<std::string> contents)
        -> std::optional<Luau::AutocompleteEntryMap> {
        tags.insert(tag);
        return std::nullopt;
    };

    auto position = convertPosition(params.position);

    Luau::FragmentAutocompleteStatusResult fragmentStatusResult;
    Luau::AutocompleteResult result;
    bool forAutocomplete = true;
    bool fragmentWasSuccessful = false;

    if (frontend.allModuleDependenciesValid(moduleName, forAutocomplete) &&
        frontend.isDirty(moduleName, forAutocomplete)) {
        Luau::FrontendOptions frontendOptions;
        frontendOptions.retainFullTypeGraphs = true;
        frontendOptions.forAutocomplete = true;
        frontendOptions.cancellationToken = cancellationToken;

        // Get parse information for this script
        frontend.parse(moduleName);
        const auto sourceModule = frontend.getSourceModule(moduleName);
        if (!sourceModule)
            return {};

        const auto sourceText = textDocument->toPlainText().toUtf8();
        const std::string newSrc(sourceText.constData(),
                                 static_cast<std::size_t>(sourceText.size()));
        Luau::ParseResult fragmentParseResult;
        fragmentParseResult.root = sourceModule->root;
        fragmentParseResult.commentLocations = sourceModule->commentLocations;
        fragmentParseResult.hotcomments = sourceModule->hotcomments;
        Luau::FragmentContext fragmentContext = {
            newSrc,
            fragmentParseResult,
            frontendOptions,
        };

        // It is important to keep the fragmentResult in scope for the whole
        // completion step Otherwise the incremental module may de-allocate
        // leading to a use-after-free when accessing the result ancestry
        fragmentStatusResult =
            Luau::tryFragmentAutocomplete(frontend, moduleName, position,
                                          fragmentContext, stringCompletionCB);
        if (cancellationToken && cancellationToken->requested()) {
            return {};
        }
        if (fragmentStatusResult.status ==
            Luau::FragmentAutocompleteStatus::Success) {
            // Result is nullopt if there are no suggestions (i.e. comments)
            if (!fragmentStatusResult.result)
                return {};

            result = fragmentStatusResult.result->acResults;
            fragmentWasSuccessful = true;
        }
    }

    if (!fragmentWasSuccessful) {
        // We must perform check before autocompletion
        checkStrict(moduleName, cancellationToken, forAutocomplete);

        if (cancellationToken && cancellationToken->requested()) {
            return {};
        }

        result = Luau::autocomplete(frontend, moduleName, position,
                                    stringCompletionCB);
    }

    QVector<lsp::CompletionItem> items;

    for (auto &[name, entry] : result.entryMap) {
        lsp::CompletionItem item;
        auto uname = QString::fromStdString(name);
        item.label = uname;

        // Remove the trailing slash in `../` and `./` as it prevents
        // completion from triggering
        if (entry.kind == Luau::AutocompleteEntryKind::RequirePath) {
            if (name == "../") {
                item.label = QLatin1String("..");
            } else if (name == "./") {
                item.label = QLatin1String(".");
            }
        }

        const auto localModule =
            fragmentWasSuccessful
                ? fragmentStatusResult.result->incrementalModule
                : getModule(moduleName, forAutocomplete);
        if (auto documentationString = getDocumentationForAutocompleteEntry(
                name, entry, result.ancestry, localModule, position))
            item.documentation = {lsp::MarkupKind::Markdown,
                                  documentationString.value()};

        item.deprecated = deprecated(entry, item.documentation);

        item.kind = entryKind(item.label, entry);
        item.sortText = QString::fromLatin1(
            sortText(frontend, item.label, item, entry, tags));

        if (entry.kind == Luau::AutocompleteEntryKind::GeneratedFunction) {
            if (entry.type) {
                if (auto ftv = Luau::get<Luau::FunctionType>(
                        Luau::follow(*entry.type))) {
                    Luau::ScopePtr scope;
                    if (localModule)
                        scope =
                            Luau::findScopeAtPosition(*localModule, position);
                    item.insertText = buildGeneratedFunctionSnippet(
                        ftv, scope,
                        // anonymousAutofilledFunction.addTypeAnnotations
                        true,
                        // anonymousAutofilledFunction.addTabstopForParameters
                        true);
                    item.insertTextFormat = lsp::InsertTextFormat::Snippet;
                } else {
                    const auto &en = entry.insertText;
                    if (en) {
                        item.insertText =
                            QString::fromStdString(entry.insertText.value());
                    }
                }
            } else {
                const auto &en = entry.insertText;
                if (en) {
                    item.insertText =
                        QString::fromStdString(entry.insertText.value());
                }
            }
        }

        if (entry.kind == Luau::AutocompleteEntryKind::RequirePath) {
            if (entry.insertText) {
                LUAU_ASSERT(!result.ancestry.empty());
                auto containingString =
                    result.ancestry.back()->as<Luau::AstExprConstantString>();
                LUAU_ASSERT(containingString);

                auto insertText = QString::fromStdString(*entry.insertText);
                if (!insertText.isEmpty() && insertText.back() == '/') {
                    insertText.removeLast();
                }

                item.textEdit = lsp::TextEdit{
                    {params.position,
                     convertPosition(Luau::Position{
                         containingString->location.end.line,
                         containingString->location.end.column - 1})},
                    insertText};

                // TextEdit cannot replace the old text for some reason, so
                // we need an additional edit
                item.additionalTextEdits.emplace_back(lsp::TextEdit{
                    {convertPosition(Luau::Position{
                         containingString->location.begin.line,
                         containingString->location.begin.column + 1}),
                     convertPosition(Luau::Position{
                         containingString->location.end.line,
                         containingString->location.end.column - 1})},
                    ""});
            }
        }

        // Handle if name is not an identifier
        if (entry.kind == Luau::AutocompleteEntryKind::Property &&
            !isIdentifier(name)) {
            auto lastAst = result.ancestry.back();
            if (auto indexName = lastAst->as<Luau::AstExprIndexName>()) {
                lsp::TextEdit textEdit;
                textEdit.newText =
                    QLatin1String("[\"") + uname + QLatin1String("\"]");
                textEdit.range = {
                    convertPosition(indexName->indexLocation.begin),
                    convertPosition(indexName->indexLocation.end)};
                item.textEdit = textEdit;

                // For some reason, the above text edit can't handle
                // replacing the index operator Hence we remove it using an
                // additional text edit
                item.additionalTextEdits.emplace_back(
                    lsp::TextEdit{{convertPosition(indexName->opPosition),
                                   {indexName->opPosition.line,
                                    indexName->opPosition.column + 1}},
                                  ""});
            }
        }

        // If autocompleting in a string and the autocompleting text
        // contains a '/' character, then it won't replace correctly due to
        // word boundaries Apply a complete text edit instead
        if (name.find('/') != std::string::npos &&
            result.context == Luau::AutocompleteContext::String &&
            entry.kind != Luau::AutocompleteEntryKind::RequirePath) {
            auto lastAst = result.ancestry.back();
            if (auto str = lastAst->as<Luau::AstExprConstantString>()) {
                lsp::TextEdit textEdit;
                textEdit.newText = uname;
                // Range is inside the quotes
                textEdit.range = {
                    convertPosition(
                        Luau::Position{str->location.begin.line,
                                       str->location.begin.column + 1}),
                    convertPosition(Luau::Position{
                        str->location.end.line, str->location.end.column - 1})};
                item.textEdit = textEdit;
            }
        }

        // Handle parentheses suggestions
        if (entry.parens == Luau::ParenthesesRecommendation::CursorAfter) {
            if (item.textEdit) {
                item.textEdit->newText += QLatin1String("()$0");
            } else {
                item.insertText = uname + QLatin1String("()$0");
            }
            item.insertTextFormat = lsp::InsertTextFormat::Snippet;
        } else if (entry.parens ==
                   Luau::ParenthesesRecommendation::CursorInside) {
            auto parenthesesSnippet = QLatin1String("($0)");
            if (item.textEdit) {
                item.textEdit->newText += parenthesesSnippet;
            } else {
                item.insertText = uname + parenthesesSnippet;
            }
            item.insertTextFormat = lsp::InsertTextFormat::Snippet;
        }

        if (entry.type.has_value()) {
            auto id = Luau::follow(entry.type.value());
            item.detail = QString::fromStdString(Luau::toString(id));

            // Try to infer more type info about the entry to provide better
            // suggestion info
            if (auto ftv = Luau::get<Luau::FunctionType>(id);
                ftv &&
                entry.kind != Luau::AutocompleteEntryKind::GeneratedFunction &&
                entry.kind != Luau::AutocompleteEntryKind::Type) {
                // Compute label details and more detailed parentheses
                // snippet
                auto [detail, parenthesesSnippet] =
                    computeLabelDetailsForFunction(entry, ftv);
                item.labelDetails = {detail};

                // If we had CursorAfter, then the function call would not
                // have any arguments
                if (entry.parens != Luau::ParenthesesRecommendation::None) {
                    // if (config.completion.addTabstopAfterParentheses)
                    parenthesesSnippet += QLatin1String("$0");
                    if (item.textEdit) {
                        item.textEdit->newText += parenthesesSnippet;
                    } else {
                        item.insertText = uname + parenthesesSnippet;
                    }
                    item.insertTextFormat = lsp::InsertTextFormat::Snippet;
                }
            }
        }

        items.emplace_back(item);
    }

    return items;
}

// Converts a FTV and function call to a nice string
// In the format "function NAME(args): ret"
struct ToStringNamedFunctionOpts {
    bool hideTableKind = false;
    bool multiline = false;
};

using NameOrExpr = std::variant<std::string, Luau::AstExpr *>;

static QString
toStringNamedFunction(const Luau::ModulePtr &module,
                      const Luau::FunctionType *ftv,
                      const NameOrExpr nameOrFuncExpr,
                      std::optional<Luau::ScopePtr> scope = std::nullopt,
                      const ToStringNamedFunctionOpts &stringOpts = {}) {
    Luau::ToStringOptions opts;
    opts.functionTypeArguments = true;
    opts.hideNamedFunctionTypeParameters = false;
    opts.hideTableKind = stringOpts.hideTableKind;
    opts.useLineBreaks = stringOpts.multiline;
    if (scope) {
        opts.scope = *scope;
    }

    auto functionString =
        QString::fromStdString(Luau::toStringNamedFunction("", *ftv, opts));

    // HACK: remove all instances of "_: " from the function string
    // They don't look great, maybe we should upstream this as an option?
    functionString.replace(QLatin1String("_: "), QLatin1String());

    // If a name has already been provided, just use that
    if (auto name = std::get_if<std::string>(&nameOrFuncExpr)) {
        return QLatin1String("function ") + QString::fromStdString(*name) +
               functionString;
    }
    auto funcExprPtr = std::get_if<Luau::AstExpr *>(&nameOrFuncExpr);

    if (!funcExprPtr) {
        return QLatin1String("function") + functionString;
    }

    auto funcExpr = *funcExprPtr;

    // See if it's just in the form `func(args)`
    if (auto local = funcExpr->as<Luau::AstExprLocal>()) {
        return QLatin1String("function") + local->local->name.value +
               functionString;
    } else if (auto global = funcExpr->as<Luau::AstExprGlobal>()) {
        return QLatin1String("function") + global->name.value + functionString;
    } else if (funcExpr->as<Luau::AstExprGroup>() ||
               funcExpr->as<Luau::AstExprFunction>()) {
        // In the form (expr)(args), which implies that it's probably a IIFE
        return QLatin1String("function") + functionString;
    }

    // See if the name belongs to a ClassType
    Luau::TypeId *parentIt = nullptr;
    QString methodName;
    QString baseName;

    if (auto indexName = funcExpr->as<Luau::AstExprIndexName>()) {
        parentIt = module->astTypes.find(indexName->expr);
        methodName = QString(1, indexName->op) + indexName->index.value;
        // If we are calling this as a method ':', we should implicitly hide
        // self, and recompute the functionString
        opts.hideFunctionSelfArgument = indexName->op == ':';
        functionString =
            QString::fromStdString(Luau::toStringNamedFunction("", *ftv, opts));
        functionString.replace(QLatin1String("_: "), QLatin1String());
        // We can try and give a temporary base name from what we can infer
        // by the index, and then attempt to improve it with proper
        // information
        baseName =
            QString::fromStdString(Luau::toString(indexName->expr)).trimmed();
    } else if (auto indexExpr = funcExpr->as<Luau::AstExprIndexExpr>()) {
        parentIt = module->astTypes.find(indexExpr->expr);
        methodName = '[' +
                     QString::fromStdString(Luau::toString(indexExpr->index)) +
                     ']';
        // We can try and give a temporary base name from what we can infer
        // by the index, and then attempt to improve it with proper
        // information
        baseName =
            QString::fromStdString(Luau::toString(indexExpr->expr)).trimmed();
    }

    if (!parentIt) {
        return QLatin1String("function") + methodName + functionString;
    }
    if (auto name = getTypeName(*parentIt)) {
        baseName = *name;
    }
    return QLatin1String("function") + baseName + methodName + functionString;
}

static std::optional<Luau::TypeId> findCallMetamethod(Luau::TypeId type) {
    type = Luau::follow(type);

    std::optional<Luau::TypeId> metatable;
    if (const auto mtType = Luau::get<Luau::MetatableType>(type))
        metatable = mtType->metatable;
    else if (const auto classType = Luau::get<Luau::ExternType>(type))
        metatable = classType->metatable;

    if (!metatable)
        return std::nullopt;

    auto unwrapped = Luau::follow(*metatable);
    if (auto prop = LuauLsp::lookupProp(unwrapped, "__call");
        prop.size() == 1 && prop[0].property.readTy) {
        return prop[0].property.readTy;
    }

    return std::nullopt;
}

std::optional<lsp::SignatureHelp> LuauLanguageServer::signatureHelp(
    const lsp::SignatureHelpParams &params,
    const LSPCancellationToken &cancellationToken) {
    auto moduleName = getModuleName(params.textDocument);
    auto textDocument = getTextDocument(moduleName);
    if (!textDocument) {
        return std::nullopt;
    }
    auto position = convertPosition(params.position);

    // Run the type checker to ensure we are up to date
    // Use the autocomplete checker until strict checking supports expressive
    // types consistently.
    checkStrict(moduleName, cancellationToken);

    if (cancellationToken && cancellationToken->requested()) {
        return std::nullopt;
    }

    auto sourceModule = frontend.getSourceModule(moduleName);
    if (!sourceModule)
        return std::nullopt;

    auto module = getModule(moduleName, /* forAutocomplete: */ true);
    auto ancestry = Luau::findAstAncestryOfPosition(*sourceModule, position);
    auto scope = Luau::findScopeAtPosition(*module, position);

    if (ancestry.size() == 0 || !scope)
        return std::nullopt;

    auto *candidate = ancestry.back()->as<Luau::AstExprCall>();
    if (!candidate && ancestry.size() >= 2)
        candidate = ancestry.at(ancestry.size() - 2)->as<Luau::AstExprCall>();

    if (!candidate)
        return std::nullopt;

    // FIXME: should not be necessary if the `ty` has the doc symbol
    // attached to it
    auto documentationSymbol = Luau::getDocumentationSymbolAtPosition(
        *sourceModule, *module,
        {candidate->func->location.end.line,
         candidate->func->location.end.column - 1});
    size_t activeParameter = 0;

    // Use the position to determine which parameter is active
    for (auto param : candidate->args) {
        if (param->location.containsClosed(position) ||
            param->location.begin > position)
            break;
        activeParameter++;
    }
    auto it = module->astTypes.find(candidate->func);
    if (!it)
        return std::nullopt;
    auto followedId = Luau::follow(*it);

    // Construct a type pack from the current list of arguments for overload
    // matching
    Luau::TypeArena typeArena;
    std::vector<Luau::TypeId> argumentTys;
    if (candidate->self)
        argumentTys.push_back(followedId);
    for (auto &&arg : candidate->args)
        if (auto ty = module->astTypes.find(arg))
            argumentTys.push_back(Luau::follow(*ty));
    Luau::TypePackId subTp =
        typeArena.addTypePack(argumentTys, frontend.builtinTypes->anyTypePack);

    ToStringNamedFunctionOpts opts;
    opts.hideTableKind = false;

    std::optional<size_t> activeSignature = std::nullopt;
    std::vector<lsp::SignatureInformation> signatures{};

    auto addSignature = [&](const Luau::TypeId &ty,
                            const Luau::FunctionType *ftv,
                            bool isOverloaded = false) {
        // Create the whole label
        auto label =
            toStringNamedFunction(module, ftv, candidate->func, scope, opts);
        lsp::MarkupContent documentation{lsp::MarkupKind::Markdown, ""};

        auto baseDocumentationSymbol = documentationSymbol;
        if (baseDocumentationSymbol && isOverloaded) {
            // We need to trim "/overload/" from the base symbol if its been
            // resolved to something
            // FIXME: can be removed once we use docSymbol from `ty`
            if (auto idx = baseDocumentationSymbol->find("/overload/");
                idx != std::string::npos)
                baseDocumentationSymbol =
                    baseDocumentationSymbol->substr(0, idx);
            baseDocumentationSymbol =
                *baseDocumentationSymbol + "/overload/" + toString(ty);
        }

        if (std::optional<QString> docs;
            baseDocumentationSymbol &&
            (docs = printDocumentation(this->documentation,
                                       *baseDocumentationSymbol)) &&
            docs)
            documentation.value = *docs;
        else if (ftv->definition && ftv->definition->definitionModuleName)
            documentation.value = printMoonwaveDocumentation(
                getComments(ftv->definition->definitionModuleName.value(),
                            ftv->definition->definitionLocation));

        // Create each parameter label
        std::vector<lsp::ParameterInformation> parameters{};
        auto it = Luau::begin(ftv->argTypes);
        auto idx = 0;
        auto previousParamPos = label.indexOf(
            '('); // start search at start of parameter list, not earlier

        // Use the same ToStringOptions as toStringNamedFunction so that
        // type names (including module-qualified ones like "second.foo")
        // are resolved identically in both the full label and each
        // parameter search string.
        Luau::ToStringOptions typeStringOpts;
        typeStringOpts.functionTypeArguments = true;
        typeStringOpts.hideNamedFunctionTypeParameters = false;
        typeStringOpts.hideTableKind = opts.hideTableKind;
        typeStringOpts.scope = scope;

        for (; it != Luau::end(ftv->argTypes); it++, idx++) {
            // If the function has self, and the caller has called as a
            // method (i.e., :), then omit the self parameter
            if (idx == 0 && candidate->self)
                continue;

            // Show parameter documentation
            lsp::MarkupContent parameterDocumentation{lsp::MarkupKind::Markdown,
                                                      ""};
            if (baseDocumentationSymbol)
                if (auto docs =
                        printDocumentation(this->documentation,
                                           *baseDocumentationSymbol +
                                               "/param/" + std::to_string(idx)))
                    parameterDocumentation.value = *docs;

            // Compute the label
            // We attempt to search for the position in the string for this
            // label, and if we don't find it, then we give up and just use
            // the string label
            std::variant<QString, std::vector<qsizetype>> paramLabel;
            QString labelString;
            if (idx < ftv->argNames.size() && ftv->argNames[idx] &&
                ftv->argNames[idx]->name != "_") {
                labelString = QString::fromStdString(ftv->argNames[idx]->name) +
                              QLatin1String(": ");
            }
            labelString += Luau::toString(*it, typeStringOpts);

            auto position = label.indexOf(labelString, previousParamPos);
            if (position != std::string::npos) {
                auto length = labelString.size();
                previousParamPos = position + length;
                paramLabel = std::vector{position, position + length};
            } else
                paramLabel = labelString;

            parameters.push_back(
                lsp::ParameterInformation{paramLabel, parameterDocumentation});
        }

        // Handle varargs
        if (auto tp = it.tail()) {
            if (auto vtp = Luau::get<Luau::VariadicTypePack>(*tp);
                !vtp || !vtp->hidden) {
                // Show parameter documentation
                lsp::MarkupContent parameterDocumentation{
                    lsp::MarkupKind::Markdown, ""};
                if (baseDocumentationSymbol)
                    if (auto docs = printDocumentation(
                            this->documentation, *baseDocumentationSymbol +
                                                     "/param/" +
                                                     std::to_string(idx)))
                        parameterDocumentation.value = *docs;

                // Compute the label
                // We attempt to search for the position in the string for
                // this label, and if we don't find it, then we give up and
                // just use the string label
                std::variant<QString, std::vector<qsizetype>> paramLabel;
                auto labelString = QStringLiteral("...: ");

                if (vtp) {
                    labelString += Luau::toString(vtp->ty, typeStringOpts);
                } else {
                    labelString += Luau::toString(*tp, typeStringOpts);
                }

                auto position = label.indexOf(labelString, previousParamPos);
                if (position >= 0) {
                    auto length = labelString.size();
                    previousParamPos = position + length;
                    paramLabel = std::vector{position, position + length};
                } else {
                    paramLabel = labelString;
                }

                parameters.push_back(lsp::ParameterInformation{
                    paramLabel, parameterDocumentation});
            }
        }

        // If this overload matches, and we haven't yet found a match, mark
        // it as the active signature
        if (!activeSignature &&
            checkOverloadMatch(subTp, ftv->argTypes, Luau::NotNull{&*scope},
                               &typeArena, frontend.builtinTypes))
            activeSignature = signatures.size();

        signatures.push_back(lsp::SignatureInformation{
            label, documentation, parameters,
            std::min(activeParameter,
                     parameters.size() == 0 ? 0 : parameters.size() - 1)});
    };

    // Handle single function
    if (auto ftv = Luau::get<Luau::FunctionType>(followedId))
        addSignature(followedId, ftv);

    // Handle overloaded function
    if (auto intersect = Luau::get<Luau::IntersectionType>(followedId))
        for (Luau::TypeId part : intersect->parts)
            if (auto candidateFunctionType =
                    Luau::get<Luau::FunctionType>(part))
                addSignature(part, candidateFunctionType,
                             /* isOverloaded = */ true);

    // Handle __call metamethod
    if (const auto metamethod = findCallMetamethod(followedId))
        if (auto ftv = Luau::get<Luau::FunctionType>(Luau::follow(*metamethod)))
            addSignature(*metamethod, ftv);

    lsp::SignatureHelp help = lsp::SignatureHelp{
        signatures, activeSignature.value_or(0), activeParameter};

    return help;
}

struct DocumentationLocation {
    Luau::ModuleName moduleName;
    Luau::Location location;
};

static constexpr auto kDocumentationBreaker = "\n----------\n";

std::optional<lsp::Hover>
LuauLanguageServer::hover(const lsp::HoverParams &params,
                          const LSPCancellationToken &cancellationToken) {
    constexpr auto showTableKinds = true;
    constexpr auto multilineFunctionDefinitions = false;

    auto moduleName = getModuleName(params.textDocument);
    auto textDocument = getTextDocument(moduleName);
    if (!textDocument) {
        return std::nullopt;
    }

    auto position = convertPosition(params.position);

    // Run the type checker to ensure we are up to date
    // Match the autocomplete checker used to render type expressions.
    checkStrict(moduleName, cancellationToken,
                /* forAutocomplete: */ !showTableKinds);

    if (cancellationToken && cancellationToken->requested()) {
        return std::nullopt;
    }

    auto sourceModule = frontend.getSourceModule(moduleName);
    auto module = getModule(moduleName,
                            /* forAutocomplete: */ !showTableKinds);
    if (!sourceModule)
        return std::nullopt;

    if (Luau::isWithinComment(*sourceModule, position))
        return std::nullopt;

    auto exprOrLocal = Luau::findExprOrLocalAtPosition(*sourceModule, position);
    auto node = findNodeOrTypeAtPosition(*sourceModule, position);
    auto scope = Luau::findScopeAtPosition(*module, position);
    if (!node || !scope)
        return std::nullopt;

    std::optional<std::pair<std::string, Luau::TypeFun>> typeAliasInformation =
        std::nullopt;
    std::optional<Luau::TypeId> type = std::nullopt;
    std::optional<std::string> documentationSymbol =
        getDocumentationSymbolAtPosition(*sourceModule, *module, position);
    std::optional<DocumentationLocation> documentationLocation = std::nullopt;

    if (auto ref = node->as<Luau::AstTypeReference>()) {
        std::string typeName;
        std::optional<Luau::TypeFun> typeFun;
        if (ref->prefix) {
            typeName = std::string(ref->prefix->value) + "." + ref->name.value;
            typeFun =
                scope->lookupImportedType(ref->prefix->value, ref->name.value);
        } else {
            typeName = ref->name.value;
            typeFun = scope->lookupType(ref->name.value);
        }
        if (!typeFun)
            return std::nullopt;
        typeAliasInformation = std::make_pair(typeName, *typeFun);
        type = typeFun->type;
    } else if (auto alias = node->as<Luau::AstStatTypeAlias>()) {
        auto typeName = alias->name.value;
        auto typeFun = scope->lookupType(typeName);
        if (!typeFun)
            return std::nullopt;
        typeAliasInformation = std::make_pair(typeName, *typeFun);
        type = typeFun->type;
    } else if (auto typeTable = node->as<Luau::AstTypeTable>()) {
        if (auto tableTy = module->astResolvedTypes.find(typeTable)) {
            type = *tableTy;

            // Check if we are inside one of the properties
            for (auto &prop : typeTable->props) {
                if (prop.location.containsClosed(position)) {
                    auto parentType = Luau::follow(*tableTy);
                    if (auto definitionModuleName =
                            Luau::getDefinitionModuleName(parentType))
                        documentationLocation = {definitionModuleName.value(),
                                                 prop.location};
                    auto resolvedProperty =
                        LuauLsp::lookupProp(parentType, prop.name.value);
                    if (resolvedProperty.size() == 1 &&
                        resolvedProperty[0].property.readTy)
                        type = resolvedProperty[0].property.readTy;
                    break;
                }
            }
        }
    } else if (auto astType = node->asType()) {
        if (auto ty = module->astResolvedTypes.find(astType)) {
            type = *ty;
        }
    } else if (auto local = exprOrLocal.getLocal()) {
        type = scope->lookup(local);
        documentationLocation = {moduleName, local->location};
    } else if (auto expr = exprOrLocal.getExpr()) {
        // Special case, we want to check if there is a parent in the
        // ancestry, and if it is an AstTable If so, and we are hovering
        // over a prop, we want to give type info for the assigned
        // expression to the prop rather than just "string"
        auto ancestry =
            Luau::findAstAncestryOfPosition(*sourceModule, position);
        if (ancestry.size() >= 2 &&
            ancestry.at(ancestry.size() - 2)->is<Luau::AstExprTable>()) {
            auto parent =
                ancestry.at(ancestry.size() - 2)->as<Luau::AstExprTable>();
            for (const auto &[kind, key, value] : parent->items) {
                if (key && key->location.contains(position)) {
                    // Return type type of the value
                    if (auto it = module->astTypes.find(value)) {
                        type = *it;
                    }
                    break;
                }
            }
        }

        // Handle table properties (so that we can get documentation info)
        if (auto index = expr->as<Luau::AstExprIndexName>()) {
            if (auto parentIt = module->astTypes.find(index->expr)) {
                auto parentType = Luau::follow(*parentIt);
                auto indexName = index->index.value;
                if (auto propInformation =
                        LuauLsp::lookupProp(parentType, indexName);
                    !propInformation.empty()) {
                    auto [baseTy, prop] = propInformation[0];
                    if (propInformation.size() == 1 && prop.readTy)
                        type = prop.readTy;
                    if (auto definitionModuleName =
                            Luau::getDefinitionModuleName(baseTy)) {
                        if (prop.location)
                            documentationLocation = {
                                definitionModuleName.value(),
                                prop.location.value()};
                        else if (prop.typeLocation)
                            documentationLocation = {
                                definitionModuleName.value(),
                                prop.typeLocation.value()};
                    }
                }
            }
        }

        // Handle local variables separately to retrieve documentation
        // location info
        if (auto local = expr->as<Luau::AstExprLocal>();
            !documentationLocation.has_value() && local && local->local) {
            documentationLocation = {moduleName, local->local->location};
        }

        if (!type) {
            if (auto it = module->astTypes.find(expr)) {
                type = *it;
            } else if (auto global = expr->as<Luau::AstExprGlobal>()) {
                type = scope->lookup(global->name);
            } else if (auto local = expr->as<Luau::AstExprLocal>()) {
                type = scope->lookup(local->local);
            }
        }
    }

    if (!type)
        return std::nullopt;
    type = Luau::follow(*type);

    if (!documentationSymbol)
        documentationSymbol = type.value()->documentationSymbol;

    Luau::ToStringOptions opts;
    opts.exhaustive = true;
    opts.useLineBreaks = true;
    opts.functionTypeArguments = true;
    opts.hideNamedFunctionTypeParameters = false;
    opts.hideTableKind = false;
    opts.scope = scope;
    auto typeString = QString::fromStdString(Luau::toString(*type, opts));

    // If we have a function and its corresponding name
    if (typeAliasInformation) {
        auto [typeName, typeFun] = typeAliasInformation.value();
        typeString =
            codeBlock("luau", "type " + toStringTypeFun(typeName, typeFun) +
                                  " = " + typeString);
    } else if (auto ftv = Luau::get<Luau::FunctionType>(*type)) {
        NameOrExpr name = "";
        if (auto localName = exprOrLocal.getName())
            name = localName->value;
        else if (auto expr = exprOrLocal.getExpr())
            name = expr;

        ToStringNamedFunctionOpts funcOpts;
        funcOpts.hideTableKind = !showTableKinds;
        funcOpts.multiline = multilineFunctionDefinitions;
        typeString = codeBlock(
            "luau", toStringNamedFunction(module, ftv, name, scope, funcOpts));
    } else if (exprOrLocal.getLocal() || node->as<Luau::AstExprLocal>()) {
        auto builder = QStringLiteral("local ");
        if (auto name = exprOrLocal.getName())
            builder += name->value;
        else
            builder += Luau::getIdentifier(node->asExpr()).value;
        builder += ": " + typeString;
        typeString = codeBlock("luau", builder);
    } else if (auto global = node->as<Luau::AstExprGlobal>()) {
        auto builder = QStringLiteral("type ");
        builder += global->name.value;
        builder += " = " + typeString;
        typeString = codeBlock("luau", builder);
    } else if (node->as<Luau::AstExprConstantString>()) {
        typeString = codeBlock("luau", "string");
    } else {
        typeString = codeBlock("luau", typeString);
    }

    if (std::optional<QString> docs;
        documentationSymbol &&
        (docs =
             printDocumentation(this->documentation, *documentationSymbol)) &&
        docs && !docs->isEmpty()) {
        typeString += kDocumentationBreaker;
        typeString += *docs;
    } else if (auto documentation = getDocumentationForType(*type);
               documentation && !documentation->isEmpty()) {
        typeString += kDocumentationBreaker;
        typeString += *documentation;
    } else if (auto documentation =
                   getDocumentationForAstNode(moduleName, node, scope);
               documentation && !documentation->isEmpty()) {
        typeString += kDocumentationBreaker;
        typeString += *documentation;
    } else if (documentationLocation) {
        if (auto text = printMoonwaveDocumentation(
                getComments(documentationLocation->moduleName,
                            documentationLocation->location));
            !text.isEmpty()) {
            typeString += kDocumentationBreaker;
            typeString += text;
        }
    }

    return lsp::Hover{{lsp::MarkupKind::Markdown, typeString}};
}

static void
fillBuiltinGlobals(std::unordered_map<Luau::AstName, Luau::TypeId> &builtins,
                   const Luau::AstNameTable &names, const Luau::ScopePtr &env) {
    Luau::ScopePtr current = env;
    while (true) {
        for (auto &[global, binding] : current->bindings) {
            Luau::AstName name = names.get(global.c_str());
            if (name.value) {
                builtins.insert_or_assign(name, binding.typeId);
            }
        }

        if (current->parent) {
            current = current->parent;
        } else {
            break;
        }
    }
}

static QVector<SemanticToken>
getSemanticTokens(const Luau::Frontend &frontend, const Luau::ModulePtr &module,
                  const Luau::SourceModule *sourceModule) {
    std::unordered_map<Luau::AstName, Luau::TypeId> builtinGlobals{};
    fillBuiltinGlobals(builtinGlobals, *sourceModule->names,
                       frontend.globals.globalScope);
    SemanticTokensVisitor visitor{module, builtinGlobals};
    visitor.visit(sourceModule->root);
    return visitor.tokens;
}

QVector<SemanticToken> LuauLanguageServer::semanticTokens(
    const lsp::SemanticTokensParams &params,
    const LSPCancellationToken &cancellationToken) {
    auto moduleName = getModuleName(params.textDocument);
    auto textDocument = getTextDocument(moduleName);
    if (!textDocument) {
        qCritical("No managed text document for \"%s\"", moduleName.c_str());
        return {};
    }

    // Run the type checker to ensure we are up to date
    // The autocomplete checker currently provides the semantic type data.
    checkStrict(moduleName, cancellationToken);

    if (cancellationToken && cancellationToken->requested()) {
        return {};
    }

    auto sourceModule = frontend.getSourceModule(moduleName);
    auto module = getModule(moduleName, /* forAutocomplete: */ true);
    if (!sourceModule || !module) {
        return {};
    }

    return getSemanticTokens(frontend, module, sourceModule);
}

struct LocationInformation {
    std::optional<std::string> definitionModuleName;
    std::optional<Luau::Location> location;
    Luau::TypeId ty;
};

static std::optional<Luau::Location> getLocation(Luau::TypeId type) {
    type = follow(type);

    if (auto ftv = Luau::get<Luau::FunctionType>(type)) {
        if (ftv->definition)
            return ftv->definition->originalNameLocation;
    } else if (auto ttv = Luau::get<Luau::TableType>(type)) {
        return ttv->definitionLocation;
    } else if (auto mtv = Luau::get<Luau::MetatableType>(type)) {
        return getLocation(mtv->table);
    } else if (auto ctv = Luau::get<Luau::ExternType>(type)) {
        return ctv->definitionLocation;
    }

    return std::nullopt;
}

static std::optional<LocationInformation>
findLocationForSymbol(const Luau::ModulePtr &module,
                      const Luau::Position &position,
                      const Luau::Symbol &symbol) {
    auto scope = Luau::findScopeAtPosition(*module, position);
    auto ty = scope->lookup(symbol);
    if (!ty)
        return std::nullopt;
    ty = Luau::follow(*ty);
    return LocationInformation{Luau::getDefinitionModuleName(*ty),
                               getLocation(*ty), *ty};
}

static std::vector<LocationInformation>
findLocationsForIndex(const Luau::ModulePtr &module, const Luau::AstExpr *base,
                      const Luau::Name &name) {
    auto baseTy = module->astTypes.find(base);
    if (!baseTy)
        return {};
    auto baseTyFollowed = Luau::follow(*baseTy);

    std::vector<LocationInformation> results;
    for (const auto &[realBaseTy, prop] :
         LuauLsp::lookupProp(baseTyFollowed, name)) {
        auto location = prop.location ? prop.location : prop.typeLocation;
        if (!prop.readTy)
            continue;

        // Deduplicate by location to avoid returning multiple identical results
        // (e.g., when a union has multiple instantiations of the same generic
        // type)
        bool isDuplicate =
            std::any_of(results.begin(), results.end(),
                        [&](const LocationInformation &existing) {
                            return existing.location == location;
                        });
        if (!isDuplicate)
            results.push_back(
                LocationInformation{Luau::getDefinitionModuleName(realBaseTy),
                                    location, *prop.readTy});
    }

    return results;
}

static std::vector<LocationInformation>
findLocationsForExpr(const Luau::ModulePtr &module, const Luau::AstExpr *expr,
                     const Luau::Position &position) {
    if (auto local = expr->as<Luau::AstExprLocal>()) {
        if (auto loc = findLocationForSymbol(module, position, local->local))
            return {*loc};
    } else if (auto global = expr->as<Luau::AstExprGlobal>()) {
        if (auto loc = findLocationForSymbol(module, position, global->name))
            return {*loc};
    } else if (auto indexname = expr->as<Luau::AstExprIndexName>())
        return findLocationsForIndex(module, indexname->expr,
                                     indexname->index.value);
    else if (auto indexexpr = expr->as<Luau::AstExprIndexExpr>()) {
        if (auto string = indexexpr->index->as<Luau::AstExprConstantString>())
            return findLocationsForIndex(
                module, indexexpr->expr,
                std::string(string->value.data, string->value.size));
    }

    return {};
}

// Duplicated from Luau/TypeInfer.h, since its static
static std::optional<Luau::AstExpr *>
matchRequire(const Luau::AstExprCall &call) {
    const char *require = "require";

    if (call.args.size != 1)
        return std::nullopt;

    const Luau::AstExprGlobal *funcAsGlobal =
        call.func->as<Luau::AstExprGlobal>();
    if (!funcAsGlobal || funcAsGlobal->name != require)
        return std::nullopt;

    if (call.args.size != 1)
        return std::nullopt;

    return call.args.data[0];
}

lsp::DefinitionResult LuauLanguageServer::gotoDefinition(
    const lsp::DefinitionParams &params,
    const LSPCancellationToken &cancellationToken) {
    lsp::DefinitionResult result{};

    auto moduleName = getModuleName(params.textDocument);
    auto textDocument = getTextDocument(moduleName);
    if (!textDocument) {
        qCritical("No managed text document for \"%s\"", moduleName.c_str());
        return {};
    }
    auto position = convertPosition(params.position);

    // Run the type checker to ensure we are up to date
    checkStrict(moduleName, cancellationToken);
    if (cancellationToken && cancellationToken->requested()) {
        return {};
    }

    auto sourceModule = frontend.getSourceModule(moduleName);
    auto module = getModule(moduleName, /* forAutocomplete: */ true);
    if (!sourceModule || !module)
        return result;

    auto binding =
        Luau::findBindingAtPosition(*module, *sourceModule, position);
    if (binding) {
        // If it points to a global definition (i.e. at pos 0,0), return
        // nothing
        if (binding->location.begin == Luau::Position{0, 0} &&
            binding->location.end == Luau::Position{0, 0})
            return result;

        // Follow through the binding reference if it is a function type
        // This is particularly useful for `local X = require(...)` where
        // `X` is a function - we want the actual function definition
        auto ftv = Luau::get<Luau::FunctionType>(Luau::follow(binding->typeId));
        if (ftv && ftv->definition && ftv->definition->definitionModuleName) {
            if (auto document = getOrCreateTextDocumentFromModuleName(
                    ftv->definition->definitionModuleName.value())) {
                result.emplace_back(lsp::Location{
                    document.uri,
                    lsp::Range{
                        convertPosition(
                            ftv->definition->originalNameLocation.begin),
                        convertPosition(
                            ftv->definition->originalNameLocation.end)}});
                return result;
            }
        }

        result.emplace_back(
            lsp::Location{params.textDocument,
                          lsp::Range{convertPosition(binding->location.begin),
                                     convertPosition(binding->location.end)}});
        return result;
    }

    auto node = findNodeOrTypeAtPosition(*sourceModule, position);
    if (!node)
        return result;

    if (auto expr = node->asExpr()) {
        for (const auto &[definitionModuleName, location, _] :
             findLocationsForExpr(module, expr, position)) {
            if (location) {
                if (definitionModuleName) {
                    // if (auto uri = resolveToRealPath(*definitionModuleName))
                    // {
                    auto uri = getUri(*definitionModuleName);
                    if (auto document = getOrCreateTextDocumentFromModuleName(
                            *definitionModuleName)) {
                        result.emplace_back(lsp::Location{
                            uri, lsp::Range{convertPosition(location->begin),
                                            convertPosition(location->end)}});
                    }
                    // }
                } else {
                    result.emplace_back(lsp::Location{
                        params.textDocument,
                        lsp::Range{convertPosition(location->begin),
                                   convertPosition(location->end)}});
                }
            }
        }
    } else if (auto reference = node->as<Luau::AstTypeReference>()) {
        DocumentHandle referenceTextDocument(params.textDocument, textDocument);
        std::optional<Luau::Location> location = std::nullopt;

        auto scope = Luau::findScopeAtPosition(*module, position);
        if (!scope)
            return result;

        if (reference->prefix) {
            if (auto importedName = lookupImportedModule(
                    *scope, reference->prefix.value().value)) {
                auto importedModule =
                    getModule(*importedName, /* forAutocomplete: */ true);
                if (!importedModule)
                    return result;

                const auto it = importedModule->exportedTypeBindings.find(
                    reference->name.value);
                if (it == importedModule->exportedTypeBindings.end() ||
                    !it->second.definitionLocation)
                    return result;

                referenceTextDocument =
                    getOrCreateTextDocumentFromModuleName(*importedName);
                location = *it->second.definitionLocation;
            } else
                return result;
        } else {
            location = lookupTypeLocation(*scope, reference->name.value);
        }

        if (!referenceTextDocument || !location)
            return result;

        result.emplace_back(
            lsp::Location{referenceTextDocument.uri,
                          lsp::Range{convertPosition(location->begin),
                                     convertPosition(location->end)}});
    }

    // Fallback: if no results found so far, we can try checking if this is
    // within a require statement
    if (result.empty()) {
        auto ancestry =
            Luau::findAstAncestryOfPosition(*sourceModule, position);
        if (ancestry.size() >= 2) {
            if (auto call =
                    ancestry[ancestry.size() - 2]->as<Luau::AstExprCall>();
                call && matchRequire(*call)) {
                if (auto moduleInfo = frontend.moduleResolver.resolveModuleInfo(
                        moduleName, *call)) {
                    result.emplace_back(lsp::Location{
                        getUri(moduleInfo->name), lsp::Range{{0, 0}, {0, 0}}});
                }
            }
        }
    }

    // Remove duplicate elements within the result
    // There are at most two candidate locations, so linear duplicate removal
    // is sufficient here.
    auto end = result.end();
    for (auto it = result.begin(); it != end; ++it)
        end = std::remove(it + 1, end, *it);

    result.erase(end, result.end());

    return result;
}

std::optional<lsp::Location> LuauLanguageServer::gotoTypeDefinition(
    const lsp::TypeDefinitionParams &params,
    const LSPCancellationToken &cancellationToken) {
    // If its a binding, we should find its assigned type if possible, and
    // then find the definition of that type If its a type, then just find
    // the definintion of that type (i.e. the type alias)

    auto moduleName = getModuleName(params.textDocument);
    auto textDocument = getTextDocument(moduleName);
    if (!textDocument) {
        qCritical("No managed text document for \"%s\"", moduleName.c_str());
        return std::nullopt;
    }
    auto position = convertPosition(params.position);

    // Run the type checker to ensure we are up to date
    checkStrict(moduleName, cancellationToken);
    if (cancellationToken && cancellationToken->requested()) {
        return {};
    }

    auto sourceModule = frontend.getSourceModule(moduleName);
    auto module = getModule(moduleName, /* forAutocomplete: */ true);
    if (!sourceModule || !module)
        return std::nullopt;

    auto node = findNodeOrTypeAtPosition(*sourceModule, position);
    if (!node)
        return std::nullopt;

    const auto uri = params.textDocument;
    auto findTypeLocation =
        [this, textDocument, uri, &module,
         &position](Luau::AstType *type) -> std::optional<lsp::Location> {
        if (auto reference = type->as<Luau::AstTypeReference>()) {
            DocumentHandle referenceTextDocument(uri, textDocument);
            std::optional<Luau::Location> location = std::nullopt;

            auto scope = Luau::findScopeAtPosition(*module, position);
            if (!scope)
                return std::nullopt;

            if (reference->prefix) {
                if (auto importedName = lookupImportedModule(
                        *scope, reference->prefix.value().value)) {
                    auto importedModule =
                        getModule(*importedName, /* forAutocomplete: */ true);
                    if (!importedModule)
                        return std::nullopt;

                    const auto it = importedModule->exportedTypeBindings.find(
                        reference->name.value);
                    if (it == importedModule->exportedTypeBindings.end() ||
                        !it->second.definitionLocation)
                        return std::nullopt;

                    referenceTextDocument =
                        getOrCreateTextDocumentFromModuleName(*importedName);
                    location = *it->second.definitionLocation;
                } else
                    return std::nullopt;
            } else {
                location = lookupTypeLocation(*scope, reference->name.value);
            }

            if (!referenceTextDocument || !location)
                return std::nullopt;

            return lsp::Location{referenceTextDocument.uri,
                                 lsp::Range{convertPosition(location->begin),
                                            convertPosition(location->end)}};
        }
        return std::nullopt;
    };

    if (auto type = node->asType()) {
        return findTypeLocation(type);
    } else if (auto typeAlias = node->as<Luau::AstStatTypeAlias>()) {
        return findTypeLocation(typeAlias->type);
    } else if (auto expr = node->asExpr()) {
        if (auto ty = module->astTypes.find(expr)) {
            auto followedTy = Luau::follow(*ty);
            auto definitionModuleName =
                Luau::getDefinitionModuleName(followedTy);
            auto location = getLocation(followedTy);

            if (definitionModuleName && location) {
                auto document = getOrCreateTextDocumentFromModuleName(
                    *definitionModuleName);
                if (document)
                    return lsp::Location{
                        document.uri,
                        lsp::Range{convertPosition(location->begin),
                                   convertPosition(location->end)}};
            }
        }
    }

    return std::nullopt;
}

void LuauLanguageServer::initialize(const InitializationOptions &options) {
    if (isReady) {
        return;
    }

    defaultConfig.mode = options.defaultMode;
    definitionsFiles.clear();
    for (auto it = options.definitionsFiles.cbegin();
         it != options.definitionsFiles.cend(); ++it) {
        const auto packageName = it.key().toUtf8();
        definitionsFiles.insert_or_assign(
            std::string(packageName.constData(),
                        static_cast<std::size_t>(packageName.size())),
            it.value());
    }

    if (!appliedFirstTimeConfiguration) {
        appliedFirstTimeConfiguration = true;
        registerTypes();
    }

    readAllowedCfgRoots = options.readCfgRoots;
    frontend.setLuauSolverMode(Luau::SolverMode::Old);
    isReady = true;
}

QTextDocument *
LuauLanguageServer::getTextDocument(const Luau::ModuleName &moduleName) {
    auto it = managedFiles.find(moduleName);
    if (it != managedFiles.end()) {
        return it->second;
    }
    return nullptr;
}

LuauLanguageServer::DocumentHandle
LuauLanguageServer::getOrCreateTextDocumentFromModuleName(
    const Luau::ModuleName &name) {
    if (auto document = getTextDocument(name)) {
        return DocumentHandle(getUri(name), document);
    }

    if (auto definition = definitionsFileState.find(name);
        definition != definitionsFileState.end())
        return DocumentHandle(getUri(name),
                              definition->second.textDocument.get());

    auto source = readSource(name);
    if (!source)
        return {};

    auto document = std::make_unique<QTextDocument>();
    document->setPlainText(QString::fromUtf8(
        source->source.data(), static_cast<qsizetype>(source->source.size())));
    return DocumentHandle(getUri(name), std::move(document));
}

std::optional<Luau::SourceCode>
LuauLanguageServer::readSource(const Luau::ModuleName &name) {
    auto doc = getTextDocument(name);
    if (doc) {
        const auto text = doc->toPlainText();
        return std::make_optional<Luau::SourceCode>(
            {text.toStdString(), sourceCodeTypeFromPath(name)});
    }

    auto path = QString::fromUtf8(name);
    QFile f(path);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        qCritical("[LuauLanguageServer] Failed to open source '%s': %s",
                  qPrintable(path), qPrintable(f.errorString()));
        return std::nullopt;
    }

    const auto sourceBytes = f.readAll();
    if (f.error() != QFile::NoError) {
        qCritical("[LuauLanguageServer] Failed to read source '%s': %s",
                  qPrintable(path), qPrintable(f.errorString()));
        return std::nullopt;
    }
    const std::string source(sourceBytes.constData(), sourceBytes.size());
    return std::make_optional<Luau::SourceCode>(
        {source, sourceCodeTypeFromPath(name)});
}

std::optional<Luau::ModuleInfo>
LuauLanguageServer::resolveModule(const Luau::ModuleInfo *context,
                                  Luau::AstExpr *expr,
                                  const Luau::TypeCheckLimits &limits) {
    if (auto *node = expr->as<Luau::AstExprConstantString>()) {
        std::string_view requiredString(node->value.data, node->value.size);
        return resolveStringRequire(context, requiredString, limits);
    }
    return std::nullopt;
}

std::string LuauLanguageServer::getHumanReadableModuleName(
    const Luau::ModuleName &name) const {
    return name;
}

std::optional<std::string> LuauLanguageServer::getEnvironmentForModule(
    const Luau::ModuleName &name) const {
    return std::nullopt;
}

std::optional<Luau::ModuleInfo>
LuauLanguageServer::resolveStringRequire(const Luau::ModuleInfo *context,
                                         const std::string_view requiredString,
                                         const Luau::TypeCheckLimits &limits) {
    Q_UNUSED(limits);
    if (!context) {
        return std::nullopt;
    }

    if (requiredString.empty()) {
        return std::nullopt;
    }

    const auto contextPath = QFileInfo(QString::fromUtf8(
        context->name.data(), static_cast<qsizetype>(context->name.size())));
    if (!contextPath.isAbsolute()) {
        return std::nullopt;
    }

    auto requested =
        QString::fromUtf8(requiredString.data(), requiredString.size());
    if (requested.contains(QChar(':'))) {
        return std::nullopt;
    }

    const auto resolution = WingLuauRequire::resolveFileModule(
        contextPath.absolutePath(), requested);
    if (resolution.kind != WingLuauRequire::FileModuleResolution::Kind::File &&
        resolution.kind !=
            WingLuauRequire::FileModuleResolution::Kind::ModuleDirectory) {
        return std::nullopt;
    }

    return Luau::ModuleInfo(resolution.path.toStdString());
}

Luau::LoadDefinitionFileResult LuauLanguageServer::registerDefinitions(
    Luau::Frontend &frontend, Luau::GlobalTypes &globals,
    const std::string &packageName, const std::string &definitions) {
    return frontend.loadDefinitionFile(globals, globals.globalScope,
                                       definitions, packageName,
                                       /* captureComments = */ true);
}

Luau::LoadDefinitionFileResult
LuauLanguageServer::loadDefinitionFile(const std::string &packageName,
                                       const std::string &source) {
    auto result =
        registerDefinitions(frontend, frontend.globals, packageName, source);
    registerDefinitions(frontend, frontend.globalsForAutocomplete, packageName,
                        source);

    if (result.success) {
        auto textDocument = std::make_unique<QTextDocument>();
        textDocument->setPlainText(QString::fromStdString(source));
        definitionsFileState.emplace(
            packageName,
            DefinitionsFileState{std::move(textDocument), result.sourceModule,
                                 std::move(result.module)});
    }

    return result;
}

const Luau::ModulePtr
LuauLanguageServer::getModule(const Luau::ModuleName &moduleName,
                              bool forAutocomplete) const {
    if (forAutocomplete) {
        return frontend.moduleResolverForAutocomplete.getModule(moduleName);
    }
    return frontend.moduleResolver.getModule(moduleName);
}

bool LuauLanguageServer::isIgnoredFile(
    const Luau::ModuleName &moduleName) const {
    return false;
}

bool LuauLanguageServer::isDefinitionFile(
    const Luau::ModuleName &moduleName) const {
    return definitionsFiles.find(moduleName) != definitionsFiles.end();
}

const Luau::Config &
LuauLanguageServer::getConfig(const Luau::ModuleName &name,
                              const Luau::TypeCheckLimits &limits) const {
    auto path = getParentPath(QString::fromStdString(name));
    if (path.isEmpty()) {
        return defaultConfig;
    }
    return readConfigRec(path, limits);
}

void LuauLanguageServer::documentDiagnostics(
    const Luau::ModuleName &moduleName) {
    if (managedFiles.find(moduleName) == managedFiles.end()) {
        qCritical("[LuauLanguageServer] Cannot diagnose unopened document '%s'",
                  moduleName.c_str());
        return;
    }

    // Keep a vector of reverse dependencies marked dirty to extend
    // diagnostics for them
    std::vector<Luau::ModuleName> markedDirty{};

    // Mark the module dirty for the typechecker
    frontend.markDirty(moduleName, &markedDirty);

    // Convert the diagnostics report into a series of diagnostics published
    // for each relevant file
    const auto documentUri = getUri(moduleName);
    auto diagnostics =
        documentDiagnostics(lsp::DocumentDiagnosticParams{documentUri},
                            /* cancellationToken= */ nullptr);
    publishDiagnostics(documentUri, diagnostics.items);

    // Compute diagnostics for reverse dependencies
    for (auto &moduleName : markedDirty) {
        auto dirtyUri = getUri(moduleName);
        if (dirtyUri != documentUri &&
            diagnostics.relatedDocuments.find(dirtyUri) ==
                diagnostics.relatedDocuments.end() &&
            !isIgnoredFile(moduleName)) {
            auto dependencyDiags =
                documentDiagnostics(lsp::DocumentDiagnosticParams{dirtyUri},
                                    /* cancellationToken=*/nullptr,
                                    /* allowUnmanagedFiles= */ true);
            publishDiagnostics(dirtyUri, dependencyDiags.items);
        }
    }
}

lsp::DocumentDiagnosticReport LuauLanguageServer::documentDiagnostics(
    const lsp::DocumentDiagnosticParams &params,
    const LSPCancellationToken &cancellationToken, bool allowUnmanagedFiles) {
    if (!isReady) {
        return {};
    }

    lsp::DocumentDiagnosticReport report;
    QHash<lsp::DocumentUri, QVector<lsp::Diagnostic>> relatedDiagnostics;

    auto moduleName = getModuleName(params.textDocument);
    DocumentHandle textDocument =
        allowUnmanagedFiles
            ? getOrCreateTextDocumentFromModuleName(moduleName)
            : DocumentHandle(params.textDocument, getTextDocument(moduleName));
    if (!textDocument)
        return report; // Bail early with empty report - file was likely
                       // closed

    // Check the module
    // In the new solver, we end up calling `checkStrict` (retain type
    // graphs), because documentation diagnostics is typically on the file a
    // user is working on. So, we will end up having to call checkStrict
    // later for Hover etc. i.e., calling 2 typechecks for no reason. In the
    // old solver, it doesn't really matter, because there is a differnce
    // between module + moduleForAutocomplete. So we prefer using
    // checkSimple as we won't use the type graphs
    Luau::CheckResult cr = checkSimple(moduleName, cancellationToken);

    if (cancellationToken && cancellationToken->requested()) {
        return report;
    }

    // If there was an error retrieving the source module
    // Bail early with an empty report - it is likely that the file was
    // closed
    if (!frontend.getSourceModule(moduleName))
        return report;

    // If the file is a definitions file, then don't display any diagnostics
    if (isDefinitionFile(moduleName))
        return report;

    // Report Type Errors
    // Note that type errors can extend to related modules in the require
    // graph - so we report related information here
    for (auto &error : cr.errors) {
        if (error.moduleName == moduleName) {
            auto diagnostic = createTypeErrorDiagnostic(error, this);
            report.items.emplace_back(diagnostic);
        } else {
            auto uri = getUri(error.moduleName);
            if (isIgnoredFile(error.moduleName))
                continue;
            auto diagnostic = createTypeErrorDiagnostic(error, this);
            auto &currentDiagnostics = relatedDiagnostics[uri];
            currentDiagnostics.emplace_back(diagnostic);
        }
    }

    // Convert the related diagnostics map into an equivalent report
    if (!relatedDiagnostics.empty()) {
        for (auto it = relatedDiagnostics.cbegin();
             it != relatedDiagnostics.cend(); ++it) {
            lsp::DocumentDiagnosticReport subReport{
                lsp::DocumentDiagnosticReportKind::Full, it.value()};
            report.relatedDocuments.insert(it.key(), std::move(subReport));
        }
    }

    // Report Lint Warnings
    // Lints only apply to the current file
    for (auto &error : cr.lintResult.errors) {
        auto diagnostic = createLintDiagnostic(error);
        diagnostic.severity =
            lsp::DiagnosticSeverity::Error; // Report this as an error
                                            // instead
        report.items.emplace_back(diagnostic);
    }
    for (auto &error : cr.lintResult.warnings)
        report.items.emplace_back(createLintDiagnostic(error));

    return report;
}

Luau::CheckResult
LuauLanguageServer::checkSimple(const Luau::ModuleName &moduleName,
                                const LSPCancellationToken &cancellationToken) {
    try {
        Luau::FrontendOptions options{/* retainFullTypeGraphs: */ false,
                                      /* forAutocomplete: */ false,
                                      /* runLintChecks: */ true};
        options.cancellationToken = cancellationToken;
        return frontend.check(moduleName, options);
    } catch (Luau::InternalCompilerError &err) {
        qCritical("[LuauLanguageServer] Internal compiler error in '%s': %s",
                  moduleName.c_str(), err.what());
        return Luau::CheckResult{};
    }
}

Luau::CheckResult
LuauLanguageServer::checkStrict(const Luau::ModuleName &moduleName,
                                const LSPCancellationToken &cancellationToken,
                                bool forAutocomplete) {
    // HACK: note that a previous call to `Frontend::check(moduleName, {
    // retainTypeGraphs: false })` and then a call
    // `Frontend::check(moduleName, { retainTypeGraphs: true })` will NOT
    // actually retain the type graph if the module is not marked dirty. We
    // do a manual check and dirty marking to fix this
    auto module = getModule(moduleName, forAutocomplete);
    if (module && module->internalTypes->types
                      .empty()) // If we didn't retain type graphs, then the
                                // internalTypes arena is empty
        frontend.markDirty(moduleName);

    Luau::FrontendOptions options{/* retainFullTypeGraphs: */ true,
                                  forAutocomplete,
                                  /* runLintChecks: */ true};
    options.cancellationToken = cancellationToken;
    return frontend.check(moduleName, options);
}

void LuauLanguageServer::publishDiagnostics(
    const lsp::DocumentUri &uri,
    const QVector<lsp::Diagnostic> &diagnostic) const {
    Q_EMIT const_cast<LuauLanguageServer *>(this)->onPublishDiagnostic(
        uri, diagnostic);
}

std::optional<QString> LuauLanguageServer::getDocumentationForAutocompleteEntry(
    const std::string &name, const Luau::AutocompleteEntry &entry,
    const std::vector<Luau::AstNode *> &ancestry,
    const Luau::ModulePtr &localModule, const Luau::Position &position) {
    if (entry.documentationSymbol)
        if (auto docs =
                printDocumentation(documentation, *entry.documentationSymbol))
            return docs;

    if (entry.type.has_value())
        if (auto documentation = getDocumentationForType(entry.type.value()))
            return documentation;

    if (entry.kind == Luau::AutocompleteEntryKind::Type) {
        std::optional<Luau::AstName> importedPrefix = std::nullopt;
        if (auto node = ancestry.back())
            if (auto typeReference = node->as<Luau::AstTypeReference>())
                importedPrefix = typeReference->prefix;

        auto scope = Luau::findScopeAtPosition(*localModule, position);
        if (auto documentation = getDocumentationForTypeReference(
                localModule->name, scope, importedPrefix, name,
                /* forAutocomplete= */ true))
            return documentation;
    }

    if (entry.prop) {
        std::optional<Luau::ModuleName> definitionModuleName;

        if (entry.containingExternType) {
            definitionModuleName =
                entry.containingExternType.value()->definitionModuleName;
        } else {
            // Derive the containing table type from the completion ancestry.
            if (localModule) {
                Luau::TypeId *parentTy = nullptr;
                if (auto node = ancestry.back()) {
                    if (auto indexName = node->as<Luau::AstExprIndexName>())
                        parentTy = localModule->astTypes.find(indexName->expr);
                    else if (auto indexExpr =
                                 node->as<Luau::AstExprIndexExpr>())
                        parentTy = localModule->astTypes.find(indexExpr->expr);
                    else if (node->is<Luau::AstExprGlobal>()) {
                        // potentially autocompleting a property inside of a
                        // table literal
                        if (ancestry.size() > 2)
                            if (auto table = ancestry[ancestry.size() - 2]
                                                 ->as<Luau::AstExprTable>())
                                parentTy = localModule->astTypes.find(table);
                    }
                }

                if (parentTy) {
                    // parentTy might be an intersected type, find the
                    // actual base ttv
                    auto followedTy = Luau::follow(*parentTy);
                    if (auto propInformation =
                            LuauLsp::lookupProp(followedTy, name);
                        !propInformation.empty())
                        definitionModuleName = Luau::getDefinitionModuleName(
                            propInformation[0].baseTableTy);
                    else
                        definitionModuleName =
                            Luau::getDefinitionModuleName(followedTy);
                }
            }
        }

        if (definitionModuleName) {
            if (auto propLocation = entry.prop.value()->location)
                if (auto text = printMoonwaveDocumentation(getComments(
                        definitionModuleName.value(), propLocation.value()));
                    !text.isEmpty())
                    return text;

            if (auto typeLocation = entry.prop.value()->typeLocation)
                if (auto text = printMoonwaveDocumentation(getComments(
                        definitionModuleName.value(), typeLocation.value()));
                    !text.isEmpty())
                    return text;
        }
    }

    return std::nullopt;
}

QStringList LuauLanguageServer::getComments(const Luau::ModuleName &moduleName,
                                            const Luau::Location &node) {
    Luau::SourceModule *sourceModule;
    DocumentHandle textDocument;

    if (auto it = definitionsFileState.find(moduleName);
        it != definitionsFileState.end()) {
        sourceModule = &it->second.sourceModule;
        textDocument =
            DocumentHandle(getUri(moduleName), it->second.textDocument.get());
    } else {
        sourceModule = frontend.getSourceModule(moduleName);
        if (!sourceModule)
            return {};

        textDocument = getOrCreateTextDocumentFromModuleName(moduleName);
        if (!textDocument)
            return {};
    }

    auto commentLocations = getCommentLocations(sourceModule, node);
    if (commentLocations.empty())
        return {};

    const auto source = textDocument.document->toPlainText();
    const auto offsetForPosition = [&source](const Luau::Position &position) {
        qsizetype offset = 0;
        for (std::uint32_t line = 0; line < position.line; ++line) {
            const auto newline = source.indexOf('\n', offset);
            if (newline < 0)
                return qsizetype(-1);
            offset = newline + 1;
        }
        const auto result = offset + position.column;
        return result <= source.size() ? result : qsizetype(-1);
    };

    QStringList comments;
    for (auto &comment : commentLocations) {
        if (comment.type == Luau::Lexeme::Type::BrokenComment)
            continue;

        const auto start = offsetForPosition(comment.location.begin);
        const auto end = offsetForPosition(comment.location.end);
        if (start < 0 || end < start)
            continue;
        auto commentText = source.sliced(start, end - start);

        if (comment.type == Luau::Lexeme::Type::Comment) {
            commentText = commentText.trimmed();
            if (commentText.startsWith(QStringLiteral("--- "))) {
                comments.append(commentText.sliced(4));
            } else if (commentText == QLatin1String("---")) {
                comments.emplace_back();
            }
            continue;
        }

        if (comment.type != Luau::Lexeme::Type::BlockComment)
            continue;

        static const QRegularExpression blockStart(R"(^--\[(=*)\[)");
        auto match = blockStart.match(commentText);
        if (!match.hasMatch()) {
            continue;
        }

        const auto closing = ']' + match.captured(1) + ']';
        auto body = commentText.sliced(match.capturedLength());
        const auto closingPosition = body.lastIndexOf(closing);
        if (closingPosition != std::string::npos) {
            body.resize(closingPosition);
        }

        auto lines = body.split('\n');
        for (QString &line : lines) {
            line = line.trimmed();
        }

        while (!lines.isEmpty() && lines.first().isEmpty()) {
            lines.removeFirst();
        }
        while (!lines.isEmpty() && lines.last().isEmpty()) {
            lines.removeLast();
        }

        int indentation = -1;
        for (const auto &line : lines) {
            if (line.isEmpty()) {
                continue;
            }
            int pos = 0;
            while (pos < line.size() && line.at(pos).isSpace()) {
                ++pos;
            }
            if (indentation == -1 || pos < indentation) {
                indentation = pos;
            }
        }

        if (indentation > 0) {
            for (QString &line : lines) {
                if (!line.isEmpty()) {
                    line.remove(0, indentation);
                }
            }
        }
        comments.append(lines);
    }

    return comments;
}

std::optional<QString>
LuauLanguageServer::getDocumentationForType(const Luau::TypeId ty) {
    auto followedTy = Luau::follow(ty);
    if (auto ftv = Luau::get<Luau::FunctionType>(followedTy);
        ftv && ftv->definition && ftv->definition->definitionModuleName) {
        return printMoonwaveDocumentation(
            getComments(ftv->definition->definitionModuleName.value(),
                        ftv->definition->definitionLocation));
    } else if (auto ttv = Luau::get<Luau::TableType>(followedTy);
               ttv && !ttv->definitionModuleName.empty()) {
        return printMoonwaveDocumentation(
            getComments(ttv->definitionModuleName, ttv->definitionLocation));
    } else if (auto etv = Luau::get<Luau::ExternType>(followedTy);
               etv && !etv->definitionModuleName.empty() &&
               etv->definitionLocation) {
        return printMoonwaveDocumentation(
            getComments(etv->definitionModuleName, *etv->definitionLocation));
    }
    return std::nullopt;
}

std::optional<QString> LuauLanguageServer::getDocumentationForTypeReference(
    const Luau::ModuleName &moduleName, const Luau::ScopePtr &scope,
    const std::optional<Luau::AstName> &prefix, const Luau::Name &typeName,
    bool forAutocomplete) {
    if (prefix) {
        auto importedModuleName = lookupImportedModule(*scope, prefix->value);
        if (!importedModuleName)
            return std::nullopt;
        auto importedModule = getModule(*importedModuleName,
                                        /* forAutocomplete: */ forAutocomplete);
        if (!importedModule)
            return std::nullopt;
        if (const auto it = importedModule->exportedTypeBindings.find(typeName);
            it != importedModule->exportedTypeBindings.end() &&
            it->second.definitionLocation)
            return printMoonwaveDocumentation(getComments(
                *importedModuleName, *it->second.definitionLocation));
        return std::nullopt;
    } else {
        auto typeLocation = lookupTypeLocation(*scope, typeName);
        if (!typeLocation)
            return std::nullopt;
        return printMoonwaveDocumentation(
            getComments(moduleName, *typeLocation));
    }
}

std::optional<QString> LuauLanguageServer::getDocumentationForAstNode(
    const Luau::ModuleName &moduleName, const Luau::AstNode *node,
    const Luau::ScopePtr scope) {
    if (auto ref = node->as<Luau::AstTypeReference>()) {
        return getDocumentationForTypeReference(moduleName, scope, ref->prefix,
                                                ref->name.value,
                                                /* forAutocomplete= */ true);
    } else if (auto alias = node->as<Luau::AstStatTypeAlias>()) {
        return printMoonwaveDocumentation(
            getComments(moduleName, alias->location));
    }
    return std::nullopt;
}

void LuauLanguageServer::registerTypes() {
    Luau::registerBuiltinGlobals(frontend, frontend.globals);
    Luau::registerBuiltinGlobals(frontend, frontend.globalsForAutocomplete);

    auto &tagRegisterGlobals = frontend.globalsForAutocomplete;
    Luau::attachTag(Luau::getGlobalBinding(tagRegisterGlobals, "require"),
                    "Require");

    std::vector<std::pair<std::string, QString>> definitionsFilesToProcess{};
    definitionsFilesToProcess.reserve(definitionsFiles.size());
    for (const auto &pair : definitionsFiles) {
        definitionsFilesToProcess.emplace_back(pair);
    }
    std::sort(definitionsFilesToProcess.begin(),
              definitionsFilesToProcess.end(),
              [](const auto &left, const auto &right) {
                  return left.first < right.first;
              });

    for (const auto &[packageName, definitionsFile] :
         definitionsFilesToProcess) {
        QFileInfo finfo(definitionsFile);
        auto resolvedFilePath = finfo.absoluteFilePath();

        QFile f(resolvedFilePath);
        if (!f.open(QFile::ReadOnly | QFile::Text)) {
            qCritical(
                "[LuauLanguageServer] Failed to open definitions '%s': %s",
                qPrintable(resolvedFilePath), qPrintable(f.errorString()));
            lsp::Diagnostic diagnostic;
            diagnostic.message = f.errorString();
            diagnostic.severity = lsp::DiagnosticSeverity::Error;
            diagnostic.source = "Luau";
            publishDiagnostics(QUrl::fromLocalFile(resolvedFilePath),
                               {diagnostic});
            continue;
        }

        const auto definitionsBytes = f.readAll();
        if (f.error() != QFile::NoError) {
            qCritical(
                "[LuauLanguageServer] Failed to read definitions '%s': %s",
                qPrintable(resolvedFilePath), qPrintable(f.errorString()));
            lsp::Diagnostic diagnostic;
            diagnostic.message = f.errorString();
            diagnostic.severity = lsp::DiagnosticSeverity::Error;
            diagnostic.source = "Luau";
            publishDiagnostics(QUrl::fromLocalFile(resolvedFilePath),
                               {diagnostic});
            continue;
        }
        const std::string definitionsContents(definitionsBytes.constData(),
                                              definitionsBytes.size());

        auto result = loadDefinitionFile(packageName, definitionsContents);

        const auto uri = QUrl::fromLocalFile(resolvedFilePath);
        if (result.success) {
            publishDiagnostics(uri, {});

            // Update the text document URI to point to the actual file on
            // disk
            if (auto it = definitionsFileState.find(packageName);
                it != definitionsFileState.end())
                it->second.textDocument->setPlainText(QString::fromUtf8(
                    definitionsContents.data(),
                    static_cast<qsizetype>(definitionsContents.size())));
        } else {
            qCritical("[LuauLanguageServer] Failed to load definitions '%s'",
                      qPrintable(resolvedFilePath));
            QVector<lsp::Diagnostic> diagnostics;
            for (auto &error : result.parseResult.errors) {
                diagnostics.emplace_back(createParseErrorDiagnostic(error));
            }

            if (result.module) {
                for (auto &error : result.module->errors) {
                    diagnostics.emplace_back(
                        createTypeErrorDiagnostic(error, this));
                }
            }

            publishDiagnostics(uri, diagnostics);
        }
    }

    Luau::freeze(frontend.globals.globalTypes);
    Luau::freeze(frontend.globalsForAutocomplete.globalTypes);
}

Luau::SourceCode::Type
LuauLanguageServer::sourceCodeTypeFromPath(const std::string &path) const {
    return Luau::SourceCode::Type::Module;
}

static std::optional<std::string> parseConfig(const QString &configPath,
                                              const std::string &contents,
                                              Luau::Config &result) {
    Luau::ConfigOptions::AliasOptions aliasOpts;
    aliasOpts.configLocation = getParentPath(configPath).toStdString();
    aliasOpts.overwriteAliases = true;

    Luau::ConfigOptions opts;
    opts.aliasOptions = std::move(aliasOpts);
    return Luau::parseConfig(contents, result, opts);
}

static std::optional<std::string>
parseLuauConfig(const QString &configPath, const std::string &contents,
                Luau::Config &result, const Luau::TypeCheckLimits &limits) {
    Luau::ConfigOptions::AliasOptions aliasOpts;
    aliasOpts.configLocation = getParentPath(configPath).toStdString();
    aliasOpts.overwriteAliases = true;

    Luau::InterruptCallbacks callbacks;
    LuauConfigInterruptInfo info{limits, configPath.toStdString()};
    callbacks.initCallback = [&info](lua_State *L) {
        lua_setthreaddata(L, &info);
    };
    callbacks.interruptCallback = [](lua_State *L, int gc) {
        auto *info =
            static_cast<LuauConfigInterruptInfo *>(lua_getthreaddata(L));
        if (info->limits.finishTime &&
            Luau::TimeTrace::getClock() > *info->limits.finishTime)
            throw Luau::TimeLimitError{info->module};
        if (info->limits.cancellationToken &&
            info->limits.cancellationToken->requested())
            throw Luau::UserCancelError{info->module};
    };

    return Luau::extractLuauConfig(contents, result, aliasOpts,
                                   std::move(callbacks));
}

const Luau::Config &
LuauLanguageServer::readConfigRec(const QString &path,
                                  const Luau::TypeCheckLimits &limits) const {
    Luau::Config result = defaultConfig;
    for (const auto &p : readAllowedCfgRoots) {
        if (!isPathSubdirectory(p, path)) {
            return defaultConfig;
        }
    }

    auto it = configCache.find(path);
    if (it != configCache.end()) {
        return it->second;
    }

    const auto parentPath = getParentPath(path);
    if (parentPath != path) {
        result = readConfigRec(parentPath, limits);
    }

    QDir pathDir(path);
    auto configPath =
        pathDir.absoluteFilePath(QString::fromLatin1(Luau::kConfigName));
    auto luauConfigPath =
        pathDir.absoluteFilePath(QString::fromLatin1(Luau::kLuauConfigName));

    if (QFile::exists(luauConfigPath)) {
        loadConfig(
            result, luauConfigPath,
            [&](const std::string &contents) -> std::optional<std::string> {
                return parseLuauConfig(luauConfigPath, contents, result,
                                       limits);
            });
    }
    if (QFile::exists(configPath)) {
        loadConfig(
            result, configPath,
            [&](const std::string &contents) -> std::optional<std::string> {
                return parseConfig(configPath, contents, result);
            });
    }

    return configCache[path] = std::move(result);
}

void LuauLanguageServer::loadConfig(
    const Luau::Config &cfg, const QString &filePath,
    const std::function<std::optional<std::string>(const std::string &)> &parse)
    const {
    QFile file(filePath);
    const auto diagnosticUri = QUrl::fromLocalFile(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qCritical("[LuauLanguageServer] Failed to open configuration '%s': %s",
                  qUtf8Printable(filePath), qUtf8Printable(file.errorString()));
        lsp::Diagnostic diagnostic;
        diagnostic.message = file.errorString();
        diagnostic.severity = lsp::DiagnosticSeverity::Error;
        diagnostic.source = QLatin1String("Luau");
        publishDiagnostics(diagnosticUri, {diagnostic});
        return;
    }

    auto total = file.size();
    std::string contents(total, 0);
    auto bc = file.read(contents.data(), total);
    if (bc != total) {
        qCritical("[LuauLanguageServer] Failed to read configuration '%s'",
                  qUtf8Printable(filePath));
        lsp::Diagnostic diagnostic;
        diagnostic.message = file.errorString();
        diagnostic.severity = lsp::DiagnosticSeverity::Error;
        diagnostic.source = QLatin1String("Luau");
        publishDiagnostics(diagnosticUri, {diagnostic});
        return;
    }

    if (const auto error = parse(contents)) {
        qCritical("[LuauLanguageServer] Failed to parse configuration '%s': %s",
                  qUtf8Printable(filePath), error->c_str());
        lsp::Diagnostic diagnostic;
        diagnostic.message = QString::fromStdString(error.value());
        diagnostic.severity = lsp::DiagnosticSeverity::Error;
        diagnostic.source = "Luau";
        publishDiagnostics(diagnosticUri, {diagnostic});
    } else {
        publishDiagnostics(diagnosticUri, {});
    }
};

void LuauLanguageServer::clearDiagnosticsForFile(
    const lsp::DocumentUri &uri) const {
    publishDiagnostics(uri, {});
}
