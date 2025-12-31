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
#include "Qt-Advanced-Docking-System/src/DockWidgetTab.h"
#include "WingCodeEdit/wingsymbolcenter.h"
#include "aboutsoftwaredialog.h"
#include "class/angellsp.h"
#include "class/languagemanager.h"
#include "class/pluginsystem.h"
#include "class/qkeysequences.h"
#include "class/scriptmachine.h"
#include "class/scriptmanager.h"
#include "class/settingmanager.h"
#include "class/wingfiledialog.h"
#include "class/winginputdialog.h"
#include "class/wingmessagebox.h"
#include "control/toast.h"
#include "dialog/mutisavedialog.h"
#include "model/asidbwatchmodel.h"

#include <QDesktopServices>
#include <QHeaderView>
#include <QLabel>
#include <QListView>
#include <QPainter>
#include <QPicture>
#include <QStatusBar>
#include <QToolTip>

constexpr auto EMPTY_FUNC = [] {};

ScriptingDialog::ScriptingDialog(SettingDialog *setdlg, QWidget *parent)
    : FramelessMainWindow(parent), m_setdialog(setdlg) {
    this->setUpdatesEnabled(false);

    // recent file manager init
    m_recentMenu = new QMenu(this);
    m_recentmanager = new RecentFileManager(m_recentMenu, true);
    connect(m_recentmanager, &RecentFileManager::triggered, this,
            [=](const RecentFileManager::RecentInfo &rinfo) {
                openFile(rinfo.url.toLocalFile());
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

    m_status = new QStatusBar(this);
    _status = new QLabel(this);
    m_status->addPermanentWidget(_status);
    layout->addWidget(m_status);
    buildUpContent(cw);

    registerMark();

    updateEditModeEnabled();

    m_Tbtneditors.value(ToolButtonIndex::EDITOR_VIEWS)->setEnabled(false);

    // ok, preparing for starting...
    updateWindowTitle();
    this->setWindowIcon(ICONRES(QStringLiteral("script")));
    this->setMinimumSize(800, 600);

    // load saved docking layout
    auto &set = SettingManager::instance();
    m_dock->restoreState(set.scriptDockLayout());
    _savedLayout = set.scriptDockLayout();
    m_lastusedpath = set.lastUsedScriptPath();
    m_watchModel->reloadExpressionList(set.watchExpressions());

    ScriptMachine::RegCallBacks callbacks;
    callbacks.getInputFn = [this]() -> QString {
        return m_consoleout->getInput();
    };
    callbacks.clearFn = [this]() { m_consoleout->clearConsole(); };
    callbacks.printMsgFn = [this](const ScriptMachine::MessageInfo &message) {
        m_consoleout->onOutput(message);
    };
    ScriptMachine::instance().registerCallBack(ScriptMachine::Scripting,
                                               callbacks);

    auto &lsp = AngelLsp::instance();
    connect(&lsp, &AngelLsp::serverStarted, this, [this]() {
        // only happened when restarting
        for (auto &view : ScriptEditor::instances()) {
            view->onReconnectLsp();
            view->setCompleterEnabled(true);
        }
    });
    connect(&lsp, &AngelLsp::serverExited, this, [this]() {
        for (auto &view : ScriptEditor::instances()) {
            view->setCompleterEnabled(false);
        }
    });
    connect(
        &lsp, &AngelLsp::diagnosticsPublished, this,
        [this](const QString &url, const QList<LSP::Diagnostics> &diagnostics) {
            if (url.startsWith(QStringLiteral("dev"))) {
                // a device not a file
                return;
            }
            QUrl path(url);
            if (path.isValid()) {
                auto fileName = path.toLocalFile();
                auto view = findEditorView(fileName);
                if (view) {
                    auto editor = view->editor();
                    editor->clearSquiggle();
                    auto lsps = [](LSP::DiagnosticSeverity s)
                        -> WingCodeEdit::SeverityLevel {
                        switch (s) {
                        case LSP::DiagnosticSeverity::None:
                            return WingCodeEdit::SeverityLevel::Information;
                        case LSP::DiagnosticSeverity::Error:
                            return WingCodeEdit::SeverityLevel::Error;
                        case LSP::DiagnosticSeverity::Warning:
                            return WingCodeEdit::SeverityLevel::Warning;
                        case LSP::DiagnosticSeverity::Information:
                            return WingCodeEdit::SeverityLevel::Information;
                        case LSP::DiagnosticSeverity::Hint:
                            return WingCodeEdit::SeverityLevel::Hint;
                        }
                        return WingCodeEdit::SeverityLevel::Information;
                    };
                    for (auto &d : diagnostics) {
                        editor->addSquiggle(
                            lsps(d.severity),
                            {d.range.start.line + 1, d.range.start.character},
                            {d.range.end.line + 1, d.range.end.character},
                            d.message);
                    }
                    editor->highlightAllSquiggle();
                }
            }
        });

    this->setUpdatesEnabled(true);
    this->setAttribute(Qt::WA_DeleteOnClose);
}

ScriptingDialog::~ScriptingDialog() {
    ScriptMachine::instance().registerCallBack(ScriptMachine::Scripting, {});
}

void ScriptingDialog::initConsole() {
    Q_ASSERT(m_consoleout);

    m_consoleout->init();
    auto &machine = ScriptMachine::instance();
    auto dbg = machine.debugger();
    Q_ASSERT(dbg);
    connect(dbg, &asDebugger::onAdjustBreakPointLine, this,
            [=](const QString &file, int oldLineNbr, int newLineNbr) {
                auto editor = findEditorView(file);
                if (editor) {
                    removeBreakPoint(editor, oldLineNbr);
                    addBreakPoint(editor, newLineNbr);
                }
            });
    connect(dbg, &asDebugger::onPullVariables, this, [=]() {
        auto dbg = ScriptMachine::instance().debugger();
        auto &cache = dbg->cache;
        cache->CacheGlobals();
        m_gvarshow->refreshWithNewRoot(cache->globals);
        auto &cs = cache->call_stack;
        if (!cs.empty()) {
            cache->CacheCallstack();
            auto l = cs.at(0).scope.locals;
            m_varshow->refreshWithNewRoot(l);
        }
        m_watchModel->refresh();
    });
    connect(
        dbg, &asDebugger::onRunCurrentLine, this,
        [=](const QString &file, int lineNr) {
            ScriptEditor *e = nullptr;
#ifdef Q_OS_WIN
            if (file.compare(m_curEditor->fileName(), Qt::CaseInsensitive)) {
#else
            if (file != m_curEditor->fileName()) {
#endif
                e = findEditorView(file);
                if (e) {
                    e->setFocus();
                    e->raise();
                } else {
                    if (_curDbgData.contains(file)) {
                        auto cs = Utilities::getMd5(file);
                        auto &data = _curDbgData[file];
                        if (data.checksum != cs) {
                            // the file has been modified outside
                            e = createFakeEditor(file, data.source);
                        }
                    }
                }

                if (e == nullptr) {
                    e = openFile(file);
                    if (e) {
                        e->setReadOnly(true);
                        _reditors.append(e);
                        e->setFocus();
                        e->raise();

                        addRecentFile(file);
                    } else {
                        e = createFakeEditor(file, _curDbgData[file].source);
                    }
                }
            } else {
                e = m_curEditor;
            }

            const auto bpMark = QStringLiteral("bp");
            const auto curSym = QStringLiteral("cur");
            const auto hitCur = QStringLiteral("curbp");

            // remove the last mark
            if (!_lastCurLine.first.isEmpty() && _lastCurLine.second >= 0) {
                auto lastCur = findEditorView(_lastCurLine.first);
                if (lastCur) {
                    auto e = lastCur->editor();
                    auto symID = e->symbolMark(_lastCurLine.second);

                    if (symID == curSym) {
                        e->removeSymbolMark(_lastCurLine.second);
                    } else if (symID == hitCur) {
                        e->addSymbolMark(_lastCurLine.second, bpMark);
                    }
                }
            }

            auto editor = e->editor();

            // add the new mark
            auto symID = editor->symbolMark(lineNr);
            if (symID == bpMark) {
                editor->addSymbolMark(lineNr, hitCur);
            } else {
                editor->addSymbolMark(lineNr, curSym);
            }

            editor->ensureLineVisible(lineNr);

            _lastCurLine = {file, lineNr};
            updateRunDebugMode();

            if (_fakeEditor) {
                if (file != _fakeEditor->windowFilePath()) {
                    destoryFakeEditor();
                }
            }
        });
    connect(dbg, &asDebugger::onDebugActionExec, this,
            [this]() { updateRunDebugMode(); });
    m_sym->setEngine(machine.engine());
}

bool ScriptingDialog::about2Close() {
    const auto &views = ScriptEditor::instances();
    if (views.isEmpty()) {
        return true;
    }

    QStringList unSavedFiles;
    QList<ScriptEditor *> need2CloseView;

    for (auto &view : views) {
        if (view->editor()->document()->isModified()) {
            unSavedFiles << view->fileName();
        } else {
            need2CloseView << view;
        }
    }

    for (auto &view : need2CloseView) {
        view->requestCloseDockWidget();
    }

    if (!views.isEmpty()) {
        return false;
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
        for (auto &p : views) {
            p->requestCloseDockWidget();
        }
        return views.isEmpty();
    } else if (ret == QMessageBox::No) {
        for (auto &p : views) {
            p->closeDockWidget(); // force close
        }
        return true;
    }

    return false;
}

void ScriptingDialog::saveDockLayout() {
    auto &set = SettingManager::instance();
    set.setScriptDockLayout(_savedLayout);
}

void ScriptingDialog::buildUpRibbonBar() {
    m_ribbon = new Ribbon(this);
    buildFilePage(m_ribbon->addTab(tr("File")));
    m_editStateWidgets << buildEditPage(m_ribbon->addTab(tr("Edit")));
    buildViewPage(m_ribbon->addTab(tr("View")));
    m_editStateWidgets << buildDebugPage(m_ribbon->addTab(tr("Debugger")));

    buildSettingPage(m_ribbon->addTab(tr("Setting")));
    buildAboutPage(m_ribbon->addTab(tr("About")));

    connect(
        m_ribbon, &Ribbon::onDragDropFiles, this,
        [=](const QStringList &files) {
            for (auto &file : files) {
                if (ScriptManager::isScriptFile(file)) {
                    if (openFile(file)) {
                        addRecentFile(file);
                        m_lastusedpath = Utilities::getAbsoluteDirPath(file);
                    }
                } else {
                    Toast::toast(this, NAMEICONRES(QStringLiteral("script")),
                                 tr("InvalidSourceFile"));
                }
            }
        });
}

RibbonTabContent *ScriptingDialog::buildFilePage(RibbonTabContent *tab) {
    updateUI();
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

        addPannelAction(pannel, QStringLiteral("exit"), tr("Exit"),
                        &ScriptingDialog::on_exit);
    }

    {
        auto pannel = tab->addGroup(tr("Save"));

        auto a = addPannelAction(pannel, QStringLiteral("save"), tr("Save"),
                                 &ScriptingDialog::on_save, QKeySequence::Save);
        m_editStateWidgets << a;

        a = addPannelAction(pannel, QStringLiteral("saveas"), tr("SaveAs"),
                            &ScriptingDialog::on_saveas,
                            shortcuts.keySequence(QKeySequences::Key::SAVE_AS));

        m_editStateWidgets << a;
    }
    return tab;
}

RibbonTabContent *ScriptingDialog::buildEditPage(RibbonTabContent *tab) {
    updateUI();
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
    updateUI();
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
    updateUI();
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
        auto &runner = ScriptMachine::instance();
        bool isRun = false;
        bool isDbg = false;
        bool isPaused = false;

        isRun = runner.isRunning(ScriptMachine::Scripting);
        isDbg = runner.isDebugMode();
        auto dbg = runner.debugger();

        isPaused = dbg->action == asIDBAction::Pause;

        if (isRun && isDbg && isPaused) {
            m_Tbtneditors[ToolButtonIndex::DBG_CONTINUE_ACTION]->animateClick();
        } else {
            m_Tbtneditors[ToolButtonIndex::DBG_RUN_DBG_ACTION]->animateClick();
        }
    });

    return tab;
}

