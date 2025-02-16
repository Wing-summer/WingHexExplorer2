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

#include "scriptingdialog.h"
#include "QWingRibbon/ribbontabcontent.h"
#include "Qt-Advanced-Docking-System/src/DockAreaWidget.h"
#include "Qt-Advanced-Docking-System/src/DockSplitter.h"
#include "aboutsoftwaredialog.h"
#include "class/langservice.h"
#include "class/languagemanager.h"
#include "class/qkeysequences.h"
#include "class/settingmanager.h"
#include "class/wingfiledialog.h"
#include "class/wingmessagebox.h"
#include "control/toast.h"
#include "plugin/pluginsystem.h"
#include "qcodeeditwidget/qeditconfig.h"
#include "qcodeeditwidget/qformatconfig.h"
#include "qdocumentline.h"
#include "qeditor.h"
#include "qformatscheme.h"
#include "qlinemarksinfocenter.h"
#include "settings/clangformatsetdialog.h"

#include <QDesktopServices>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPicture>
#include <QStatusBar>

constexpr auto EMPTY_FUNC = [] {};

ScriptingDialog::ScriptingDialog(QWidget *parent)
    : FramelessMainWindow(parent) {
    this->setUpdatesEnabled(false);

    // recent file manager init
    m_recentMenu = new QMenu(this);
    m_recentmanager = new RecentFileManager(m_recentMenu, true);
    connect(m_recentmanager, &RecentFileManager::triggered, this,
            [=](const RecentFileManager::RecentInfo &rinfo) {
                openFile(rinfo.fileName);
            });
    m_recentmanager->apply(this,
                           SettingManager::instance().recentScriptFiles());

    // build up UI
    buildUpRibbonBar();

    auto cw = new QWidget(this);
    cw->setSizePolicy(
        QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    auto layout = new QVBoxLayout(cw);
    layout->setContentsMargins(1, 0, 1, 0);
    layout->setSpacing(0);
    layout->addWidget(q_check_ptr(m_ribbon));

    buildUpDockSystem(cw);
    _defaultLayout = m_dock->saveState();
    layout->addWidget(m_dock, 1);

    auto status = new QStatusBar(this);
    m_status = new QLabel(this);
    status->addWidget(m_status);
    layout->addWidget(status);
    buildUpContent(cw);

    buildUpSettingDialog();

    auto lmic = QLineMarksInfoCenter::instance();
    lmic->loadMarkTypes(QCE::fetchDataFile(":/qcodeedit/marks.qxm"));
    // get symbol ID
    m_symID.insert(Symbols::BreakPoint, lmic->markTypeId("breakpoint"));
    m_symID.insert(Symbols::DbgRunCurrentLine, lmic->markTypeId("current"));
    m_symID.insert(Symbols::DbgRunHitBreakPoint,
                   lmic->markTypeId("breakpoint-current"));

    updateEditModeEnabled();

    m_Tbtneditors.value(ToolButtonIndex::EDITOR_VIEWS)->setEnabled(false);

    // ok, preparing for starting...
    this->setWindowTitle(tr("ScriptEditor"));
    this->setWindowIcon(ICONRES(QStringLiteral("script")));
    this->setMinimumSize(800, 600);

    // load saved docking layout
    auto &set = SettingManager::instance();
    m_dock->restoreState(set.scriptDockLayout());
    _savedLayout = set.scriptDockLayout();

    connect(&LangService::instance(), &LangService::onScriptEditorTip, m_status,
            [this](const QString &message) { m_status->setText(message); });

    this->setUpdatesEnabled(true);
}

ScriptingDialog::~ScriptingDialog() {}

void ScriptingDialog::initConsole() {
    Q_ASSERT(m_consoleout);

    auto fmt = new QFormatScheme(this);
    LangService::addAdditionalFormat(fmt);
    m_consoleout->document()->setFormatScheme(fmt);

    m_consoleout->init();
    auto machine = m_consoleout->machine();
    connect(machine, &ScriptMachine::onDebugFinished, this, [=] {
        this->updateRunDebugMode();
        m_callstack->updateData({});
        m_varshow->updateData({});
        m_gvarshow->updateData({});

        // clean up
        if (!(_lastCurLine.first.isEmpty() || _lastCurLine.second < 0)) {
            auto bpMark = m_symID.value(Symbols::BreakPoint);
            auto curMark = m_symID.value(Symbols::DbgRunCurrentLine);
            auto curHitMark = m_symID.value(Symbols::DbgRunHitBreakPoint);

            // remove the last mark
            if (!_lastCurLine.first.isEmpty() && _lastCurLine.second >= 0) {
                auto lastCur = QCodeEdit::managed(_lastCurLine.first);
                auto doc = lastCur->document();
                auto line = doc->line(_lastCurLine.second - 1);
                if (line.hasMark(curMark)) {
                    line.removeMark(curMark);
                } else if (line.hasMark(curHitMark)) {
                    line.removeMark(curHitMark);
                    line.addMark(bpMark);
                }
            }
            _lastCurLine.first.clear();
            _lastCurLine.second = -1;
        }

        if (_needRestart) {
            _needRestart = false;
            startDebugScript(_DebugingScript);
        } else {
            _DebugingScript.clear();
        }
    });
    auto dbg = machine->debugger();
    Q_ASSERT(dbg);
    connect(dbg, &asDebugger::onAdjustBreakPointLine, this,
            [=](const asDebugger::BreakPoint &old, int newLineNr) {
                auto editor = QCodeEdit::managed(old.name);
                if (editor) {
                    removeBreakPoint(editor, old.lineNbr - 1);
                    addBreakPoint(editor, newLineNr - 1);
                }
            });
    connect(dbg, &asDebugger::onPullVariables, this,
            [=](const QVector<asDebugger::VariablesInfo> &globalvars,
                const QVector<asDebugger::VariablesInfo> &localvars) {
                m_varshow->updateData(localvars);
                m_gvarshow->updateData(globalvars);
            });
    connect(dbg, &asDebugger::onPullCallStack, m_callstack,
            &DbgCallStackModel::updateData);
    connect(
        dbg, &asDebugger::onRunCurrentLine, this,
        [=](const QString &file, int lineNr) {
            ScriptEditor *e = m_curEditor;
#ifdef Q_OS_WIN
            if (file.compare(m_curEditor->fileName(), Qt::CaseInsensitive)) {
#else
            if (file != e->fileName()) {
#endif
                e = openFile(file);
                if (e) {
                    e->setFocus();
                    e->raise();
                } else {
                    WingMessageBox::critical(this, this->windowTitle(),
                                             tr("ScriptPermissionDenied"));
                    return;
                }
            }

            auto bpMark = m_symID.value(Symbols::BreakPoint);
            auto curMark = m_symID.value(Symbols::DbgRunCurrentLine);
            auto curHitMark = m_symID.value(Symbols::DbgRunHitBreakPoint);

            // remove the last mark
            if (!_lastCurLine.first.isEmpty() && _lastCurLine.second >= 0) {
                auto lastCur = QCodeEdit::managed(_lastCurLine.first);
                auto doc = lastCur->document();
                auto line = doc->line(_lastCurLine.second - 1);
                if (line.hasMark(curMark)) {
                    line.removeMark(curMark);
                } else if (line.hasMark(curHitMark)) {
                    line.removeMark(curHitMark);
                    line.addMark(bpMark);
                }
            }

            auto editor = e->editor();

            // add the new mark
            auto doc = editor->document();
            auto line = doc->line(lineNr - 1);
            if (line.hasMark(bpMark)) {
                line.removeMark(bpMark);
                line.addMark(curHitMark);
            } else {
                line.addMark(curMark);
            }

            editor->ensureVisible(lineNr - 1);

            _lastCurLine = {file, lineNr};
            updateRunDebugMode();
        });
    connect(dbg, &asDebugger::onDebugActionExec, this,
            [this]() { updateRunDebugMode(); });
    m_sym->setEngine(machine->engine());
}

bool ScriptingDialog::about2Close() {
    if (m_views.isEmpty()) {
        return true;
    }

    QStringList unSavedFiles;
    QList<ScriptEditor *> need2CloseView;

    for (auto &view : m_views) {
        if (view->editor()->isContentModified()) {
            unSavedFiles << view->fileName();
        } else {
            need2CloseView << view;
        }
    }

    for (auto &view : need2CloseView) {
        emit view->closeRequested();
    }

    if (unSavedFiles.isEmpty()) {
        return true;
    }

    this->show();
    this->raise();

    auto ret =
        unSavedFiles.isEmpty()
            ? QMessageBox::No
            : WingMessageBox::warning(this, qAppName(), tr("ConfirmScriptSave"),
                                      QMessageBox::Yes | QMessageBox::No |
                                          QMessageBox::Cancel);
    if (ret == QMessageBox::Yes) {
        for (auto &p : m_views) {
            emit p->closeRequested();
        }

        return m_views.isEmpty();
    } else if (ret == QMessageBox::No) {
        for (auto &p : m_views) {
            p->closeDockWidget(); // force close
        }
        return true;
    }

    return false;
}

void ScriptingDialog::saveDockLayout() {
    auto &set = SettingManager::instance();
    set.setScriptDockLayout(_savedLayout);
    set.save(SettingManager::NONE);
}

void ScriptingDialog::buildUpRibbonBar() {
    m_ribbon = new Ribbon(this);
    buildFilePage(m_ribbon->addTab(tr("File")));
    m_editStateWidgets << buildEditPage(m_ribbon->addTab(tr("Edit")));
    buildViewPage(m_ribbon->addTab(tr("View")));
    m_editStateWidgets << buildDebugPage(m_ribbon->addTab(tr("Debugger")));

    buildSettingPage(m_ribbon->addTab(tr("Setting")));
    buildAboutPage(m_ribbon->addTab(tr("About")));

    connect(m_ribbon, &Ribbon::onDragDropFiles, this,
            [=](const QStringList &files) {
                for (auto &file : files) {
                    QFileInfo info(file);
                    auto suffix = info.suffix();
                    if (info.exists() && Utilities::isTextFile(info) &&
                        (suffix.compare(QStringLiteral("as"),
                                        Qt::CaseInsensitive) == 0 ||
                         suffix.compare(QStringLiteral("anglescript"),
                                        Qt::CaseInsensitive) == 0)) {
                        openFile(file);
                    } else {
                        Toast::toast(this,
                                     NAMEICONRES(QStringLiteral("script")),
                                     tr("InvalidSourceFile"));
                    }
                }
            });
}

RibbonTabContent *ScriptingDialog::buildFilePage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();
    {
        auto pannel = tab->addGroup(tr("Basic"));
        addPannelAction(pannel, QStringLiteral("new"), tr("New"),
                        &ScriptingDialog::on_newfile, QKeySequence::New);

        addPannelAction(pannel, QStringLiteral("open"), tr("OpenF"),
                        &ScriptingDialog::on_openfile, QKeySequence::Open);

        addPannelAction(pannel, QStringLiteral("recent"), tr("RecentFiles"),
                        EMPTY_FUNC, {}, m_recentMenu);

        m_editStateWidgets << addPannelAction(pannel, QStringLiteral("reload"),
                                              tr("Reload"),
                                              &ScriptingDialog::on_reload);
    }

    {
        auto pannel = tab->addGroup(tr("Save"));

        auto a = addPannelAction(pannel, QStringLiteral("save"), tr("Save"),
                                 &ScriptingDialog::on_save, QKeySequence::Save);
        m_Tbtneditors.insert(ToolButtonIndex::SAVE_ACTION, a);
        m_editStateWidgets << a;

        a = addPannelAction(pannel, QStringLiteral("saveas"), tr("SaveAs"),
                            &ScriptingDialog::on_saveas,
                            shortcuts.keySequence(QKeySequences::Key::SAVE_AS));

        m_editStateWidgets << a;
    }
    return tab;
}

RibbonTabContent *ScriptingDialog::buildEditPage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();
    {
        auto pannel = tab->addGroup(tr("General"));

        auto a = addPannelAction(pannel, QStringLiteral("undo"), tr("Undo"),
                                 &ScriptingDialog::on_undofile);
        m_Tbtneditors.insert(ToolButtonIndex::UNDO_ACTION, a);
        setPannelActionToolTip(a, QKeySequence::Undo);

        a = addPannelAction(pannel, QStringLiteral("redo"), tr("Redo"),
                            &ScriptingDialog::on_redofile);
        m_Tbtneditors.insert(ToolButtonIndex::REDO_ACTION, a);
        setPannelActionToolTip(a,
                               shortcuts.keySequence(QKeySequences::Key::REDO));

        a = addPannelAction(pannel, QStringLiteral("cut"), tr("Cut"),
                            &ScriptingDialog::on_cutfile);
        setPannelActionToolTip(a, QKeySequence::Cut);

        a = addPannelAction(pannel, QStringLiteral("copy"), tr("Copy"),
                            &ScriptingDialog::on_copyfile);
        m_Tbtneditors.insert(ToolButtonIndex::COPY_ACTION, a);
        setPannelActionToolTip(a, QKeySequence::Copy);

        a = addPannelAction(pannel, QStringLiteral("paste"), tr("Paste"),
                            &ScriptingDialog::on_pastefile);
        setPannelActionToolTip(a, QKeySequence::Paste);

        a = addPannelAction(pannel, QStringLiteral("del"), tr("Delete"),
                            &ScriptingDialog::on_delete);
        setPannelActionToolTip(a, QKeySequence::Delete);
    }

    {
        auto pannel = tab->addGroup(tr("Lookup"));
        auto a = addPannelAction(pannel, QStringLiteral("find"), tr("Find"),
                                 &ScriptingDialog::on_findfile);
        setPannelActionToolTip(a, QKeySequence::Find);

        a = addPannelAction(pannel, QStringLiteral("replace"), tr("Replace"),
                            &ScriptingDialog::on_replace);
        setPannelActionToolTip(
            a, shortcuts.keySequence(QKeySequences::Key::REPLACE));

        a = addPannelAction(pannel, QStringLiteral("jmp"), tr("Goto"),
                            &ScriptingDialog::on_gotoline);
        setPannelActionToolTip(a,
                               shortcuts.keySequence(QKeySequences::Key::GOTO));
    }

    {
        auto pannel = tab->addGroup(tr("Format"));
        addPannelAction(pannel, QStringLiteral("codefmt"), tr("CodeFormat"),
                        &ScriptingDialog::on_codefmt);
    }

    return tab;
}

