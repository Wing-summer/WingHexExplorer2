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

#ifndef _QCODE_STREAM_H_
#define _QCODE_STREAM_H_

#include "qcm-config.h"

/*!
        \file qcodestream.h
        \brief Definition of the QCodeStream class.
*/

#include <QString>

class QByteArray;

class QCM_EXPORT QCodeStream {
public:
    virtual ~QCodeStream() {}

    virtual char getChar() = 0;
    virtual void ungetChar(char c) = 0;

    virtual QByteArray readLine() = 0;

    virtual QString contextFile() const;
    virtual void setContextFile(const QString &f);

    virtual bool tryFetchOtherContext(const QString &f);

private:
    QString sContext;
};

#endif // !_QCODE_STREAM_H_
