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

#ifndef _QCODE_DEVICE_H_
#define _QCODE_DEVICE_H_

#include "qcodestream.h"

/*!
        \file qcodedevice.h
        \brief Definition of the QCodeDevice class.
*/

class QIODevice;

class QCM_EXPORT QCodeDevice : public QCodeStream {
public:
    QCodeDevice(const QString &f);

    virtual char getChar();
    virtual void ungetChar(char c);

    virtual QByteArray readLine();

    virtual bool tryFetchOtherContext(const QString &f);

private:
    int m_pos, m_beg;
    QByteArray buffer;
};

#endif // _QCODE_DEVICE_H_