RibbonTabContent *ScriptingDialog::buildViewPage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();
    {
        auto pannel = tab->addGroup(tr("Display"));
        auto menu = new QMenu(this);
        menu->addAction(newAction(QStringLiteral("80%"), [this] {
            this->setCurrentEditorScale(0.8);
        }));
        menu->addAction(newAction(QStringLiteral("90%"), [this] {
            this->setCurrentEditorScale(0.9);
        }));
        menu->addAction(newAction(QStringLiteral("100%"), [this] {
            this->setCurrentEditorScale(1.0);
        }));
        menu->addSeparator();
        menu->addAction(newAction(QStringLiteral("120%"), [this] {
            this->setCurrentEditorScale(1.2);
        }));
        menu->addAction(newAction(QStringLiteral("150%"), [this] {
            this->setCurrentEditorScale(1.5);
        }));
        menu->addAction(newAction(QStringLiteral("200%"), [this] {
            this->setCurrentEditorScale(2.0);
        }));
        menu->addAction(newAction(QStringLiteral("250%"), [this] {
            this->setCurrentEditorScale(2.5);
        }));
        menu->addAction(newAction(QStringLiteral("300%"), [this] {
            this->setCurrentEditorScale(3.0);
        }));
        addPannelAction(pannel, QStringLiteral("scale"), tr("Scale"),
                        EMPTY_FUNC, {}, menu);
        addPannelAction(pannel, QStringLiteral("scalereset"), tr("ResetScale"),
                        [this] { this->setCurrentEditorScale(1.0); });
        m_editStateWidgets << pannel;
    }

    {
        auto pannel = tab->addGroup(tr("Window"));
        m_Tbtneditors.insert(ToolButtonIndex::EDITOR_VIEWS,
                             addPannelAction(pannel, QStringLiteral("file"),
                                             tr("Editor"), EMPTY_FUNC, {},
                                             new QMenu(this)));
        m_Tbtneditors.insert(ToolButtonIndex::TOOL_VIEWS,
                             addPannelAction(pannel, QStringLiteral("general"),
                                             tr("Tools"), EMPTY_FUNC, {},
                                             new QMenu(this)));
    }

    {
        auto pannel = tab->addGroup(tr("Layout"));
        addPannelAction(pannel, QStringLiteral("fullscreen"), tr("Fullscreen"),
                        &ScriptingDialog::on_fullScreen);
        addPannelAction(pannel, QStringLiteral("layout"), tr("RestoreLayout"),
                        &ScriptingDialog::on_restoreLayout);
    }

    return tab;
}

