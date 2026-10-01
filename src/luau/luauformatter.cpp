/*==============================================================================
 ** Copyright (C) 2026-2029 WingSummer
 **
 ** This program is free software: you can redistribute it and/or modify it
 ** under the terms of the GNU Affero General Public License as published by the
 ** Free Software Foundation, version 3.
 **
 ** This program is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 ** for more details.
 **
 ** You should have received a copy of the GNU Affero General Public License
 ** along with this program. If not, see <https://www.gnu.org/licenses/>.
 ** =============================================================================
 */

#include "luauformatter.h"

#include "Luau/Parser.h"

namespace {
qsizetype advanceUtf16Offset(const QByteArray &text, size_t targetByteOffset,
                             size_t &currentByteOffset,
                             qsizetype &currentUtf16Offset) {
    const auto textSize = size_t(text.size());
    targetByteOffset = std::min(targetByteOffset, textSize);
    while (currentByteOffset < targetByteOffset) {
        const unsigned char first =
            static_cast<unsigned char>(text[currentByteOffset]);
        const size_t characterSize = first < 0x80             ? 1
                                     : (first & 0xe0) == 0xc0 ? 2
                                     : (first & 0xf0) == 0xe0 ? 3
                                                              : 4;
        if (currentByteOffset + characterSize > targetByteOffset)
            break;
        currentByteOffset += characterSize;
        currentUtf16Offset += characterSize == 4 ? 2 : 1;
    }
    return currentUtf16Offset;
}
} // namespace

LuauFormat::LuauFormat(const FormatOptions &options) : options_(options) {}

// ============ Low-level output ============

void LuauFormat::write(const QByteArray &str) {
    if (str.isEmpty())
        return;
    if (atLineStart_) {
        writeIndent();
        atLineStart_ = false;
    }
    output_ += str;
}

void LuauFormat::writeIndent() {
    if (options_.useTabs)
        output_ += QByteArray(indentLevel_, '\t');
    else
        output_ += QByteArray(indentLevel_ * options_.indentSize, ' ');
}

void LuauFormat::newline() {
    // trim trailing spaces on current line
    while (!output_.isEmpty() &&
           (output_.back() == ' ' || output_.back() == '\t')) {
        output_.chop(1);
    }
    output_ += '\n';
    atLineStart_ = true;
}

void LuauFormat::space() {
    if (!output_.isEmpty() && output_.back() != '\n' && output_.back() != ' ' &&
        output_.back() != '\t') {
        output_.append(' ');
    }
}

void LuauFormat::writeKeyword(const QByteArray &kw) {
    // Add a space before keyword if the previous char is an identifier char
    if (!output_.isEmpty() && !atLineStart_) {
        char last = output_.back();
        bool needSpace = std::isalnum(last) || last == '_';
        if (needSpace)
            output_.append(' ');
    }
    write(kw);
}

void LuauFormat::writeIdentifier(const QByteArray &ident) {
    if (!output_.isEmpty() && !atLineStart_) {
        char last = output_.back();
        bool needSpace = std::isalnum(last) || last == '_';
        if (needSpace)
            output_.append(' ');
    }
    write(ident);
}

void LuauFormat::writeSymbol(const QByteArray &sym) { write(sym); }

void LuauFormat::writeLiteral(const QByteArray &lit) {
    // Prevent merging with previous identifier chars (e.g., `1` after `x`)
    if (!output_.isEmpty() && !atLineStart_) {
        char last = output_.back();
        bool lastIsIdent = std::isalnum(last) || last == '_';
        bool firstIsDigit = !lit.isEmpty() && !std::isdigit(lit[0]);
        if (lastIsIdent && firstIsDigit)
            output_.append(' ');
    }
    write(lit);
}

void LuauFormat::writeString(
    const QString &value,
    Luau::CstExprConstantString::QuoteStyle originalQuoteStyle,
    unsigned blockDepth) {
    if (options_.quoteStyle == QuoteStyle::Preserve &&
        originalQuoteStyle ==
            Luau::CstExprConstantString::QuoteStyle::QuotedRaw) {
        const QByteArray blocks(blockDepth, '=');
        write(QByteArrayLiteral("[") + blocks + QByteArrayLiteral("["));
        write(value.toUtf8());
        write(QByteArrayLiteral("]") + blocks + QByteArrayLiteral("]"));
        return;
    }

    QChar quote = originalQuoteStyle ==
                          Luau::CstExprConstantString::QuoteStyle::QuotedSingle
                      ? QLatin1Char('\'')
                      : QLatin1Char('"');
    switch (options_.quoteStyle) {
    case QuoteStyle::Preserve:
        if (originalQuoteStyle ==
            Luau::CstExprConstantString::QuoteStyle::QuotedInterp) {
            write(value.toUtf8());
            return;
        }
        break;
    case QuoteStyle::PreferDouble:
        quote = QLatin1Char(value.count(QLatin1Char('"')) <=
                                    value.count(QLatin1Char('\''))
                                ? '"'
                                : '\'');
        break;
    case QuoteStyle::PreferSingle:
        quote = QLatin1Char(value.count(QLatin1Char('\'')) <=
                                    value.count(QLatin1Char('"'))
                                ? '\''
                                : '"');
        break;
    case QuoteStyle::ForceDouble:
        quote = QLatin1Char('"');
        break;
    case QuoteStyle::ForceSingle:
        quote = QLatin1Char('\'');
        break;
    }

    QString escaped;
    escaped.reserve(value.size() + 2);
    escaped.append(quote);
    for (const QChar character : value) {
        switch (character.unicode()) {
        case '\a':
            escaped.append(QStringLiteral("\\a"));
            break;
        case '\b':
            escaped.append(QStringLiteral("\\b"));
            break;
        case '\f':
            escaped.append(QStringLiteral("\\f"));
            break;
        case '\n':
            escaped.append(QStringLiteral("\\n"));
            break;
        case '\r':
            escaped.append(QStringLiteral("\\r"));
            break;
        case '\t':
            escaped.append(QStringLiteral("\\t"));
            break;
        case '\v':
            escaped.append(QStringLiteral("\\v"));
            break;
        default:
            if (character == quote || character == QLatin1Char('\\')) {
                escaped.append(QLatin1Char('\\'));
                escaped.append(character);
            } else if (character.unicode() < 0x20) {
                escaped.append(QLatin1Char('\\'));
                escaped.append(QStringLiteral("%1").arg(
                    static_cast<uint>(character.unicode()), 3, 10,
                    QLatin1Char('0')));
            } else {
                escaped.append(character);
            }
            break;
        }
    }
    escaped.append(quote);
    write(escaped.toUtf8());
}

// ============ CST helpers ============

QByteArray LuauFormat::getIndentString() const {
    if (options_.useTabs)
        return QByteArray(indentLevel_, '\t');
    return QByteArray(indentLevel_ * options_.indentSize, ' ');
}

