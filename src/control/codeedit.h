/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#ifndef CODEEDIT_H
#define CODEEDIT_H

#include "WingCodeEdit/wingcodeedit.h"
#include "control/gotolinewidget.h"

class SearchReplaceWidget;

class CodeEdit : public WingCodeEdit {
    Q_OBJECT

public:
    explicit CodeEdit(QWidget *parent = nullptr);

public:
    void showSearchReplaceBar(bool show, bool replace);
    void showGotoBar(bool show);

    SearchReplaceWidget *searchWidget() const;

signals:
    void contentModified(bool b);

protected slots:
    virtual void onCompletion(const QModelIndex &index) override;

private:
    void addEditorBasicShortcut();
    void addMoveLineShortcut();

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void applyEditorSetStyle();

private:
    SearchReplaceWidget *m_searchWidget;
    GotoLineWidget *m_gotoWidget;
};

#endif // CODEEDIT_H
