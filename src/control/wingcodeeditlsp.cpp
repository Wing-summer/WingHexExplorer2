#include "wingcodeeditlsp.h"

WingCodeEditLSP::WingCodeEditLSP(QWidget *parent) : WingCodeEdit(parent) {
    connect(document(), &QTextDocument::contentsChange, this,
            &WingCodeEditLSP::onDocumentContentsChange);
}

WingCodeEditLSP::~WingCodeEditLSP() {}

void WingCodeEditLSP::onDocumentContentsChange(int position, int charsRemoved,
                                               int charsAdded) {
    // This slot is called for *local* and *programmatic* document changes.
    // We must ignore changes that were programmatically applied from LSP edits.

    // Compute LSP start position from offset
    int startLine = 0, startChar = 0;
    offsetToPosition(document(), position, startLine, startChar);

    // Compute end position: position + charsRemoved
    int endOffset = position + charsRemoved;
    int endLine = 0, endChar = 0;
    offsetToPosition(document(), endOffset, endLine, endChar);

    // Get inserted text (if any)
    QString inserted = docTextMid(document(), position, charsAdded);

    // Build LSP change object: include range, rangeLength (number of UTF-16
    // code units removed), text
    LSP::TextDocumentContentChangeEvent e;
    e.range.start.line = startLine;
    e.range.start.character = startChar;
    e.range.end.line = endLine;
    e.range.end.character = endChar;
    e.rangeLength = charsRemoved;
    e.text = inserted;

    // Emit signal for client to handle (or subscribe)
    Q_EMIT incrementalDidChange(e);
}

void WingCodeEditLSP::offsetToPosition(QTextDocument *doc, int offset,
                                       int &outLine, int &outChar) {
    outLine = 0;
    outChar = 0;
    if (!doc)
        return;
    int total = doc->characterCount();
    if (offset < 0)
        offset = 0;
    if (offset > total)
        offset = total;

    QTextBlock blk = doc->findBlock(offset);
    if (!blk.isValid()) {
        // fallback compute by splitting
        QString up = doc->toPlainText().left(offset);
        outLine = up.count('\n');
        int last = up.lastIndexOf('\n');
        outChar = (last == -1) ? up.size() : up.size() - (last + 1);
        return;
    }
    outLine = blk.blockNumber();
    outChar = offset - blk.position();
}

QString WingCodeEditLSP::docTextMid(QTextDocument *doc, int offset, int len) {
    if (!doc)
        return QString();
    QString whole = doc->toPlainText();
    if (offset < 0)
        offset = 0;
    if (offset > whole.size())
        offset = whole.size();
    if (len < 0)
        len = 0;
    if (offset + len > whole.size())
        len = whole.size() - offset;
    return whole.mid(offset, len);
}
