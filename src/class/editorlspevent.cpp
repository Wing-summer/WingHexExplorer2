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

#include "angellsp.h"

#include <QJsonArray>
#include <QToolTip>

bool EditorLspEvent::processEvent(QEvent *event, LspEditorInterace *editor) {
    auto type = event->type();
    if (type == QEvent::KeyPress) {
        auto e = static_cast<QKeyEvent *>(event);
        if (e->modifiers() == Qt::NoModifier) {
            auto key = e->key();
            if (key == Qt::Key_Comma) {
                auto &lsp = AngelLsp::instance();
                if (!lsp.isActive()) {
                    return false;
                }

                auto url = editor->lspFileNameURL();
                auto tc = editor->currentPosition();
                auto line = tc.blockNumber;
                auto character = tc.positionInBlock;

                editor->sendDocChange();
                while (editor->isContentLspUpdated()) {
                    // wait for a moment
                }

                auto r = lsp.requestSignatureHelp(url, line, character);
                auto sigs = r["signatures"].toArray();
                QList<WingSignatureTooltip::Signature> ss;
                for (const auto &&sig : std::as_const(sigs)) {
                    QJsonValue js = sig;
                    WingSignatureTooltip::Signature s;
                    s.label = js["label"].toString();
                    s.doc = js["documentation"].toString();
                    ss.append(s);
                }
                editor->showFunctionTip(ss);
            } else if (key == Qt::Key_Semicolon) {
                editor->clearFunctionTip();
            }
        }
    } else if (type == QEvent::ToolTip) {
        auto &lsp = AngelLsp::instance();
        if (!lsp.isActive()) {
            return false;
        }

        auto helpEvent = static_cast<QHelpEvent *>(event);
        auto eptr = editor->editorPtr();
        auto point = helpEvent->pos();
        point.setX(point.x() - eptr->lineMarginWidth());
        auto cursor = eptr->cursorForPosition(point);
        auto pos = editor->cursorPosition(cursor);

        auto url = editor->lspFileNameURL();
        auto line = pos.blockNumber;
        auto character = pos.positionInBlock;

        auto r = lsp.requestHover(url, line, character);
        if (!r.isNull()) {
            auto c = r["contents"];
            if (!c.isNull()) {
                auto v = c["value"].toString();
                if (v.isEmpty()) {
                    QToolTip::hideText();
                } else {
                    QString text;
                    if (c["kind"].toString() == QLatin1String("markdown")) {
                        QTextDocument doc;
                        doc.setMarkdown(v);
                        text = doc.toHtml();
                    } else {
                        text = v;
                    }
                    QToolTip::showText(helpEvent->globalPos(), text);
                }
            }
        }
        return true;
    }
    return false;
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
    QTextDocument *doc, const QVector<LSP::SemanticToken> &tokens,
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

        if (t.tokenType == QStringLiteral("keyword") ||
            t.tokenType == QStringLiteral("operator") ||
            t.tokenType == QStringLiteral("comment") ||
            t.tokenType == QStringLiteral("string") ||
            t.tokenType == QStringLiteral("number") ||
            t.tokenType == QStringLiteral("regexp")) {
            continue;
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
