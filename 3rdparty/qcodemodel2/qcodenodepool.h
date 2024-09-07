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

#ifndef _QCODE_NODE_POOL_H_
#define _QCODE_NODE_POOL_H_

#include "qcm-config.h"

/*!
        \file qcodenodepool.h
        \brief Definition of the QCodeNodePool class.
*/

struct QCodeNode;

class QCM_EXPORT QCodeNodePool {
public:
    QCodeNodePool(int size);
    QCodeNodePool(void *data, int size);
    ~QCodeNodePool();

    int count() const;
    int bufferSize() const;
    int occupied() const;
    int remaining() const;

    QCodeNode *alloc();
    void free(QCodeNode *n);

private:
    int m_size, m_last, m_free;

    bool m_own;
    QCodeNode *m_nodes;
};

#endif // !_QCODE_NODE_POOL_H_
