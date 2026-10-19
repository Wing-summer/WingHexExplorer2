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

#include "wingconsolehighligher.h"

#include <KSyntaxHighlighting/FoldingRegion>
#include <KSyntaxHighlighting/Format>
#include <KSyntaxHighlighting/Theme>

#include <QFontDatabase>
#include <QFontMetricsF>
#include <QScopedValueRollback>
#include <QTextCharFormat>
#include <QtMath>

class ConsoleTextBlockUserData : public WingTextBlockUserData {
public:
    enum class Type {
        Default,
        TextOnly,
        TextWithFormat,
        CodeWithPrefix
    } type = Type::Default;
    int prefixLength = 0;
    bool startsCommand = false;
    QTextCharFormat fmt;
    WingEditorMetaInfo meta;
};

WingConsoleHighligher::WingConsoleHighligher(QTextDocument *document)
    : WingSyntaxHighlighter(document) {}

WingTextBlockUserData *WingConsoleHighligher::createTextBlockUserData() {
    return new ConsoleTextBlockUserData;
}

void WingConsoleHighligher::setBlockAsTextOnly(QTextBlock &block,
                                               const WingEditorMetaInfo &info) {
    auto data = dynamic_cast<ConsoleTextBlockUserData *>(block.userData());
    if (data == nullptr) {
        data = new ConsoleTextBlockUserData;
        block.setUserData(data);
    }
    data->type = ConsoleTextBlockUserData::Type::TextOnly;
    data->prefixLength = 0;
    data->fmt = {};
    data->meta = info;
    rehighlightBlock(block);
}

void WingConsoleHighligher::setBlockAsTextWithFormat(
    QTextBlock &block, const QTextCharFormat &fmt,
    const WingEditorMetaInfo &info) {
    auto data = dynamic_cast<ConsoleTextBlockUserData *>(block.userData());
    if (data == nullptr) {
        data = new ConsoleTextBlockUserData;
        block.setUserData(data);
    }
    data->type = ConsoleTextBlockUserData::Type::TextWithFormat;
    data->prefixLength = 0;
    data->fmt = fmt;
    data->meta = info;
    rehighlightBlock(block);
}

void WingConsoleHighligher::setBlockAsCodeWithPrefix(
    QTextBlock &block, int codePrefix, const WingEditorMetaInfo &info,
    bool startsCommand) {
    auto data = dynamic_cast<ConsoleTextBlockUserData *>(block.userData());
    if (data == nullptr) {
        data = new ConsoleTextBlockUserData;
        block.setUserData(data);
    }
    data->type = ConsoleTextBlockUserData::Type::CodeWithPrefix;
    data->prefixLength = codePrefix;
    data->startsCommand = startsCommand;
    data->fmt = {};
    data->meta = info;
    rehighlightBlock(block);
}

void WingConsoleHighligher::setBlockEditorMetaInfo(
    QTextBlock &block, const WingEditorMetaInfo &info) {
    auto data = dynamic_cast<ConsoleTextBlockUserData *>(block.userData());
    if (data == nullptr) {
        data = new ConsoleTextBlockUserData;
        block.setUserData(data);
    }
    data->meta = info;
    rehighlightBlock(block);
}

bool WingConsoleHighligher::isDefaultBlock(const QTextBlock &block) const {
    auto data = dynamic_cast<ConsoleTextBlockUserData *>(block.userData());
    if (data) {
        return data->type == ConsoleTextBlockUserData::Type::Default;
    }
    return true;
}

bool WingConsoleHighligher::hasContinuationPrompt(
    const QTextBlock &block) const {
    const auto *data =
        dynamic_cast<const ConsoleTextBlockUserData *>(block.userData());
    return data &&
           data->type == ConsoleTextBlockUserData::Type::CodeWithPrefix &&
           data->prefixLength > 0 && !data->startsCommand;
}

WingEditorMetaInfo
WingConsoleHighligher::blockEditorMetaInfo(const QTextBlock &block) {
    auto data =
        dynamic_cast<const ConsoleTextBlockUserData *>(block.userData());
    if (data == nullptr) {
        return {};
    }
    return data->meta;
}

int WingConsoleHighligher::blockPrefixLength(const QTextBlock &block) {
    auto data =
        dynamic_cast<const ConsoleTextBlockUserData *>(block.userData());
    if (data == nullptr) {
        return 0;
    }
    return data->prefixLength;
}

