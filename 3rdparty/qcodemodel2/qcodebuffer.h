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

#ifndef _QCODE_BUFFER_H_
#define _QCODE_BUFFER_H_

#include "qcodestream.h"

/*!
        \file qcodebuffer.h
        \brief Definition of the QCodeBuffer class.
*/

#include <QByteArray>

class QCM_EXPORT QCodeBuffer : public QCodeStream {
public:
    QCodeBuffer(const QString &s);
    QCodeBuffer(const QByteArray &s);

    virtual char getChar();
    virtual void ungetChar(char c);

    virtual QByteArray readLine();

private:
    int iPos;
    QByteArray sBuffer;
};

#endif // _QCODE_BUFFER_H_
