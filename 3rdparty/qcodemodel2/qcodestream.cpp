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

#include "qcodestream.h"

/*!
        \file qcodestream.cpp
        \brief Implementation of the QCodeStream class.
*/

/*!
        \class QCodeStream
        \brief Base class for code input streams
*/

QString QCodeStream::contextFile() const {
    // qDebug("    <= context query : %s", qPrintable(sContext));
    return sContext;
}

void QCodeStream::setContextFile(const QString &f) {
    // qDebug("    => context switch : %s", qPrintable(f));
    sContext = f;
}

bool QCodeStream::tryFetchOtherContext(const QString &) { return false; }
