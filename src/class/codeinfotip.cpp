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

#include "codeinfotip.h"

#include "class/editorlspevent.h"
#include "class/snippetprocessor.h"

#include <QApplication>
#include <QClipboard>
#include <QDate>
#include <QFileInfo>
#include <QHash>
#include <QMetaEnum>
#include <QRandomGenerator>
#include <QTime>
#include <QUuid>

#include <functional>

namespace {

struct PendingTextEdit {
    int start = 0;
    int end = 0;
    QString text;
};

bool resolveRange(const QTextDocument *document, const lsp::Range &range,
                  int &start, int &end) {
    if (!document) {
        return false;
    }
    start = EditorLspEvent::absolutePositionForLineCharacter(
        document, range.start.line, range.start.character);
    end = EditorLspEvent::absolutePositionForLineCharacter(
        document, range.end.line, range.end.character);
    return start >= 0 && end >= start;
}

} // namespace

using IconHash = QHash<lsp::CompletionItemKind, QIcon>;
Q_GLOBAL_STATIC(IconHash, q_icon_cache)

inline static QIcon getIcon(const QString &name) {
    return QIcon(QStringLiteral(":/completion/images/completion/") + name +
                 QStringLiteral(".svg"));
}

QIcon CodeInfoTip::getDisplayIcon(lsp::CompletionItemKind kind) {
    static bool setup = false;

    if (!setup) {
        auto e = QMetaEnum::fromType<lsp::CompletionItemKind>();
        auto total = e.keyCount();
        for (int i = 0; i < total; ++i) {
            auto k = e.key(i);
            auto v = lsp::CompletionItemKind(e.value(i));
            q_icon_cache->insert(v, getIcon(QString::fromLatin1(k)));
        }
        setup = true;
    }

    return q_icon_cache->value(kind);
}

CodeInfoTip::CodeInfoTip(const lsp::CompletionItem &serverItem) {
    label = serverItem.label;
    kind = serverItem.kind.value_or(lsp::CompletionItemKind::Text);
    documentationText = serverItem.documentation.value;
    if (serverItem.detail && !serverItem.detail->isEmpty()) {
        detail = serverItem.detail.value();
    }
    if (serverItem.insertText) {
        insertionText = serverItem.insertText.value();
    }
    insertionFormat = serverItem.insertTextFormat;
    if (serverItem.textEdit) {
        textEditRange = serverItem.textEdit->range;
        insertionText = serverItem.textEdit->newText;
    }
    additionalTextEdits = serverItem.additionalTextEdits;
}

QString CodeInfoTip::documentation() const {
    if (detail.isEmpty()) {
        return documentationText.isEmpty() ? label : documentationText;
    }
    if (documentationText.isEmpty()) {
        return detail;
    }
    return detail + QStringLiteral("\n") + documentationText;
}

QString CodeInfoTip::insertionTextOrLabel() const {
    return insertionText.isEmpty() ? label : insertionText;
}

bool CodeInfoTip::usesSnippet() const {
    return insertionFormat == lsp::InsertTextFormat::Snippet;
}

