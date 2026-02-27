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

#include "DockAreaWidget.h"

#include "class/appmanager.h"
#include "class/showinshell.h"
#include "dialog/fileinfodialog.h"
#include "editorview.h"
#include "scripteditor.h"

template <typename T,
          typename = std::enable_if<std::is_same_v<T, EditorView> ||
                                    std::is_same_v<T, ScriptEditor>>>
void createCloseExt(ads::CDockWidget *dockWidget, QMenu *menu,
                    QWidget *parent) {
    Q_ASSERT(parent);
    auto cmenu = new QMenu(DockWidgetTab::tr("CloseGroup"), parent);
    cmenu->addAction(DockWidgetTab::tr("Left"), parent, [dockWidget]() {
        auto app = AppManager::instance();
        LinkedList<T *> cviews;

        auto area = dockWidget->dockAreaWidget();
        auto total = area->dockWidgetsCount();
        for (qsizetype i = 0; i < total; ++i) {
            auto w = area->dockWidget(i);
            if (w == dockWidget) {
                break;
            }

            if (w->isClosed()) {
                continue;
            }

            auto view = qobject_cast<T *>(w);
            if (view) {
                cviews.append(view);
            }
        }

        if (cviews.isEmpty()) {
            return;
        }

        if constexpr (std::is_same_v<T, EditorView>) {
            app->mainWindow()->try2CloseHexViews(cviews);
        } else {
            app->mainWindow()->try2CloseScriptViews(cviews);
        }
    });
    cmenu->addAction(DockWidgetTab::tr("Right"), parent, [dockWidget]() {
        auto app = AppManager::instance();
        LinkedList<T *> cviews;

        auto area = dockWidget->dockAreaWidget();
        auto total = area->dockWidgetsCount();
        for (qsizetype i = total - 1; i >= 0; --i) {
            auto w = area->dockWidget(i);
            if (w == dockWidget) {
                break;
            }

            if (w->isClosed()) {
                continue;
            }

            auto view = qobject_cast<T *>(w);
            if (view) {
                cviews.append(view);
            }
        }

        if (cviews.isEmpty()) {
            return;
        }

        if constexpr (std::is_same_v<T, EditorView>) {
            app->mainWindow()->try2CloseHexViews(cviews);
        } else {
            app->mainWindow()->try2CloseScriptViews(cviews);
        }
    });
    cmenu->addAction(DockWidgetTab::tr("All"), parent, [dockWidget]() {
        auto app = AppManager::instance();
        LinkedList<T *> cviews;

        auto area = dockWidget->dockAreaWidget();
        auto total = area->dockWidgetsCount();
        for (qsizetype i = 0; i < total; ++i) {
            auto w = area->dockWidget(i);
            if (w->isClosed()) {
                continue;
            }

            auto view = qobject_cast<T *>(w);
            if (view) {
                cviews.append(view);
            }
        }

        if constexpr (std::is_same_v<T, EditorView>) {
            app->mainWindow()->try2CloseHexViews(cviews);
        } else {
            app->mainWindow()->try2CloseScriptViews(cviews);
        }
    });
    cmenu->addAction(DockWidgetTab::tr("Others"), parent, [dockWidget]() {
        auto app = AppManager::instance();
        LinkedList<T *> cviews;
        for (auto &view : T::instances()) {
            if (view->isClosed()) {
                continue;
            }
            if (view != dockWidget) {
                cviews.append(view);
            }
        }
        if constexpr (std::is_same_v<T, EditorView>) {
            app->mainWindow()->try2CloseHexViews(cviews);
        } else {
            app->mainWindow()->try2CloseScriptViews(cviews);
        }
    });
    menu->addMenu(cmenu);
}

DockWidgetTab::DockWidgetTab(ads::CDockWidget *DockWidget, QWidget *parent)
    : ads::CDockWidgetTab(DockWidget, parent) {}

QMenu *DockWidgetTab::buildContextMenu(QMenu *menu) {
    if (menu == nullptr) {
        menu = new QMenu(this);
    }
    auto dw = dockWidget();
    enum class State { None, EditorView, ScriptView } state = State::None;

    auto v = qobject_cast<EditorView *>(dw);
    if (v) {
        state = State::EditorView;
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
            state = State::ScriptView;
            initMenuItems(menu, v->lspFileNameURL());
            menu->addSeparator();
        }
    }

    Q_ASSERT(ads::CDockManager::testAutoHideConfigFlag(
        ads::CDockManager::AutoHideFeatureEnabled));
    menu->addAction(ads::CDockWidgetTab::tr("Pin"), this,
                    SLOT(autoHideDockWidget()));
    auto dmenu = menu->addMenu(ads::CDockWidgetTab::tr("Pin To..."));
    createAutoHideToAction(ads::CDockWidgetTab::tr("Top"), ads::SideBarTop,
                           dmenu);
    createAutoHideToAction(ads::CDockWidgetTab::tr("Left"), ads::SideBarLeft,
                           dmenu);
    createAutoHideToAction(ads::CDockWidgetTab::tr("Right"), ads::SideBarRight,
                           dmenu);
    createAutoHideToAction(ads::CDockWidgetTab::tr("Bottom"),
                           ads::SideBarBottom, dmenu);

    menu->addSeparator();

    if (state == State::EditorView) {
        createCloseExt<EditorView>(dw, menu, this);
    } else if (state == State::ScriptView) {
        createCloseExt<ScriptEditor>(dw, menu, this);
    }

    menu->addAction(ads::CDockWidgetTab::tr("Close"), this,
                    SIGNAL(closeRequested()));

    return menu;
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

QAction *DockWidgetTab::createAutoHideToAction(const QString &Title,
                                               ads::SideBarLocation Location,
                                               QMenu *Menu) {
    auto a = Menu->addAction(Title);
    a->setProperty(ads::internal::LocationProperty, Location);
    connect(a, SIGNAL(triggered(bool)), this,
            SLOT(onAutoHideToActionClicked()));
    return a;
}
