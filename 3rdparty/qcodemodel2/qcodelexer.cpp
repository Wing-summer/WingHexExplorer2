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

#include "qcodelexer.h"

/*!
        \file qcodelexer.cpp
        \brief Implementation of the QCodeLexer class.
*/

#include "qcodestream.h"

/*!
        \class QCodeLexer
        \brief Base class for code lexers
*/

QCodeLexer::QCodeLexer() {}

QCodeLexer::QCodeLexer(QCodeStream *s) : input(s) {}

QCodeLexer::~QCodeLexer() {}

void QCodeLexer::setInput(QCodeStream *s, LexMode m) {
    Q_UNUSED(m)

    input = s;
}

QString QCodeLexer::contextFile() const {
    return input ? input->contextFile() : QString();
}