QByteArray LuauFormat::sourceText(const Luau::Location &location) const {
    const size_t begin = sourceOffset(location.begin);
    const size_t end = std::max(begin, sourceOffset(location.end));
    return source_.sliced(begin, end - begin);
}

size_t LuauFormat::sourceOffset(const Luau::Position &position) const {
    if (position.line >= sourceLineOffsets_.size()) {
        return size_t(source_.size());
    }
    return std::min(size_t(source_.size()), sourceLineOffsets_[position.line] +
                                                size_t(position.column));
}

void LuauFormat::buildTokenAnchors(Luau::AstNameTable &names) {
    std::vector<Luau::Location> sourceTokens;
    std::vector<Luau::Location> outputTokens;
    const auto collectTokens = [&names](const QByteArray &text,
                                        std::vector<Luau::Location> &tokens) {
        Luau::Lexer lexer(text.constData(), static_cast<size_t>(text.size()),
                          names);
        lexer.setSkipComments(false);
        while (true) {
            const Luau::Lexeme &lexeme = lexer.next();
            if (lexeme.type == Luau::Lexeme::Eof)
                break;
            tokens.push_back(lexeme.location);
        }
    };
    collectTokens(source_, sourceTokens);
    collectTokens(output_, outputTokens);

    std::vector<size_t> outputLineOffsets{0};
    for (size_t i = 0; i < static_cast<size_t>(output_.size()); ++i) {
        if (output_[static_cast<qsizetype>(i)] == '\n')
            outputLineOffsets.push_back(i + 1);
    }
    const auto offsetForPosition = [](const Luau::Position &position,
                                      const std::vector<size_t> &lineOffsets,
                                      size_t textSize) {
        if (position.line >= lineOffsets.size())
            return textSize;
        return std::min(textSize, lineOffsets[position.line] +
                                      static_cast<size_t>(position.column));
    };

    const size_t tokenCount =
        std::min(sourceTokens.size(), outputTokens.size());
    tokenAnchors_.reserve(tokenCount);
    size_t sourceByteOffset = 0;
    qsizetype sourceUtf16Offset = 0;
    size_t outputByteOffset = 0;
    qsizetype outputUtf16Offset = 0;
    for (size_t i = 0; i < tokenCount; ++i) {
        const size_t sourceBegin = sourceOffset(sourceTokens[i].begin);
        const size_t sourceEnd = sourceOffset(sourceTokens[i].end);
        const size_t outputBegin =
            offsetForPosition(outputTokens[i].begin, outputLineOffsets,
                              static_cast<size_t>(output_.size()));
        const size_t outputEnd =
            offsetForPosition(outputTokens[i].end, outputLineOffsets,
                              static_cast<size_t>(output_.size()));
        tokenAnchors_.push_back({
            advanceUtf16Offset(source_, sourceBegin, sourceByteOffset,
                               sourceUtf16Offset),
            advanceUtf16Offset(source_, sourceEnd, sourceByteOffset,
                               sourceUtf16Offset),
            advanceUtf16Offset(output_, outputBegin, outputByteOffset,
                               outputUtf16Offset),
            advanceUtf16Offset(output_, outputEnd, outputByteOffset,
                               outputUtf16Offset),
        });
    }
}

qsizetype LuauFormat::mapCursorPosition(qsizetype sourcePosition) const {
    const TokenAnchor *previousToken = nullptr;
    for (const TokenAnchor &anchor : tokenAnchors_) {
        if (sourcePosition >= anchor.sourceBegin &&
            sourcePosition < anchor.sourceEnd) {
            const qsizetype relativePosition =
                sourcePosition - anchor.sourceBegin;
            return anchor.outputBegin +
                   std::min(relativePosition,
                            anchor.outputEnd - anchor.outputBegin);
        }

        if (sourcePosition < anchor.sourceBegin)
            break;
        if (sourcePosition >= anchor.sourceEnd)
            previousToken = &anchor;
    }

    if (previousToken)
        return previousToken->outputEnd;
    if (!tokenAnchors_.empty())
        return tokenAnchors_.front().outputBegin;
    return 0;
}

template <typename T>
T *LuauFormat::getCstNode(Luau::AstNode *node) const {
    Luau::CstNode *const *cstPtr = cstNodeMap_.find(node);
    if (cstPtr && *cstPtr) {
        return (*cstPtr)->as<T>();
    }
    return nullptr;
}

bool LuauFormat::hasCstNode(Luau::AstNode *node) const {
    Luau::CstNode *const *cstPtr = cstNodeMap_.find(node);
    return cstPtr != nullptr && *cstPtr != nullptr;
}

// ============ Entry point ============

std::optional<LuauFormat::FormatResult>
LuauFormat::format(const QString &source, const FormatOptions &options) {
    return formatWithCursor(source, -1, -1, options);
}

std::optional<LuauFormat::FormatResult>
LuauFormat::formatWithCursor(const QString &source, qsizetype cursorAnchor,
                             qsizetype cursorPosition,
                             const FormatOptions &options) {
    const QByteArray sourceUtf8 = source.toUtf8();

    Luau::Allocator allocator;
    Luau::AstNameTable names(allocator);

    Luau::ParseOptions parseOptions;
    parseOptions.storeCstData = true;
    parseOptions.captureComments = true;
    parseOptions.allowDeclarationSyntax = true;

    Luau::ParseResult parseResult =
        Luau::Parser::parse(sourceUtf8.constData(), sourceUtf8.size(), names,
                            allocator, parseOptions);

    if (!parseResult.errors.empty()) {
        for (const auto &e : parseResult.errors) {
            const Luau::Position &position = e.getLocation().begin;
            qCritical("[LuauFormat] (%d:%d): %s", position.line + 1,
                      position.column + 1, e.what());
        }
        return std::nullopt;
    }

    if (!parseResult.root) {
        qCritical("[LuauFormat] Internal error: parser returned null root");
        return std::nullopt;
    }

    LuauFormat formatter(options);
    formatter.source_ = sourceUtf8;
    formatter.comments_ = std::move(parseResult.commentLocations);
    formatter.sourceLineOffsets_.push_back(0);
    for (size_t i = 0; i < sourceUtf8.size(); ++i) {
        if (sourceUtf8[i] == '\n') {
            formatter.sourceLineOffsets_.push_back(i + 1);
        }
    }
    formatter.cstNodeMap_ = parseResult.cstNodeMap;
    auto r = formatter.format(parseResult.root, parseResult.cstNodeMap);
    if (!r) {
        return std::nullopt;
    }

    FormatResult result;
    result.formatted = r.value();

    bool processAchor = cursorAnchor >= 0;
    bool processPos = cursorPosition >= 0;

    if (processAchor || processPos) {
        formatter.tokenAnchors_.clear();
        formatter.buildTokenAnchors(names);
        const auto srcLen = source.size();
        if (processAchor) {
            const qsizetype boundedCursorAnchor =
                std::clamp(cursorAnchor, qsizetype(0), srcLen);
            result.cursorAnchor =
                formatter.mapCursorPosition(boundedCursorAnchor);
        }
        if (processPos) {
            const qsizetype boundedCursorPosition =
                std::clamp(cursorPosition, qsizetype(0), srcLen);
            result.cursorPosition =
                formatter.mapCursorPosition(boundedCursorPosition);
        }
    }
    return result;
}

