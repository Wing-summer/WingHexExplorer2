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

#include "editorlspevent.h"

#include "luau/lsp/luaulanguageserver.h"

#include <QJsonArray>
#include <QTextDocument>
#include <QToolTip>

bool EditorLspEvent::processEvent(QEvent *event, LspEditorInterace *editor) {
    auto type = event->type();
    if (type == QEvent::KeyPress) {
        auto e = static_cast<QKeyEvent *>(event);
        if (e->modifiers() == Qt::NoModifier) {
            auto key = e->key();
            if (key == Qt::Key_Comma || key == Qt::Key_ParenLeft) {
                showSignatureHelp(editor);
            } else if (key == Qt::Key_Semicolon) {
                editor->clearFunctionTip();
            }
        }
    } else if (type == QEvent::ToolTip) {
        auto uri = editor->lspFileNameURL();
        if (uri.isEmpty()) {
            return false;
        }

        auto helpEvent = static_cast<QHelpEvent *>(event);
        auto point = helpEvent->pos();
        auto eptr = editor->editorPtr();
        point.setX(point.x() - eptr->lineMarginWidth());
        auto cursor = eptr->cursorForPosition(point);
        auto pos = editor->cursorPosition(cursor);

        auto line = pos.blockNumber;
        auto character = pos.positionInBlock;
        if (pos.blockNumber < 0 || pos.positionInBlock < 0) {
            return false;
        }

        lsp::HoverParams params{
            uri, {uint(pos.blockNumber), uint(pos.positionInBlock)}};
        auto hover = LuauLanguageServer::instance().hover(params, nullptr);
        if (!hover || hover->contents.value.isEmpty()) {
            QToolTip::hideText();
            return false;
        }

        const auto value = hover->contents.value;
        if (hover->contents.kind == lsp::MarkupKind::Markdown) {
            QTextDocument markdown;
            markdown.setMarkdown(value);
            QToolTip::showText(helpEvent->globalPos(), markdown.toHtml());
        } else {
            QToolTip::showText(helpEvent->globalPos(), value);
        }
        return true;
    }
    return false;
}

bool EditorLspEvent::showSignatureHelp(LspEditorInterace *editor) {
    if (!editor) {
        return false;
    }
    const auto documentUrl = editor->lspFileNameURL();
    if (documentUrl.isEmpty()) {
        return false;
    }

    const auto position = editor->currentPosition();
    if (position.blockNumber < 0 || position.positionInBlock < 0) {
        return false;
    }

    editor->syncDocChange();

    lsp::SignatureHelpParams params{
        documentUrl,
        {uint(position.blockNumber), uint(position.positionInBlock)}};
    const auto signatureHelp =
        LuauLanguageServer::instance().signatureHelp(params, nullptr);
    if (!signatureHelp) {
        return false;
    }

    QList<WingSignatureTooltip::Signature> signatures;
    for (const auto &signature : signatureHelp->signatures) {
        WingSignatureTooltip::Signature item;
        item.label = signature.label;
        item.doc = signature.documentation.value;
        signatures.append(std::move(item));
    }
    if (signatures.isEmpty()) {
        return false;
    }
    editor->showFunctionTip(signatures);
    return true;
}

int EditorLspEvent::absolutePositionForLineCharacter(const QTextDocument *doc,
                                                     int line, int character) {
    if (!doc || line < 0 || character < 0) {
        return -1;
    }

    const QTextBlock block = doc->findBlockByNumber(line);
    if (!block.isValid()) {
        return -1;
    }

    const int blockStart = block.position();
    const int blockLenWithoutSeparator = qMax(0, block.length() - 1);

    if (character > blockLenWithoutSeparator) {
        return -1;
    }

    return blockStart + character;
}

QList<QTextEdit::ExtraSelection>
EditorLspEvent::semanticTokensToExtraSelections(
    QTextDocument *doc, const QVector<lsp::SemanticToken> &tokens,
    const std::function<QTextCharFormat(const QString &, const QStringList &)>
        &formatForToken) {
    Q_ASSERT(formatForToken);

    QList<QTextEdit::ExtraSelection> out;
    if (!doc) {
        return out;
    }

    const int maxPos = qMax(0, doc->characterCount() - 1);

    for (const auto &t : tokens) {
        if (t.length <= 0) {
            continue;
        }

        if (!t.modifiers.contains("inactive")) {
            if (t.tokenType == QStringLiteral("keyword") ||
                t.tokenType == QStringLiteral("operator") ||
                t.tokenType == QStringLiteral("comment") ||
                t.tokenType == QStringLiteral("string") ||
                t.tokenType == QStringLiteral("number") ||
                t.tokenType == QStringLiteral("regexp")) {
                continue;
            }
        }

        const int start =
            absolutePositionForLineCharacter(doc, t.line, t.character);
        if (start < 0 || start >= maxPos) {
            continue;
        }

        const int end = qMin(start + t.length, maxPos);
        if (end <= start) {
            continue;
        }

        QTextCursor cursor(doc);
        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        if (!cursor.hasSelection()) {
            continue;
        }

        QTextEdit::ExtraSelection sel;
        sel.cursor = cursor;
        sel.format = formatForToken(t.tokenType, t.modifiers);
        out.append(sel);
    }

    return out;
}