RibbonTabContent *ScriptingDialog::buildDebugPage(RibbonTabContent *tab) {
    auto dbgkey = QKeySequence(Qt::Key_F5);

    {
        auto pannel = tab->addGroup(tr("Debug"));

        auto a = addPannelAction(pannel, QStringLiteral("dbgrun"), tr("Run"),
                                 &ScriptingDialog::on_runscript);
        addPerformClickShortcut(a, QKeySequence(Qt::CTRL | Qt::Key_F5));
        m_Tbtneditors.insert(ToolButtonIndex::DBG_RUN_ACTION, a);

        a = addPannelAction(pannel, QStringLiteral("dbgdebug"),
                            tr("RunWithDbg"),
                            &ScriptingDialog::on_rundbgscript);
        setPannelActionToolTip(a, dbgkey);
        m_Tbtneditors.insert(ToolButtonIndex::DBG_RUN_DBG_ACTION, a);

        m_Tbtneditors.insert(ToolButtonIndex::DBG_PAUSE_ACTION,
                             addPannelAction(pannel, QStringLiteral("dbgpause"),
                                             tr("Pause"),
                                             &ScriptingDialog::on_pausescript));

        a = addPannelAction(pannel, QStringLiteral("dbgcontinue"),
                            tr("Continue"),
                            &ScriptingDialog::on_continuescript);
        setPannelActionToolTip(a, dbgkey);
        m_Tbtneditors.insert(ToolButtonIndex::DBG_CONTINUE_ACTION, a);

        m_Tbtneditors.insert(ToolButtonIndex::DBG_STOP_ACTION,
                             addPannelAction(pannel, QStringLiteral("dbgstop"),
                                             tr("Stop"),
                                             &ScriptingDialog::on_stopscript));

        m_Tbtneditors.insert(
            ToolButtonIndex::DBG_RESTART_ACTION,
            addPannelAction(pannel, QStringLiteral("dbgrestart"), tr("Restart"),
                            &ScriptingDialog::on_restartscript));

        a = addPannelAction(pannel, QStringLiteral("dbgstepinto"),
                            tr("StepInto"), &ScriptingDialog::on_stepinscript);
        addPerformClickShortcut(a, QKeySequence(Qt::Key_F11));
        m_Tbtneditors.insert(ToolButtonIndex::DBG_STEPINTO_ACTION, a);

        a = addPannelAction(pannel, QStringLiteral("dbgstepover"),
                            tr("StepOver"),
                            &ScriptingDialog::on_stepoverscript);
        addPerformClickShortcut(a, QKeySequence(Qt::Key_F10));
        m_Tbtneditors.insert(ToolButtonIndex::DBG_STEPOVER_ACTION, a);

        a = addPannelAction(pannel, QStringLiteral("dbgstepout"), tr("StepOut"),
                            &ScriptingDialog::on_stepoutscript);
        addPerformClickShortcut(a, QKeySequence(Qt::SHIFT | Qt::Key_F11));
        m_Tbtneditors.insert(ToolButtonIndex::DBG_STEPOUT_ACTION, a);

        m_editStateWidgets << pannel;
    }

    {
        auto pannel = tab->addGroup(tr("BreakPoint"));
        addPannelAction(
            pannel, QStringLiteral("scriptdbg/tbp"), tr("ToggleBreakPoint"),
            &ScriptingDialog::on_togglebreakpoint, QKeySequence(Qt::Key_F9));
        addPannelAction(pannel, QStringLiteral("scriptdbg/bp"),
                        tr("AddBreakPoint"),
                        &ScriptingDialog::on_addbreakpoint);
        addPannelAction(pannel, QStringLiteral("scriptdbg/delbp"),
                        tr("RemoveBreakPoint"),
                        &ScriptingDialog::on_removebreakpoint);
    }

    auto dbgShortCut = new QShortcut(dbgkey, this);
    connect(dbgShortCut, &QShortcut::activated, this, [this]() {
        auto runner = m_consoleout->machine();
        bool isRun = false;
        bool isDbg = false;
        bool isPaused = false;
        if (runner) {
            isRun = runner->isRunning();
            isDbg = runner->isDebugMode();
            auto dbg = runner->debugger();
            isPaused = dbg->currentState() == asDebugger::PAUSE;
        }

        if (isRun && isDbg && isPaused) {
            m_Tbtneditors[ToolButtonIndex::DBG_CONTINUE_ACTION]->animateClick();
        } else {
            m_Tbtneditors[ToolButtonIndex::DBG_RUN_DBG_ACTION]->animateClick();
        }
    });

    return tab;
}