void CodeInfoTip::applyEdit(QPlainTextEdit *editor,
                            const QString &completionPrefix,
                            bool hasCodeLineContext) const {
    if (!editor) {
        return;
    }

    auto *document = editor->document();
    auto cursor = editor->textCursor();
    int primaryStart = cursor.position();
    int primaryEnd = cursor.position();

    if (textEditRange) {
        if (!resolveRange(document, *textEditRange, primaryStart, primaryEnd)) {
            return;
        }
    } else if (!completionPrefix.isEmpty()) {
        cursor.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor);
        primaryStart = cursor.selectionStart();
        primaryEnd = cursor.selectionEnd();
    }

    const auto sourceText = insertionTextOrLabel();
    QString insertedText = sourceText;
    SnippetResult snippetResult;
    const bool hasSnippet = usesSnippet();
    if (hasSnippet) {
        SnippetProcessor processor(std::bind(&CodeInfoTip::processSnippt,
                                             std::placeholders::_1, editor,
                                             hasCodeLineContext));
        snippetResult = processor.process(sourceText);
        insertedText = snippetResult.expandedText;
    }

    std::vector<PendingTextEdit> edits;
    edits.reserve(additionalTextEdits.size() + 1);
    int insertionStart = primaryStart;
    for (const auto &additional : additionalTextEdits) {
        int start = 0;
        int end = 0;
        if (!resolveRange(document, additional.range, start, end)) {
            return;
        }
        if (start < primaryEnd && end > primaryStart) {
            return;
        }

        const auto text = additional.newText;
        if (end <= primaryStart) {
            insertionStart += text.size() - (end - start);
        }
        edits.push_back({start, end, text});
    }

    edits.push_back({primaryStart, primaryEnd, insertedText});
    std::sort(edits.begin(), edits.end(),
              [](const PendingTextEdit &a, const PendingTextEdit &b) {
                  if (a.start != b.start) {
                      return a.start > b.start;
                  }

                  return a.end > b.end;
              });

    QTextCursor editCursor(document);
    editCursor.beginEditBlock();
    for (const auto &edit : edits) {
        editCursor.setPosition(edit.start);
        editCursor.setPosition(edit.end, QTextCursor::KeepAnchor);
        editCursor.insertText(edit.text);
    }
    editCursor.endEditBlock();
    editCursor.setPosition(insertionStart + (hasSnippet
                                                 ? snippetResult.cursorOffset
                                                 : insertedText.size()));
    if (hasSnippet && snippetResult.selectionLength > 0) {
        editCursor.setPosition(insertionStart + snippetResult.cursorOffset +
                                   snippetResult.selectionLength,
                               QTextCursor::KeepAnchor);
    }
    editor->setTextCursor(editCursor);
}

