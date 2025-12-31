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

#ifndef SCRIPTINGDIALOG_H
#define SCRIPTINGDIALOG_H

#include "control/asidbtreeview.h"
#include "control/asobjtreewidget.h"
#include "control/scriptingconsole.h"
#include "dialog/settingdialog.h"
#include "framelessmainwindow.h"

#include "QWingRibbon/ribbon.h"
#include "QWingRibbon/ribbonbuttongroup.h"
#include "Qt-Advanced-Docking-System/src/DockManager.h"
#include "Qt-Advanced-Docking-System/src/DockWidget.h"
#include "WingCodeEdit/wingsquiggleinfomodel.h"
#include "class/recentfilemanager.h"
#include "control/scripteditor.h"
#include "model/asidbwatchmodel.h"
#include "model/dbgcallstackmodel.h"
#include "utilities.h"

#include <QKeySequence>
#include <QMessageBox>
#include <QShortcut>
#include <QStatusBar>
#include <QTableView>
#include <QTextBrowser>

class ScriptingDialog : public FramelessMainWindow, public EditorsCtl {
    Q_OBJECT

private:
    enum ToolButtonIndex : uint {
        UNDO_ACTION,
        REDO_ACTION,
        COPY_ACTION,
        DBG_RUN_ACTION,
        DBG_RUN_DBG_ACTION,
        DBG_PAUSE_ACTION,
        DBG_CONTINUE_ACTION,
        DBG_STOP_ACTION,
        DBG_RESTART_ACTION,
        DBG_STEPINTO_ACTION,
        DBG_STEPOVER_ACTION,
        DBG_STEPOUT_ACTION,
        EDITOR_VIEWS,
        TOOL_VIEWS
    };

public:
    explicit ScriptingDialog(SettingDialog *setdlg, QWidget *parent = nullptr);
    virtual ~ScriptingDialog();

    void initConsole();

    bool about2Close();

private:
    void buildUpRibbonBar();
    void saveDockLayout();

    RibbonTabContent *buildFilePage(RibbonTabContent *tab);
    RibbonTabContent *buildEditPage(RibbonTabContent *tab);
    RibbonTabContent *buildViewPage(RibbonTabContent *tab);
    RibbonTabContent *buildDebugPage(RibbonTabContent *tab);
    RibbonTabContent *buildSettingPage(RibbonTabContent *tab);
    RibbonTabContent *buildAboutPage(RibbonTabContent *tab);

    ads::CDockAreaWidget *
    buildUpVarShowDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                       ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpVarWatchDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                        ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpOutputShowDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                          ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpStackShowDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                         ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildSymbolShowDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                        ads::CDockAreaWidget *areaw = nullptr);

    ads::CDockAreaWidget *
    buildDiagnosisDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                       ads::CDockAreaWidget *areaw = nullptr);

    void buildUpDockSystem(QWidget *container);

    bool newOpenFileSafeCheck();

    ads::CDockWidget *buildDockWidget(ads::CDockManager *dock,
                                      const QString &widgetName,
                                      const QString &displayName,
                                      QWidget *content,
                                      ToolButtonIndex index = TOOL_VIEWS);

    QMessageBox::StandardButton saveRequest();

private:
    void addPerformClickShortcut(QToolButton *btn,
                                 const QKeySequence &shortcut) {
        Q_ASSERT(btn && !shortcut.isEmpty());
        auto shortCut = new QShortcut(shortcut, this);
        shortCut->setContext(Qt::WindowShortcut);
        connect(shortCut, &QShortcut::activated, btn,
                [btn]() { btn->animateClick(); });
        setPannelActionToolTip(btn, shortcut);
    }

    template <typename Func>
    inline QToolButton *
    addPannelAction(RibbonButtonGroup *pannel, const QString &iconName,
                    const QString &title, Func &&slot,
                    const QKeySequence &shortcut = QKeySequence(),
                    QMenu *menu = nullptr) {
        return addPannelAction(pannel, ICONRES(iconName), title, slot, shortcut,
                               menu);
    }

    template <typename Func>
    inline QToolButton *
    addPannelAction(RibbonButtonGroup *pannel, const QIcon &icon,
                    const QString &title, Func &&slot,
                    const QKeySequence &shortcut = QKeySequence(),
                    QMenu *menu = nullptr) {
        Q_ASSERT(pannel);
        Q_ASSERT(pannel);
        auto a = new QToolButton(pannel);
#if QT_VERSION <= QT_VERSION_CHECK(6, 6, 0)
        if (menu) {
            a->setText(title + QChar(' ') + QChar(char16_t(0x25BC)));
        } else
#endif
        {
            a->setText(title);
        }

        a->setIcon(icon);
        setPannelActionToolTip(a, shortcut);

        if (!shortcut.isEmpty()) {
            auto shortCut = new QShortcut(shortcut, this);
            shortCut->setContext(Qt::WindowShortcut);
            connect(shortCut, &QShortcut::activated, a, &QToolButton::click);
        }

        a->setMenu(menu);
        if (menu) {
#if QT_VERSION > QT_VERSION_CHECK(6, 6, 0)
            a->setArrowType(Qt::DownArrow);
#endif
            a->setPopupMode(QToolButton::InstantPopup);
        }
        connect(a, &QToolButton::clicked, this, slot);
        pannel->addButton(a);
        return a;
    }

    template <typename Func>
    inline QAction *newAction(const QString &title, Func &&slot,
                              const QKeySequence &shortcut = QKeySequence()) {
        auto a = new QAction(this);
        a->setText(title);
        a->setShortcutVisibleInContextMenu(true);
        a->setShortcut(shortcut);
        connect(a, &QAction::triggered, this, slot);
        return a;
    }

    inline void
    setPannelActionToolTip(QToolButton *action,
                           const QKeySequence &shortcut = QKeySequence()) {
        Q_ASSERT(action);
        auto title = action->text();
        action->setToolTip(
            shortcut.isEmpty()
                ? QStringLiteral("<p align=\"center\">%1</p>").arg(title)
                : QStringLiteral(
                      "<p align=\"center\">%1</p><p align=\"center\">%2</p>")
                      .arg(title, shortcut.toString()));
    }