void WingConsoleHighligher::highlightPrompt(QStringView prompt) {
    const auto doc = document();
    const auto font = doc->defaultFont();
    const auto tabWidth = doc->defaultTextOption().tabStopDistance();
    const auto color =
        QColor::fromRgb(theme().textColor(KSyntaxHighlighting::Theme::Normal));
    if (!_promptCacheValid || _promptDocumentFont != font ||
        _promptTabWidth != tabWidth || _promptText != prompt ||
        _promptFormat.foreground().color() != color) {
        auto promptFont = font;
        const auto family =
            QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
        promptFont.setFamilies({family});
        promptFont.setWeight(QFont::Normal);
        promptFont.setItalic(false);
        promptFont.setLetterSpacing(QFont::AbsoluteSpacing, 0);

        _promptDocumentFont = font;
        _promptTabWidth = tabWidth;
        _promptText = prompt.toString();
        _promptFormat = {};
        _promptFormat.setForeground(color);
        _promptFormat.setFontFamilies({family});
        _promptFormat.setFontWeight(QFont::Normal);
        _promptFormat.setFontItalic(false);
        _promptFormat.setFontLetterSpacingType(QFont::AbsoluteSpacing);
        _promptFormat.setFontLetterSpacing(0);
        _promptTailFormat = _promptFormat;
        if (tabWidth > 0) {
            const auto width =
                QFontMetricsF(promptFont).horizontalAdvance(_promptText);
            // Keep the source origin on a tab stop without adding whitespace.
            _promptTailFormat.setFontLetterSpacing(
                qCeil(width / tabWidth) * tabWidth - width);
        }
        _promptCacheValid = true;
    }
    setFormat(0, prompt.size(), _promptFormat);
    if (tabWidth > 0) {
        setFormat(prompt.size() - 1, 1, _promptTailFormat);
    }
}

void WingConsoleHighligher::highlightBlock(const QString &text) {
    auto data =
        dynamic_cast<ConsoleTextBlockUserData *>(currentBlockUserData());
    if (!data) {
        // Unclassified output needs neither lexical state nor block metadata.
        return;
    }

    const auto block = currentBlock();
    KSyntaxHighlighting::State state;
    if (data->type == ConsoleTextBlockUserData::Type::CodeWithPrefix) {
        if (!data->startsCommand) {
            const auto *previous =
                dynamic_cast<const ConsoleTextBlockUserData *>(
                    block.previous().userData());
            if (previous &&
                previous->type ==
                    ConsoleTextBlockUserData::Type::CodeWithPrefix) {
                state = previous->state;
            }
        }
        const auto prefixLength =
            qBound(0, data->prefixLength, int(text.size()));
        const auto view = QStringView(text);
        if (prefixLength > 0) {
            highlightPrompt(view.sliced(0, prefixLength));
        }
        const QScopedValueRollback offset(_formatOffset, prefixLength);
        state = highlightLine(view.sliced(prefixLength), state);
    } else if (data->type == ConsoleTextBlockUserData::Type::TextWithFormat) {
        setFormat(0, text.size(), data->fmt);
    }

    if (data->state == state) {
        return;
    }
    data->state = std::move(state);
    const auto next = block.next();
    const auto *nextData =
        dynamic_cast<const ConsoleTextBlockUserData *>(next.userData());
    if (nextData &&
        nextData->type == ConsoleTextBlockUserData::Type::CodeWithPrefix &&
        !nextData->startsCommand) {
        QMetaObject::invokeMethod(
            this,
            [this, next]() {
                if (next.isValid() && next.document() == document()) {
                    rehighlightBlock(next);
                }
            },
            Qt::QueuedConnection);
    }
}

void WingConsoleHighligher::applyFormat(
    int offset, int length, const KSyntaxHighlighting::Format &format) {
    WingSyntaxHighlighter::applyFormat(offset + _formatOffset, length, format);
}

void WingConsoleHighligher::applyFolding(
    int offset, int length, KSyntaxHighlighting::FoldingRegion region) {
    Q_UNUSED(offset);
    Q_UNUSED(length);
    Q_UNUSED(region);

    // Console needs no folding
}
