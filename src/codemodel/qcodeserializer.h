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

#ifndef _QCODE_SERIALIZER_H_
#define _QCODE_SERIALIZER_H_

#include "qcm-config.h"

/*!
        \file qcodeserializer.h
        \brief Definition of the QCodeSerializer class.
*/

#include <QDateTime>
#include <QThread>

struct QCodeNode;
class QCodeModel;

class QIODevice;
class QTextStream;

class QCM_EXPORT QCodeSerializer {
public:
    QCodeSerializer();
    virtual ~QCodeSerializer();

    QCodeModel *targetModel() const;
    void setTargetModel(QCodeModel *m);

    void serialize(QCodeNode *n, QTextStream &out, int indent = 0);

    void deserialize(const QString &file, bool *ok = 0, QString *source = 0,
                     const QDateTime &t = QDateTime());

    void deserialize(QIODevice *device, bool *ok = 0, QString *source = 0,
                     const QDateTime &t = QDateTime());

private:
    QCodeModel *m_model;
};

#endif // !_QCODE_SERIALIZER_H_