std::optional<QString> LuauFormat::format(Luau::AstStatBlock *root,
                                          const Luau::CstNodeMap &cstNodeMap) {
    cstNodeMap_ = cstNodeMap;

    output_.clear();
    atLineStart_ = true;
    indentLevel_ = 0;
    nextComment_ = 0;
    hasFmtError_ = false;
    lastSourceLine_ = 0;
    hasSourceLine_ = false;

    formatBlock(root);
    if (hasFmtError_) {
        return std::nullopt;
    }
    emitRemainingComments();

    // strip trailing newlines, then add exactly one
    while (output_.endsWith(QByteArrayLiteral("\n"))) {
        output_.chop(1);
        if (output_.endsWith(QByteArrayLiteral("\r"))) {
            output_.chop(1);
        }
    }
    output_ += '\n';
    return QString::fromUtf8(output_);
}

// ============ Blocks / Statements ============

void LuauFormat::formatBlock(Luau::AstStatBlock *block) {
    for (size_t i = 0; i < block->body.size; i++) {
        Luau::AstStat *stat = block->body.data[i];
        const size_t commentsBefore = nextComment_;
        const bool isFirstStatement = i == 0;
        const bool removeLeadingGap =
            isFirstStatement && !options_.preserveBlockNewlineGaps;
        const bool preserveLeadingGap =
            isFirstStatement && options_.preserveBlockNewlineGaps;
        emitCommentsBefore(stat->location.begin, removeLeadingGap,
                           preserveLeadingGap);
        prepareSourceLine(stat->location.begin.line, i > 0,
                          removeLeadingGap && commentsBefore == nextComment_,
                          preserveLeadingGap);
        lastSourceLine_ = std::max(lastSourceLine_, stat->location.begin.line);
        hasSourceLine_ = true;
        formatStat(stat);
        lastSourceLine_ = std::max(lastSourceLine_, stat->location.end.line);
    }
}

void LuauFormat::emitCommentsBefore(const Luau::Position &position,
                                    bool suppressLeadingGap,
                                    bool preserveLeadingGap) {
    bool firstComment = true;
    while (nextComment_ < comments_.size() &&
           comments_[nextComment_].location.begin < position) {
        const Luau::Comment &comment = comments_[nextComment_++];
        prepareCommentLine(comment.location.begin.line,
                           suppressLeadingGap && firstComment,
                           preserveLeadingGap && firstComment);
        writeComment(comment, false);
        lastSourceLine_ = comment.location.end.line;
        hasSourceLine_ = true;
        firstComment = false;
    }
}

void LuauFormat::emitTrailingComments(unsigned int line) {
    while (nextComment_ < comments_.size() &&
           comments_[nextComment_].location.begin.line == line) {
        const Luau::Comment &comment = comments_[nextComment_++];
        writeComment(comment, true);
        lastSourceLine_ = comment.location.end.line;
        hasSourceLine_ = true;
    }
}

void LuauFormat::emitRemainingComments() {
    while (nextComment_ < comments_.size()) {
        const Luau::Comment &comment = comments_[nextComment_++];
        prepareCommentLine(comment.location.begin.line);
        writeComment(comment, false);
        lastSourceLine_ = comment.location.end.line;
        hasSourceLine_ = true;
    }
}

void LuauFormat::writeComment(const Luau::Comment &comment,
                              bool inlineComment) {
    const QByteArray text = sourceText(comment.location);

    if (inlineComment && !output_.isEmpty() && !atLineStart_)
        output_.append(' ');
    else {
        while (!output_.isEmpty() &&
               (output_.back() == ' ' || output_.back() == '\t'))
            output_.chop(1);
        writeIndent();
    }
    output_ += text;
    atLineStart_ = false;
    newline();
}

void LuauFormat::prepareSourceLine(unsigned int line, bool forceNewLine,
                                   bool suppressLeadingGap, bool preserveGap) {
    if (!hasSourceLine_)
        return;

    const unsigned int lineGap = !suppressLeadingGap && line > lastSourceLine_
                                     ? line - lastSourceLine_
                                     : 0;
    unsigned int newlines = std::max(
        forceNewLine ? 1u : 0u, preserveGap ? lineGap : std::min(lineGap, 2u));
    if (atLineStart_ && newlines > 0)
        --newlines;
    while (newlines-- > 0)
        newline();
    hasSourceLine_ = true;
}

void LuauFormat::prepareCommentLine(unsigned int line, bool suppressLeadingGap,
                                    bool preserveGap) {
    const unsigned int lineGap =
        !suppressLeadingGap && hasSourceLine_ && line > lastSourceLine_
            ? line - lastSourceLine_
            : 0;
    unsigned int newlines =
        std::max(1u, preserveGap ? lineGap : std::min(lineGap, 2u));
    if (atLineStart_)
        --newlines;
    while (newlines-- > 0)
        newline();
}

void LuauFormat::prepareBlockCloseLine(unsigned int line) {
    if (options_.preserveBlockNewlineGaps)
        prepareSourceLine(line, false, false, true);
}

void LuauFormat::formatStat(Luau::AstStat *stat) {
    if (auto s = stat->as<Luau::AstStatIf>())
        formatIf(s);
    else if (auto s = stat->as<Luau::AstStatWhile>())
        formatWhile(s);
    else if (auto s = stat->as<Luau::AstStatRepeat>())
        formatRepeat(s);
    else if (auto s = stat->as<Luau::AstStatFor>())
        formatFor(s);
    else if (auto s = stat->as<Luau::AstStatForIn>())
        formatForIn(s);
    else if (auto s = stat->as<Luau::AstStatLocal>())
        formatLocal(s);
    else if (auto s = stat->as<Luau::AstStatAssign>())
        formatAssign(s);
    else if (auto s = stat->as<Luau::AstStatCompoundAssign>())
        formatCompoundAssign(s);
    else if (auto s = stat->as<Luau::AstStatReturn>())
        formatReturn(s);
    else if (stat->is<Luau::AstStatBreak>())
        formatBreak();
    else if (stat->is<Luau::AstStatContinue>())
        formatContinue();
    else if (auto s = stat->as<Luau::AstStatFunction>())
        formatFunction(s);
    else if (auto s = stat->as<Luau::AstStatLocalFunction>())
        formatLocalFunction(s);
    else if (auto s = stat->as<Luau::AstStatExpr>())
        formatExprStat(s);
    else if (auto s = stat->as<Luau::AstStatTypeAlias>())
        formatTypeAlias(s);
    else if (auto s = stat->as<Luau::AstStatTypeFunction>())
        formatTypeFunction(s);
    else if (auto s = stat->as<Luau::AstStatDeclareGlobal>())
        formatDeclareGlobal(s);
    else if (auto s = stat->as<Luau::AstStatDeclareFunction>())
        formatDeclareFunction(s);
    else if (auto s = stat->as<Luau::AstStatBlock>())
        formatDo(s);
    else {
        formatPanic(stat);
    }

    if (stat->hasSemicolon) {
        writeSymbol(QByteArrayLiteral(";"));
    }
    emitTrailingComments(stat->location.end.line);
}

