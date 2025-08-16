#ifndef WINGCODEEDITLSP_H
#define WINGCODEEDITLSP_H

#include "WingCodeEdit/wingcodeedit.h"
#include "class/lsp.h"

#include <QList>

class WingCodeEditLSP : public WingCodeEdit {
    Q_OBJECT
public:
    explicit WingCodeEditLSP(QWidget *parent = nullptr);
    virtual ~WingCodeEditLSP();

signals:
    void incrementalDidChange(const LSP::TextDocumentContentChangeEvent &e);

private slots:
    void onDocumentContentsChange(int position, int charsRemoved,
                                  int charsAdded);

private:
    void offsetToPosition(QTextDocument *doc, int offset, int &outLine,
                          int &outChar);

    QString docTextMid(QTextDocument *doc, int offset, int len);
};

#endif // WINGCODEEDITLSP_H