RibbonTabContent *ScriptingDialog::buildSettingPage(RibbonTabContent *tab) {
    auto pannel = tab->addGroup(tr("Settings"));

    addPannelAction(pannel, QStringLiteral("file"), tr("Editor"),
                    [=] { m_setdialog->showConfig(QStringLiteral("Edit")); });
    addPannelAction(
        pannel, QStringLiteral("codeformat"), tr("ClangFormat"),
        [=] { m_setdialog->showConfig(QStringLiteral("ClangFormat")); });
    addPannelAction(pannel, QStringLiteral("scheme"), tr("Format"),
                    [=] { m_setdialog->showConfig(QStringLiteral("Scheme")); });
    return tab;
}

RibbonTabContent *ScriptingDialog::buildAboutPage(RibbonTabContent *tab) {
    auto pannel = tab->addGroup(tr("Info"));

    addPannelAction(pannel, QStringLiteral("soft"), tr("Software"),
                    &ScriptingDialog::on_about);

    addPannelAction(pannel, QStringLiteral("sponsor"), tr("Sponsor"),
                    &ScriptingDialog::on_sponsor);

    addPannelAction(pannel, QStringLiteral("wiki"), tr("Wiki"),
                    &ScriptingDialog::on_wiki);

    addPannelAction(pannel, QStringLiteral("qt"), tr("AboutQT"),
                    [this] { WingMessageBox::aboutQt(this); });
    return tab;
}

