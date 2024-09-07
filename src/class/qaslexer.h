#ifndef _QAS_LEXER_H_
#define _QAS_LEXER_H_

#include "qcodelexer.h"

#include <QHash>

class QAsLexer : public QCodeLexer {
public:
    QAsLexer();
    QAsLexer(QCodeStream *s);

    bool keepMacroBlocks() const;
    void setKeepMacroBlocks(bool keep);

    virtual void setInput(QCodeStream *s, LexMode m = Normal);

    virtual QToken nextToken();
    virtual QToken previousToken();

    virtual QTokenList tokens();
    virtual int lineForToken(int tokenId, int minLine = 0) const;

    void initMacros(const QTokenList &l);

protected:
    void refreshTokens(LexMode m);

    QList<int> m_jumps;
    QList<int> m_suspicious;
    QTokenList tokenBuffer;
    bool bTokenized, bKeepMacros;
    static QHash<QToken, QToken> m_macros;
    QTokenList::const_iterator tokenPointer;
};

#endif // _QCPP_LEXER_H_