void LuauFormat::formatPanic(Luau::AstNode *stat) {
    hasFmtError_ = true;
    auto &loc = stat->location.begin;
    auto line = loc.line + 1;
    auto end = loc.column + 1;
    const char *content = "???";
    if (auto s = stat->asStat()) {
        content = "statement";
    } else if (auto expr = stat->asExpr()) {
        content = "expression";
    } else if (auto attr = stat->asAttr()) {
        content = "attribute";
    } else if (auto t = stat->asType()) {
        content = "type";
    }
    qCritical("[LuauFormat] (%d, %d) Unsupported Luau %s", line, end, content);
}

void LuauFormat::formatDo(Luau::AstStatBlock *block) {
    writeKeyword(QByteArrayLiteral("do"));
    newline();
    increaseIndent();
    formatBlock(block);
    decreaseIndent();
    emitCommentsBefore(block->location.end);
    prepareBlockCloseLine(block->location.end.line);
    newline();
    writeIndent();
    writeKeyword(QByteArrayLiteral("end"));
}

void LuauFormat::formatIf(Luau::AstStatIf *stmt) {
    formatIfChain(stmt);
    emitCommentsBefore(stmt->location.end);
    prepareBlockCloseLine(stmt->location.end.line);
    if (!atLineStart_)
        newline();
    writeKeyword(QByteArrayLiteral("end"));
}

void LuauFormat::formatIfChain(Luau::AstStatIf *stmt) {
    writeKeyword(QByteArrayLiteral("if"));
    space();
    formatExpr(stmt->condition);
    space();
    writeKeyword(QByteArrayLiteral("then"));
    newline();
    increaseIndent();
    formatBlock(stmt->thenbody);
    decreaseIndent();

    if (stmt->elsebody) {
        emitCommentsBefore(stmt->elsebody->location.begin);
        lastSourceLine_ =
            std::max(lastSourceLine_, stmt->elsebody->location.begin.line);
        hasSourceLine_ = true;
        if (!atLineStart_)
            newline();

        if (auto elseifStmt = stmt->elsebody->as<Luau::AstStatIf>()) {
            writeKeyword(QByteArrayLiteral("elseif"));
            space();
            formatIfChainInner(elseifStmt);
            return;
        } else {
            writeKeyword(QByteArrayLiteral("else"));
            newline();
            increaseIndent();
            if (auto elseBlock = stmt->elsebody->as<Luau::AstStatBlock>())
                formatBlock(elseBlock);
            else
                formatStat(stmt->elsebody);
            decreaseIndent();
        }
    }
}

void LuauFormat::formatIfChainInner(Luau::AstStatIf *stmt) {
    // We already wrote "elseif", now write condition then body
    formatExpr(stmt->condition);
    space();
    writeKeyword(QByteArrayLiteral("then"));
    newline();
    increaseIndent();
    formatBlock(stmt->thenbody);
    decreaseIndent();

    if (stmt->elsebody) {
        emitCommentsBefore(stmt->elsebody->location.begin);
        lastSourceLine_ =
            std::max(lastSourceLine_, stmt->elsebody->location.begin.line);
        hasSourceLine_ = true;
        if (!atLineStart_)
            newline();

        if (auto elseifStmt = stmt->elsebody->as<Luau::AstStatIf>()) {
            writeKeyword(QByteArrayLiteral("elseif"));
            space();
            formatIfChainInner(elseifStmt);
            return;
        } else {
            writeKeyword(QByteArrayLiteral("else"));
            newline();
            increaseIndent();
            if (auto elseBlock = stmt->elsebody->as<Luau::AstStatBlock>())
                formatBlock(elseBlock);
            else
                formatStat(stmt->elsebody);
            decreaseIndent();
        }
    }
}

void LuauFormat::formatWhile(Luau::AstStatWhile *stmt) {
    writeKeyword(QByteArrayLiteral("while"));
    space();
    formatExpr(stmt->condition);
    space();
    writeKeyword(QByteArrayLiteral("do"));
    newline();
    increaseIndent();
    formatBlock(stmt->body);
    decreaseIndent();
    emitCommentsBefore(stmt->location.end);
    prepareBlockCloseLine(stmt->location.end.line);
    newline();
    writeIndent();
    writeKeyword(QByteArrayLiteral("end"));
}

void LuauFormat::formatRepeat(Luau::AstStatRepeat *stmt) {
    writeKeyword(QByteArrayLiteral("repeat"));
    newline();
    increaseIndent();
    formatBlock(stmt->body);
    decreaseIndent();
    emitCommentsBefore(stmt->location.end);
    prepareBlockCloseLine(stmt->location.end.line);
    newline();
    writeIndent();
    writeKeyword(QByteArrayLiteral("until"));
    space();
    formatExpr(stmt->condition);
}

void LuauFormat::formatFor(Luau::AstStatFor *stmt) {
    writeKeyword(QByteArrayLiteral("for"));
    space();
    writeIdentifier(stmt->var->name.value);
    if (stmt->var->annotation) {
        writeSymbol(QByteArrayLiteral(":"));
        space();
        formatType(stmt->var->annotation);
    }
    space();
    writeSymbol(QByteArrayLiteral("="));
    space();
    formatExpr(stmt->from);
    writeSymbol(QByteArrayLiteral(","));
    space();
    formatExpr(stmt->to);
    if (stmt->step) {
        writeSymbol(QByteArrayLiteral(","));
        space();
        formatExpr(stmt->step);
    }
    space();
    writeKeyword(QByteArrayLiteral("do"));
    newline();
    increaseIndent();
    formatBlock(stmt->body);
    decreaseIndent();
    emitCommentsBefore(stmt->location.end);
    prepareBlockCloseLine(stmt->location.end.line);
    newline();
    writeIndent();
    writeKeyword(QByteArrayLiteral("end"));
}

void LuauFormat::formatForIn(Luau::AstStatForIn *stmt) {
    writeKeyword(QByteArrayLiteral("for"));
    space();
    formatNameList(stmt->vars);
    space();
    writeKeyword(QByteArrayLiteral("in"));
    space();
    formatExprList(stmt->values);
    space();
    writeKeyword(QByteArrayLiteral("do"));
    newline();
    increaseIndent();
    formatBlock(stmt->body);
    decreaseIndent();
    emitCommentsBefore(stmt->location.end);
    prepareBlockCloseLine(stmt->location.end.line);
    newline();
    writeIndent();
    writeKeyword(QByteArrayLiteral("end"));
}

