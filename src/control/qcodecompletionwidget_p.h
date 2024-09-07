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

#ifndef _QCOMPLETION_WIDGET_P_H_
#define _QCOMPLETION_WIDGET_P_H_

#include "qcodecompletionwidget.h"

#include <QAbstractListModel>
#include <QPointer>

struct QCodeNode;
class QCodeModel;

class QCodeCompletionModel : public QAbstractListModel {
    Q_OBJECT

public:
    QCodeCompletionModel(QObject *p = 0);

    void clear();
    void update();

    QString prefix() const;
    void setPrefix(const QString &prefix);

    QCodeCompletionWidget::Filter filter() const;
    void setFilter(QCodeCompletionWidget::Filter filter);

    QList<QCodeNode *> focusNodes() const;
    void setFocusNodes(const QList<QCodeNode *> &node);

    static bool match(QCodeNode *node, QCodeCompletionWidget::Filter filter,
                      const QByteArray &prefix = QByteArray());

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

signals:
    void changed() const;

    void prefixChanged(const QString &newPrefix);
    void filterChanged(QCodeCompletionWidget::Filter f);

private:
    void forceUpdate() const;

    mutable bool bUpdate;
    QByteArray m_prefix;
    QList<QCodeNode *> m_nodes;
    QCodeCompletionWidget::Filter m_filter;

    mutable QList<QCodeNode *> m_visibles;
};

#endif // !_QCOMPLETION_WIDGET_P_H_
