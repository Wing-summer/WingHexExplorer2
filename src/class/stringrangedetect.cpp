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

#include "stringrangedetect.h"

#include <QTextBlock>

StringRangeDetect::StringRangeDetect() {
    m_singleQuote.setPattern(R"('([^'\\]|\\.)*')");
    m_doubleQuote.setPattern(R"("([^"\\]|\\.)*")");
    m_tripleStart.setPattern(R"(^[ \t]*""")");
    m_tripleEnd.setPattern(R"(""")");

    m_lineComment.setPattern(R"(//[^\n]*)");
    m_blockCommentStart.setPattern(R"(/\*)");
    m_blockCommentEnd.setPattern(R"(\*/)");
}

bool StringRangeDetect::isInsideString(const QTextCursor &cursor) const {
    auto doc = cursor.document();
    int position = cursor.position();
    auto currentBlock = doc->findBlock(position);

    if (isInsideComment(doc, currentBlock, position)) {
        return false;
    }

    if (isInsideSingleLineString(currentBlock, position)) {
        return true;
    }

    return isInsideTripleQuoteString(doc, currentBlock, position);
}

bool StringRangeDetect::isInsideSingleLineString(const QTextBlock &block,
                                                 int position) const {
    auto text = block.text();
    int blockPos = block.position();
    int posInBlock = position - blockPos;

    auto singleIt = m_singleQuote.globalMatch(text);
    while (singleIt.hasNext()) {
        QRegularExpressionMatch match = singleIt.next();
        if (posInBlock > match.capturedStart() &&
            posInBlock < match.capturedEnd()) {
            return true;
        }
    }

    auto doubleIt = m_doubleQuote.globalMatch(text);
    while (doubleIt.hasNext()) {
        auto match = doubleIt.next();
        if (posInBlock > match.capturedStart() &&
            posInBlock < match.capturedEnd()) {
            return true;
        }
    }

    return false;
}

bool StringRangeDetect::isInsideTripleQuoteString(
    QTextDocument *doc, const QTextBlock &currentBlock, int position) const {
    auto block = currentBlock;
    int blockPos = block.position();
    int posInBlock = position - blockPos;

    auto startBlock = findTripleQuoteStart(block, m_tripleStart);
    if (!startBlock.isValid())
        return false;

    if (startBlock == currentBlock) {
        auto match = m_tripleStart.match(block.text());
        if (match.hasMatch() && posInBlock <= match.capturedEnd()) {
            return false;
        }
    }

    auto searchBlock = startBlock;
    while (searchBlock.isValid() &&
           searchBlock.position() <= currentBlock.position()) {
        auto text = searchBlock.text();

        if (searchBlock == startBlock) {
            auto startMatch = m_tripleStart.match(text);
            if (startMatch.hasMatch()) {
                text = text.sliced(startMatch.capturedEnd());
            }
        }

        if (searchBlock == currentBlock) {
            auto endIt = m_tripleEnd.globalMatch(text);
            int offset = blockPos + (text.length() - text.length());

            while (endIt.hasNext()) {
                auto endMatch = endIt.next();
                int endPos = offset + endMatch.capturedStart();

                if (position < endPos) {
                    return true;
                }
            }

            return true;
        } else {
            auto endMatch = m_tripleEnd.match(text);
            if (endMatch.hasMatch()) {
                return false;
            }
        }

        searchBlock = searchBlock.next();
    }

    return false;
}

QTextBlock StringRangeDetect::findTripleQuoteStart(
    const QTextBlock &fromBlock, const QRegularExpression &pattern) const {
    auto block = fromBlock;

    while (block.isValid()) {
        auto text = block.text();

        auto startMatch = pattern.match(text);
        if (startMatch.hasMatch()) {
            auto afterStart = text.sliced(startMatch.capturedEnd());
            auto endMatch = m_tripleEnd.match(afterStart);

            if (!endMatch.hasMatch()) {
                return block;
            }
        }

        block = block.previous();
    }

    return {};
}

bool StringRangeDetect::isInsideComment(QTextDocument *doc,
                                        const QTextBlock &currentBlock,
                                        int position) const {
    return isInsideLineComment(currentBlock, position) ||
           isInsideBlockComment(doc, currentBlock, position);
}

bool StringRangeDetect::isInsideLineComment(const QTextBlock &block,
                                            int position) const {
    auto text = block.text();
    int blockPos = block.position();
    int posInBlock = position - blockPos;

    auto commentIt = m_lineComment.globalMatch(text);
    while (commentIt.hasNext()) {
        QRegularExpressionMatch match = commentIt.next();
        if (posInBlock >= match.capturedStart() &&
            posInBlock <= match.capturedEnd()) {
            return true;
        }
    }

    return false;
}

bool StringRangeDetect::isInsideBlockComment(QTextDocument *doc,
                                             const QTextBlock &currentBlock,
                                             int position) const {
    auto block = currentBlock;
    int blockPos = block.position();
    int posInBlock = position - blockPos;

    auto startBlock = findBlockCommentStart(block);
    if (!startBlock.isValid())
        return false;

    if (startBlock == currentBlock) {
        auto match = m_blockCommentStart.match(block.text());
        if (match.hasMatch() && posInBlock <= match.capturedEnd()) {
            return false;
        }
    }

    auto searchBlock = startBlock;
    while (searchBlock.isValid() &&
           searchBlock.position() <= currentBlock.position()) {
        auto text = searchBlock.text();

        if (searchBlock == startBlock) {
            auto startMatch = m_blockCommentStart.match(text);
            if (startMatch.hasMatch()) {
                text = text.sliced(startMatch.capturedEnd());
            }
        }

        if (searchBlock == currentBlock) {
            auto endIt = m_blockCommentEnd.globalMatch(text);
            int offset = blockPos + (text.length() - text.length());

            while (endIt.hasNext()) {
                auto endMatch = endIt.next();
                int endPos = offset + endMatch.capturedStart();

                if (position < endPos) {
                    return true;
                }
            }

            return true;
        } else {
            auto endMatch = m_blockCommentEnd.match(text);
            if (endMatch.hasMatch()) {
                return false;
            }
        }

        searchBlock = searchBlock.next();
    }

    return false;
}

QTextBlock
StringRangeDetect::findBlockCommentStart(const QTextBlock &fromBlock) const {
    auto block = fromBlock;

    while (block.isValid()) {
        auto text = block.text();

        auto startMatch = m_blockCommentStart.match(text);
        if (startMatch.hasMatch()) {
            auto afterStart = text.sliced(startMatch.capturedEnd());
            auto endMatch = m_blockCommentEnd.match(afterStart);

            if (!endMatch.hasMatch()) {
                return block;
            }
        }

        block = block.previous();
    }

    return {};
}
