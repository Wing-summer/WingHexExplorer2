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

#ifndef CODEINFOTIP_H
#define CODEINFOTIP_H

#include "luau/lsp/lsp.h"

#include <QIcon>
#include <QPlainTextEdit>

class CodeInfoTip {
public:
    using SnippetResolver = std::function<QString(const QString &name)>;

public:
    static QIcon getDisplayIcon(lsp::CompletionItemKind kind);

    explicit CodeInfoTip(const lsp::CompletionItem &item);
    explicit CodeInfoTip() = default;

public:
    // UI / model data.
    QString label;
    lsp::CompletionItemKind kind = lsp::CompletionItemKind::Missing;
    QString detail;

    QList<CodeInfoTip> children;

public:
    QString documentation() const;
    QString insertionTextOrLabel() const;
    bool usesSnippet() const;

    void applyEdit(QPlainTextEdit *editor, const QString &completionPrefix,
                   bool hasCodeLineContext) const;

private:
    static QString processSnippt(const QString &name, QPlainTextEdit *editor,
                                 bool hasCodeLineContext);

private:
    QString documentationText;
    QString insertionText;

    lsp::InsertTextFormat insertionFormat = lsp::InsertTextFormat::PlainText;

    std::optional<lsp::Range> textEditRange;
    std::vector<lsp::TextEdit> additionalTextEdits;
};

Q_DECLARE_METATYPE(CodeInfoTip);

#endif // CODEINFOTIP_H
