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

#ifndef _QCOMPLETION_WIDGET_H_
#define _QCOMPLETION_WIDGET_H_

#include <QListView>
#include <QPointer>

#include "class/qcodenode.h"
#include "qdocumentcursor.h"
#include "qeditor.h"

class QCodeCompletionModel;

class QCodeCompletionWidget : public QListView {
    Q_OBJECT

public:
    enum FilterFlag {
        None,

        Public = 1,
        Protected = 2,
        Private = 4,

        KeepAllVisibilities = Public | Protected | Private,

        IsPointer = 8,
        IsStatic = 16,
        IsConst = 32,

        KeepConst = 64,
        KeepStatic = 128,
        KeepDtor = 256,
        KeepCtor = 512,

        KeepAllFunctions = KeepConst | KeepStatic | KeepDtor | KeepCtor,

        KeepEnums = 1024,
        KeepEnumerators = 2048,
        KeepSubTypes = 4096,

        KeepAllSub = KeepEnums | KeepEnumerators | KeepSubTypes,

        KeepAll = KeepAllFunctions | KeepAllSub | KeepAllVisibilities
    };

    typedef QFlags<FilterFlag> Filter;

    QCodeCompletionWidget(QEditor *p = nullptr);

    QEditor *editor() const;
    void setEditor(QEditor *e);

    bool hasEntries() const;

    Filter filter() const;
    void setFilter(Filter filter);

    QString prefix() const;
    void setPrefix(const QString &prefix);

    QStringList completions() const;
    void setCompletions(const QList<QCodeNode *> &nodes);

    void setTemporaryNodes(const QList<QCodeNode *> &l);

    bool isCompleting() const;

public slots:
    void popup();
    void clear();

protected:
    virtual void showEvent(QShowEvent *e);
    virtual void hideEvent(QHideEvent *e);

    virtual void keyPressEvent(QKeyEvent *e);

    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);

private slots:
    void changed();
    void complete(const QModelIndex &index);

private:
    void adjustGeometry();

    int offset;
    QCodeCompletionModel *pModel;
    QPointer<QObject> pEditor;
    QList<QCodeNode *> m_temps;
    bool _completing = false;
};

#endif // _QCOMPLETION_WIDGET_H_
