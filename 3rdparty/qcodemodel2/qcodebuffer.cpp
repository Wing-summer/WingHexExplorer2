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

#include "qcodebuffer.h"

/*!
        \file qcodebuffer.cpp
        \brief Implementation of the QCodebuffer class.
*/

#include <QString>

/*!
        \class QCodebuffer
        \brief Code stream that operates on a static string buffer
*/

/*!
        \brief Constructs a code buffer from an Unicode string

        \note The string is converted to local8Bit()
*/
QCodeBuffer::QCodeBuffer(const QString &s)
    : QCodeStream(), iPos(0), sBuffer(s.toLocal8Bit()) {
    sBuffer.replace("\r\n", "\n");
    sBuffer.replace("\r", "\n");
}

/*!
        \brief Constructs a code buffer from a byte array
*/
QCodeBuffer::QCodeBuffer(const QByteArray &s)
    : QCodeStream(), iPos(0), sBuffer(s) {
    sBuffer.replace("\r\n", "\n");
    sBuffer.replace("\r", "\n");
}

/*!
        \brief return the char pointed to by the internal pointer and update the
   pointer
*/
char QCodeBuffer::getChar() {
    if (iPos < sBuffer.count())
        return sBuffer.at(iPos++);
    else
        return '\0';
}

/*!
        \brief Move back by one character
*/
void QCodeBuffer::ungetChar(char) {
    if (iPos > 0)
        --iPos;
}

/*!
        \return the content of the current line
*/
QByteArray QCodeBuffer::readLine() {
    char c;
    QByteArray s;

    while ((c = getChar())) {
        if (c == '\n')
            break;

        s += c;
    }

    return s;
}
