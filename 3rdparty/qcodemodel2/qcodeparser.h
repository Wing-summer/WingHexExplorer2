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

#ifndef _QCODE_PARSER_H_
#define _QCODE_PARSER_H_

#include "qcm-config.h"

#include <QStringList>

struct QCodeNode;
class QCodeModel;
class QCodeLexer;
class QCodeStream;
class QCodeNodePool;

class QCM_EXPORT QCodeParser {
    friend class QCodeLoader;

public:
    QCodeParser();
    virtual ~QCodeParser();

    virtual QString language() const = 0;
    virtual bool canParse(const QString &fn) const = 0;

    QCodeNodePool *nodePool() const;
    void setNodePool(QCodeNodePool *p);

    QCodeNode *parse(const QString &f);

    virtual QCodeLexer *lexer(QCodeStream *s) = 0;

    virtual void update(QCodeNode *n, const QString &file);
    virtual void update(QCodeNode *n, QCodeLexer *l, bool check = true) = 0;

protected:
    QCodeNode *getNode();

private:
    QCodeNodePool *m_pool;
};

#endif // !_QCODE_PARSER_H_