ads::CDockAreaWidget *
ScriptingDialog::buildUpVarShowDock(ads::CDockManager *dock,
                                    ads::DockWidgetArea area,
                                    ads::CDockAreaWidget *areaw) {
    auto vars = new QTabWidget(this);
    vars->setTabPosition(QTabWidget::South);

    auto varview = new QTableView(this);
    Utilities::applyTableViewProperty(varview);
    m_varshow = new DbgVarShowModel(varview);
    varview->setModel(m_varshow);
    vars->addTab(varview, tr("Local"));

    varview = new QTableView(this);
    Utilities::applyTableViewProperty(varview);
    m_gvarshow = new DbgVarShowModel(varview);
    varview->setModel(m_gvarshow);
    vars->addTab(varview, tr("Global"));

    auto dw = buildDockWidget(dock, QStringLiteral("Variables"),
                              tr("Variables"), vars);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
ScriptingDialog::buildUpOutputShowDock(ads::CDockManager *dock,
                                       ads::DockWidgetArea area,
                                       ads::CDockAreaWidget *areaw) {
    m_consoleout = new ScriptingConsole(this);
    m_consoleout->setMode(ScriptingConsole::Output);
    auto dw = buildDockWidget(dock, QStringLiteral("ConsoleOutput"),
                              tr("ConsoleOutput"), m_consoleout);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
ScriptingDialog::buildUpStackShowDock(ads::CDockManager *dock,
                                      ads::DockWidgetArea area,
                                      ads::CDockAreaWidget *areaw) {
    auto callstack = new QTableView(this);
    Utilities::applyTableViewProperty(callstack);
    m_callstack = new DbgCallStackModel(callstack);
    callstack->setModel(m_callstack);

    auto dw = buildDockWidget(dock, QStringLiteral("StackTrace"),
                              tr("StackTrace"), callstack);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
ScriptingDialog::buildSymbolShowDock(ads::CDockManager *dock,
                                     ads::DockWidgetArea area,
                                     ads::CDockAreaWidget *areaw) {
    Q_ASSERT(m_consoleout);
    m_sym = new ASObjTreeWidget(this);
    m_sym->header()->setSectionResizeMode(QHeaderView::Stretch);
    auto dw =
        buildDockWidget(dock, QStringLiteral("Symbol"), tr("Symbol"), m_sym);
    return dock->addDockWidget(area, dw, areaw);
}

void ScriptingDialog::buildUpDockSystem(QWidget *container) {
    Q_ASSERT(container);

    using namespace ads;
    CDockManager::setConfigFlags(CDockManager::DefaultOpaqueConfig);
    CDockManager::setConfigFlag(CDockManager::DragPreviewIsDynamic, true);
    CDockManager::setConfigFlag(CDockManager::DragPreviewShowsContentPixmap,
                                false);
    CDockManager::setConfigFlag(CDockManager::DragPreviewHasWindowFrame, false);
    CDockManager::setConfigFlag(CDockManager::EqualSplitOnInsertion, true);
    CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
    CDockManager::setConfigFlag(CDockManager::DockAreaHideDisabledButtons,
                                true);

    CDockManager::setAutoHideConfigFlags(CDockManager::DefaultAutoHideConfig);

    m_dock = new CDockManager;
    m_dock->setStyleSheet(QString());
    m_dock->setParent(this);
    connect(m_dock, &CDockManager::focusedDockWidgetChanged, this,
            [this](CDockWidget *old, CDockWidget *now) {
                Q_UNUSED(old);
                auto editview = qobject_cast<ScriptEditor *>(now);
                if (editview) {
                    swapEditor(m_curEditor, editview);
                }
                updateEditModeEnabled();
            });

    // add empty area
    auto label = new QLabel(this);

    // background image that idicates whether it's
    // running under the admin/root previlige
    QPicture backimg;
    QPainter painter(&backimg);
    painter.setOpacity(0.15);
    painter.drawPixmap(0, 0,
                       QPixmap(Utilities::isRoot()
                                   ? NAMEICONRES(QStringLiteral("iconroot"))
                                   : NAMEICONRES(QStringLiteral("icon"))));

    label->setPicture(backimg);
    label->setAlignment(Qt::AlignCenter);
    CDockWidget *CentralDockWidget =
        m_dock->createDockWidget(QStringLiteral("CentralWidget"));
    CentralDockWidget->setWidget(label);
    CentralDockWidget->setFeature(ads::CDockWidget::DockWidgetFocusable, false);
    CentralDockWidget->setFeature(ads::CDockWidget::NoTab, true);
    m_editorViewArea = m_dock->setCentralWidget(CentralDockWidget);

    // build up basic docking widgets
    auto bottomArea = buildUpOutputShowDock(m_dock, ads::BottomDockWidgetArea);

    auto splitter =
        ads::internal::findParent<ads::CDockSplitter *>(m_editorViewArea);
    if (splitter) {
        constexpr auto bottomHeight = 200;
        splitter->setSizes({height() - bottomHeight, bottomHeight});
    }

    buildUpStackShowDock(m_dock, ads::RightDockWidgetArea, bottomArea);
    auto rightArea =
        buildUpVarShowDock(m_dock, ads::RightDockWidgetArea, m_editorViewArea);
    buildSymbolShowDock(m_dock, ads::CenterDockWidgetArea, rightArea);

    // set the first tab visible
    for (auto &item : m_dock->openedDockAreas()) {
        for (int i = 0; i < item->dockWidgetsCount(); ++i) {
            auto d = item->dockWidget(i);
            if (d->features().testFlag(ads::CDockWidget::NoTab)) {
                continue;
            }
            item->setCurrentIndex(i);
            break;
        }
    }
}

bool ScriptingDialog::newOpenFileSafeCheck() {
    if (m_views.size() >=
        std::numeric_limits<decltype(m_views)::size_type>::max() - 1) {
        WingMessageBox::critical(this, tr("Error"),
                                 tr("Too much opened files"));
        return false;
    }
    return true;
}

ads::CDockWidget *ScriptingDialog::buildDockWidget(ads::CDockManager *dock,
                                                   const QString &widgetName,
                                                   const QString &displayName,
                                                   QWidget *content,
                                                   ToolButtonIndex index) {
    using namespace ads;
    auto dw = dock->createDockWidget(displayName, dock);
    dw->setObjectName(widgetName);
    dw->setFeatures(CDockWidget::DockWidgetMovable |
                    CDockWidget::DockWidgetClosable |
                    CDockWidget::DockWidgetPinnable);

    dw->setWidget(content);
    m_Tbtneditors.value(index)->menu()->addAction(dw->toggleViewAction());
    return dw;
}

QMessageBox::StandardButton ScriptingDialog::saveRequest() {
    auto ret = WingMessageBox::warning(this, qAppName(), tr("ConfirmSave"),
                                       QMessageBox::Yes | QMessageBox::No |
                                           QMessageBox::Cancel);
    return ret;
}

void ScriptingDialog::registerEditorView(ScriptEditor *editor) {
    connect(editor, &ScriptEditor::closeRequested, this, [this] {
        auto editor = qobject_cast<ScriptEditor *>(sender());
        Q_ASSERT(editor);
        Q_ASSERT(m_views.contains(editor));

        auto m = m_consoleout->machine();
        if (m->isDebugMode() && _DebugingScript == editor->fileName()) {
            if (WingMessageBox::warning(
                    this, this->windowTitle(), tr("ScriptStillRunning"),
                    QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
                return;
            }
            on_stopscript();
        }

        if (editor->editor()->isContentModified()) {
            auto ret = saveRequest();
            if (ret == QMessageBox::Cancel) {
                return;
            } else if (ret == QMessageBox::Yes) {
                if (!editor->save()) {
                    if (WingMessageBox::critical(this, this->windowTitle(),
                                                 tr("ScriptSaveFailedClose"),
                                                 QMessageBox::Yes |
                                                     QMessageBox::No) ==
                        QMessageBox::No) {
                        return;
                    }
                }
            }
        }

        m_views.removeOne(editor);
        if (currentEditor() == editor) {
            m_curEditor = nullptr;
        }
        m_Tbtneditors.value(ToolButtonIndex::EDITOR_VIEWS)
            ->setEnabled(m_views.size() != 0);

        if (m_dock->focusedDockWidget() == editor) {
            if (!m_views.isEmpty()) {
                for (auto &p : m_views) {
                    if (p != editor && p->isCurrentTab()) {
                        p->setFocus();
                    }
                }
            }
        }

        editor->deleteDockWidget();
        updateEditModeEnabled();
    });

    LangService::instance().applyLanguageSerivce(editor->editor());

    m_views.append(editor);

    auto ev = m_Tbtneditors.value(ToolButtonIndex::EDITOR_VIEWS);
    auto menu = ev->menu();
    Q_ASSERT(menu);
    auto ta = editor->toggleViewAction();
    menu->addAction(ta);
    ev->setEnabled(true);
}

ads::CDockAreaWidget *ScriptingDialog::editorViewArea() const {
    return m_dock->centralWidget()->dockAreaWidget();
}

void ScriptingDialog::updateEditModeEnabled() {
    auto editor = currentEditor();
    auto b = (editor != nullptr);

    for (auto &item : m_editStateWidgets) {
        item->setEnabled(b);
    }

    updateRunDebugMode();
}

ScriptEditor *ScriptingDialog::currentEditor() const { return m_curEditor; }

void ScriptingDialog::swapEditor(ScriptEditor *old, ScriptEditor *cur) {
    if (old == cur) {
        return;
    }

    if (old != nullptr) {
        old->disconnect(SIGNAL(onToggleMark(int)));
        auto editor = old->editor();
        editor->disconnect(SIGNAL(copyAvailable(bool)));
        editor->disconnect(SIGNAL(contentModified(bool)));
        editor->disconnect(SIGNAL(undoAvailable(bool)));
        editor->disconnect(SIGNAL(redoAvailable(bool)));
        editor->disconnect(SIGNAL(zoomed()));
    }

    auto editor = cur->editor();
    m_Tbtneditors.value(ToolButtonIndex::UNDO_ACTION)
        ->setEnabled(editor->canUndo());
    m_Tbtneditors.value(ToolButtonIndex::REDO_ACTION)
        ->setEnabled(editor->canRedo());
    m_Tbtneditors.value(ToolButtonIndex::SAVE_ACTION)
        ->setEnabled(Utilities::fileCanWrite(editor->fileName()) &&
                     editor->isContentModified());
    m_Tbtneditors.value(ToolButtonIndex::COPY_ACTION)
        ->setEnabled(editor->cursor().hasSelection());

    connect(cur, &ScriptEditor::onToggleMark, this, [=](int lineIndex) {
        auto editor = qobject_cast<ScriptEditor *>(sender());
        Q_ASSERT(editor);
        toggleBreakPoint(editor->editor(), lineIndex);
    });
    connect(editor, &QEditor::copyAvailable,
            m_Tbtneditors.value(ToolButtonIndex::COPY_ACTION),
            &QToolButton::setEnabled);
    connect(editor, &QEditor::contentModified, this, [=] {
        m_Tbtneditors.value(ToolButtonIndex::SAVE_ACTION)
            ->setEnabled(Utilities::fileCanWrite(editor->fileName()) &&
                         editor->isContentModified());
    });
    connect(editor, &QEditor::undoAvailable,
            m_Tbtneditors.value(ToolButtonIndex::UNDO_ACTION),
            &QToolButton::setEnabled);
    connect(editor, &QEditor::redoAvailable,
            m_Tbtneditors.value(ToolButtonIndex::REDO_ACTION),
            &QToolButton::setEnabled);
    connect(editor, &QEditor::zoomed, this, [=] {
        Toast::toast(this, NAMEICONRES(QStringLiteral("scale")),
                     QString::number(editor->scaleRate() * 100) +
                         QStringLiteral("%"));
    });

    m_curEditor = cur;
}

void ScriptingDialog::updateRunDebugMode(bool disable) {
    auto runner = m_consoleout->machine();
    auto enable = !disable;
    bool isRun = false;
    bool isDbg = false;
    bool isPaused = false;
    if (runner) {
        isRun = runner->isRunning();
        isDbg = runner->isDebugMode();
        auto dbg = runner->debugger();
        isPaused = dbg->currentState() == asDebugger::PAUSE;
    }
    m_Tbtneditors.value(ToolButtonIndex::DBG_RUN_ACTION)->setEnabled(!isRun);
    m_Tbtneditors.value(ToolButtonIndex::DBG_RUN_DBG_ACTION)
        ->setEnabled(!isRun);
    m_Tbtneditors.value(ToolButtonIndex::DBG_RESTART_ACTION)
        ->setEnabled(isRun && isDbg);
    m_Tbtneditors.value(ToolButtonIndex::DBG_STOP_ACTION)->setEnabled(isRun);
    m_Tbtneditors.value(ToolButtonIndex::DBG_PAUSE_ACTION)
        ->setEnabled(isRun && isDbg && !isPaused);

    auto dbgop = isRun && isDbg && isPaused && enable;
    m_Tbtneditors.value(ToolButtonIndex::DBG_CONTINUE_ACTION)
        ->setEnabled(dbgop);
    m_Tbtneditors.value(ToolButtonIndex::DBG_STEPINTO_ACTION)
        ->setEnabled(dbgop);
    m_Tbtneditors.value(ToolButtonIndex::DBG_STEPOVER_ACTION)
        ->setEnabled(dbgop);
    m_Tbtneditors.value(ToolButtonIndex::DBG_STEPOUT_ACTION)->setEnabled(dbgop);

    if (isRun) {
        if (isDbg) {
            m_status->setText(QStringLiteral("<font color=\"gold\">") +
                              tr("Debuging...") + QStringLiteral("</font>"));
        } else {
            m_status->setText(QStringLiteral("<font color=\"green\">") +
                              tr("Running...") + QStringLiteral("</font>"));
        }
    } else {
        m_status->setText({});
    }
}

ScriptEditor *ScriptingDialog::findEditorView(const QString &filename) {
    for (auto &p : m_views) {
#ifdef Q_OS_WIN
        if (p->fileName().compare(filename, Qt::CaseInsensitive) == 0) {
#else
        if (p->fileName() == filename) {
#endif
            return p;
        }
    }
    return nullptr;
}

void ScriptingDialog::setCurrentEditorScale(qreal rate) {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    editor->editor()->setScaleRate(rate);
}

bool ScriptingDialog::isCurrentDebugging() const {
    auto m = m_consoleout->machine();
    return m && m->isDebugMode();
}

ScriptEditor *ScriptingDialog::openFile(const QString &filename) {
    auto e = findEditorView(filename);
    if (e) {
        e->raise();
        e->setFocus();
        return e;
    }

    auto editor = new ScriptEditor(this);
    auto res = editor->openFile(filename);
    if (!res) {
        WingMessageBox::critical(this, tr("Error"), tr("FilePermission"));
        return nullptr;
    }

    registerEditorView(editor);
    m_dock->addDockWidget(ads::CenterDockWidgetArea, editor, editorViewArea());

    return editor;
}

void ScriptingDialog::runDbgCommand(asDebugger::DebugAction action) {
    updateRunDebugMode(true);
    auto machine = m_consoleout->machine();
    if (machine->isDebugMode()) {
        auto dbg = machine->debugger();
        dbg->runDebugAction(action);
    }
}

void ScriptingDialog::buildUpSettingDialog() {
    m_setdialog = new SettingDialog(this);

    auto edit = new QEditConfig(m_setdialog);
    m_setdialog->addPage(edit);

    auto clang = new ClangFormatSetDialog(m_setdialog);
    m_setdialog->addPage(clang);

    auto &langsev = LangService::instance();
    auto scheme = new QFormatConfig(langsev.formatSchemes(),
                                    langsev.defaultSchemeName(), m_setdialog);
    m_setdialog->addPage(scheme);

    m_setdialog->build();
}

void ScriptingDialog::startDebugScript(const QString &fileName) {
    m_ribbon->setCurrentIndex(3);

    m_consoleout->clear();

    // add breakpoints
    auto marks = QLineMarksInfoCenter::instance()->marks(
        fileName, m_symID.value(Symbols::BreakPoint));
    auto dbg = m_consoleout->machine()->debugger();
    for (auto &bp : marks) {
        dbg->addFileBreakPoint(bp.file, bp.line);
    }

    _DebugingScript = fileName;
    PluginSystem::instance().scriptPragmaBegin();

    auto view = findEditorView(fileName);
    Q_ASSERT(view);
    view->setReadOnly(true);
    m_consoleout->machine()->executeScript(fileName, true);
    view->setReadOnly(false);

    updateRunDebugMode();
}

void ScriptingDialog::addBreakPoint(QEditor *editor, int lineIndex) {
    Q_ASSERT(editor);
    auto bpMark = m_symID.value(Symbols::BreakPoint);
    auto curLine = lineIndex + 1;
    auto dbg = m_consoleout->machine()->debugger();

    if (m_consoleout->machine()->isDebugMode()) {
        auto line = editor->document()->line(lineIndex);
        auto hitCur = m_symID.value(Symbols::DbgRunHitBreakPoint);
        auto curSym = m_symID.value(Symbols::DbgRunCurrentLine);

        if (line.hasMark(curSym)) {
            line.removeMark(curSym);
            line.addMark(hitCur);
            dbg->addFileBreakPoint(editor->fileName(), curLine);
        } else {
            if (!line.hasMark(bpMark)) {
                line.addMark(bpMark);
                dbg->addFileBreakPoint(editor->fileName(), curLine);
            }
        }
    } else {
        auto line = editor->document()->line(lineIndex);
        if (!line.hasMark(bpMark)) {
            line.addMark(bpMark);
            dbg->addFileBreakPoint(editor->fileName(), curLine);
        }
    }
}

void ScriptingDialog::removeBreakPoint(QEditor *editor, int lineIndex) {
    Q_ASSERT(editor);
    auto bpMark = m_symID.value(Symbols::BreakPoint);
    auto curLine = lineIndex + 1;

    auto dbg = m_consoleout->machine()->debugger();
    if (m_consoleout->machine()->isDebugMode()) {
        auto line = editor->document()->line(lineIndex);
        auto hitCur = m_symID.value(Symbols::DbgRunHitBreakPoint);
        auto curSym = m_symID.value(Symbols::DbgRunCurrentLine);

        if (line.hasMark(hitCur)) {
            line.removeMark(hitCur);
            line.addMark(curSym);
            dbg->removeFileBreakPoint(editor->fileName(), curLine);
        } else {
            if (line.hasMark(bpMark)) {
                line.removeMark(bpMark);
                dbg->removeFileBreakPoint(editor->fileName(), curLine);
            }
        }
    } else {
        auto line = editor->document()->line(lineIndex);
        if (line.hasMark(bpMark)) {
            line.removeMark(bpMark);
            dbg->removeFileBreakPoint(editor->fileName(), curLine);
        }
    }
}

void ScriptingDialog::toggleBreakPoint(QEditor *editor, int lineIndex) {
    Q_ASSERT(editor);
    auto curLine = lineIndex + 1;

    auto dbg = m_consoleout->machine()->debugger();
    if (m_consoleout->machine()->isDebugMode()) {
        auto line = editor->document()->line(lineIndex);
        auto bpMark = m_symID.value(Symbols::BreakPoint);
        auto hitCur = m_symID.value(Symbols::DbgRunHitBreakPoint);
        auto curSym = m_symID.value(Symbols::DbgRunCurrentLine);

        if (line.hasMark(hitCur)) {
            line.removeMark(hitCur);
            line.addMark(curSym);
            dbg->removeFileBreakPoint(editor->fileName(), curLine);
        } else if (line.hasMark(curSym)) {
            line.removeMark(curSym);
            line.addMark(hitCur);
            dbg->addFileBreakPoint(editor->fileName(), curLine);
        } else {
            if (line.hasMark(bpMark)) {
                line.removeMark(bpMark);
                dbg->removeFileBreakPoint(editor->fileName(), curLine);
            } else {
                line.addMark(bpMark);
                dbg->addFileBreakPoint(editor->fileName(), curLine);
            }
        }
    } else {
        auto bpMark = m_symID.value(Symbols::BreakPoint);
        QLineMark mrk(editor->fileName(), curLine, bpMark);
        QLineMarksInfoCenter::instance()->toggleLineMark(mrk);
        auto line = editor->document()->line(lineIndex);
        if (line.hasMark(bpMark)) {
            dbg->addFileBreakPoint(editor->fileName(), curLine);
        } else {
            dbg->removeFileBreakPoint(editor->fileName(), curLine);
        }
    }
}

void ScriptingDialog::on_newfile() {
    if (!newOpenFileSafeCheck()) {
        return;
    }
    auto filename = WingFileDialog::getSaveFileName(
        this, tr("ChooseFile"), m_lastusedpath,
        QStringLiteral("AngelScript (*.as *.angelscript)"));
    if (!filename.isEmpty()) {
        m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();

        auto e = findEditorView(filename);

        // create an empty file
        QFile f(filename);
        f.open(QFile::WriteOnly | QFile::Text);
        f.write("int main() {\n    return 0;\n}\n");
        f.close();

        if (e) {
            if (_DebugingScript == e->fileName()) {
                on_stopscript();
            }
            e->reload();
            e->raise();
            e->setFocus();
            return;
        }

        auto editor = new ScriptEditor(this);
        auto res = editor->openFile(filename);
        if (!res) {
            WingMessageBox::critical(this, tr("Error"), tr("FilePermission"));
            return;
        }

        registerEditorView(editor);
        m_dock->addDockWidget(ads::CenterDockWidgetArea, editor,
                              editorViewArea());
    }
}

void ScriptingDialog::on_openfile() {
    if (!newOpenFileSafeCheck()) {
        return;
    }
    auto filename = WingFileDialog::getOpenFileName(
        this, tr("ChooseFile"), m_lastusedpath,
        QStringLiteral("AngelScript (*.as *.angelscript)"));
    if (!filename.isEmpty()) {
        m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();
        openFile(filename);
    }

    RecentFileManager::RecentInfo info;
    info.fileName = filename;
    m_recentmanager->addRecentFile(info);
}

void ScriptingDialog::on_reload() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    if (editor->reload()) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("reload")),
                     tr("ReloadSuccessfully"));
    } else {
        WingMessageBox::critical(this, tr("Error"), tr("ReloadUnSuccessfully"));
    }
}

void ScriptingDialog::on_save() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }

    if (editor->fileName().isEmpty()) {
        on_saveas();
        return;
    }

    auto res = editor->save();
    if (res) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("save")),
                     tr("SaveSuccessfully"));
    } else {
        WingMessageBox::critical(this, tr("Error"), tr("FilePermission"));
        return;
    }
}

