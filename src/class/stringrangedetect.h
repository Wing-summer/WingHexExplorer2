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

#ifndef STRINGRANGEDETECT_H
#define STRINGRANGEDETECT_H

#include <QRegularExpression>
#include <QTextCursor>

class StringRangeDetect {
public:
    explicit StringRangeDetect();

public:
    bool isInsideString(const QTextCursor &cursor) const;

private:
    bool isInsideSingleLineString(const QTextBlock &block, int position) const;

    bool isInsideTripleQuoteString(QTextDocument *doc,
                                   const QTextBlock &currentBlock,
                                   int position) const;

    QTextBlock findTripleQuoteStart(const QTextBlock &fromBlock,
                                    const QRegularExpression &pattern) const;

    bool isInsideComment(QTextDocument *doc, const QTextBlock &currentBlock,
                         int position) const;

    bool isInsideLineComment(const QTextBlock &block, int position) const;

    bool isInsideBlockComment(QTextDocument *doc,
                              const QTextBlock &currentBlock,
                              int position) const;

    QTextBlock findBlockCommentStart(const QTextBlock &fromBlock) const;

private:
    QRegularExpression m_singleQuote;
    QRegularExpression m_doubleQuote;
    QRegularExpression m_tripleStart;
    QRegularExpression m_tripleEnd;

    QRegularExpression m_lineComment;
    QRegularExpression m_blockCommentStart;
    QRegularExpression m_blockCommentEnd;
};
#endif // STRINGRANGEDETECT_H
