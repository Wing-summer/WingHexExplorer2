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

#include "qsourcecodewatcher.h"

#include "qcodemodel.h"
#include "qcodenode.h"

#include <QTimerEvent>

QList<QSourceCodeWatcher *> QSourceCodeWatcher::m_instances;

QSourceCodeWatcher *QSourceCodeWatcher::watcher(QCodeNode *t, QCodeParser *c) {
    if (!t)
        return 0;

    for (int i = 0; i < m_instances.count(); ++i) {
        if (m_instances.at(i)->m_target == t) {
            return m_instances[i];
        }
    }

    return c ? new QSourceCodeWatcher(t, c, t->model) : 0;
}

QSourceCodeWatcher::QSourceCodeWatcher(QCodeNode *n, QCodeParser *c, QObject *p)
    : QFileSystemWatcher(p), m_target(n), m_parser(c) {
    connect(this, SIGNAL(fileChanged(QString)), this,
            SLOT(sourceChanged(QString)));

    m_instances << this;

    // qDebug("%i watchers", m_instances.count());
}

QSourceCodeWatcher::~QSourceCodeWatcher() {
    m_instances.removeAll(this);

    // qDebug("%i watchers left", m_instances.count());
}

void QSourceCodeWatcher::timerEvent(QTimerEvent *e) {
    if (e->timerId() != m_timer.timerId())
        return QFileSystemWatcher::timerEvent(e);

    if (!m_parser)
        return;

    QHash<QString, char>::iterator it = m_state.begin();

    while (it != m_state.end()) {
        if (*it & Duplicate) {
            // postpone...
            *it = Recent;
            ++it;
        } else {
            // process
            // m_parser->update(m_target, it.key());
            // TODO
            it = m_state.erase(it);
        }
    }

    if (m_state.count()) {
        m_timer.start(50, this);
    }
}

void QSourceCodeWatcher::sourceChanged(const QString &filepath) {
    if (!m_target)
        return;

    m_timer.stop();

    if (!m_state.contains(filepath)) {
        m_state[filepath] = Recent;
    } else {
        m_state[filepath] = Recent | Duplicate;
    }

    m_timer.start(50, this);
}
