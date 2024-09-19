#include "scriptingdialog.h"
#include "DockSplitter.h"
#include "QWingRibbon/ribbontabcontent.h"
#include "Qt-Advanced-Docking-System/src/DockAreaWidget.h"
#include "aboutsoftwaredialog.h"
#include "class/languagemanager.h"
#include "class/qkeysequences.h"
#include "class/settingmanager.h"
#include "class/skinmanager.h"
#include "class/wingfiledialog.h"
#include "class/wingmessagebox.h"
#include "control/toast.h"
#include "qcodeeditwidget/qeditconfig.h"
#include "qdocumentline.h"
#include "qeditor.h"
#include "qformatscheme.h"
#include "qlinemarksinfocenter.h"

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
    m_recentmanager = new RecentFileManager(m_recentMenu);
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
    layout->addWidget(m_dock, 1);

    m_status = new QStatusBar(this);
    layout->addWidget(m_status);
    buildUpContent(cw);

    buildUpSettingDialog();

    QFormatScheme *format = nullptr;

    switch (SkinManager::instance().currentTheme()) {
    case SkinManager::Theme::Dark:
        format =
            new QFormatScheme(QStringLiteral(":/qcodeedit/as_dark.qxf"), this);
        break;
    case SkinManager::Theme::Light:
        format =
            new QFormatScheme(QStringLiteral(":/qcodeedit/as_light.qxf"), this);
        break;
    }
    QDocument::setDefaultFormatScheme(format);
    m_language = new QLanguageFactory(format, this);
    m_language->addDefinitionPath(QStringLiteral(":/qcodeedit"));

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

    this->setUpdatesEnabled(true);
}