private:
    void registerMark();
    QPixmap markFromPath(const QString &name);

    void registerEditorView(ScriptEditor *editor);
    inline ads::CDockAreaWidget *editorViewArea() const;

    void updateEditModeEnabled();
    ScriptEditor *currentEditor() const;
    void swapEditor(ScriptEditor *old, ScriptEditor *cur);
    void updateWindowTitle();
    void addRecentFile(const QString &fileName);

    void updateRunDebugMode(bool disable = false);

    ScriptEditor *findEditorView(const QString &filename);

    bool isCurrentDebugging() const;

    void runDbgCommand(asIDBAction action);

    void startDebugScript(const QString &fileName);

    void addBreakPoint(ScriptEditor *editor, int line);
    void removeBreakPoint(ScriptEditor *editor, int line);
    void toggleBreakPoint(ScriptEditor *editor, int line);

    void updateCursorPosition();

    void reloadEditor(ScriptEditor *editor);

    void updateUI();

public:
    ScriptEditor *openFile(const QString &filename);

    bool try2CloseScriptViews(const LinkedList<ScriptEditor *> views);

private slots:
    void on_newfile();
    void on_openfile();
    void on_reload();
    void on_exit();

    void on_save();
    void on_saveas();

    void on_undofile();
    void on_redofile();
    void on_cutfile();
    void on_copyfile();
    void on_pastefile();
    void on_delete();

    void on_findfile();
    void on_replace();
    void on_gotoline();

    void on_codefmt();

    void on_about();
    void on_sponsor();
    void on_wiki();

    void on_fullScreen();
    void on_restoreLayout();

    void on_runscript();
    void on_rundbgscript();
    void on_pausescript();
    void on_continuescript();
    void on_stopscript();
    void on_restartscript();
    void on_stepinscript();
    void on_stepoutscript();
    void on_stepoverscript();

    void on_togglebreakpoint();
    void on_addbreakpoint();
    void on_removebreakpoint();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    ScriptEditor *createFakeEditor(const QString &fileName,
                                   const QString &text);
    void destoryFakeEditor();

    void destoryEditor(ScriptEditor *editor);

private:
    ads::CDockManager *m_dock = nullptr;
    ads::CDockAreaWidget *m_editorViewArea = nullptr;
    QStatusBar *m_status = nullptr;

    QByteArray _defaultLayout;
    QByteArray _savedLayout;

    ads::CDockWidget *m_outConsole = nullptr;
    ads::CDockWidget *m_dbgVarView = nullptr;
    ads::CDockWidget *m_dbgWatchView = nullptr;

    ScriptEditor *m_curEditor = nullptr;
    QList<QMetaObject::Connection> m_curConnections;
    WingSquiggleInfoModel *_squinfoModel = nullptr;
    QList<QWidget *> m_editStateWidgets;

    QMap<ToolButtonIndex, QToolButton *> m_Tbtneditors;

    QMenu *m_recentMenu = nullptr;
    RecentFileManager *m_recentmanager = nullptr;
    Ribbon *m_ribbon = nullptr;
    SettingDialog *m_setdialog = nullptr;

    size_t m_newIndex = 1;
    QPair<QString, int> _lastCurLine = {QString(), -1};
    bool _needRestart = false;

    QString m_lastusedpath;

    QHash<QString, AsPreprocesser::Result> _curDbgData;
    QVector<ScriptEditor *> _reditors;

    // widgets for debugging
    ScriptingConsole *m_consoleout = nullptr;
    asIDBTreeView *m_varshow = nullptr;
    asIDBTreeView *m_gvarshow = nullptr;
    AsIDBWatchModel *m_watchModel = nullptr;
    DbgCallStackModel *m_callstack = nullptr;
    ASObjTreeWidget *m_sym = nullptr;
    ScriptEditor *_fakeEditor = nullptr;

    QLabel *_status = nullptr;

    // EditorsCtl interface
public:
    virtual bool save(EditorInfo *info) override;
    virtual void discard(EditorInfo *info) override;
};

#endif // SCRIPTINGDIALOG_H
