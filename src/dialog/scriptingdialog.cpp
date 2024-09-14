#include "scriptingdialog.h"
#include "DockSplitter.h"
#include "QWingRibbon/ribbontabcontent.h"
#include "Qt-Advanced-Docking-System/src/DockAreaWidget.h"
#include "aboutsoftwaredialog.h"
#include "class/languagemanager.h"
#include "class/qkeysequences.h"
#include "class/settingmanager.h"
#include "class/wingfiledialog.h"
#include "class/wingmessagebox.h"
#include "control/toast.h"
#include "qcodeeditwidget/qeditconfig.h"
#include "qdocumentline.h"
#include "qeditor.h"
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

    QLineMarksInfoCenter::instance()->loadMarkTypes(
        QCE::fetchDataFile(":/qcodeedit/marks.qxm"));
    // get symbol ID
    m_symID.insert(Symbols::BreakPoint,
                   QLineMarksInfoCenter::instance()->markTypeId("breakpoint"));

    updateEditModeEnabled();

    // ok, preparing for starting...
    this->setWindowTitle(tr("ScriptEditor"));
    this->setWindowIcon(ICONRES(QStringLiteral("script")));
    this->setMinimumSize(800, 600);

    this->setUpdatesEnabled(true);
}

void ScriptingDialog::initConsole() {
    Q_ASSERT(m_consoleout);
    m_consoleout->init();
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
                    openFile(file);
                }
            });
}

RibbonTabContent *ScriptingDialog::buildFilePage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();
    {
        auto pannel = tab->addGroup(tr("Basic"));
        addPannelAction(pannel, QStringLiteral("new"), tr("New"),
                        &ScriptingDialog::on_newfile, QKeySequence::Open);

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
        m_Tbtneditors.insert(
            ToolButtonIndex::UNDO_ACTION,
            addPannelAction(pannel, QStringLiteral("undo"), tr("Undo"),
                            &ScriptingDialog::on_undofile, QKeySequence::Undo));
        m_Tbtneditors.insert(
            ToolButtonIndex::REDO_ACTION,
            addPannelAction(pannel, QStringLiteral("redo"), tr("Redo"),
                            &ScriptingDialog::on_redofile, QKeySequence::Redo));
        addPannelAction(pannel, QStringLiteral("cut"), tr("Cut"),
                        &ScriptingDialog::on_cutfile, QKeySequence::Cut);
        addPannelAction(pannel, QStringLiteral("copy"), tr("Copy"),
                        &ScriptingDialog::on_copyfile, QKeySequence::Copy);
        addPannelAction(pannel, QStringLiteral("paste"), tr("Paste"),
                        &ScriptingDialog::on_pastefile, QKeySequence::Paste);
        addPannelAction(pannel, QStringLiteral("del"), tr("Delete"),
                        &ScriptingDialog::on_delete, QKeySequence::Delete);
    }

    {
        auto pannel = tab->addGroup(tr("Lookup"));
        addPannelAction(pannel, QStringLiteral("find"), tr("Find"),
                        &ScriptingDialog::on_findfile, QKeySequence::Find);
        addPannelAction(pannel, QStringLiteral("replace"), tr("Replace"),
                        &ScriptingDialog::on_replace, QKeySequence::Replace);
        addPannelAction(pannel, QStringLiteral("jmp"), tr("Goto"),
                        &ScriptingDialog::on_gotoline,
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
        m_Tbtneditors[ToolButtonIndex::DBG_RUN] =
            addPannelAction(pannel, QStringLiteral("dbgrun"), tr("Run"),
                            &ScriptingDialog::on_runscript,
                            QKeySequence(Qt::CTRL | Qt::Key_F5));
        m_Tbtneditors[ToolButtonIndex::DBG_RUN_DBG] = addPannelAction(
            pannel, QStringLiteral("dbgdebug"), tr("RunWithDbg"),
            &ScriptingDialog::on_rundbgscript, QKeySequence(Qt::Key_F5));
        m_dbgStateWidgets << addPannelAction(pannel, QStringLiteral("dbgpause"),
                                             tr("Pause"),
                                             &ScriptingDialog::on_pausescript);
        m_dbgStateWidgets << addPannelAction(
            pannel, QStringLiteral("dbgcontinue"), tr("Continue"),
            &ScriptingDialog::on_continuescript);
        m_dbgStateWidgets << addPannelAction(pannel, QStringLiteral("dbgstop"),
                                             tr("Stop"),
                                             &ScriptingDialog::on_stopscript);
        m_dbgStateWidgets << addPannelAction(
            pannel, QStringLiteral("dbgrestart"), tr("Restart"),
            &ScriptingDialog::on_restartscript);
        m_dbgStateWidgets << addPannelAction(
            pannel, QStringLiteral("dbgstepinto"), tr("StepInto"),
            &ScriptingDialog::on_stepinscript, QKeySequence(Qt::Key_F11));
        m_dbgStateWidgets << addPannelAction(
            pannel, QStringLiteral("dbgstepover"), tr("StepOver"),
            &ScriptingDialog::on_stepoverscript, QKeySequence(Qt::Key_F10));
        m_dbgStateWidgets << addPannelAction(
            pannel, QStringLiteral("dbgstepout"), tr("StepOut"),
            &ScriptingDialog::on_stepoutscript,
            QKeySequence(Qt::SHIFT | Qt::Key_F11));

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
                m_curEditor = editview;
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

        m_views.removeOne(editor);
        if (currentEditor() == editor) {
            m_curEditor = nullptr;
        }

        editor->deleteDockWidget();
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
    });

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

    auto runner = m_consoleout->machine();
    if (runner) {
        setRunDebugMode(runner->isRunning(), runner->isInDebugMode());
    } else {
        setRunDebugMode(false);
    }
}