void LuauFormat::formatLocal(Luau::AstStatLocal *stmt) {
    if (stmt->isExported)
        writeKeyword(QByteArrayLiteral("export "));
    if (stmt->isConst)
        writeKeyword(QByteArrayLiteral("const "));
    else
        writeKeyword(QByteArrayLiteral("local "));

    formatNameList(stmt->vars);

    if (stmt->values.size > 0) {
        space();
        writeSymbol(QByteArrayLiteral("="));
        space();
        formatExprList(stmt->values);
    }
}

void LuauFormat::formatAssign(Luau::AstStatAssign *stmt) {
    formatExprList(stmt->vars);
    space();
    writeSymbol(QByteArrayLiteral("="));
    space();
    formatExprList(stmt->values);
}

void LuauFormat::formatCompoundAssign(Luau::AstStatCompoundAssign *stmt) {
    formatExpr(stmt->var);
    space();
    const char *op = nullptr;
    using Op = Luau::AstExprBinary::Op;
    switch (stmt->op) {
    case Op::Add:
        op = "+=";
        break;
    case Op::Sub:
        op = "-=";
        break;
    case Op::Mul:
        op = "*=";
        break;
    case Op::Div:
        op = "/=";
        break;
    case Op::FloorDiv:
        op = "//=";
        break;
    case Op::Mod:
        op = "%=";
        break;
    case Op::Pow:
        op = "^=";
        break;
    case Op::Concat:
        op = "..=";
        break;
    default:
        op = "=";
        break;
    }
    writeSymbol(op);
    space();
    formatExpr(stmt->value);
}

void LuauFormat::formatReturn(Luau::AstStatReturn *stmt) {
    writeKeyword(QByteArrayLiteral("return"));
    if (stmt->list.size > 0) {
        space();
        formatExprList(stmt->list);
    }
}

void LuauFormat::formatBreak() { writeKeyword(QByteArrayLiteral("break")); }

void LuauFormat::formatContinue() {
    writeKeyword(QByteArrayLiteral("continue"));
}

void LuauFormat::formatFunction(Luau::AstStatFunction *stmt) {
    formatAttributes(stmt->func->attributes);
    writeKeyword(QByteArrayLiteral("function"));
    space();
    formatExpr(stmt->name);
    formatFunctionBody(stmt->func);
}

void LuauFormat::formatLocalFunction(Luau::AstStatLocalFunction *stmt) {
    formatAttributes(stmt->func->attributes);
    if (stmt->name->isExported)
        writeKeyword(QByteArrayLiteral("export "));
    else if (stmt->name->isConst)
        writeKeyword(QByteArrayLiteral("const "));
    else
        writeKeyword(QByteArrayLiteral("local "));
    writeKeyword(QByteArrayLiteral("function"));
    space();
    writeIdentifier(stmt->name->name.value);
    formatFunctionBody(stmt->func);
}

void LuauFormat::formatExprStat(Luau::AstStatExpr *stmt) {
    formatExpr(stmt->expr);
}

void LuauFormat::formatTypeAlias(Luau::AstStatTypeAlias *stmt) {
    if (stmt->exported)
        writeKeyword(QByteArrayLiteral("export "));
    writeKeyword(QByteArrayLiteral("type"));
    space();
    writeIdentifier(stmt->name.value);

    if (stmt->generics.size > 0 || stmt->genericPacks.size > 0)
        formatGenericTypeList(stmt->generics, stmt->genericPacks);

    space();
    writeSymbol(QByteArrayLiteral("="));
    space();
    formatType(stmt->type);
}

void LuauFormat::formatTypeFunction(Luau::AstStatTypeFunction *stmt) {
    if (stmt->exported)
        writeKeyword(QByteArrayLiteral("export "));
    writeKeyword(QByteArrayLiteral("type"));
    space();
    writeKeyword(QByteArrayLiteral("function"));
    space();
    writeIdentifier(stmt->name.value);
    formatFunctionBody(stmt->body);
}

void LuauFormat::formatDeclareGlobal(Luau::AstStatDeclareGlobal *stmt) {
    writeKeyword(QByteArrayLiteral("declare"));
    space();
    writeIdentifier(stmt->name.value);
    writeSymbol(QByteArrayLiteral(":"));
    space();
    formatType(stmt->type);
}

void LuauFormat::formatDeclareFunction(Luau::AstStatDeclareFunction *stmt) {
    formatAttributes(stmt->attributes);
    writeKeyword(QByteArrayLiteral("declare"));
    space();
    writeKeyword(QByteArrayLiteral("function"));
    space();
    writeIdentifier(stmt->name.value);
    if (stmt->generics.size > 0 || stmt->genericPacks.size > 0)
        formatGenericTypeList(stmt->generics, stmt->genericPacks);

    writeSymbol(QByteArrayLiteral("("));
    for (size_t i = 0; i < stmt->params.types.size; i++) {
        if (i > 0) {
            writeSymbol(QByteArrayLiteral(","));
            space();
        }
        if (i < stmt->paramNames.size) {
            writeIdentifier(stmt->paramNames.data[i].first.value);
            writeSymbol(QByteArrayLiteral(":"));
            space();
        }
        formatType(stmt->params.types.data[i]);
    }
    if (stmt->params.tailType) {
        if (stmt->params.types.size > 0) {
            writeSymbol(QByteArrayLiteral(","));
            space();
        }
        formatTypePack(stmt->params.tailType);
    } else if (stmt->vararg) {
        if (stmt->params.types.size > 0) {
            writeSymbol(QByteArrayLiteral(","));
            space();
        }
        writeSymbol(QByteArrayLiteral("..."));
    }
    writeSymbol(QByteArrayLiteral(")"));
    if (stmt->retTypes) {
        writeSymbol(QByteArrayLiteral(":"));
        space();
        formatTypePack(stmt->retTypes);
    }
}

// ============ Expressions ============

