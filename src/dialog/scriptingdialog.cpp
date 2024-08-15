#include "scriptingdialog.h"
#include "QWingRibbon/ribbontabcontent.h"
#include "Qt-Advanced-Docking-System/src/DockAreaWidget.h"
#include "class/languagemanager.h"
#include "class/qkeysequences.h"
#include "class/settingmanager.h"
#include "class/skinmanager.h"
#include "class/wingmessagebox.h"

#include <QDesktopServices>
#include <QLabel>
#include <QPainter>
#include <QPicture>
#include <QStatusBar>

ScriptingDialog::ScriptingDialog(QWidget *parent)
    : FramelessMainWindow(parent) {
    auto &skin = SkinManager::instance();
    switch (skin.currentTheme()) {
    case SkinManager::Theme::Dark: {

    } break;
    case SkinManager::Theme::Light:

        break;
    }

    // recent file manager init
    m_recentMenu = new QMenu(this);
    m_recentmanager = new RecentFileManager(m_recentMenu);
    m_recentmanager->apply(this,
                           SettingManager::instance().recentScriptFiles());

    // build up UI
    buildUpRibbonBar();

    auto cw = new QWidget(this);
    auto layout = new QVBoxLayout(cw);
    layout->setContentsMargins(1, 0, 1, 0);
    layout->setSpacing(0);
    layout->addWidget(q_check_ptr(m_ribbon));

    buildUpDockSystem(this);
    layout->addWidget(m_dock, 1);

    m_status = new QStatusBar(this);
    m_status->setContentsMargins(0, 1, 2, 2);

    layout->addWidget(m_status);
    buildUpContent(cw);

    // ok, preparing for starting...
    this->setWindowTitle(tr("ScriptEditor"));
    this->setWindowIcon(ICONRES(QStringLiteral("script")));
    this->setMinimumSize(400, 450);
}

void ScriptingDialog::buildUpRibbonBar() {
    m_ribbon = new Ribbon(this);
    buildFilePage(m_ribbon->addTab(tr("File")));
    m_editStateWidgets << buildEditPage(m_ribbon->addTab(tr("Edit")));
    m_editStateWidgets << buildDebugPage(m_ribbon->addTab(tr("Debugger")));
    buildScriptPage(m_ribbon->addTab(tr("Script")));
    buildAboutPage(m_ribbon->addTab(tr("About")));
}

RibbonTabContent *ScriptingDialog::buildFilePage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();
    {
        auto pannel = tab->addGroup(tr("Basic"));
        addPannelAction(pannel, QStringLiteral("new"), tr("New"),
                        &ScriptingDialog::on_newfile, QKeySequence::Open);

        addPannelAction(pannel, QStringLiteral("open"), tr("OpenF"),
                        &ScriptingDialog::on_openfile, QKeySequence::Open);

        addPannelAction(
            pannel, QStringLiteral("recent"), tr("RecentFiles"), [] {}, {},
            m_recentMenu);

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

        a = addPannelAction(pannel, QStringLiteral("export"), tr("Export"),
                            &ScriptingDialog::on_exportfile,
                            shortcuts.keySequence(QKeySequences::Key::EXPORT));

        m_editStateWidgets << a;

        a = addPannelAction(pannel, QStringLiteral("savesel"), tr("SaveSel"),
                            &ScriptingDialog::on_savesel);
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

        addPannelAction(pannel, QStringLiteral("jmp"), tr("Goto"),
                        &ScriptingDialog::on_gotoline,
                        shortcuts.keySequence(QKeySequences::Key::GOTO));

        addPannelAction(pannel, QStringLiteral("encoding"), tr("Encoding"),
                        &ScriptingDialog::on_encoding,
                        shortcuts.keySequence(QKeySequences::Key::ENCODING));
        addPannelAction(pannel, QStringLiteral("info"), tr("FileInfo"),
                        &ScriptingDialog::on_fileInfo,
                        shortcuts.keySequence(QKeySequences::Key::FILE_INFO));
    }

    return tab;
}

RibbonTabContent *ScriptingDialog::buildScriptPage(RibbonTabContent *tab) {

    {
        auto pannel = tab->addGroup(tr("Scripts"));
        connect(pannel, &RibbonButtonGroup::emptyStatusChanged, this,
                [pannel](bool isEmpty) { pannel->setVisible(!isEmpty); });
        // TODO
    }
    return tab;
}

