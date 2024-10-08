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

#ifndef _QCODE_EDIT_H_
#define _QCODE_EDIT_H_

#include "qce-config.h"

/*!
        \file qcodeedit.h
        \brief Definition of the QCodeEdit class
*/

#include <QList>
#include <QPointer>

class QMenu;
class QPanel;
class QEditor;
class QWidget;
class QString;
class QAction;
class QPanelLayout;
class QPanelWatcher;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QMetaMethodArgument>
#define Q_COMMAND QList<QMetaMethodArgument>()
#else
#include <QGenericArgument>
#define Q_COMMAND QList<QGenericArgument>()
#endif

class QCE_EXPORT QCodeEdit {
    friend class QPanelWatcher;

public:
    enum Position { West, North, South, East };

    QCodeEdit(QWidget *p = nullptr);
    QCodeEdit(bool actions, QWidget *p = nullptr);
    QCodeEdit(const QString &layout, QWidget *p = nullptr);
    QCodeEdit(const QString &layout, bool actions, QWidget *p = nullptr);
    virtual ~QCodeEdit();

    QEditor *editor() const;
    QPanelLayout *panelLayout() const;

    QAction *addPanel(QPanel *panel, Position pos, bool _add = false);
    QAction *addPanel(const QString &name, Position pos, bool _add = false);

    bool hasPanel(const QString &type) const;
    QList<QPanel *> panels(const QString &type = QString()) const;

    QAction *toggleViewAction(QPanel *p) const;

    void sendPanelCommand(const QString &type, const char *signature,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                          const QList<QMetaMethodArgument> &args = Q_COMMAND);
#else
                          const QList<QGenericArgument> &args = Q_COMMAND);
#endif

    static QCodeEdit *manager(QEditor *e);
    static QEditor *managed(const QString &f);

    // protected:
    QCodeEdit(QEditor *e, QPanelLayout *p);
    QCodeEdit(QEditor *e, const QString &l);

private:
    QPanelWatcher *m_watcher;
    QPointer<QEditor> m_editor;
    QPointer<QPanelLayout> m_layout;

    QMenu *m_panelsMenu;
    QList<QAction *> m_actions;

    static QList<QCodeEdit *> m_instances;
};

#endif // _QCODE_EDIT_H_