void LuauFormat::formatExpr(Luau::AstExpr *expr) {
    if (!expr)
        return;

    if (expr->is<Luau::AstExprGroup>())
        formatGroup(static_cast<Luau::AstExprGroup *>(expr));
    else if (expr->is<Luau::AstExprConstantNil>())
        formatConstantNil();
    else if (auto e = expr->as<Luau::AstExprConstantBool>())
        formatConstantBool(e->value);
    else if (auto e = expr->as<Luau::AstExprConstantNumber>())
        formatConstantNumberWithCst(e);
    else if (auto e = expr->as<Luau::AstExprConstantInteger>())
        formatConstantInteger(e);
    else if (auto e = expr->as<Luau::AstExprConstantString>())
        formatConstantString(e);
    else if (auto e = expr->as<Luau::AstExprLocal>())
        formatLocal(e);
    else if (auto e = expr->as<Luau::AstExprGlobal>())
        formatGlobal(e);
    else if (expr->is<Luau::AstExprVarargs>())
        formatVarargs();
    else if (auto e = expr->as<Luau::AstExprCall>())
        formatCall(e);
    else if (auto e = expr->as<Luau::AstExprIndexName>())
        formatIndexName(e);
    else if (auto e = expr->as<Luau::AstExprIndexExpr>())
        formatIndexExpr(e);
    else if (auto e = expr->as<Luau::AstExprFunction>())
        formatFunction(e);
    else if (auto e = expr->as<Luau::AstExprTable>())
        formatTable(e);
    else if (auto e = expr->as<Luau::AstExprUnary>())
        formatUnary(e);
    else if (auto e = expr->as<Luau::AstExprBinary>())
        formatBinary(e);
    else if (auto e = expr->as<Luau::AstExprTypeAssertion>())
        formatTypeAssertion(e);
    else if (auto e = expr->as<Luau::AstExprIfElse>())
        formatIfElse(e);
    else if (auto e = expr->as<Luau::AstExprInterpString>())
        formatInterpString(e);
    else {
        formatPanic(expr);
    }
}

void LuauFormat::formatGroup(Luau::AstExprGroup *expr) {
    writeSymbol(QByteArrayLiteral("("));
    formatExpr(expr->expr);
    writeSymbol(QByteArrayLiteral(")"));
}

void LuauFormat::formatConstantNil() { writeKeyword(QByteArrayLiteral("nil")); }

void LuauFormat::formatConstantBool(bool value) {
    writeKeyword(value ? QByteArrayLiteral("true")
                       : QByteArrayLiteral("false"));
}

void LuauFormat::formatConstantNumber(double value) {
    if (value == (long long)value && value > -1e15 && value < 1e15)
        writeLiteral(QByteArray::number(static_cast<qlonglong>(value)));
    else
        writeLiteral(QByteArray::number(value, 'g', 17));
}

void LuauFormat::formatConstantNumberWithCst(
    Luau::AstExprConstantNumber *expr) {
    auto cst = getCstNode<Luau::CstExprConstantNumber>(expr);
    if (cst) {
        // Use the original source representation
        writeLiteral(QByteArray(cst->value.data, cst->value.size));
    } else {
        formatConstantNumber(expr->value);
    }
}

void LuauFormat::formatConstantInteger(Luau::AstExprConstantInteger *expr) {
    auto cst = getCstNode<Luau::CstExprConstantInteger>(expr);
    if (cst) {
        writeLiteral(QByteArray(cst->value.data, cst->value.size));
    } else {
        writeLiteral(QByteArray::number(expr->value));
    }
}

void LuauFormat::formatConstantString(Luau::AstExprConstantString *expr) {
    auto cst = getCstNode<Luau::CstExprConstantString>(expr);
    if (cst && options_.quoteStyle == QuoteStyle::Preserve)
        write(sourceText(expr->location));
    else {
        const QByteArray valueBytes(expr->value.data,
                                    static_cast<qsizetype>(expr->value.size));
        const QString value = QString::fromUtf8(valueBytes);
        if (value.toUtf8() != valueBytes) {
            write(sourceText(expr->location));
            return;
        }
        const auto originalStyle =
            cst ? cst->quoteStyle
                : Luau::CstExprConstantString::QuoteStyle::QuotedDouble;
        const unsigned int blockDepth = cst ? cst->blockDepth : 0;
        writeString(value, originalStyle, blockDepth);
    }
}

void LuauFormat::formatLocal(Luau::AstExprLocal *expr) {
    writeIdentifier(expr->local->name.value);
}

void LuauFormat::formatGlobal(Luau::AstExprGlobal *expr) {
    writeIdentifier(expr->name.value);
}

void LuauFormat::formatVarargs() { writeSymbol(QByteArrayLiteral("...")); }

void LuauFormat::formatUnary(Luau::AstExprUnary *expr) {
    using Op = Luau::AstExprUnary::Op;
    switch (expr->op) {
    case Op::Not:
        writeKeyword(QByteArrayLiteral("not"));
        space();
        break;
    case Op::Minus:
        writeSymbol(QByteArrayLiteral("-"));
        break;
    case Op::Len:
        writeSymbol(QByteArrayLiteral("#"));
        break;
    }
    formatExpr(expr->expr);
}

void LuauFormat::formatBinary(Luau::AstExprBinary *expr) {
    formatExpr(expr->left);
    space();

    using Op = Luau::AstExprBinary::Op;
    QByteArray opStr;
    bool wordOp = false;
    switch (expr->op) {
    case Op::Add:
        opStr = QByteArrayLiteral("+");
        break;
    case Op::Sub:
        opStr = QByteArrayLiteral("-");
        break;
    case Op::Mul:
        opStr = QByteArrayLiteral("*");
        break;
    case Op::Div:
        opStr = QByteArrayLiteral("/");
        break;
    case Op::FloorDiv:
        opStr = QByteArrayLiteral("//");
        break;
    case Op::Mod:
        opStr = QByteArrayLiteral("%");
        break;
    case Op::Pow:
        opStr = QByteArrayLiteral("^");
        break;
    case Op::Concat:
        opStr = QByteArrayLiteral("..");
        break;
    case Op::CompareNe:
        opStr = QByteArrayLiteral("~=");
        break;
    case Op::CompareEq:
        opStr = QByteArrayLiteral("==");
        break;
    case Op::CompareLt:
        opStr = QByteArrayLiteral("<");
        break;
    case Op::CompareLe:
        opStr = QByteArrayLiteral("<=");
        break;
    case Op::CompareGt:
        opStr = QByteArrayLiteral(">");
        break;
    case Op::CompareGe:
        opStr = QByteArrayLiteral(">=");
        break;
    case Op::And:
        opStr = QByteArrayLiteral("and");
        wordOp = true;
        break;
    case Op::Or:
        opStr = QByteArrayLiteral("or");
        wordOp = true;
        break;
    default:
        opStr = QByteArrayLiteral("?");
        break;
    }

    if (wordOp)
        writeKeyword(opStr);
    else
        writeSymbol(opStr);

    space();
    formatExpr(expr->right);
}

void LuauFormat::formatCall(Luau::AstExprCall *expr) {
    formatExpr(expr->func);
    writeSymbol(QByteArrayLiteral("("));

    for (size_t i = 0; i < expr->args.size; i++) {
        if (i > 0) {
            writeSymbol(QByteArrayLiteral(","));
            space();
        }
        formatExpr(expr->args.data[i]);
    }

    writeSymbol(QByteArrayLiteral(")"));
}

void LuauFormat::formatIndexName(Luau::AstExprIndexName *expr) {
    formatExpr(expr->expr);
    write(QByteArray(1, expr->op));
    writeIdentifier(expr->index.value);
}