QString CodeInfoTip::processSnippt(const QString &name, QPlainTextEdit *editor,
                                   bool hasCodeLineContext) {
    static QHash<QString, SnippetProcessor::TM_CODE> maps;

    if (maps.isEmpty()) {
        auto e = QMetaEnum::fromType<SnippetProcessor::TM_CODE>();
        auto total = e.keyCount();
        for (int i = 0; i < total; ++i) {
            maps.insert(QString::fromLatin1(e.key(i)),
                        SnippetProcessor::TM_CODE(e.value(i)));
        }
    }

    if (!maps.contains(name)) {
        return {};
    }

    auto en = maps.value(name);
    switch (en) {
    case SnippetProcessor::TM_CODE::TM_SELECTED_TEXT: {
        auto tc = editor->textCursor();
        return tc.selectedText();
    }
    case SnippetProcessor::TM_CODE::TM_CURRENT_LINE: {
        auto tc = editor->textCursor();
        return tc.block().text();
    }
    case SnippetProcessor::TM_CODE::TM_CURRENT_WORD: {
        auto tc = editor->textCursor();
        tc.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
        return tc.selectedText();
    }
    case SnippetProcessor::TM_CODE::TM_LINE_INDEX: {
        if (hasCodeLineContext) {
            auto tc = editor->textCursor();
            return QString::number(tc.blockNumber());
        } else {
            return QStringLiteral("-1");
        }
    }
    case SnippetProcessor::TM_CODE::TM_LINE_NUMBER: {
        if (hasCodeLineContext) {
            auto tc = editor->textCursor();
            return QString::number(tc.blockNumber() + 1);
        } else {
            return QStringLiteral("-1");
        }
    }
    case SnippetProcessor::TM_CODE::TM_FILENAME: {
        // Assuming fileName is stored in it
        return editor->windowFilePath();
    }
    case SnippetProcessor::TM_CODE::RELATIVE_FILEPATH:
    case SnippetProcessor::TM_CODE::TM_FILENAME_BASE: {
        auto fileName = editor->windowFilePath();
        QFileInfo info(fileName);
        return info.fileName();
    }
    case SnippetProcessor::TM_CODE::TM_DIRECTORY: {
        auto fileName = editor->windowFilePath();
        QFileInfo info(fileName);
        return info.filePath();
    }
    case SnippetProcessor::TM_CODE::TM_FILEPATH: {
        auto fileName = editor->windowFilePath();
        QFileInfo info(fileName);
        return info.absoluteFilePath();
    }
    case SnippetProcessor::TM_CODE::CLIPBOARD:
        return QApplication::clipboard()->text();
    case SnippetProcessor::TM_CODE::WORKSPACE_NAME:
    case SnippetProcessor::TM_CODE::WORKSPACE_FOLDER:
        return {};
    case SnippetProcessor::TM_CODE::CURRENT_YEAR: {
        auto date = QDate::currentDate();
        return date.toString(QStringLiteral("yyyy"));
    }
    case SnippetProcessor::TM_CODE::CURRENT_YEAR_SHORT: {
        auto date = QDate::currentDate();
        return date.toString(QStringLiteral("yy"));
    }
    case SnippetProcessor::TM_CODE::CURRENT_MONTH: {
        auto date = QDate::currentDate();
        return date.toString(QStringLiteral("M"));
    }
    case SnippetProcessor::TM_CODE::CURRENT_MONTH_NAME: {
        auto date = QDate::currentDate();
        return date.toString(QStringLiteral("MMMM"));
    }
    case SnippetProcessor::TM_CODE::CURRENT_MONTH_NAME_SHORT: {
        auto date = QDate::currentDate();
        return date.toString(QStringLiteral("MMM"));
    }
    case SnippetProcessor::TM_CODE::CURRENT_DATE: {
        auto date = QDate::currentDate();
        return date.toString(QStringLiteral("d"));
    }
    case SnippetProcessor::TM_CODE::CURRENT_DAY_NAME: {
        auto date = QDate::currentDate();
        return date.toString(QStringLiteral("dddd"));
    }
    case SnippetProcessor::TM_CODE::CURRENT_DAY_NAME_SHORT: {
        auto date = QDate::currentDate();
        return date.toString(QStringLiteral("ddd"));
    }
    case SnippetProcessor::TM_CODE::CURRENT_HOUR: {
        auto time = QTime::currentTime();
        return time.toString(QStringLiteral("h"));
    }
    case SnippetProcessor::TM_CODE::CURRENT_MINUTE: {
        auto time = QTime::currentTime();
        return time.toString(QStringLiteral("m"));
    }
    case SnippetProcessor::TM_CODE::CURRENT_SECOND: {
        auto time = QTime::currentTime();
        return time.toString(QStringLiteral("s"));
    }
    case SnippetProcessor::TM_CODE::CURRENT_SECONDS_UNIX:
        return QString::number(QDateTime::currentSecsSinceEpoch());
    case SnippetProcessor::TM_CODE::RANDOM: {
        auto ran = QRandomGenerator::global();
        QString buffer(6, QChar{});
        for (int i = 0; i < 6; ++i) {
            buffer[i] = QChar(ran->bounded(0, 9) + '0');
        }
        return buffer;
    }
    case SnippetProcessor::TM_CODE::RANDOM_HEX: {
        auto ran = QRandomGenerator::global();
        QString buffer(6, QChar{});
        for (int i = 0; i < 6; ++i) {
            auto n = ran->bounded(0, 16);
            if (n >= 10) {
                buffer[i] = QChar(n - 10 + 'A');
            } else {
                buffer[i] = QChar(n + '0');
            }
        }
        return buffer;
    }
    case SnippetProcessor::TM_CODE::UUID:
        return QUuid::createUuid().toString();
    case SnippetProcessor::TM_CODE::BLOCK_COMMENT_START:
        return QStringLiteral("--[[");
    case SnippetProcessor::TM_CODE::BLOCK_COMMENT_END:
        return QStringLiteral("]]");
    case SnippetProcessor::TM_CODE::LINE_COMMENT:
        return QStringLiteral("--");
        break;
    }
    return {};
}