void ScriptingDialog::initConsole() {
    Q_ASSERT(m_consoleout);
    m_consoleout->init();
    auto machine = m_consoleout->machine();
    connect(machine, &ScriptMachine::onDebugFinished, this, [=] {
        _DebugingScript.clear();

        this->updateRunDebugMode();
        m_callstack->updateData({});

        // clean up
        if (_lastCurLine.first.isEmpty() || _lastCurLine.second < 0) {
            // no need
            return;
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
        _lastCurLine.first.clear();
        _lastCurLine.second = -1;
    });
    auto dbg = machine->debugger();
    Q_ASSERT(dbg);
    connect(dbg, &asDebugger::onAdjustBreakPointLine, this,
            [=](const asDebugger::BreakPoint &old, int newLineNr) {

            });
    connect(dbg, &asDebugger::onPullVariables, this,
            [=](const QVector<asDebugger::VariablesInfo> &globalvars,
                const QVector<asDebugger::VariablesInfo> &localvars) {

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
                    // error permission
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

            // add the new mark
            auto doc = e->editor()->document();
            auto line = doc->line(lineNr - 1);
            if (line.hasMark(bpMark)) {
                line.removeMark(bpMark);
                line.addMark(curHitMark);
            } else {
                line.addMark(curMark);
            }

            _lastCurLine = {file, lineNr};

            updateRunDebugMode();
        });
}

void ScriptingDialog::buildUpRibbonBar() {
    m_ribbon = new Ribbon(this);
    buildFilePage(m_ribbon->addTab(tr("File")));
    m_editStateWidgets << buildEditPage(m_ribbon->addTab(tr("Edit")));
    buildViewPage(m_ribbon->addTab(tr("View")));
    m_editStateWidgets << buildDebugPage(m_ribbon->addTab(tr("Debugger")));

    // TODO: not available for v1.0.0
    // buildSettingPage(m_ribbon->addTab(tr("Setting")));
    buildAboutPage(m_ribbon->addTab(tr("About")));

    connect(m_ribbon, &Ribbon::onDragDropFiles, this,
            [=](const QStringList &files) {
                for (auto &file : files) {
                    openFile(file);
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

    return tab;
}

RibbonTabContent *ScriptingDialog::buildDebugPage(RibbonTabContent *tab) {
    {
        auto pannel = tab->addGroup(tr("Debug"));
        m_Tbtneditors.insert(
            ToolButtonIndex::DBG_RUN_ACTION,
            addPannelAction(pannel, QStringLiteral("dbgrun"), tr("Run"),
                            &ScriptingDialog::on_runscript,
                            QKeySequence(Qt::CTRL | Qt::Key_F5)));
        m_Tbtneditors.insert(ToolButtonIndex::DBG_RUN_DBG_ACTION,
                             addPannelAction(pannel, QStringLiteral("dbgdebug"),
                                             tr("RunWithDbg"),
                                             &ScriptingDialog::on_rundbgscript,
                                             QKeySequence(Qt::Key_F5)));

        m_Tbtneditors.insert(ToolButtonIndex::DBG_PAUSE_ACTION,
                             addPannelAction(pannel, QStringLiteral("dbgpause"),
                                             tr("Pause"),
                                             &ScriptingDialog::on_pausescript));
        m_Tbtneditors.insert(
            ToolButtonIndex::DBG_CONTINUE_ACTION,
            addPannelAction(pannel, QStringLiteral("dbgcontinue"),
                            tr("Continue"),
                            &ScriptingDialog::on_continuescript));

        m_Tbtneditors.insert(ToolButtonIndex::DBG_STOP_ACTION,
                             addPannelAction(pannel, QStringLiteral("dbgstop"),
                                             tr("Stop"),
                                             &ScriptingDialog::on_stopscript));
        m_Tbtneditors.insert(
            ToolButtonIndex::DBG_RESTART_ACTION,
            addPannelAction(pannel, QStringLiteral("dbgrestart"), tr("Restart"),
                            &ScriptingDialog::on_restartscript));
        m_Tbtneditors.insert(
            ToolButtonIndex::DBG_STEPINTO_ACTION,
            addPannelAction(pannel, QStringLiteral("dbgstepinto"),
                            tr("StepInto"), &ScriptingDialog::on_stepinscript,
                            QKeySequence(Qt::Key_F11)));
        m_Tbtneditors.insert(
            ToolButtonIndex::DBG_STEPOVER_ACTION,
            addPannelAction(pannel, QStringLiteral("dbgstepover"),
                            tr("StepOver"), &ScriptingDialog::on_stepoverscript,
                            QKeySequence(Qt::Key_F10)));
        m_Tbtneditors.insert(
            ToolButtonIndex::DBG_STEPOUT_ACTION,
            addPannelAction(pannel, QStringLiteral("dbgstepout"), tr("StepOut"),
                            &ScriptingDialog::on_stepoutscript,
                            QKeySequence(Qt::SHIFT | Qt::Key_F11)));

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
    return tab;
}

RibbonTabContent *ScriptingDialog::buildSettingPage(RibbonTabContent *tab) {
    auto pannel = tab->addGroup(tr("Settings"));

    addPannelAction(pannel, QStringLiteral("file"), tr("Editor"),
                    &ScriptingDialog::on_setting);

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
ScriptingDialog::buildUpBreakpointShowDock(ads::CDockManager *dock,
                                           ads::DockWidgetArea area,
                                           ads::CDockAreaWidget *areaw) {
    auto bpview = new QTableView(this);
    Utilities::applyTableViewProperty(bpview);
    m_breakpoints = new DbgBreakpointModel(bpview);
    bpview->setModel(m_breakpoints);
    auto dw = buildDockWidget(dock, QStringLiteral("BreakPoints"),
                              tr("BreakPoints"), bpview);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
ScriptingDialog::buildUpOutputShowDock(ads::CDockManager *dock,
                                       ads::DockWidgetArea area,
                                       ads::CDockAreaWidget *areaw) {
    m_consoleout = new ScriptingConsole(this);
    m_consoleout->clear();
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
ScriptingDialog::buildUpWatchDock(ads::CDockManager *dock,
                                  ads::DockWidgetArea area,
                                  ads::CDockAreaWidget *areaw) {
    auto watch = new QTableView(this);
    Utilities::applyTableViewProperty(watch);
    m_watch = new DbgVarShowModel(watch);
    watch->setModel(m_watch);

    auto dw =
        buildDockWidget(dock, QStringLiteral("Watch"), tr("Watch"), watch);
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
        new CDockWidget(QStringLiteral("CentralWidget"));
    CentralDockWidget->setWidget(label);
    CentralDockWidget->setFeature(ads::CDockWidget::DockWidgetFocusable, false);
    CentralDockWidget->setFeature(ads::CDockWidget::NoTab, true);
    m_editorViewArea = m_dock->setCentralWidget(CentralDockWidget);

    // build up basic docking widgets
    // TODO
    auto bottomArea = buildUpOutputShowDock(m_dock, ads::BottomDockWidgetArea);

    auto splitter =
        ads::internal::findParent<ads::CDockSplitter *>(m_editorViewArea);
    if (splitter) {
        constexpr auto bottomHeight = 200;
        splitter->setSizes({height() - bottomHeight, bottomHeight});
    }

    buildUpStackShowDock(m_dock, ads::CenterDockWidgetArea, bottomArea);

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
    auto dw = new CDockWidget(displayName, dock);
    dw->setObjectName(widgetName);
    dw->setFeatures(CDockWidget::DockWidgetMovable |
                    CDockWidget::DockWidgetClosable |
                    CDockWidget::DockWidgetPinnable);

    dw->setWidget(content);
    m_Tbtneditors.value(index)->menu()->addAction(dw->toggleViewAction());
    return dw;
}

void ScriptingDialog::registerEditorView(ScriptEditor *editor) {
    // TODO
    connect(editor, &ScriptEditor::closeRequested, this, [this] {
        auto editor = qobject_cast<ScriptEditor *>(sender());
        Q_ASSERT(editor);
        Q_ASSERT(m_views.contains(editor));

        auto m = m_consoleout->machine();
        if (m->isInDebugMode()) {
            // TODO
        }

        m_views.removeOne(editor);
        if (currentEditor() == editor) {
            m_curEditor = nullptr;
        }
        m_Tbtneditors.value(ToolButtonIndex::EDITOR_VIEWS)
            ->setEnabled(m_views.size() != 0);

        if (m_dock->focusedDockWidget() == editor) {
            if (!m_views.isEmpty()) {
                for (auto p : m_views) {
                    if (p != editor && p->isCurrentTab()) {
                        p->setFocus();
                    }
                }
            }
        }

        editor->deleteDockWidget();
    });

    m_language->setLanguage(editor->editor(), QStringLiteral("AngelScript"));

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
        old->disconnect(SLOT(onToggleMark(int)));
        auto editor = old->editor();
        editor->disconnect(SLOT(copyAvailable(bool)));
        editor->disconnect(SLOT(contentModified(bool)));
        editor->disconnect(SLOT(undoAvailable(bool)));
        editor->disconnect(SLOT(redoAvailable(bool)));
        editor->disconnect(SLOT(zoomed()));
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
        toggleBreakPoint(editor, lineIndex);
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

void ScriptingDialog::updateRunDebugMode() {
    auto runner = m_consoleout->machine();
    bool isRun = false;
    bool isDbg = false;
    bool isPaused = false;
    if (runner) {
        isRun = runner->isRunning();
        isDbg = runner->isInDebugMode();
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

    auto dbgop = isRun && isDbg && isPaused;
    m_Tbtneditors.value(ToolButtonIndex::DBG_CONTINUE_ACTION)
        ->setEnabled(dbgop);
    m_Tbtneditors.value(ToolButtonIndex::DBG_STEPINTO_ACTION)
        ->setEnabled(dbgop);
    m_Tbtneditors.value(ToolButtonIndex::DBG_STEPOVER_ACTION)
        ->setEnabled(dbgop);
    m_Tbtneditors.value(ToolButtonIndex::DBG_STEPOUT_ACTION)->setEnabled(dbgop);
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
    return m && m->isInDebugMode();
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
    auto machine = m_consoleout->machine();
    if (machine->isInDebugMode()) {
        auto dbg = machine->debugger();
        dbg->runDebugAction(action);
    }
}

void ScriptingDialog::buildUpSettingDialog() {
    m_setdialog = new SettingDialog(this);

    auto edit = new QEditConfig(m_setdialog);
    m_setdialog->addPage(edit);

    m_setdialog->build();
}

void ScriptingDialog::startDebugScript(const QString &fileName) {
    m_consoleout->clear();

    // add breakpoints
    auto marks = QLineMarksInfoCenter::instance()->marks(
        fileName, m_symID.value(Symbols::BreakPoint));
    auto dbg = m_consoleout->machine()->debugger();
    for (auto &bp : marks) {
        dbg->addFileBreakPoint(bp.file, bp.line);
    }
    m_consoleout->machine()->executeScript(fileName, true);
    updateRunDebugMode();
}

void ScriptingDialog::toggleBreakPoint(ScriptEditor *editor, int lineIndex) {
    Q_ASSERT(editor);
    auto curLine = lineIndex + 1;

#ifdef Q_OS_WIN
    if (m_consoleout->machine()->isInDebugMode() &&
        editor->fileName().compare(_lastCurLine.first, Qt::CaseInsensitive) == 0
#else
    if (editor->fileName() == _lastCurLine.first
#endif
        && _lastCurLine.second == curLine) {
        auto line = editor->editor()->document()->line(lineIndex);
        auto bpMark = m_symID.value(Symbols::BreakPoint);
        auto hitCur = m_symID.value(Symbols::DbgRunHitBreakPoint);
        auto curSym = m_symID.value(Symbols::DbgRunCurrentLine);

        auto dbg = m_consoleout->machine()->debugger();
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
        QLineMark mrk(editor->fileName(), curLine,
                      m_symID.value(Symbols::BreakPoint));
        QLineMarksInfoCenter::instance()->toggleLineMark(mrk);
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
        if (e) {
            e->raise();
            e->setFocus();
            // TODO
            return;
        }

        // create an empty file
        QFile f(filename);
        f.open(QFile::WriteOnly | QFile::Text);
        f.close();

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

    auto filename = WingFileDialog::getSaveFileName(
        this, tr("ChooseSaveFile"), m_lastusedpath,
        QStringLiteral("AngelScript (*.as *.angelscript)"));
    if (filename.isEmpty())
        return;
    m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();

    auto oldFileName = editor->fileName();

    auto res = editor->save(filename);
    if (res) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("saveas")),
                     tr("SaveSuccessfully"));
    } else {
        WingMessageBox::critical(this, tr("Error"), tr("SaveUnSuccessfully"));
    }
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

void ScriptingDialog::on_setting() { m_setdialog->showConfig(); }

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
        m_consoleout->machine()->executeScript(e->fileName());
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
    startDebugScript(_DebugingScript);
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
        toggleBreakPoint(editor, editor->editor()->cursor().lineNumber());
    }
}

void ScriptingDialog::on_addbreakpoint() {
    auto editor = currentEditor();
    if (editor) {
        auto curLineInd = editor->editor()->cursor().lineNumber();
        auto bpMark = m_symID.value(Symbols::BreakPoint);

#ifdef Q_OS_WIN
        if (m_consoleout->machine()->isInDebugMode() &&
            editor->fileName().compare(_lastCurLine.first,
                                       Qt::CaseInsensitive) == 0
#else
        if (editor->fileName() == _lastCurLine.first
#endif
            && _lastCurLine.second == curLineInd + 1) {
            auto line = editor->editor()->document()->line(curLineInd);
            auto hitCur = m_symID.value(Symbols::DbgRunHitBreakPoint);
            auto curSym = m_symID.value(Symbols::DbgRunCurrentLine);

            auto dbg = m_consoleout->machine()->debugger();
            if (line.hasMark(curSym)) {
                line.removeMark(curSym);
                line.addMark(hitCur);
                dbg->addFileBreakPoint(editor->fileName(), curLineInd + 1);
            } else {
                if (!line.hasMark(bpMark)) {
                    line.addMark(bpMark);
                    dbg->addFileBreakPoint(editor->fileName(), curLineInd + 1);
                }
            }
        } else {
            auto line = editor->editor()->document()->line(curLineInd);
            if (!line.hasMark(bpMark)) {
                line.addMark(bpMark);
            }
        }
    }
}

void ScriptingDialog::on_removebreakpoint() {
    auto editor = currentEditor();
    if (editor) {
        auto curLineInd = editor->editor()->cursor().lineNumber();
        auto bpMark = m_symID.value(Symbols::BreakPoint);

#ifdef Q_OS_WIN
        if (m_consoleout->machine()->isInDebugMode() &&
            editor->fileName().compare(_lastCurLine.first,
                                       Qt::CaseInsensitive) == 0
#else
        if (editor->fileName() == _lastCurLine.first
#endif
            && _lastCurLine.second == curLineInd + 1) {
            auto line = editor->editor()->document()->line(curLineInd);
            auto hitCur = m_symID.value(Symbols::DbgRunHitBreakPoint);
            auto curSym = m_symID.value(Symbols::DbgRunCurrentLine);

            auto dbg = m_consoleout->machine()->debugger();
            if (line.hasMark(hitCur)) {
                line.removeMark(hitCur);
                line.addMark(curSym);
                dbg->removeFileBreakPoint(editor->fileName(), curLineInd + 1);
            } else {
                if (line.hasMark(bpMark)) {
                    line.removeMark(bpMark);
                    dbg->removeFileBreakPoint(editor->fileName(),
                                              curLineInd + 1);
                }
            }
        } else {
            auto line = editor->editor()->document()->line(curLineInd);
            if (line.hasMark(bpMark)) {
                line.removeMark(bpMark);
            }
        }
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

    auto &set = SettingManager::instance();

    set.setScriptDockLayout(m_dock->saveState());
    set.setRecentScriptFiles(m_recentmanager->saveRecent());
    set.save();

    FramelessMainWindow::closeEvent(event);
}