void ScriptingDialog::on_saveas() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }

    QString lastPath = editor->fileName();
    if (lastPath.isEmpty()) {
        lastPath = m_lastusedpath;
    }

    auto filename = WingFileDialog::getSaveFileName(
        this, tr("ChooseSaveFile"), m_lastusedpath,
        QStringLiteral("AngelScript (*.as *.angelscript)"));
    if (filename.isEmpty())
        return;
    m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();

    auto res = editor->save(filename);
    if (res) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("saveas")),
                     tr("SaveSuccessfully"));
    } else {
        WingMessageBox::critical(this, tr("Error"), tr("SaveUnSuccessfully"));
    }

    RecentFileManager::RecentInfo info;
    info.fileName = filename;
    m_recentmanager->addRecentFile(info);
}

void ScriptingDialog::on_undofile() {
    auto e = currentEditor();
    if (e) {
        e->editor()->undo();
    }
}

void ScriptingDialog::on_redofile() {
    auto e = currentEditor();
    if (e) {
        e->editor()->redo();
    }
}

void ScriptingDialog::on_cutfile() {
    auto e = currentEditor();
    if (e) {
        e->editor()->cut();
    }
}

void ScriptingDialog::on_copyfile() {
    auto e = currentEditor();
    if (e) {
        e->editor()->copy();
    }
}

