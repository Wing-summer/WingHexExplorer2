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

#ifndef LUAUFORMATTER_H
#define LUAUFORMATTER_H

#include "Luau/Ast.h"
#include "Luau/Cst.h"
#include "Luau/ParseResult.h"

#include <QString>

#include <vector>

class LuauFormat {
public:
    enum class QuoteStyle : int {
        Preserve,
        PreferDouble,
        PreferSingle,
        ForceDouble,
        ForceSingle
    };

    struct FormatOptions {
        unsigned int indentSize;
        bool useTabs;
        QuoteStyle quoteStyle = QuoteStyle::Preserve;
        bool preserveBlockNewlineGaps = false;

        FormatOptions(unsigned int indentSize = 4, bool useTabs = true)
            : indentSize(indentSize), useTabs(useTabs) {}
    };

    struct FormatResult {
        QString formatted;
        qsizetype cursorAnchor = -1;
        qsizetype cursorPosition = -1;
    };

    static std::optional<FormatResult>
    format(const QString &source,
           const FormatOptions &options = FormatOptions());
    static std::optional<FormatResult>
    formatWithCursor(const QString &source, qsizetype cursorAnchor,
                     qsizetype cursorPosition,
                     const FormatOptions &options = FormatOptions());

private:
    struct TokenAnchor {
        qsizetype sourceBegin;
        qsizetype sourceEnd;
        qsizetype outputBegin;
        qsizetype outputEnd;
    };

    explicit LuauFormat(const FormatOptions &options);
    std::optional<QString> format(Luau::AstStatBlock *root,
                                  const Luau::CstNodeMap &cstNodeMap);
    size_t sourceOffset(const Luau::Position &position) const;
    void buildTokenAnchors(Luau::AstNameTable &names);
    qsizetype mapCursorPosition(qsizetype sourcePosition) const;
    void write(const QByteArray &str);
    void writeIndent();
    void newline();
    void space();
    void writeKeyword(const QByteArray &keyword);
    void writeIdentifier(const QByteArray &identifier);
    void writeSymbol(const QByteArray &symbol);
    void writeLiteral(const QByteArray &literal);
    void writeString(const QString &value,
                     Luau::CstExprConstantString::QuoteStyle quoteStyle,
                     unsigned int blockDepth);
    QByteArray getIndentString() const;
    QByteArray sourceText(const Luau::Location &location) const;
    inline void increaseIndent() { ++indentLevel_; }
    inline void decreaseIndent() {
        if (indentLevel_ > 0) {
            --indentLevel_;
        }
    }

    template <typename T>
    T *getCstNode(Luau::AstNode *node) const;
    bool hasCstNode(Luau::AstNode *node) const;

    void formatBlock(Luau::AstStatBlock *block);
    void formatStat(Luau::AstStat *stat);
    void formatPanic(Luau::AstNode *stat);
    void emitCommentsBefore(const Luau::Position &position,
                            bool suppressLeadingGap = false,
                            bool preserveLeadingGap = false);
    void emitTrailingComments(unsigned int line);
    void emitRemainingComments();
    void writeComment(const Luau::Comment &comment, bool inlineComment);
    void prepareSourceLine(unsigned int line, bool forceNewLine,
                           bool suppressLeadingGap = false,
                           bool preserveGap = false);
    void prepareCommentLine(unsigned int line, bool suppressLeadingGap = false,
                            bool preserveGap = false);
    void prepareBlockCloseLine(unsigned int line);

private:
    void formatDo(Luau::AstStatBlock *block);
    void formatIf(Luau::AstStatIf *stmt);
    void formatIfChain(Luau::AstStatIf *stmt);
    void formatIfChainInner(Luau::AstStatIf *stmt);
    void formatWhile(Luau::AstStatWhile *stmt);
    void formatRepeat(Luau::AstStatRepeat *stmt);
    void formatFor(Luau::AstStatFor *stmt);
    void formatForIn(Luau::AstStatForIn *stmt);
    void formatLocal(Luau::AstStatLocal *stmt);
    void formatAssign(Luau::AstStatAssign *stmt);
    void formatCompoundAssign(Luau::AstStatCompoundAssign *stmt);
    void formatReturn(Luau::AstStatReturn *stmt);
    void formatBreak();
    void formatContinue();
    void formatFunction(Luau::AstStatFunction *stmt);
    void formatLocalFunction(Luau::AstStatLocalFunction *stmt);
    void formatExprStat(Luau::AstStatExpr *stmt);
    void formatTypeAlias(Luau::AstStatTypeAlias *stmt);
    void formatTypeFunction(Luau::AstStatTypeFunction *stmt);
    void formatDeclareGlobal(Luau::AstStatDeclareGlobal *stmt);
    void formatDeclareFunction(Luau::AstStatDeclareFunction *stmt);

    void formatExpr(Luau::AstExpr *expr);
    void formatGroup(Luau::AstExprGroup *expr);
    void formatConstantNil();
    void formatConstantBool(bool value);
    void formatConstantNumber(double value);
    void formatConstantNumberWithCst(Luau::AstExprConstantNumber *expr);
    void formatConstantInteger(Luau::AstExprConstantInteger *expr);
    void formatConstantString(Luau::AstExprConstantString *expr);
    void formatLocal(Luau::AstExprLocal *expr);
    void formatGlobal(Luau::AstExprGlobal *expr);
    void formatVarargs();
    void formatUnary(Luau::AstExprUnary *expr);
    void formatBinary(Luau::AstExprBinary *expr);
    void formatCall(Luau::AstExprCall *expr);
    void formatIndexName(Luau::AstExprIndexName *expr);
    void formatIndexExpr(Luau::AstExprIndexExpr *expr);
    void formatFunction(Luau::AstExprFunction *expr);
    void formatFunctionSignature(Luau::AstExprFunction *func);
    void formatFunctionBody(Luau::AstExprFunction *func);
    void formatTable(Luau::AstExprTable *expr);
    void formatIfElse(Luau::AstExprIfElse *expr);
    void formatInterpString(Luau::AstExprInterpString *expr);
    void formatTypeAssertion(Luau::AstExprTypeAssertion *expr);

    void formatType(Luau::AstType *type);
    void formatTypePack(Luau::AstTypePack *typePack);
    void formatTypeListInternal(const Luau::AstTypeList &typeList);
    void
    formatNameList(const Luau::AstArray<Luau::AstLocal *> &vars,
                   const Luau::AstArray<Luau::Position> *locations = nullptr);
    void
    formatExprList(const Luau::AstArray<Luau::AstExpr *> &exprs,
                   const Luau::AstArray<Luau::Position> *locations = nullptr);
    void formatGenericTypeList(
        const Luau::AstArray<Luau::AstGenericType *> &generics,
        const Luau::AstArray<Luau::AstGenericTypePack *> &genericPacks,
        const Luau::AstArray<Luau::Position> *locations = nullptr);
    void formatAttributes(const Luau::AstArray<Luau::AstAttr *> &attributes);

private:
    FormatOptions options_;
    Luau::CstNodeMap cstNodeMap_;
    QByteArray source_;
    QByteArray output_;
    std::vector<Luau::Comment> comments_;
    std::vector<size_t> sourceLineOffsets_;
    std::vector<TokenAnchor> tokenAnchors_;

    size_t nextComment_ = 0;
    unsigned int lastSourceLine_ = 0;
    unsigned int indentLevel_ = 0;
    bool hasSourceLine_ = false;
    bool hasFmtError_ = false;
    bool atLineStart_ = true;
};

#endif