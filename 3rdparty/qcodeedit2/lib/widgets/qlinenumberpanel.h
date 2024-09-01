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

#ifndef _QLINE_NUMBER_PANEL_H_
#define _QLINE_NUMBER_PANEL_H_

/*!
        \file qlinenumberpanel.h
        \brief Definition of the QLineNumberPanel class

        \see QLineNumberPanel
*/

#include "qpanel.h"

class QCE_EXPORT QLineNumberPanel : public QPanel {
    Q_OBJECT

public:
    Q_PANEL(QLineNumberPanel, "Line Number Panel")

    QLineNumberPanel(QWidget *p = nullptr);
    virtual ~QLineNumberPanel();

    bool isVerboseMode() const;

    virtual QString type() const;

public slots:
    void setVerboseMode(bool y);

protected:
    virtual void editorChange(QEditor *e);
    virtual bool paint(QPainter *p, QEditor *e);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    bool m_verbose;
    int mutable m_minWidth = 0;
};

#endif // _QLINE_NUMBER_PANEL_H_