void ScriptingDialog::on_pastefile() {
    auto e = currentEditor();
    if (e) {
        e->editor()->paste();
    }
}

void ScriptingDialog::on_delete() {
    auto e = currentEditor();
    if (e) {
        e->editor()->cursor().removeSelectedText();
    }
}

void ScriptingDialog::on_findfile() {
    auto e = currentEditor();
    if (e) {
        e->editor()->find();
    }
}

void ScriptingDialog::on_replace() {
    auto e = currentEditor();
    if (e) {
        e->editor()->replace();
    }
}

void ScriptingDialog::on_gotoline() {
    auto e = currentEditor();
    if (e) {
        e->editor()->gotoLine();
    }
}

void ScriptingDialog::on_codefmt() {
    auto e = currentEditor();
    if (e) {
        if (!e->formatCode()) {
            Toast::toast(this, NAMEICONRES(QStringLiteral("codefmt")),
                         tr("FormatCodeFailed"));
        }
    }
}

void ScriptingDialog::on_about() { AboutSoftwareDialog().exec(); }

void ScriptingDialog::on_sponsor() {
    // Github is not easy to access for Chinese people,
    // Gitee mirror instead
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    if (LanguageManager::instance().defaultLocale().territory() ==
#else
    if (LanguageManager::instance().defaultLocale().country() ==
#endif
        QLocale::China) {
        QDesktopServices::openUrl(
            QUrl(QStringLiteral("https://gitee.com/wing-cloud/"
                                "WingHexExplorer2#%E6%8D%90%E5%8A%A9")));
    } else {
        QDesktopServices::openUrl(
            QUrl(QStringLiteral("https://github.com/Wing-summer/"
                                "WingHexExplorer2#%E6%8D%90%E5%8A%A9")));
    }
}

void ScriptingDialog::on_wiki() {
    QDesktopServices::openUrl(QUrl(
        QStringLiteral("https://www.angelcode.com/angelscript/sdk/docs/manual/"
                       "doc_script.html")));
}

void ScriptingDialog::on_fullScreen() {
    if (this->isFullScreen()) {
        this->showMaximized();
    } else {
        this->showFullScreen();
    }
}

void ScriptingDialog::on_restoreLayout() {
    m_dock->restoreState(_defaultLayout);
}

void ScriptingDialog::on_runscript() {
    auto editor = currentEditor();
    if (editor) {
        auto e = editor->editor();
        if (!e->save()) {
            WingMessageBox::critical(this, qAppName(),
                                     tr("CannotSave2RunScript"));
            return;
        }
        m_consoleout->clear();
        PluginSystem::instance().scriptPragmaBegin();

        editor->setReadOnly(true);
        m_consoleout->machine()->executeScript(e->fileName());
        editor->setReadOnly(false);
        updateRunDebugMode();
    }
}

void ScriptingDialog::on_rundbgscript() {
    auto editor = currentEditor();
    if (editor) {
        auto e = editor->editor();
        if (!e->save()) {
            WingMessageBox::critical(this, qAppName(),
                                     tr("CannotSave2RunScript"));
            return;
        }
        startDebugScript(e->fileName());
    }
}

void ScriptingDialog::on_pausescript() { runDbgCommand(asDebugger::PAUSE); }

void ScriptingDialog::on_continuescript() {
    runDbgCommand(asDebugger::CONTINUE);
}

void ScriptingDialog::on_stopscript() { runDbgCommand(asDebugger::ABORT); }

void ScriptingDialog::on_restartscript() {
    on_stopscript();
    _needRestart = true;
}

void ScriptingDialog::on_stepinscript() {
    runDbgCommand(asDebugger::STEP_INTO);
}

void ScriptingDialog::on_stepoutscript() {
    runDbgCommand(asDebugger::STEP_OUT);
}

void ScriptingDialog::on_stepoverscript() {
    runDbgCommand(asDebugger::STEP_OVER);
}

void ScriptingDialog::on_togglebreakpoint() {
    auto editor = currentEditor();
    if (editor) {
        auto e = editor->editor();
        toggleBreakPoint(e, e->cursor().lineNumber());
    }
}

void ScriptingDialog::on_addbreakpoint() {
    auto editor = currentEditor();
    if (editor) {
        auto e = editor->editor();
        addBreakPoint(e, e->cursor().lineNumber());
    }
}

void ScriptingDialog::on_removebreakpoint() {
    auto editor = currentEditor();
    if (editor) {
        auto e = editor->editor();
        removeBreakPoint(e, e->cursor().lineNumber());
    }
}

void ScriptingDialog::closeEvent(QCloseEvent *event) {
    auto runner = m_consoleout->machine();
    if (runner->isRunning()) {
        if (WingMessageBox::warning(
                this, this->windowTitle(), tr("ScriptStillRunning"),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
            event->ignore();
            return;
        }
        on_stopscript();
    }

    _savedLayout = m_dock->saveState();

    auto &set = SettingManager::instance();
    set.setRecentScriptFiles(m_recentmanager->saveRecent());

    FramelessMainWindow::closeEvent(event);
}

SettingDialog *ScriptingDialog::settingDialog() const { return m_setdialog; }
