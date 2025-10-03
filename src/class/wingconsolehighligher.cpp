/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
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

#include <QTextCharFormat>

class ConsoleTextBlockUserData : public WingTextBlockUserData {
public:
    enum class Type {
        TextOnly,
        TextWithFormat,
        CodeWithPrefix
    } type = Type::CodeWithPrefix;
    int prefixLength = 0;
    QTextCharFormat fmt;
};

WingConsoleHighligher::WingConsoleHighligher(QTextDocument *document)
    : WingSyntaxHighlighter(document) {}

WingTextBlockUserData *WingConsoleHighligher::createTextBlockUserData() {
    return new ConsoleTextBlockUserData;
}

void WingConsoleHighligher::setBlockAsTextOnly(QTextBlock &block) {
    auto data = dynamic_cast<ConsoleTextBlockUserData *>(block.userData());
    if (data == nullptr) {
        data = new ConsoleTextBlockUserData;
        block.setUserData(data);
    }
    data->type = ConsoleTextBlockUserData::Type::TextOnly;
    data->prefixLength = 0;
    data->fmt = {};
    rehighlightBlock(block);
}

void WingConsoleHighligher::setBlockAsTextWithFormat(
    QTextBlock &block, const QTextCharFormat &fmt) {
    auto data = dynamic_cast<ConsoleTextBlockUserData *>(block.userData());
    if (data == nullptr) {
        data = new ConsoleTextBlockUserData;
        block.setUserData(data);
    }
    data->type = ConsoleTextBlockUserData::Type::TextWithFormat;
    data->prefixLength = 0;
    data->fmt = fmt;
    rehighlightBlock(block);
}

void WingConsoleHighligher::setBlockAsCodeWithPrefix(QTextBlock &block,
                                                     int codePrefix) {
    auto data = dynamic_cast<ConsoleTextBlockUserData *>(block.userData());
    if (data == nullptr) {
        data = new ConsoleTextBlockUserData;
        block.setUserData(data);
    }
    data->type = ConsoleTextBlockUserData::Type::CodeWithPrefix;
    data->prefixLength = codePrefix;
    data->fmt = {};
    rehighlightBlock(block);
}

int WingConsoleHighligher::blockPrefixLength(const QTextBlock &block) {
    auto data =
        dynamic_cast<const ConsoleTextBlockUserData *>(block.userData());
    if (data == nullptr) {
        return 0;
    }
    return data->prefixLength;
}

void WingConsoleHighligher::highlightBlock(const QString &text) {
    WingSyntaxHighlighter::highlightBlock(text);
}

void WingConsoleHighligher::applyFormat(
    int offset, int length, const KSyntaxHighlighting::Format &format) {
    auto blk = currentBlock();

    auto data = dynamic_cast<ConsoleTextBlockUserData *>(blk.userData());
    if (data == nullptr) {
        return;
    }

    switch (data->type) {
    case ConsoleTextBlockUserData::Type::TextOnly:
        WingSyntaxHighlighter::applyFormat(offset, length,
                                           KSyntaxHighlighting::Format());
        break;
    case ConsoleTextBlockUserData::Type::TextWithFormat:
        QSyntaxHighlighter::setFormat(offset, length, data->fmt);
        break;
    case ConsoleTextBlockUserData::Type::CodeWithPrefix: {
        auto off = data->prefixLength;
        if (offset <= off) {
            auto div = off - offset;
            WingSyntaxHighlighter::applyFormat(offset, div,
                                               KSyntaxHighlighting::Format());
            auto rest = length - div;
            if (rest <= 0) {
                return;
            }
            WingSyntaxHighlighter::applyFormat(off, rest, format);
        } else {
            WingSyntaxHighlighter::applyFormat(offset, length, format);
        }
    } break;
    }
}

void WingConsoleHighligher::applyFolding(
    int offset, int length, KSyntaxHighlighting::FoldingRegion region) {
    Q_UNUSED(offset);
    Q_UNUSED(length);
    Q_UNUSED(region);

    // Console needs no folding
}
