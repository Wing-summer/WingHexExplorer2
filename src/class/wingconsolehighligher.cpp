#include "wingconsolehighligher.h"

#include <KSyntaxHighlighting/FoldingRegion>
#include <KSyntaxHighlighting/Format>
#include <KSyntaxHighlighting/Theme>

WingConsoleHighligher::WingConsoleHighligher(QTextDocument *document)
    : WingSyntaxHighlighter(document) {}

void WingConsoleHighligher::highlightBlock(const QString &text) {
    WingSyntaxHighlighter::highlightBlock(text);
}

void WingConsoleHighligher::applyFormat(
    int offset, int length, const KSyntaxHighlighting::Format &format) {
    auto blk = currentBlock();
    auto offsetv = property(blk, "cmdoff");

    bool b;
    auto off = offsetv.toInt(&b);
    if (b) {
        if (off < 0) {
            if (off == -2) {
                WingSyntaxHighlighter::applyFormat(
                    offset, length, KSyntaxHighlighting::Format());
            }
        } else {
            if (offset <= off) {
                auto div = off - offset;
                WingSyntaxHighlighter::applyFormat(
                    offset, div, KSyntaxHighlighting::Format());
                auto rest = length - div;
                if (rest <= 0) {
                    return;
                }
                WingSyntaxHighlighter::applyFormat(off, rest, format);
            } else {
                WingSyntaxHighlighter::applyFormat(offset, length, format);
            }
        }
    } else {
        WingSyntaxHighlighter::applyFormat(offset, length, format);
    }
}

void WingConsoleHighligher::applyFolding(
    int offset, int length, KSyntaxHighlighting::FoldingRegion region) {
    Q_UNUSED(offset);
    Q_UNUSED(length);
    Q_UNUSED(region);

    // Console needs no folding
}
