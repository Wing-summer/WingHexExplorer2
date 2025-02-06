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
            initMenuItems(menu, v->fileName());
        }
        auto a = new QAction(ICONRES("info"), tr("FileInfo"), menu);
        connect(a, &QAction::triggered, this, [this]() {
            auto editor = qobject_cast<EditorView *>(dockWidget());
            if (editor) {
                FileInfoDialog d(editor->fileName());
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

void DockWidgetTab::initMenuItems(QMenu *menu, const QString &path) {
    Q_ASSERT(menu);
    auto a = new QAction(ICONRES("shell"), tr("ShowInShell"), menu);
    connect(a, &QAction::triggered, this, [path]() {
        ShowInShell::showInGraphicalShell(AppManager::instance()->mainWindow(),
                                          path, false);
    });
    menu->addAction(a);
}
