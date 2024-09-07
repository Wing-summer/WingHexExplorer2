/****************************************************************************
**
** Copyright (C) 2006-2009 fullmetalcoder <fullmetalcoder@hotmail.fr>
**
** This file is part of the Edyuk project <http://edyuk.org>
**
** This file may be used under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation and appearing in the
** file GPL.txt included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef _QCODE_LEXER_H_
#define _QCODE_LEXER_H_

#include "qcm-config.h"

/*!
        \file qcodelexer.h
        \brief Definition of the QCodeLexer class.
*/

#include <QByteArray>
#include <QList>

class QCodeStream;

class QCM_EXPORT QCodeLexer {
public:
    enum LexMode { Normal, KeepLineSeparators };

    QCodeLexer();
    QCodeLexer(QCodeStream *s);
    virtual ~QCodeLexer();

    virtual void setInput(QCodeStream *s, LexMode m = Normal);

    virtual QToken nextToken() = 0;
    virtual QToken previousToken() = 0;

    virtual QTokenList tokens() = 0;

    virtual int lineForToken(int tokenId, int minLine = 0) const = 0;

    QString contextFile() const;

protected:
    QCodeStream *input;
};

#endif // _QCODE_LEXER_H_
