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

#ifndef _QCODE_VIEW_H_
#define _QCODE_VIEW_H_

#include "qcm-config.h"

/*!
        \file qcodeview.h
        \brief Definition of the QCodeView class.
*/

#include <QTreeView>

class QCodeModel;

class QCM_EXPORT QCodeView : public QTreeView {
    Q_OBJECT

public:
    QCodeView(QWidget *p = nullptr);
    QCodeView(QCodeModel *m, QWidget *p = nullptr);
    virtual ~QCodeView();

    virtual void setModel(QAbstractItemModel *model);

signals:
    void actionRequested(const QString &action, const QStringList &params);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *e);

protected slots:
    void indexActivated(const QModelIndex &idx);

private:
    QCodeModel *m_model;
};

#endif // !_QCODE_VIEW_H_
