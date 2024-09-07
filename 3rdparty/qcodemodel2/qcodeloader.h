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

#ifndef _QCODE_LOADER_H_
#define _QCODE_LOADER_H_

#include "qcm-config.h"

/*!
        \file qcodeloader.h
        \brief Definition of the QCodeLoader class.
*/

#include <QList>
#include <QQueue>
#include <QStringList>
#include <QThread>

struct QCodeNode;
class QCodeModel;
class QCodeParser;

class QCM_EXPORT QCodeLoader : public QThread {
    Q_OBJECT

public:
    QCodeLoader(QObject *p = 0);
    virtual ~QCodeLoader();

public slots:
    void update(QCodeNode *group, const QString &file);
    bool load(const QString &group, QStringList files, QCodeModel *dest);

    void addParser(QCodeParser *p);
    void removeParser(QCodeParser *p);

protected:
    virtual void run();

protected slots:
    virtual void process();

private:
    struct OpenRequest {
        QString group;
        QStringList files;
        QCodeModel *model;
    };

    QQueue<OpenRequest> m_request;
    QList<QCodeParser *> m_parsers;
};

#endif // !_QCODE_LOADER_H_
