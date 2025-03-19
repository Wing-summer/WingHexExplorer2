#ifndef WINGCONSOLEHIGHLIGHER_H
#define WINGCONSOLEHIGHLIGHER_H

#include "WingCodeEdit/wingsyntaxhighlighter.h"

class WingConsoleHighligher : public WingSyntaxHighlighter {
    Q_OBJECT
public:
    explicit WingConsoleHighligher(QTextDocument *document = nullptr);

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
