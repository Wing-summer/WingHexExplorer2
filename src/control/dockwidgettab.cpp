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

#include "dockwidgettab.h"

#include <QMenu>

#include "class/appmanager.h"
#include "class/showinshell.h"
#include "dialog/fileinfodialog.h"
#include "editorview.h"
#include "scripteditor.h"

DockWidgetTab::DockWidgetTab(ads::CDockWidget *DockWidget, QWidget *parent)
    : ads::CDockWidgetTab(DockWidget, parent) {}

QMenu *DockWidgetTab::buildContextMenu(QMenu *menu) {
    if (menu == nullptr) {
        menu = new QMenu(this);
    }
    auto dw = dockWidget();
    auto v = qobject_cast<EditorView *>(dw);
    if (v) {
        if (v->isCommonFile()) {
            if (v->isWorkSpace()) {
                initMenuItems(menu, QUrl::fromLocalFile(v->workSpaceName()));
            } else {
                initMenuItems(menu, v->fileNameUrl());
            }
        }
        auto a = new QAction(ICONRES("info"), tr("FileInfo"), menu);
        connect(a, &QAction::triggered, this, [this]() {
            auto editor = qobject_cast<EditorView *>(dockWidget());
            if (editor) {
                FileInfoDialog d(editor);
                d.exec();
            }
        });
        menu->addAction(a);
        menu->addSeparator();
    } else {
        auto v = qobject_cast<ScriptEditor *>(dw);
        if (v) {
            initMenuItems(menu, v->fileName());
            menu->addSeparator();
        }
    }

    return ads::CDockWidgetTab::buildContextMenu(menu);
}

void DockWidgetTab::initMenuItems(QMenu *menu, const QUrl &path) {
    Q_ASSERT(menu);
    if (path.isLocalFile() && !EditorView::isNewFileUrl(path)) {
        auto a = new QAction(ICONRES("shell"), tr("ShowInShell"), menu);
        auto file = path.toLocalFile();
        connect(a, &QAction::triggered, this, [file]() {
            ShowInShell::showInGraphicalShell(
                AppManager::instance()->mainWindow(), file, false);
        });
        menu->addAction(a);
    }
}
