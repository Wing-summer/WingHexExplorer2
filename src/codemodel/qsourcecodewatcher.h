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

#ifndef _QSOURCE_CODE_WATCHER_H_
#define _QSOURCE_CODE_WATCHER_H_

#include "qcm-config.h"

#include <QFileSystemWatcher>
#include <QHash>
#include <QTimer>

struct QCodeNode;
class QCodeParser;

class QCM_EXPORT QSourceCodeWatcher : public QFileSystemWatcher {
    friend struct QCodeNode;

    Q_OBJECT

public:
    static QSourceCodeWatcher *watcher(QCodeNode *t, QCodeParser *c = 0);

protected:
    QSourceCodeWatcher(QCodeNode *n, QCodeParser *c, QObject *p = 0);
    virtual ~QSourceCodeWatcher();

    virtual void timerEvent(QTimerEvent *e);

private slots:
    void sourceChanged(const QString &filepath);

private:
    enum State { Recent = 1, Duplicate = 2 };

    QHash<QString, char> m_state;

    QBasicTimer m_timer;
    QCodeNode *m_target;
    QCodeParser *m_parser;

    static QList<QSourceCodeWatcher *> m_instances;
};

#endif // !_QSOURCE_CODE_WATCHER_H_
