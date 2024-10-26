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

#include "qcodeproxymodel.h"

/*!
        \file qcodeproxymodel.cpp
        \brief Implementation of the QCodeProxyModel class.
*/

#include "qcodemodel.h"
#include "qcodenode.h"

static QList<int> priority = QList<int>()
                             << QCodeNode::Group << QCodeNode::Language
                             << QCodeNode::Namespace
                             << QCodeNode::Class
                             // << QCodeNode::Struct << QCodeNode::Union
                             << QCodeNode::Enum << QCodeNode::Typedef
                             << QCodeNode::Function << QCodeNode::Variable;

/*!
        \class QCodeProxyModel
        \brief Special proxy model for code models
*/

QCodeProxyModel::QCodeProxyModel(QObject *p) : QSortFilterProxyModel(p) {
    setDynamicSortFilter(true);
}

QCodeProxyModel::~QCodeProxyModel() {}

bool QCodeProxyModel::lessThan(const QModelIndex &left,
                               const QModelIndex &right) const {
    int lt = priority.indexOf(
            sourceModel()->data(left, QCodeModel::TypeRole).toInt()),
        rt = priority.indexOf(
            sourceModel()->data(right, QCodeModel::TypeRole).toInt());

    // if ( lt == rt )
    //	return QSortFilterProxyModel::lessThan(left, right);

    /*
    foreach ( int p, priority )
    {
            if ( lt == p )
                    return false;
            else if ( rt == p )
                    return true;
    }

    int lv = sourceModel()->data(left, QCodeModel::VisibilityRole).toInt(),
            rv = sourceModel()->data(right, QCodeModel::VisibilityRole).toInt();

    if ( lv != rv )
            return lv > rv;
    */

    QString ld = sourceModel()->data(left, Qt::DisplayRole).toString(),
            rd = sourceModel()->data(right, Qt::DisplayRole).toString();

    // return !((lt == rt) ? (QString::localeAwareCompare(ld, rd) < 0) : (lt <
    // rt));
    return !((lt == rt) ? (ld.toUpper() < rd.toUpper()) : (lt < rt));
}