RibbonTabContent *ScriptingDialog::buildDebugPage(RibbonTabContent *tab) {
    {
        auto pannel = tab->addGroup(tr("Debug"));
        addPannelAction(pannel, QStringLiteral("dbgrun"), tr("Run"),
                        &ScriptingDialog::on_runscript,
                        QKeySequence(Qt::CTRL | Qt::Key_F5));
        addPannelAction(pannel, QStringLiteral("dbgdebug"), tr("RunWithDbg"),
                        &ScriptingDialog::on_rundbgscript,
                        QKeySequence(Qt::Key_F5));
        addPannelAction(pannel, QStringLiteral("dbgpause"), tr("Pause"),
                        &ScriptingDialog::on_pausescript);
        addPannelAction(pannel, QStringLiteral("dbgcontinue"), tr("Continue"),
                        &ScriptingDialog::on_continuescript);
        addPannelAction(pannel, QStringLiteral("dbgstop"), tr("Stop"),
                        &ScriptingDialog::on_stopscript);
        addPannelAction(pannel, QStringLiteral("dbgrestart"), tr("Restart"),
                        &ScriptingDialog::on_restartscript);
        addPannelAction(pannel, QStringLiteral("dbgstepinto"), tr("StepInto"),
                        &ScriptingDialog::on_stepinscript,
                        QKeySequence(Qt::Key_F11));
        addPannelAction(pannel, QStringLiteral("dbgstepover"), tr("StepOver"),
                        &ScriptingDialog::on_stepoverscript,
                        QKeySequence(Qt::Key_F10));
        addPannelAction(pannel, QStringLiteral("dbgstepout"), tr("StepOut"),
                        &ScriptingDialog::on_stepoutscript,
                        QKeySequence(Qt::SHIFT | Qt::Key_F11));
    }
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
                // auto editview = qobject_cast<EditorView *>(now);
                // if (editview) {
                //     swapEditorConnection(m_curEditor, editview);
                // }
                // m_curEditor = editview;
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
}

void ScriptingDialog::registerEditorView(ScriptEditor *editor) {
    // TODO
}

ads::CDockAreaWidget *ScriptingDialog::editorViewArea() const {
    return m_dock->centralWidget()->dockAreaWidget();
}

void ScriptingDialog::on_newfile() {
    auto editor = new ScriptEditor(this);
    // auto index = m_newIndex++;
    // editor->newFile(index);
    // m_openedFileNames << editor->fileName();
    registerEditorView(editor);
    m_dock->addDockWidget(ads::CenterDockWidgetArea, editor, editorViewArea());
}

void ScriptingDialog::on_savesel() {}

void ScriptingDialog::on_openfile() {}

void ScriptingDialog::on_reload() {}

void ScriptingDialog::on_save() {}

void ScriptingDialog::on_saveas() {}

void ScriptingDialog::on_exportfile() {}

void ScriptingDialog::on_undofile() {}

void ScriptingDialog::on_redofile() {}

void ScriptingDialog::on_cutfile() {}

void ScriptingDialog::on_copyfile() {}

void ScriptingDialog::on_pastefile() {}

void ScriptingDialog::on_delete() {}

void ScriptingDialog::on_findfile() {}

void ScriptingDialog::on_gotoline() {}

void ScriptingDialog::on_encoding() {}

void ScriptingDialog::on_fileInfo() {}

void ScriptingDialog::on_about() {}

void ScriptingDialog::on_sponsor() {
    // Github is not easy to access for Chinese people,
    // Gitee mirror instead
    if (LanguageManager::instance().defaultLocale().country() ==
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

void ScriptingDialog::on_runscript() {}

void ScriptingDialog::on_rundbgscript() {}

void ScriptingDialog::on_pausescript() {}

void ScriptingDialog::on_continuescript() {}

void ScriptingDialog::on_stopscript() {}

void ScriptingDialog::on_restartscript() {}

void ScriptingDialog::on_stepinscript() {}

void ScriptingDialog::on_stepoutscript() {}

void ScriptingDialog::on_stepoverscript() {}
