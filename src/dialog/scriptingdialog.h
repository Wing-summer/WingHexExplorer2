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

#include "control/scriptingconsole.h"
#include "dialog/settingdialog.h"
#include "framelessmainwindow.h"

#include "QWingRibbon/ribbon.h"
#include "QWingRibbon/ribbonbuttongroup.h"
#include "Qt-Advanced-Docking-System/src/DockManager.h"
#include "Qt-Advanced-Docking-System/src/DockWidget.h"
#include "class/recentfilemanager.h"
#include "control/scripteditor.h"
#include "model/dbgbreakpointmodel.h"
#include "model/dbgcallstackmodel.h"
#include "model/dbgvarshowmodel.h"
#include "qlanguagefactory.h"
#include "utilities.h"

#include <QMessageBox>
#include <QShortcut>
#include <QStatusBar>
#include <QTableView>
#include <QTextBrowser>

class ScriptingDialog : public FramelessMainWindow {
    Q_OBJECT
private:
    enum ToolButtonIndex : uint {
        UNDO_ACTION,
        REDO_ACTION,
        SAVE_ACTION,
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

    enum Symbols {
        BreakPoint,
        ConditionBreakPoint,
        DisabledBreakPoint,
        DbgRunCurrentLine,
        DbgRunHitBreakPoint,
    };

public:
    explicit ScriptingDialog(QWidget *parent = nullptr);
    virtual ~ScriptingDialog();

    void initConsole();

    bool about2Close();

    void saveDockLayout();

private:
    void buildUpRibbonBar();
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
    buildUpBreakpointShowDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                              ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpOutputShowDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                          ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpStackShowDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                         ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpWatchDock(ads::CDockManager *dock, ads::DockWidgetArea area,
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
        auto a = new QToolButton(pannel);
        a->setText(title);
        a->setIcon(icon);
        setPannelActionToolTip(a, shortcut);

        if (!shortcut.isEmpty()) {
            auto shortCut = new QShortcut(shortcut, this);
            shortCut->setContext(Qt::WindowShortcut);
            connect(shortCut, &QShortcut::activated, a, &QToolButton::click);
        }

        a->setMenu(menu);
        if (menu) {
            a->setPopupMode(QToolButton::InstantPopup);
        }
        connect(a, &QToolButton::clicked, this, slot);
        pannel->addButton(a);
        return a;
    }

    template <typename Func>
    inline QAction *newAction(const QString &title, Func &&slot,
                              const QKeySequence &shortcut = QKeySequence()) {
        auto a = new QAction;
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
    void registerEditorView(ScriptEditor *editor);
    inline ads::CDockAreaWidget *editorViewArea() const;

    void updateEditModeEnabled();
    ScriptEditor *currentEditor() const;
    void swapEditor(ScriptEditor *old, ScriptEditor *cur);

    void updateRunDebugMode();

    ScriptEditor *findEditorView(const QString &filename);

    void setCurrentEditorScale(qreal rate);

    bool isCurrentDebugging() const;

    ScriptEditor *openFile(const QString &filename);

    void runDbgCommand(asDebugger::DebugAction action);

    void buildUpSettingDialog();

    void startDebugScript(const QString &fileName);

    void addBreakPoint(QEditor *editor, int lineIndex);

    void removeBreakPoint(QEditor *editor, int lineIndex);

    void toggleBreakPoint(QEditor *editor, int lineIndex);

private slots:
    void on_newfile();
    void on_openfile();
    void on_reload();

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

    void on_setting();
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
    ads::CDockManager *m_dock = nullptr;
    ads::CDockAreaWidget *m_editorViewArea = nullptr;

    QByteArray _defaultLayout;
    QByteArray _savedLayout;

    ScriptEditor *m_curEditor = nullptr;
    QList<QWidget *> m_editStateWidgets;

    QMap<ToolButtonIndex, QToolButton *> m_Tbtneditors;

    QMenu *m_recentMenu = nullptr;
    RecentFileManager *m_recentmanager = nullptr;
    Ribbon *m_ribbon = nullptr;
    SettingDialog *m_setdialog = nullptr;

    size_t m_newIndex = 1;
    QPair<QString, int> _lastCurLine = {QString(), -1};

    QList<ScriptEditor *> m_views;
    QMap<Symbols, int> m_symID;
    QString m_lastusedpath;

    // widgets for debugging
    ScriptingConsole *m_consoleout = nullptr;
    DbgVarShowModel *m_varshow = nullptr;
    DbgVarShowModel *m_gvarshow = nullptr;
    DbgBreakpointModel *m_breakpoints = nullptr;
    DbgVarShowModel *m_watch = nullptr;
    DbgCallStackModel *m_callstack = nullptr;

    QString _DebugingScript;

    QStatusBar *m_status = nullptr;
};

#endif // SCRIPTINGDIALOG_H
