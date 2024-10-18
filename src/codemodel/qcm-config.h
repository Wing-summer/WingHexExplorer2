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

#ifndef _QCM_CONFIG_H_
#define _QCM_CONFIG_H_

#include <qglobal.h>

#ifndef QCM_EXPORT
#define QCM_EXPORT
#endif

class QByteArray;

template <typename T>
class QList;

typedef QByteArray QToken;
typedef QList<QToken> QTokenList;

#endif // _QCM_CONFIG_H_