ScriptEditor *ScriptingDialog::currentEditor() const { return m_curEditor; }

void ScriptingDialog::swapEditor(ScriptEditor *old, ScriptEditor *cur) {}

void ScriptingDialog::setRunDebugMode(bool isRun, bool isDebug) {
    m_Tbtneditors.value(ToolButtonIndex::DBG_RUN)->setEnabled(!isRun);
    m_Tbtneditors.value(ToolButtonIndex::DBG_RUN_DBG)->setEnabled(!isRun);
    for (auto &w : m_dbgStateWidgets) {
        w->setEnabled(isRun && isDebug);
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
    return m && m->isInDebugMode();
}

void ScriptingDialog::openFile(const QString &filename) {
    auto e = findEditorView(filename);
    if (e) {
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
    m_dock->addDockWidget(ads::CenterDockWidgetArea, editor, editorViewArea());
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

void ScriptingDialog::on_setting() {
    // TODO
    QEditConfig cfg;
    cfg.show();
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

void ScriptingDialog::on_runscript() {
    auto editor = currentEditor();
    if (editor) {
        auto e = editor->editor();
        m_consoleout->clear();
        setRunDebugMode(true, false);
        m_consoleout->machine()->executeScript(e->fileName());
        setRunDebugMode(false);
    }
}

void ScriptingDialog::on_rundbgscript() {
    auto editor = currentEditor();
    if (editor) {
        auto e = editor->editor();
        m_consoleout->clear();
        setRunDebugMode(true, true);
        m_consoleout->machine()->executeScript(e->fileName(), true);
        m_consoleout->appendCommandPrompt();
    }
}

void ScriptingDialog::on_pausescript() {}

void ScriptingDialog::on_continuescript() {}

void ScriptingDialog::on_stopscript() {}

void ScriptingDialog::on_restartscript() {}

void ScriptingDialog::on_stepinscript() {}

void ScriptingDialog::on_stepoutscript() {}

void ScriptingDialog::on_stepoverscript() {}

void ScriptingDialog::on_togglebreakpoint() {
    auto editor = currentEditor();
    if (editor) {
        QLineMark mrk(editor->fileName(),
                      editor->editor()->cursor().lineNumber() + 1,
                      m_symID.value(Symbols::BreakPoint));
        QLineMarksInfoCenter::instance()->toggleLineMark(mrk);
    }
}

void ScriptingDialog::on_addbreakpoint() {
    auto editor = currentEditor();
    if (editor) {
        auto line = editor->editor()->document()->line(
            editor->editor()->cursor().lineNumber());
        auto id = m_symID.value(Symbols::BreakPoint);
        if (!line.hasMark(id)) {
            line.addMark(id);
        }
    }
}

void ScriptingDialog::on_removebreakpoint() {
    auto editor = currentEditor();
    if (editor) {
        auto line = editor->editor()->document()->line(
            editor->editor()->cursor().lineNumber());
        auto id = m_symID.value(Symbols::BreakPoint);
        if (line.hasMark(id)) {
            line.removeMark(id);
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
        // TODO stop the script
    }

    auto &set = SettingManager::instance();

    set.setScriptDockLayout(m_dock->saveState());
    set.setRecentScriptFiles(m_recentmanager->saveRecent());
    set.save();

    FramelessMainWindow::closeEvent(event);
}
