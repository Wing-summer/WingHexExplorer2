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

#include "qcodeloader.h"

/*!
        \file qcodeloader.cpp
        \brief Implementation of the QCodeLoader class.
*/

#include "qcodemodel.h"
#include "qcodenode.h"
#include "qcodeparser.h"

#include <QByteArray>
#include <QFileInfo>
#include <QTimer>

/*!
        \class QCodeLoader
        \brief A parsers management class

        QCodeLoader owns a collection of QCodeParser object
        and request them to generate project trees from files.
*/

/*!
        \brief ctor
*/
QCodeLoader::QCodeLoader(QObject *p) : QThread(p) {}

/*!
        \brief dtor
*/
QCodeLoader::~QCodeLoader() {
    // qDeleteAll(m_parsers);
}

/*!
        \brief Add a parser to the collection

        \warning The parser is not taken over. It won't get deleted
        from QCodeLoader dtor.
*/
void QCodeLoader::addParser(QCodeParser *p) {
    if (!m_parsers.contains(p))
        m_parsers << p;
}

/*!
        \brief Remove a parser from the collection

        \warning The parser is NOT deleted.
*/
void QCodeLoader::removeParser(QCodeParser *p) { m_parsers.removeAll(p); }

/*!
        \brief Update the content of a given node to
*/
void QCodeLoader::update(QCodeNode *group, const QString &file) {
    if (!group)
        return;

    if (!QFile::exists(file)) {
        QStack<QCodeNode *> l;

        l.push(group);

        while (l.count()) {
            QCodeNode *n = l.pop();

            if (n->context() == file) {
                QCodeNode *p = n->parent;

                n->detach();
                delete n;

                while (p && p->children.isEmpty() && (p != group)) {
                    n = p;
                    p = p->parent;

                    n->detach();
                    delete n;
                }
            } else {
                foreach (QCodeNode *c, n->children)
                    l.push(c);
            }
        }

        return;
    }

    // qDebug("update(%s, %s)", group->role(QCodeNode::Name).constData(),
    // qPrintable(file));

    foreach (QCodeParser *p, m_parsers) {
        if (!p->canParse(file))
            continue;

        QCodeNode *ln = 0;
        QByteArray roles("l@");
        roles += p->language().toLocal8Bit();

        foreach (QCodeNode *c, group->children) {
            if (c->roles == roles) {
                ln = c;
                break;
            }
        }

        if (!ln) {
            ln = p->getNode();
            ln->roles = roles;
            ln->attach(group);
        }

        p->update(ln, file);
        break;
    }
}

/*!
        \brief Attempt to open load class hierarchy from a set of files
        \return true on success
        \param group Name of the toplevel group which will get created on
   success \param files Files to load \param dest Destination model
*/
bool QCodeLoader::load(const QString &group, QStringList files,
                       QCodeModel *dest) {
    // qDebug("load(%s, (%s), %p)", qPrintable(group), qPrintable(files.join(",
    // ")), dest);
    bool done = false;

    QCodeNode *g = new QCodeNode;
    g->roles = QByteArray("g@") + QFileInfo(group).fileName().toLocal8Bit() +
               "@" + group.toLocal8Bit();
    dest->appendTopLevelNode(g);

    foreach (QCodeParser *p, m_parsers) {
        QStringList lf;

        foreach (QString fn, files)
            if (p->canParse(fn))
                lf << fn;

        if (lf.isEmpty())
            continue;

        done = true;
        break;
    }

    if (!done)
        return false;

    OpenRequest r;
    r.files = files;
    r.group = group;
    r.model = dest;

    m_request.enqueue(r);

    if (!isRunning())
        QTimer::singleShot(0, this, SLOT(start()));

    return true;
}

/*!
        \internal
*/
void QCodeLoader::run() {
    QTimer::singleShot(0, this, SLOT(process()));

    exec();

    // process();
}

/*!
        \internal
*/
void QCodeLoader::process() {
    while (m_request.count()) {
        OpenRequest r = m_request.dequeue();

        bool done = false;

        QCodeNode *g;
        QList<QCodeNode *> gl = r.model->topLevelNodes();

        foreach (g, gl) {
            if (g->role(QCodeNode::Context) != r.group)
                continue;

            foreach (QCodeParser *p, m_parsers) {
                QStringList lf;

                foreach (QString fn, r.files)
                    if (p->canParse(fn))
                        lf << fn;

                if (lf.isEmpty())
                    continue;

                done = true;

                QCodeNode *ln = p->getNode();
                ln->roles = QByteArray("l@") + p->language().toLocal8Bit();

                foreach (QString fn, lf) {
                    r.files.removeAll(fn);

                    p->update(ln, fn);
                }

                ln->attach(g);
            }

            break;
        }
    }

    quit();
}
