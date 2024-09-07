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

#include "qcodeparser.h"

/*!
        \file qcodeparser.cpp
        \brief Implementation of the QCodeParser class.
*/

#include "qcodedevice.h"
#include "qcodelexer.h"
#include "qcodemodel.h"
#include "qcodenode.h"
#include "qcodenodepool.h"
#include "qcodestream.h"

#include "qsourcecodewatcher.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTime>

/*!
        \class QCodeParser
        \brief Base class for code parsers

        \see QCodeLexer
        \see QCodeLoader
*/

/*!
        \brief ctor
*/
QCodeParser::QCodeParser() : m_pool(0) {}

/*!
        \brief dtor
*/
QCodeParser::~QCodeParser() {}

/*!
        \return The node pool to which this parser has access
*/
QCodeNodePool *QCodeParser::nodePool() const { return m_pool; }

/*!
        \brief Set the node pool from which the parser should
        allocate its node (a null pointer causes heap allocation
        to be used all the time.)
*/
void QCodeParser::setNodePool(QCodeNodePool *p) { m_pool = p; }

/*!
        \return A valid QCodeNode object
        If the parser has access to a nodePool(), it allocates
        the new node from it, otherwise it allocates it on heap.
*/
QCodeNode *QCodeParser::getNode() {
    return m_pool ? m_pool->alloc() : new QCodeNode;
}

/*!
        \brief Parse a single file and construct code tree
        \param f file to parse
        \return The group node (named after the file) containing parsing result
*/
QCodeNode *QCodeParser::parse(const QString &f) {
    if (!QFile::exists(f) || !QFileInfo(f).isReadable())
        return 0;

    QCodeDevice stream(f);
    QCodeNode *node = getNode();
    QCodeLexer *lex = lexer(&stream);

    node->roles = QByteArray("g@") + f.toLocal8Bit() + "@" + f.toLocal8Bit();

    update(node, lex);

    delete lex;

    return node;
}

/*!
        \brief Update the content of a node according to the content of a file
        \param n node to update (most probably a group...)
        \param file file from which to extract code hierarchy
*/
void QCodeParser::update(QCodeNode *n, const QString &file) {
    if (!QFile::exists(file) || !QFileInfo(file).isReadable()) {
        QStack<QCodeNode *> l;

        l.push(n);

        while (l.count()) {
            QCodeNode *tn = l.pop();

            if (tn->context() == file) {
                QCodeNode *p = tn->parent;

                tn->detach();
                delete tn;

                while (p && p->children.isEmpty() &&
                       (p->type() != QCodeNode::Group)) {
                    tn = p;
                    p = p->parent;

                    tn->detach();
                    delete tn;
                }
            } else {
                foreach (QCodeNode *c, tn->children)
                    l.push(c);
            }
        }

        return;
    }

    // qDebug("updating %s in %s", qPrintable(file),
    // n->role(QCodeNode::Name).constData());

    QCodeDevice stream(file);

    QSourceCodeWatcher *w = QSourceCodeWatcher::watcher(n, this);

    if (w && !w->files().contains(file))
        w->addPath(file);

    QCodeLexer *lex = lexer(&stream);

    update(n, lex);

    delete lex;
}