void LuauFormat::formatIndexExpr(Luau::AstExprIndexExpr *expr) {
    formatExpr(expr->expr);
    writeSymbol(QByteArrayLiteral("["));
    formatExpr(expr->index);
    writeSymbol(QByteArrayLiteral("]"));
}

void LuauFormat::formatFunction(Luau::AstExprFunction *expr) {
    formatAttributes(expr->attributes);
    writeKeyword(QByteArrayLiteral("function"));
    formatFunctionBody(expr);
}

void LuauFormat::formatFunctionBody(Luau::AstExprFunction *func) {
    formatFunctionSignature(func);

    // body
    newline();
    increaseIndent();
    formatBlock(func->body);
    decreaseIndent();
    emitCommentsBefore(func->location.end);
    prepareBlockCloseLine(func->location.end.line);
    newline();
    writeIndent();
    writeKeyword(QByteArrayLiteral("end"));
}

void LuauFormat::formatFunctionSignature(Luau::AstExprFunction *func) {
    // generics
    if (func->generics.size > 0 || func->genericPacks.size > 0)
        formatGenericTypeList(func->generics, func->genericPacks);

    // args
    writeSymbol(QByteArrayLiteral("("));
    bool first = true;
    for (size_t i = 0; i < func->args.size; i++) {
        if (!first) {
            writeSymbol(QByteArrayLiteral(","));
            space();
        }
        first = false;
        writeIdentifier(func->args.data[i]->name.value);
        if (func->args.data[i]->annotation) {
            writeSymbol(QByteArrayLiteral(":"));
            space();
            formatType(func->args.data[i]->annotation);
        }
    }
    if (func->vararg) {
        if (!first) {
            writeSymbol(QByteArrayLiteral(","));
            space();
        }
        writeSymbol(QByteArrayLiteral("..."));
    }
    writeSymbol(QByteArrayLiteral(")"));

    // return annotation
    if (func->returnAnnotation) {
        writeSymbol(QByteArrayLiteral(":"));
        space();
        formatTypePack(func->returnAnnotation);
    }
}

void LuauFormat::formatTable(Luau::AstExprTable *expr) {
    if (expr->items.size == 0) {
        writeSymbol(QByteArrayLiteral("{}"));
        return;
    }

    // check if all items are simple list values (no keys) that could fit on one
    // line
    bool allSimpleList = true;
    for (size_t i = 0; i < expr->items.size; i++) {
        auto &item = expr->items.data[i];
        if (item.kind != Luau::AstExprTable::Item::Kind::List) {
            allSimpleList = false;
            break;
        }
    }

    if (allSimpleList) {
        writeSymbol(QByteArrayLiteral("{"));
        space();
        for (size_t i = 0; i < expr->items.size; i++) {
            if (i > 0) {
                writeSymbol(QByteArrayLiteral(","));
                space();
            }
            formatExpr(expr->items.data[i].value);
        }
        space();
        writeSymbol(QByteArrayLiteral("}"));
        return;
    }

    // Multi-line format for tables with keys or complex values
    writeSymbol(QByteArrayLiteral("{"));
    increaseIndent();
    newline();
    for (size_t i = 0; i < expr->items.size; i++) {
        if (i > 0) {
            writeSymbol(QByteArrayLiteral(","));
            newline();
        }
        auto &item = expr->items.data[i];
        switch (item.kind) {
        case Luau::AstExprTable::Item::Kind::List:
            formatExpr(item.value);
            break;
        case Luau::AstExprTable::Item::Kind::Record: {
            auto key = item.key->as<Luau::AstExprConstantString>();
            writeIdentifier(QByteArray(
                key->value.data, static_cast<qsizetype>(key->value.size)));
            space();
            writeSymbol(QByteArrayLiteral("="));
            space();
            formatExpr(item.value);
            break;
        }
        case Luau::AstExprTable::Item::Kind::General:
            writeSymbol(QByteArrayLiteral("["));
            formatExpr(item.key);
            writeSymbol(QByteArrayLiteral("]"));
            space();
            writeSymbol(QByteArrayLiteral("="));
            space();
            formatExpr(item.value);
            break;
        }
    }
    decreaseIndent();
    newline();
    writeIndent();
    writeSymbol(QByteArrayLiteral("}"));
}

void LuauFormat::formatIfElse(Luau::AstExprIfElse *expr) {
    writeKeyword(QByteArrayLiteral("if"));
    space();
    formatExpr(expr->condition);
    space();
    writeKeyword(QByteArrayLiteral("then"));
    space();
    formatExpr(expr->trueExpr);
    space();
    writeKeyword(QByteArrayLiteral("else"));
    space();
    formatExpr(expr->falseExpr);
}

void LuauFormat::formatInterpString(Luau::AstExprInterpString *expr) {
    write(sourceText(expr->location));
}

void LuauFormat::formatTypeAssertion(Luau::AstExprTypeAssertion *expr) {
    formatExpr(expr->expr);
    space();
    writeSymbol(QByteArrayLiteral("::"));
    space();
    formatType(expr->annotation);
}

// ============ Types ============

