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

#include <functional>

#include <QApplication>
#include <QClipboard>
#include <QDate>
#include <QFileInfo>
#include <QHash>
#include <QMetaEnum>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QTime>
#include <QUuid>

#include "class/settings/thememanager.h"
#include "editorlspevent.h"
#include "snippetprocessor.h"

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

inline static QIcon getIcon(QString name) {
    static QRegularExpression normalize(QStringLiteral("([a-z0-9])([A-Z])"));
    name.replace(normalize, QStringLiteral("\\1-\\2"));
    name = name.toLower();
    return ThemeManager::instance().resourceIcon(
        QStringLiteral(":/completion/images/completion/") + name +
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
    m_documentationText = serverItem.documentation.value;
    if (serverItem.detail && !serverItem.detail->isEmpty()) {
        detail = serverItem.detail.value();
    }
    if (serverItem.insertText) {
        m_insertionText = serverItem.insertText.value();
    }
    m_insertionFormat = serverItem.insertTextFormat;
    if (serverItem.textEdit) {
        m_textEditRange = serverItem.textEdit->range;
        m_insertionText = serverItem.textEdit->newText;
    }
    m_additionalTextEdits = serverItem.additionalTextEdits;
}

QString CodeInfoTip::documentation() const {
    if (detail.isEmpty()) {
        return m_documentationText.isEmpty() ? label : m_documentationText;
    }
    if (m_documentationText.isEmpty()) {
        return detail;
    }
    return detail + QStringLiteral("\n") + m_documentationText;
}

QString CodeInfoTip::insertionTextOrLabel() const {
    return m_insertionText.isEmpty() ? label : m_insertionText;
}

bool CodeInfoTip::usesSnippet() const {
    return m_insertionFormat == lsp::InsertTextFormat::Snippet;
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

    if (m_textEditRange) {
        if (!resolveRange(document, *m_textEditRange, primaryStart,
                          primaryEnd)) {
            return;
        }
    } else if (!completionPrefix.isEmpty()) {
        if (completionPrefix.size() > primaryEnd) {
            return;
        }
        primaryStart = primaryEnd - completionPrefix.size();
        cursor.setPosition(primaryStart);
        cursor.setPosition(primaryEnd, QTextCursor::KeepAnchor);
        if (cursor.selectedText() != completionPrefix) {
            return;
        }
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
    edits.reserve(m_additionalTextEdits.size() + 1);
    int insertionStart = primaryStart;
    for (const auto &additional : m_additionalTextEdits) {
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
    static QHash<QString, SnippetProcessor::TmCode> maps;

    if (maps.isEmpty()) {
        auto e = QMetaEnum::fromType<SnippetProcessor::TmCode>();
        auto total = e.keyCount();
        for (int i = 0; i < total; ++i) {
            maps.insert(QString::fromLatin1(e.key(i)),
                        SnippetProcessor::TmCode(e.value(i)));
        }
    }

    if (!maps.contains(name)) {
        return {};
    }

    auto en = maps.value(name);
    switch (en) {
    case SnippetProcessor::TmCode::TM_SELECTED_TEXT: {
        auto tc = editor->textCursor();
        return tc.selectedText();
    }
    case SnippetProcessor::TmCode::TM_CURRENT_LINE: {
        auto tc = editor->textCursor();
        return tc.block().text();
    }
    case SnippetProcessor::TmCode::TM_CURRENT_WORD: {
        auto tc = editor->textCursor();
        tc.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
        return tc.selectedText();
    }
    case SnippetProcessor::TmCode::TM_LINE_INDEX: {
        if (hasCodeLineContext) {
            auto tc = editor->textCursor();
            return QString::number(tc.blockNumber());
        } else {
            return QStringLiteral("-1");
        }
    }
    case SnippetProcessor::TmCode::TM_LINE_NUMBER: {
        if (hasCodeLineContext) {
            auto tc = editor->textCursor();
            return QString::number(tc.blockNumber() + 1);
        } else {
            return QStringLiteral("-1");
        }
    }
    case SnippetProcessor::TmCode::TM_FILENAME: {
        // Assuming fileName is stored in it
        return editor->windowFilePath();
    }
    case SnippetProcessor::TmCode::RELATIVE_FILEPATH:
    case SnippetProcessor::TmCode::TM_FILENAME_BASE: {
        auto fileName = editor->windowFilePath();
        QFileInfo info(fileName);
        return info.fileName();
    }
    case SnippetProcessor::TmCode::TM_DIRECTORY: {
        auto fileName = editor->windowFilePath();
        QFileInfo info(fileName);
        return info.filePath();
    }
    case SnippetProcessor::TmCode::TM_FILEPATH: {
        auto fileName = editor->windowFilePath();
        QFileInfo info(fileName);
        return info.absoluteFilePath();
    }
    case SnippetProcessor::TmCode::CLIPBOARD:
        return QApplication::clipboard()->text();
    case SnippetProcessor::TmCode::WORKSPACE_NAME:
    case SnippetProcessor::TmCode::WORKSPACE_FOLDER:
        return {};
    case SnippetProcessor::TmCode::CURRENT_YEAR: {
        auto date = QDate::currentDate();
        return date.toString(QStringLiteral("yyyy"));
    }
    case SnippetProcessor::TmCode::CURRENT_YEAR_SHORT: {
        auto date = QDate::currentDate();
        return date.toString(QStringLiteral("yy"));
    }
    case SnippetProcessor::TmCode::CURRENT_MONTH: {
        auto date = QDate::currentDate();
        return date.toString(QStringLiteral("M"));
    }
    case SnippetProcessor::TmCode::CURRENT_MONTH_NAME: {
        auto date = QDate::currentDate();
        return date.toString(QStringLiteral("MMMM"));
    }
    case SnippetProcessor::TmCode::CURRENT_MONTH_NAME_SHORT: {
        auto date = QDate::currentDate();
        return date.toString(QStringLiteral("MMM"));
    }
    case SnippetProcessor::TmCode::CURRENT_DATE: {
        auto date = QDate::currentDate();
        return date.toString(QStringLiteral("d"));
    }
    case SnippetProcessor::TmCode::CURRENT_DAY_NAME: {
        auto date = QDate::currentDate();
        return date.toString(QStringLiteral("dddd"));
    }
    case SnippetProcessor::TmCode::CURRENT_DAY_NAME_SHORT: {
        auto date = QDate::currentDate();
        return date.toString(QStringLiteral("ddd"));
    }
    case SnippetProcessor::TmCode::CURRENT_HOUR: {
        auto time = QTime::currentTime();
        return time.toString(QStringLiteral("h"));
    }
    case SnippetProcessor::TmCode::CURRENT_MINUTE: {
        auto time = QTime::currentTime();
        return time.toString(QStringLiteral("m"));
    }
    case SnippetProcessor::TmCode::CURRENT_SECOND: {
        auto time = QTime::currentTime();
        return time.toString(QStringLiteral("s"));
    }
    case SnippetProcessor::TmCode::CURRENT_SECONDS_UNIX:
        return QString::number(QDateTime::currentSecsSinceEpoch());
    case SnippetProcessor::TmCode::RANDOM: {
        auto ran = QRandomGenerator::global();
        QString buffer(6, QChar{});
        for (int i = 0; i < 6; ++i) {
            buffer[i] = QChar(ran->bounded(0, 9) + '0');
        }
        return buffer;
    }
    case SnippetProcessor::TmCode::RANDOM_HEX: {
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
    case SnippetProcessor::TmCode::UUID:
        return QUuid::createUuid().toString();
    case SnippetProcessor::TmCode::BLOCK_COMMENT_START:
        return QStringLiteral("--[[");
    case SnippetProcessor::TmCode::BLOCK_COMMENT_END:
        return QStringLiteral("]]");
    case SnippetProcessor::TmCode::LINE_COMMENT:
        return QStringLiteral("--");
        break;
    }
    return {};
}
