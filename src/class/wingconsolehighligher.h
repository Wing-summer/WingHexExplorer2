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

#ifndef WINGCONSOLEHIGHLIGHER_H
#define WINGCONSOLEHIGHLIGHER_H

#include "WingCodeEdit/wingsyntaxhighlighter.h"

class WingConsoleHighligher : public WingSyntaxHighlighter {
    Q_OBJECT
public:
    explicit WingConsoleHighligher(QTextDocument *document = nullptr);

public:
    virtual WingTextBlockUserData *createTextBlockUserData() override;

public:
    void setBlockAsTextOnly(QTextBlock &block);
    void setBlockAsTextWithFormat(QTextBlock &block,
                                  const QTextCharFormat &fmt);
    void setBlockAsCodeWithPrefix(QTextBlock &block, int codePrefix = 0);

protected:
    virtual void highlightBlock(const QString &text) override;
    virtual void
    applyFormat(int offset, int length,
                const KSyntaxHighlighting::Format &format) override;
    virtual void
    applyFolding(int offset, int length,
                 KSyntaxHighlighting::FoldingRegion region) override;
};

#endif // WINGCONSOLEHIGHLIGHER_H