void LuauFormat::formatType(Luau::AstType *type) {
    if (!type)
        return;

    if (auto t = type->as<Luau::AstTypeReference>()) {
        if (t->prefix) {
            writeIdentifier(t->prefix->value);
            writeSymbol(QByteArrayLiteral("."));
        }
        writeIdentifier(t->name.value);
        if (t->parameters.size > 0 || t->hasParameterList) {
            writeSymbol(QByteArrayLiteral("<"));
            for (size_t i = 0; i < t->parameters.size; i++) {
                if (i > 0) {
                    writeSymbol(QByteArrayLiteral(","));
                    space();
                }
                if (t->parameters.data[i].type)
                    formatType(t->parameters.data[i].type);
                else if (t->parameters.data[i].typePack)
                    formatTypePack(t->parameters.data[i].typePack);
            }
            writeSymbol(QByteArrayLiteral(">"));
        }
    } else if (auto t = type->as<Luau::AstTypeSingletonBool>())
        writeKeyword(t->value ? QByteArrayLiteral("true")
                              : QByteArrayLiteral("false"));
    else if (auto t = type->as<Luau::AstTypeSingletonString>()) {
        if (options_.quoteStyle == QuoteStyle::Preserve)
            write(sourceText(t->location));
        else {
            const QByteArray original = sourceText(t->location);
            const QByteArray valueBytes(t->value.data,
                                        static_cast<qsizetype>(t->value.size));
            const QString value = QString::fromUtf8(valueBytes);
            if (value.toUtf8() != valueBytes) {
                write(original);
                return;
            }
            const auto originalStyle =
                !original.isEmpty() && original.front() == '\''
                    ? Luau::CstExprConstantString::QuoteStyle::QuotedSingle
                    : Luau::CstExprConstantString::QuoteStyle::QuotedDouble;
            writeString(value, originalStyle, 0);
        }
    } else if (auto t = type->as<Luau::AstTypeTable>()) {
        writeSymbol(QByteArrayLiteral("{"));
        bool first = true;
        for (auto &prop : t->props) {
            if (!first) {
                writeSymbol(QByteArrayLiteral(","));
                space();
            }
            first = false;
            writeIdentifier(prop.name.value);
            writeSymbol(QByteArrayLiteral(":"));
            space();
            formatType(prop.type);
        }
        if (t->indexer) {
            if (!first) {
                writeSymbol(QByteArrayLiteral(","));
            }
            space();
            writeSymbol(QByteArrayLiteral("["));
            formatType(t->indexer->indexType);
            writeSymbol(QByteArrayLiteral("]"));
            writeSymbol(QByteArrayLiteral(":"));
            space();
            formatType(t->indexer->resultType);
        }
        space();
        writeSymbol(QByteArrayLiteral("}"));
    } else if (auto t = type->as<Luau::AstTypeFunction>()) {
        if (t->generics.size > 0 || t->genericPacks.size > 0)
            formatGenericTypeList(t->generics, t->genericPacks);
        writeSymbol(QByteArrayLiteral("("));
        const auto &argTypes = t->argTypes;
        for (size_t i = 0; i < argTypes.types.size; i++) {
            if (i > 0) {
                writeSymbol(QByteArrayLiteral(","));
                space();
            }
            if (i < t->argNames.size && t->argNames.data[i].has_value()) {
                writeIdentifier(t->argNames.data[i]->first.value);
                writeSymbol(QByteArrayLiteral(":"));
                space();
            }
            formatType(argTypes.types.data[i]);
        }
        if (argTypes.tailType) {
            if (argTypes.types.size > 0) {
                writeSymbol(QByteArrayLiteral(","));
                space();
            }
            writeSymbol(QByteArrayLiteral("..."));
            // formatTypePack would add "..." but we just want the variadic
            // content
        }
        writeSymbol(QByteArrayLiteral(")"));
        space();
        writeSymbol(QByteArrayLiteral("->"));
        space();
        formatTypePack(t->returnTypes);
    } else if (auto t = type->as<Luau::AstTypeGroup>())
        formatType(t->type);
    else if (auto t = type->as<Luau::AstTypeUnion>()) {
        for (size_t i = 0; i < t->types.size; i++) {
            if (i > 0) {
                space();
                writeSymbol(QByteArrayLiteral("|"));
                space();
            }
            formatType(t->types.data[i]);
        }
    } else if (auto t = type->as<Luau::AstTypeIntersection>()) {
        for (size_t i = 0; i < t->types.size; i++) {
            if (i > 0) {
                space();
                writeSymbol(QByteArrayLiteral("&"));
                space();
            }
            formatType(t->types.data[i]);
        }
    } else if (auto t = type->as<Luau::AstTypeTypeof>()) {
        writeKeyword(QByteArrayLiteral("typeof"));
        writeSymbol(QByteArrayLiteral("("));
        formatExpr(t->expr);
        writeSymbol(QByteArrayLiteral(")"));
    } else {
        formatPanic(type);
    }
}

void LuauFormat::formatTypePack(Luau::AstTypePack *typePack) {
    if (!typePack)
        return;

    if (auto p = typePack->as<Luau::AstTypePackVariadic>()) {
        writeSymbol(QByteArrayLiteral("..."));
        formatType(p->variadicType);
    } else if (auto p = typePack->as<Luau::AstTypePackGeneric>()) {
        writeIdentifier(p->genericName.value);
        writeSymbol(QByteArrayLiteral("..."));
    } else if (auto p = typePack->as<Luau::AstTypePackExplicit>()) {
        formatTypeListInternal(p->typeList);
    }
}

void LuauFormat::formatTypeListInternal(const Luau::AstTypeList &typeList) {
    writeSymbol(QByteArrayLiteral("("));
    for (size_t i = 0; i < typeList.types.size; i++) {
        if (i > 0) {
            writeSymbol(QByteArrayLiteral(","));
            space();
        }
        formatType(typeList.types.data[i]);
    }
    if (typeList.tailType) {
        if (typeList.types.size > 0) {
            writeSymbol(QByteArrayLiteral(","));
            space();
        }
        formatTypePack(typeList.tailType);
    }
    writeSymbol(QByteArrayLiteral(")"));
}

// ============ Helpers ============

void LuauFormat::formatNameList(const Luau::AstArray<Luau::AstLocal *> &vars,
                                const Luau::AstArray<Luau::Position> *) {
    for (size_t i = 0; i < vars.size; i++) {
        if (i > 0) {
            writeSymbol(QByteArrayLiteral(","));
            space();
        }
        writeIdentifier(vars.data[i]->name.value);
        if (vars.data[i]->annotation) {
            writeSymbol(QByteArrayLiteral(":"));
            space();
            formatType(vars.data[i]->annotation);
        }
    }
}

void LuauFormat::formatExprList(const Luau::AstArray<Luau::AstExpr *> &exprs,
                                const Luau::AstArray<Luau::Position> *) {
    for (size_t i = 0; i < exprs.size; i++) {
        if (i > 0) {
            writeSymbol(QByteArrayLiteral(","));
            space();
        }
        formatExpr(exprs.data[i]);
    }
}

void LuauFormat::formatGenericTypeList(
    const Luau::AstArray<Luau::AstGenericType *> &generics,
    const Luau::AstArray<Luau::AstGenericTypePack *> &genericPacks,
    const Luau::AstArray<Luau::Position> *) {
    writeSymbol(QByteArrayLiteral("<"));
    bool first = true;
    for (size_t i = 0; i < generics.size; i++) {
        if (!first) {
            writeSymbol(QByteArrayLiteral(","));
            space();
        }
        first = false;
        writeIdentifier(generics.data[i]->name.value);
        if (generics.data[i]->defaultValue) {
            space();
            writeSymbol(QByteArrayLiteral("="));
            space();
            formatType(generics.data[i]->defaultValue);
        }
    }
    for (size_t i = 0; i < genericPacks.size; i++) {
        if (!first) {
            writeSymbol(QByteArrayLiteral(","));
            space();
        }
        first = false;
        writeIdentifier(genericPacks.data[i]->name.value);
        writeSymbol(QByteArrayLiteral("..."));
        if (genericPacks.data[i]->defaultValue) {
            space();
            writeSymbol(QByteArrayLiteral("="));
            space();
            formatTypePack(genericPacks.data[i]->defaultValue);
        }
    }
    writeSymbol(QByteArrayLiteral(">"));
}

void LuauFormat::formatAttributes(
    const Luau::AstArray<Luau::AstAttr *> &attributes) {
    for (size_t i = 0; i < attributes.size; i++) {
        writeSymbol(QByteArrayLiteral("@"));
        writeIdentifier(attributes.data[i]->name.value);
        space();
    }
}
