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

#include "qcodenodepool.h"

/*!
        \file qcodenodepool.cpp
        \brief Implementation of the QCodeNodePool class.
*/

#include <cstdlib>

#include "qcodenode.h"

/*!
        \class QCodeNodePool
        \brief Custom allocator for QCodeNode objects
*/

QCodeNodePool::QCodeNodePool(int size)
    : m_size(size), m_last(0), m_free(0), m_own(true),
      m_nodes(static_cast<QCodeNode *>(malloc(size * sizeof(QCodeNode)))) {}

QCodeNodePool::QCodeNodePool(void *data, int size)
    : m_size(size), m_last(0), m_free(0), m_own(false),
      m_nodes(static_cast<QCodeNode *>(data)) {}

QCodeNodePool::~QCodeNodePool() {
    if (m_own && m_nodes)
        free(m_nodes);
}

int QCodeNodePool::count() const { return m_last; }

int QCodeNodePool::bufferSize() const { return m_size; }

int QCodeNodePool::occupied() const { return (100 * m_last) / m_size; }

int QCodeNodePool::remaining() const {
    return (100 * (m_size - m_last)) / m_size;
}

QCodeNode *QCodeNodePool::alloc() {
    if (m_last == m_size) {
        // try to fill blanks...
        if (m_free > 0) {
        }

        // fallback use malloc()
        return new QCodeNode(0);
    } else {
        return new (m_nodes + m_last++) QCodeNode(this);
    }

    return 0;
}

void QCodeNodePool::free(QCodeNode *n) {
    if ((n < m_nodes) || (n >= (m_nodes + m_size))) {
        // not from pool... fallback to regular allocator
        delete n;
    } else {

        if ((void *)(n + 1) == (void *)(m_nodes + m_last)) {
            --m_last;

            QCodeNode &n = m_nodes[m_last];

            while (!n.m_pool && !n.parent && !n.model && (m_last > 0) &&
                   (m_free > 0)) {
                --m_free;
                --m_last;

                n = m_nodes[m_last];
            }

        } else {
            ++m_free;
        }
    }
}
