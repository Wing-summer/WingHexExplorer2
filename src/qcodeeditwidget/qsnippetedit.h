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

#ifndef _QSNIPPET_EDIT_H_
#define _QSNIPPET_EDIT_H_

#include "plugin/settingpage.h"
#include "qce-config.h"

/*!
        \file qsnippetedit.h
        \brief Definition of the QSnippetEdit widget
*/

#include <QWidget>

class QSnippet;
class QSnippetManager;

namespace Ui {
class QSnippetEdit;
}

class QCE_EXPORT QSnippetEdit : public WingHex::SettingPage {
    Q_OBJECT

public:
    QSnippetEdit(QWidget *p = nullptr);
    QSnippetEdit(QSnippetManager *mgr, QWidget *p = nullptr);

    QSnippetManager *snippetManager() const;

    // PageBase interface
public:
    QIcon categoryIcon() const;
    QString name() const;
    QString id() const;

    // SettingPage interface
public:
    void apply();
    void reset();
    void cancel();

public slots:
    void setSnippetManager(QSnippetManager *mgr);

    bool maybeSave();

private slots:
    void on_lwSnippets_currentRowChanged(int idx);

    void on_leSnippetName_editingFinished();
    void on_leSnippetScope_editingFinished();

    void on_tbCreateSnippet_clicked();
    void on_tbDeleteSnippet_clicked();

    void snippetRemoved(int i);
    void snippetAdded(QSnippet *s);

private:
    Ui::QSnippetEdit *ui;

    int m_editedSnippet;
    QSnippetManager *m_manager;
};

#endif
