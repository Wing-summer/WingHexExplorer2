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

#ifndef _QCODE_PROXY_MODEL_H_
#define _QCODE_PROXY_MODEL_H_

#include "qcm-config.h"

/*!
        \file qcodeproxymodel.h
        \brief Definition of the QCodeProxyModel class.
*/

#include <QSortFilterProxyModel>

class QCM_EXPORT QCodeProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    QCodeProxyModel(QObject *p = nullptr);
    virtual ~QCodeProxyModel();

protected:
    virtual bool lessThan(const QModelIndex &left,
                          const QModelIndex &right) const;
};

#endif // !_QCODE_PROXY_MODEL_H_