RibbonTabContent *ScriptingDialog::buildSettingPage(RibbonTabContent *tab) {
    updateUI();
    auto pannel = tab->addGroup(tr("Settings"));

    addPannelAction(pannel, QStringLiteral("file"), tr("Editor"),
                    [=] { m_setdialog->showConfig(QStringLiteral("Edit")); });
    addPannelAction(pannel, QStringLiteral("console"), tr("Console"), [=] {
        m_setdialog->showConfig(QStringLiteral("Console"));
    });
    addPannelAction(pannel, QStringLiteral("angellsp"), tr("AngelLSP"), [=] {
        m_setdialog->showConfig(QStringLiteral("AngelLSP"));
    });

    addPannelAction(pannel, QStringLiteral("angelrestart"), tr("RestartAngel"),
                    [this] { AngelLsp::instance().restartWithGUI(this); });

    return tab;
}

RibbonTabContent *ScriptingDialog::buildAboutPage(RibbonTabContent *tab) {
    updateUI();
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

    m_varshow = new asIDBTreeView(this);
    vars->addTab(m_varshow, tr("Local"));

    m_gvarshow = new asIDBTreeView(this);
    vars->addTab(m_gvarshow, tr("Global"));

    auto dw = buildDockWidget(dock, QStringLiteral("Variables"),
                              tr("Variables"), vars);
    m_dbgVarView = dw;
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
ScriptingDialog::buildUpVarWatchDock(ads::CDockManager *dock,
                                     ads::DockWidgetArea area,
                                     ads::CDockAreaWidget *areaw) {

    m_watchModel = new AsIDBWatchModel;
    auto watchVar = new asIDBTreeView(this);
    Utilities::applyTreeViewProperty(watchVar);
    watchVar->setEditTriggers(QTreeView::DoubleClicked);
    watchVar->setModel(m_watchModel);
    watchVar->setSelectionMode(QTreeView::ExtendedSelection);

    watchVar->setContextMenuPolicy(Qt::ActionsContextMenu);
    auto a = newAction(tr("Add"), [this]() {
        bool b;
        auto exp = WingInputDialog::getText(this, tr("Add expresion"),
                                            tr("PleaseInput"),
                                            QLineEdit::Normal, {}, &b);
        if (b) {
            m_watchModel->addWatchExpression(exp);
        }
    });
    watchVar->addAction(a);
    a = newAction(
        tr("Remove"),
        [this, watchVar]() {
            auto sel = watchVar->selectionModel();
            if (sel) {
                m_watchModel->removeWatchExpressions(sel->selectedIndexes());
            }
        },
        QKeySequence(Qt::Key_Delete));
    a->setShortcutContext(Qt::WidgetShortcut);
    watchVar->addAction(a);
    a = newAction(
        tr("Edit"),
        [this, watchVar]() {
            auto sel = watchVar->selectionModel();
            if (sel) {
                auto idx = sel->currentIndex();
                if (idx.parent().isValid()) {
                    Toast::toast(this, NAMEICONRES(QStringLiteral("watch")),
                                 tr("CanNotEditChild"));
                    return;
                }
                watchVar->edit(idx);
            }
        },
        QKeySequence(Qt::Key_F2));
    a->setShortcutContext(Qt::WidgetShortcut);
    watchVar->addAction(a);
    a = new QAction(this);
    a->setSeparator(true);
    watchVar->addAction(a);
    a = newAction(tr("Clear"), [this]() {
        if (m_watchModel->rowCount({})) {
            auto ret = WingMessageBox::critical(
                this, tr("ClearWatchVars"), tr("Sure2Clear"),
                QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                m_watchModel->clearAll();
            }
        }
    });
    watchVar->addAction(a);

    auto dw = buildDockWidget(dock, QStringLiteral("WatchVars"),
                              tr("WatchVars"), watchVar);
    m_dbgWatchView = dw;
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
ScriptingDialog::buildUpOutputShowDock(ads::CDockManager *dock,
                                       ads::DockWidgetArea area,
                                       ads::CDockAreaWidget *areaw) {
    m_consoleout = new ScriptingConsole(this);
    m_consoleout->setMode(ScriptingConsole::Output);
    m_consoleout->setIsTerminal(false);
    auto dw = buildDockWidget(dock, QStringLiteral("ConsoleOutput"),
                              tr("ConsoleOutput"), m_consoleout);
    m_outConsole = dw;
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
    auto dw =
        buildDockWidget(dock, QStringLiteral("Symbol"), tr("Symbol"), m_sym);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
ScriptingDialog::buildDiagnosisDock(ads::CDockManager *dock,
                                    ads::DockWidgetArea area,
                                    ads::CDockAreaWidget *areaw) {
    auto dview = new QListView(this);
    _squinfoModel = new WingSquiggleInfoModel(this);
    _squinfoModel->setSeverityLevelIcon(
        WingSquiggleInfoModel::SeverityLevel::Error,
        QIcon(QStringLiteral(":/completion/images/completion/error.svg")));
    _squinfoModel->setSeverityLevelIcon(
        WingSquiggleInfoModel::SeverityLevel::Hint,
        QIcon(QStringLiteral(":/completion/images/completion/hint.svg")));
    _squinfoModel->setSeverityLevelIcon(
        WingSquiggleInfoModel::SeverityLevel::Information,
        QIcon(
            QStringLiteral(":/completion/images/completion/Information.svg")));
    _squinfoModel->setSeverityLevelIcon(
        WingSquiggleInfoModel::SeverityLevel::Warning,
        QIcon(QStringLiteral(":/completion/images/completion/warn.svg")));
    dview->setModel(_squinfoModel);
    auto dw = buildDockWidget(dock, QStringLiteral("Diagnosis"),
                              tr("Diagnosis"), dview);
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
                swapEditor(m_curEditor, editview);
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
    auto bottomArea = buildDiagnosisDock(m_dock, ads::BottomDockWidgetArea);
    buildUpOutputShowDock(m_dock, ads::CenterDockWidgetArea, bottomArea);

    auto splitter =
        ads::internal::findParent<ads::CDockSplitter *>(m_editorViewArea);
    if (splitter) {
        constexpr auto bottomHeight = 200;
        splitter->setSizes({height() - bottomHeight, bottomHeight});
    }

    bottomArea =
        buildUpVarWatchDock(m_dock, ads::RightDockWidgetArea, bottomArea);
    buildUpStackShowDock(m_dock, ads::CenterDockWidgetArea, bottomArea);

    auto rightArea =
        buildUpVarShowDock(m_dock, ads::RightDockWidgetArea, m_editorViewArea);
    buildSymbolShowDock(m_dock, ads::CenterDockWidgetArea, rightArea);

    // set the first tab visible
    for (auto &item : m_dock->openedDockAreas()) {
        for (int i = 0; i < item->dockWidgetsCount(); ++i) {
            updateUI();
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
    const auto &views = ScriptEditor::instances();
    if (views.size() >= std::numeric_limits<size_t>::max() - 1) {
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
    updateUI();
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
    auto ret = WingMessageBox::warning(
        this, qAppName(), tr("ConfirmScriptSave"),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    return ret;
}

void ScriptingDialog::registerMark() {
    auto &sc = WingSymbolCenter::instance();
    sc.regsiterSymbol(QStringLiteral("bp"), markFromPath("bp"));
    sc.regsiterSymbol(QStringLiteral("cur"), markFromPath("hitcur"));
    sc.regsiterSymbol(QStringLiteral("curbp"), markFromPath("hitbp"));
}

void ScriptingDialog::registerEditorView(ScriptEditor *editor) {
    connect(editor, &ScriptEditor::closeRequested, this, [this] {
        auto editor = qobject_cast<ScriptEditor *>(sender());
        Q_ASSERT(editor);

        auto &m = ScriptMachine::instance();
        if (m.isRunning(ScriptMachine::Scripting) &&
            editor->editor()->isReadOnly()) {
            if (WingMessageBox::warning(
                    this, this->windowTitle(), tr("ScriptStillRunning"),
                    QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
                return;
            }
            on_stopscript();
        }

        if (editor->isModified()) {
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

        destoryEditor(editor);
    });

    connect(editor, &ScriptEditor::onToggleMark, this, [=](int lineIndex) {
        auto editor = qobject_cast<ScriptEditor *>(sender());
        Q_ASSERT(editor);
        toggleBreakPoint(editor, lineIndex);
    });

    connect(editor, &ScriptEditor::need2Reload, this, [editor, this]() {
        auto e = editor->editor();
        e->setContentModified(true);
        if (currentEditor() == editor &&
            !_curDbgData.contains(editor->fileName())) {
            reloadEditor(editor);
        } else {
            editor->setProperty("__RELOAD__", true);
        }
    });

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

    if (b) {
        auto fn = editor->fileName();
        setWindowFilePath(fn);
        m_status->setToolTip(fn);
    } else {
        setWindowFilePath({});
        _squinfoModel->setEditor(nullptr);
        m_status->clearMessage();
        m_status->setToolTip({});
    }
    updateWindowTitle();
    updateRunDebugMode();
}

ScriptEditor *ScriptingDialog::currentEditor() const { return m_curEditor; }

void ScriptingDialog::swapEditor(ScriptEditor *old, ScriptEditor *cur) {
    if (old == cur) {
        return;
    }

    if (!m_curConnections.isEmpty()) {
        for (auto &c : m_curConnections) {
            disconnect(c);
        }
        m_curConnections.clear();
    }

    if (cur == nullptr) {
        return;
    }

    auto editor = cur->editor();
    m_Tbtneditors.value(ToolButtonIndex::UNDO_ACTION)
        ->setEnabled(editor->document()->isUndoAvailable());
    m_Tbtneditors.value(ToolButtonIndex::REDO_ACTION)
        ->setEnabled(editor->document()->isRedoAvailable());
    m_Tbtneditors.value(ToolButtonIndex::COPY_ACTION)
        ->setEnabled(editor->textCursor().hasSelection());

    m_curConnections << connect(
        editor, &CodeEdit::copyAvailable,
        m_Tbtneditors.value(ToolButtonIndex::COPY_ACTION),
        &QToolButton::setEnabled);
    m_curConnections << connect(
        editor, &CodeEdit::undoAvailable,
        m_Tbtneditors.value(ToolButtonIndex::UNDO_ACTION),
        &QToolButton::setEnabled);
    m_curConnections << connect(
        editor, &CodeEdit::redoAvailable,
        m_Tbtneditors.value(ToolButtonIndex::REDO_ACTION),
        &QToolButton::setEnabled);

    m_curConnections << connect(editor, &CodeEdit::cursorPositionChanged, this,
                                &ScriptingDialog::updateCursorPosition);
    m_curConnections << connect(editor, &CodeEdit::selectionChanged, this,
                                &ScriptingDialog::updateCursorPosition);

    m_curEditor = cur;
    _squinfoModel->setEditor(editor);
    updateCursorPosition();

    if (cur && !_curDbgData.contains(cur->fileName())) {
        auto needReload = cur->property("__RELOAD__").toBool();
        if (needReload) {
            reloadEditor(cur);
            cur->setProperty("__RELOAD__", false);
        }
    }
}

void ScriptingDialog::updateWindowTitle() {
    auto title = tr("ScriptEditor");
    auto fp = windowFilePath();
    if (fp.isEmpty()) {
        this->setWindowTitle(title);
    } else {
        QFileInfo info(fp);
        this->setWindowTitle(title + QStringLiteral(" - ") + info.fileName());
    }
}

void ScriptingDialog::addRecentFile(const QString &fileName) {
    RecentFileManager::RecentInfo info;
    info.url = QUrl::fromLocalFile(fileName);
    m_recentmanager->addRecentFile(info);
}

void ScriptingDialog::updateRunDebugMode(bool disable) {
    auto &runner = ScriptMachine::instance();
    auto enable = !disable;
    bool isRun = false;
    bool isDbg = false;
    bool isPaused = false;

    isRun = runner.isRunning(ScriptMachine::Scripting);
    isDbg = runner.isDebugMode();
    auto dbg = runner.debugger();
    isPaused = dbg->action == asIDBAction::Pause;

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
            _status->setText(QStringLiteral("<font color=\"gold\">") +
                             tr("Debuging...") + QStringLiteral("</font>"));
        } else {
            _status->setText(QStringLiteral("<font color=\"green\">") +
                             tr("Running...") + QStringLiteral("</font>"));
        }
    } else {
        _status->setText({});
    }
}

ScriptEditor *ScriptingDialog::findEditorView(const QString &filename) {
    const auto &views = ScriptEditor::instances();
    for (auto &p : views) {
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

bool ScriptingDialog::isCurrentDebugging() const {
    auto &m = ScriptMachine::instance();
    return m.isDebugMode();
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
        WingMessageBox::critical(this, tr("Error"),
                                 tr("InvalidFileOrPermission"));
        return nullptr;
    }

    if (_curDbgData.contains(filename)) {
        editor->setReadOnly(true);
    }

    registerEditorView(editor);
    m_dock->addDockWidget(ads::CenterDockWidgetArea, editor, editorViewArea());
    editor->setFocus();
    return editor;
}

bool ScriptingDialog::try2CloseScriptViews(
    const LinkedList<ScriptEditor *> views) {
    auto &runner = ScriptMachine::instance();
    if (runner.isRunning(ScriptMachine::Scripting)) {
        if (WingMessageBox::warning(
                this, this->windowTitle(), tr("ScriptStillRunning"),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
            return false;
        }
        on_stopscript();
    }

    for (auto &editor : views) {
        bool saved = !editor->isModified();
        if (saved) {
            destoryEditor(editor);
        }
    }

    QVector<EditorInfo *> infos;
    infos.reserve(views.size());
    for (auto &view : views) {
        if (!view->isClosed()) {
            infos.append(view);
        }
    }
    if (!infos.isEmpty()) {
        MutiSaveDialog sd(infos, this, this);
        auto ret = MutiSaveDialog::StatusCode(sd.exec());
        switch (ret) {
        case MutiSaveDialog::SAVE_DISCARD: {
            for (auto &editor : views) {
                destoryEditor(editor);
            }
        } break;
        case MutiSaveDialog::SAVE_SAVE: {
            for (auto &editor : views) {
                if (editor->save()) {
                    destoryEditor(editor);
                }
            }

            if (!views.isEmpty()) {
                WingMessageBox::critical(this, qAppName(),
                                         tr("SaveUnSuccessfully"));
                return false;
            }
        } break;
        case MutiSaveDialog::SAVE_CANCEL:
            return false;
        case MutiSaveDialog::SAVE_NOOP:
            break;
        }
    }

    return true;
}

void ScriptingDialog::runDbgCommand(asIDBAction action) {
    updateRunDebugMode(true);
    auto &machine = ScriptMachine::instance();
    if (machine.isDebugMode()) {
        auto dbg = machine.debugger();
        dbg->SetAction(action);
    }
}

void ScriptingDialog::startDebugScript(const QString &fileName) {
    m_ribbon->setCurrentIndex(3);
    m_consoleout->clear();

    auto dbg = ScriptMachine::instance().debugger();
    m_callstack->attachDebugger(dbg);
    m_watchModel->attachDebugger(dbg);

    ScriptMachine::instance().executeScript(
        ScriptMachine::Scripting, fileName, true, nullptr,
        [this](const QHash<QString, AsPreprocesser::Result> &sdata) -> void {
            _curDbgData = sdata;

            auto dbg = ScriptMachine::instance().debugger();
            for (auto &&[file, data] : sdata.asKeyValueRange()) {
                auto view = findEditorView(file);
                if (view) {
                    auto e = view->editor();
                    auto totalblk = e->blockCount();
                    // add breakpoints
                    for (int i = 0; i < totalblk; ++i) {
                        if (!e->symbolMark(i).isEmpty()) {
                            dbg->addFileBreakPoint(file, i);
                        }
                    }
                    view->setReadOnly(true);
                    _reditors.append(view);
                }
            }

            PluginSystem::instance().scriptPragmaBegin();
            updateRunDebugMode();
        });

    for (auto &e : _reditors) {
        e->setReadOnly(false);
    }

    this->updateRunDebugMode();
    m_callstack->attachDebugger(nullptr);
    m_watchModel->attachDebugger(nullptr);
    m_varshow->refreshWithNewRoots({});
    m_gvarshow->refreshWithNewRoots({});

    // clean up
    if (!(_lastCurLine.first.isEmpty() || _lastCurLine.second < 0)) {
        // remove the last mark
        if (!_lastCurLine.first.isEmpty() && _lastCurLine.second >= 0) {
            auto lastCur = findEditorView(_lastCurLine.first);
            auto e = lastCur->editor();
            auto symID = e->symbolMark(_lastCurLine.second);

            const auto bpMark = QStringLiteral("bp");
            const auto curSym = QStringLiteral("cur");
            const auto hitCur = QStringLiteral("curbp");

            if (symID == curSym) {
                e->removeSymbolMark(_lastCurLine.second);
            } else if (symID == hitCur) {
                e->addSymbolMark(_lastCurLine.second, bpMark);
            }
        }
        _lastCurLine.first.clear();
        _lastCurLine.second = -1;
    }
    _reditors.clear();
    _curDbgData.clear();
    destoryFakeEditor();

    if (_needRestart) {
        _needRestart = false;
        startDebugScript(fileName);
    }
}

void ScriptingDialog::addBreakPoint(ScriptEditor *editor, int line) {
    Q_ASSERT(editor);
    auto e = editor->editor();

    const auto bpMark = QStringLiteral("bp");
    const auto curSym = QStringLiteral("cur");
    const auto hitCur = QStringLiteral("curbp");

    auto &m = ScriptMachine::instance();
    if (m.isDebugMode() && _curDbgData.contains(editor->fileName())) {
        auto dbg = m.debugger();
        auto symID = e->symbolMark(line);
        if (curSym == symID) {
            e->addSymbolMark(line, hitCur);
            dbg->addFileBreakPoint(editor->fileName(), line);
        } else {
            if (symID.isEmpty()) {
                e->addSymbolMark(line, bpMark);
                dbg->addFileBreakPoint(editor->fileName(), line);
            }
        }
    } else {
        e->addSymbolMark(line, bpMark);
    }
}

void ScriptingDialog::removeBreakPoint(ScriptEditor *editor, int line) {
    Q_ASSERT(editor);
    auto e = editor->editor();

    auto &m = ScriptMachine::instance();
    if (m.isDebugMode() && _curDbgData.contains(editor->fileName())) {
        auto dbg = m.debugger();
        auto symID = e->symbolMark(line);

        const auto bpMark = QStringLiteral("bp");
        const auto curSym = QStringLiteral("cur");
        const auto hitCur = QStringLiteral("curbp");

        if (hitCur == symID) {
            e->addSymbolMark(line, curSym);
            dbg->removeFileBreakPoint(editor->fileName(), line);
        } else {
            if (bpMark == symID) {
                e->removeSymbolMark(line);
                dbg->removeFileBreakPoint(editor->fileName(), line);
            }
        }
    } else {
        e->removeSymbolMark(line);
    }
}

void ScriptingDialog::toggleBreakPoint(ScriptEditor *editor, int line) {
    Q_ASSERT(editor);
    auto e = editor->editor();

    auto &m = ScriptMachine::instance();
    if (m.isDebugMode() && _curDbgData.contains(editor->fileName())) {
        auto dbg = m.debugger();
        auto symID = e->symbolMark(line);

        const auto bpMark = QStringLiteral("bp");
        const auto curSym = QStringLiteral("cur");
        const auto hitCur = QStringLiteral("curbp");

        auto fileName = editor->fileName();
        if (hitCur == symID) {
            e->addSymbolMark(line, curSym);
            dbg->removeFileBreakPoint(fileName, line);
        } else if (curSym == symID) {
            e->addSymbolMark(line, hitCur);
            dbg->addFileBreakPoint(fileName, line);
        } else {
            if (bpMark == symID) {
                e->removeSymbolMark(line);
                dbg->removeFileBreakPoint(fileName, line);
            } else {
                e->addSymbolMark(line, bpMark);
                dbg->addFileBreakPoint(fileName, line);
            }
        }
    } else {
        auto symID = e->symbolMark(line);
        if (symID.isEmpty()) {
            e->addSymbolMark(line, QStringLiteral("bp"));
        } else {
            e->removeSymbolMark(line);
        }
    }
}

void ScriptingDialog::updateCursorPosition() {
    auto editor = currentEditor();
    if (editor) {
        auto e = editor->editor();
        const QTextCursor cursor = e->textCursor();
        const int column =
            e->textColumn(cursor.block().text(), cursor.positionInBlock());
        const int selectedChars =
            std::abs(cursor.selectionEnd() - cursor.selectionStart());
        QString positionText =
            tr("Line %1, Col %2").arg(cursor.blockNumber() + 1).arg(column + 1);
        if (selectedChars)
            positionText += tr(" (Selected: %1)").arg(selectedChars);
        m_status->showMessage(positionText);
    } else {
        m_status->showMessage({});
    }
}

void ScriptingDialog::reloadEditor(ScriptEditor *editor) {
    activateWindow();
    raise();
    editor->editor()->document()->setModified();
    if (QFile::exists(editor->fileName())) {
        auto ret = WingMessageBox::question(this, tr("Reload"),
                                            tr("ReloadNeededYesOrNo"));
        if (ret == QMessageBox::Yes) {
            if (editor->reload()) {
                Toast::toast(this, NAMEICONRES(QStringLiteral("reload")),
                             tr("ReloadSuccessfully"));
            } else {
                WingMessageBox::critical(this, tr("Error"),
                                         tr("ReloadUnSuccessfully"));
            }
        }
    }
}

void ScriptingDialog::updateUI() { QApplication::processEvents(); }

void ScriptingDialog::on_newfile() {
    if (!newOpenFileSafeCheck()) {
        return;
    }
    auto filename = WingFileDialog::getSaveFileName(
        this, tr("ChooseFile"), m_lastusedpath,
        QStringLiteral("AngelScript (*.as *.angelscript)"));
    if (!filename.isEmpty()) {
        m_lastusedpath = Utilities::getAbsoluteDirPath(filename);
        if (_curDbgData.contains(filename)) {
            auto ret = WingMessageBox::warning(
                this, tr("New"), tr("NewFileWithDbgExists"),
                QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::No) {
                return;
            }
            if (ScriptMachine::instance().isRunning(ScriptMachine::Scripting)) {
                on_stopscript();
            }
        }

        auto e = findEditorView(filename);

        // create an empty file
        QFile f(filename);
        if (f.open(QFile::WriteOnly | QFile::Text)) {
            f.write(QByteArrayLiteral("int main() {\n    return 0;\n}\n"));
            f.close();
        }

        if (e) {
            e->reload();
            e->raise();
            e->setFocus();
            return;
        }

        auto editor = new ScriptEditor(this);
        auto res = editor->openFile(filename);
        if (!res) {
            WingMessageBox::critical(this, tr("Error"),
                                     tr("InvalidFileOrPermission"));
            return;
        }

        registerEditorView(editor);
        m_dock->addDockWidget(ads::CenterDockWidgetArea, editor,
                              editorViewArea());
        editor->setFocus();
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
        m_lastusedpath = Utilities::getAbsoluteDirPath(filename);
        if (openFile(filename)) {
            addRecentFile(filename);
        }
    }
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

void ScriptingDialog::on_exit() {
    const auto &views = ScriptEditor::instances();
    if (try2CloseScriptViews(views)) {
        close();
    }
}

void ScriptingDialog::on_save() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }

    if (AngelLsp::instance().autofmt()) {
        editor->formatCode();
    }

    auto res = editor->save();
    if (res) {
        setWindowFilePath(editor->fileName());
        updateWindowTitle();
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

    auto filename = WingFileDialog::getSaveFileName(
        this, tr("ChooseSaveFile"), m_lastusedpath,
        QStringLiteral("AngelScript (*.as *.angelscript)"));
    if (filename.isEmpty())
        return;
    m_lastusedpath = Utilities::getAbsoluteDirPath(filename);

    auto res = editor->save(filename);
    if (res) {
        updateWindowTitle();
        Toast::toast(this, NAMEICONRES(QStringLiteral("saveas")),
                     tr("SaveSuccessfully"));
    } else {
        WingMessageBox::critical(this, tr("Error"), tr("SaveUnSuccessfully"));
    }

    addRecentFile(filename);
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
        e->editor()->textCursor().removeSelectedText();
    }
}

void ScriptingDialog::on_findfile() {
    auto e = currentEditor();
    if (e) {
        e->find();
    }
}

void ScriptingDialog::on_replace() {
    auto e = currentEditor();
    if (e) {
        e->replace();
    }
}

void ScriptingDialog::on_gotoline() {
    auto e = currentEditor();
    if (e) {
        e->gotoLine();
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
    const auto &views = ScriptEditor::instances();
    if (views.isEmpty()) {
        m_dock->restoreState(_defaultLayout);
        return;
    }

    auto curEditor = m_curEditor;
    if (views.size() > 1) {
        auto notSameContainer = std::any_of(
            views.begin(), views.end(), [curEditor](ScriptEditor *view) {
                return curEditor->dockAreaWidget() != view->dockAreaWidget();
            });
        if (notSameContainer) {
            auto ret = WingMessageBox::warning(
                this, qAppName(), tr("LayoutRestoreBreak"),
                QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::No) {
                return;
            }
        }
    }

    // remove temperaily
    QVector<ScriptEditor *> hiddenView;
    for (auto &view : views) {
        if (view->isClosed()) {
            hiddenView.append(view);
        }
        m_dock->removeDockWidget(view);
    }

    m_dock->restoreState(_defaultLayout);

    // add back
    auto centeralWidget = m_dock->centralWidget();
    auto area = centeralWidget->dockAreaWidget();
    for (auto &view : views) {
        m_dock->addDockWidget(ads::CenterDockWidgetArea, view, area);
        if (hiddenView.contains(view)) {
            view->toggleView(false);
        }
    }

    if (curEditor) {
        curEditor->raise();
        curEditor->setFocus();
    }
}

void ScriptingDialog::on_runscript() {
    auto editor = currentEditor();
    if (editor) {
        if (!editor->save()) {
            WingMessageBox::critical(this, qAppName(),
                                     tr("CannotSave2RunScript"));
            return;
        }
        m_consoleout->clear();
        PluginSystem::instance().scriptPragmaBegin();

        editor->setReadOnly(true);
        ScriptMachine::instance().executeScript(ScriptMachine::Scripting,
                                                editor->fileName());
        editor->setReadOnly(false);
        m_outConsole->raise();
        destoryFakeEditor();
        updateRunDebugMode();
    }
}

void ScriptingDialog::on_rundbgscript() {
    const auto &views = ScriptEditor::instances();
    for (auto &editor : views) {
        if (!editor->save()) {
            WingMessageBox::critical(this, qAppName(),
                                     tr("CannotSave2RunScript"));
            editor->raise();
            return;
        }
    }

    auto editor = currentEditor();
    if (editor) {
        m_outConsole->raise();
        m_dbgVarView->raise();
        m_dbgWatchView->raise();
        startDebugScript(editor->fileName());
    }
}

void ScriptingDialog::on_pausescript() { runDbgCommand(asIDBAction::Pause); }

void ScriptingDialog::on_continuescript() {
    runDbgCommand(asIDBAction::Continue);
}

void ScriptingDialog::on_stopscript() {
    ScriptMachine::instance().abortDbgScript();
    destoryFakeEditor();
}

void ScriptingDialog::on_restartscript() {
    on_stopscript();
    _needRestart = true;
}

void ScriptingDialog::on_stepinscript() {
    runDbgCommand(asIDBAction::StepInto);
}

void ScriptingDialog::on_stepoutscript() {
    runDbgCommand(asIDBAction::StepOut);
}

void ScriptingDialog::on_stepoverscript() {
    runDbgCommand(asIDBAction::StepOver);
}

void ScriptingDialog::on_togglebreakpoint() {
    auto editor = currentEditor();
    if (editor) {
        auto e = editor->editor();
        toggleBreakPoint(editor, e->textCursor().blockNumber() + 1);
    }
}

void ScriptingDialog::on_addbreakpoint() {
    auto editor = currentEditor();
    if (editor) {
        auto e = editor->editor();
        addBreakPoint(editor, e->textCursor().blockNumber() + 1);
    }
}

void ScriptingDialog::on_removebreakpoint() {
    auto editor = currentEditor();
    if (editor) {
        auto e = editor->editor();
        removeBreakPoint(editor, e->textCursor().blockNumber() + 1);
    }
}

void ScriptingDialog::closeEvent(QCloseEvent *event) {
    auto &runner = ScriptMachine::instance();
    if (runner.isRunning(ScriptMachine::Scripting)) {
        if (WingMessageBox::warning(
                this, this->windowTitle(), tr("ScriptStillRunning"),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
            event->ignore();
            return;
        }
        on_stopscript();
    }

    const auto &views = ScriptEditor::instances();
    if (!views.isEmpty()) {
        event->ignore();
        this->hide();
        return;
    }

    _savedLayout = m_dock->saveState();
    auto &set = SettingManager::instance();
    set.setRecentScriptFiles(m_recentmanager->saveRecent());
    set.setLastUsedScriptPath(m_lastusedpath);
    saveDockLayout();
    FramelessMainWindow::closeEvent(event);
}

ScriptEditor *ScriptingDialog::createFakeEditor(const QString &fileName,
                                                const QString &text) {
    if (_fakeEditor == nullptr) {
        _fakeEditor = new ScriptEditor(this);
        _fakeEditor->setReadOnly(true);
        _fakeEditor->setFeature(
            ads::CDockWidget::DockWidgetFeature ::DockWidgetClosable, false);
        _fakeEditor->editor()->disconnect(); // disconnect all

        auto tab = _fakeEditor->tabWidget();
        tab->setStyleSheet(
            QStringLiteral("QLabel{text-decoration:line-through;}"));

        auto ev = m_Tbtneditors.value(ToolButtonIndex::EDITOR_VIEWS);
        auto menu = ev->menu();
        Q_ASSERT(menu);
        auto ta = _fakeEditor->toggleViewAction();
        menu->addAction(ta);
        ev->setEnabled(true);
        m_dock->addDockWidget(ads::CenterDockWidgetArea, _fakeEditor,
                              m_dock->centralWidget()->dockAreaWidget());
    }

    QFileInfo finfo(fileName);
    auto name = finfo.fileName();
    _fakeEditor->setWindowTitle(name);
    _fakeEditor->setWindowFilePath(fileName);

    auto editor = _fakeEditor->editor();
    editor->setPlainText(text);

    return _fakeEditor;
}

void ScriptingDialog::destoryFakeEditor() {
    if (_fakeEditor) {
        m_dock->removeDockWidget(_fakeEditor);
        delete _fakeEditor;
        _fakeEditor = nullptr;
    }
}

void ScriptingDialog::destoryEditor(ScriptEditor *editor) {
    Q_ASSERT(editor);
    const auto &views = ScriptEditor::instances();
    if (currentEditor() == editor) {
        m_curEditor = nullptr;
    }
    m_Tbtneditors.value(ToolButtonIndex::EDITOR_VIEWS)
        ->setEnabled(views.size() != 0);

    if (m_dock->focusedDockWidget() == editor) {
        if (!views.isEmpty()) {
            for (auto &p : views) {
                if (p != editor && p->isCurrentTab()) {
                    p->setFocus();
                }
            }
        }
    }

    editor->deleteDockWidget();
    updateEditModeEnabled();
}

QPixmap ScriptingDialog::markFromPath(const QString &name) {
    return QPixmap(
        QStringLiteral(":/com.wingsummer.winghex/images/scriptdbg/") + name +
        QStringLiteral(".png"));
}

bool ScriptingDialog::save(EditorInfo *info) {
    auto editor = dynamic_cast<ScriptEditor *>(info);
    if (editor == nullptr) {
        return false;
    }

    return editor->save();
}

void ScriptingDialog::discard(EditorInfo *info) {
    auto editor = dynamic_cast<ScriptEditor *>(info);
    if (editor == nullptr) {
        return;
    }

    destoryEditor(editor);
}
