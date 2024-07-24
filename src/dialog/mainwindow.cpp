#include "mainwindow.h"

#include "Qt-Advanced-Docking-System/src/DockAreaWidget.h"
#include "aboutsoftwaredialog.h"
#include "driverselectordialog.h"
#include "encodingdialog.h"
#include "fileinfodialog.h"
#include "finddialog.h"
#include "metadialog.h"
#include "openregiondialog.h"
#include "sponsordialog.h"
#include "src/class/logger.h"
#include "src/class/qkeysequences.h"
#include "src/class/scriptmanager.h"
#include "src/class/settingmanager.h"
#include "src/class/wingfiledialog.h"
#include "src/class/winginputdialog.h"
#include "src/class/wingmessagebox.h"
#include "src/control/toast.h"
#include "src/plugin/pluginsystem.h"
#include "src/settings/editorsettingdialog.h"
#include "src/settings/generalsettingdialog.h"
#include "src/settings/pluginsettingdialog.h"
#include "src/settings/scriptsettingdialog.h"

#include <QAction>
#include <QActionGroup>
#include <QClipboard>
#include <QDesktopServices>
#include <QGridLayout>
#include <QHeaderView>
#include <QPainter>
#include <QPicture>
#include <QPushButton>
#include <QRadioButton>
#include <QStatusBar>
#include <QTimer>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringDecoder>
#else
#include <QTextCodec>
#endif

constexpr auto EMPTY_FUNC = [] {};

MainWindow::MainWindow(QWidget *parent) : FramelessMainWindow(parent) {
    // recent file manager init
    m_recentMenu = new QMenu(this);
    m_recentmanager = new RecentFileManager(m_recentMenu);
    m_recentmanager->apply(this, SettingManager::instance().recentHexFiles());

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
    m_status->setContentsMargins(0, 1, 2, 2);

    // init statusbar
    {
        m_lblloc = new QLabel(QStringLiteral("(0,0)"), this);
        auto l = new QLabel(tr("loc:"), this);
        l->setMinimumWidth(50);
        l->setAlignment(Qt::AlignCenter);
        m_status->addWidget(l);
        m_status->addWidget(m_lblloc);

        l = new QLabel(tr("sel:"), this);
        l->setMinimumWidth(50);
        l->setAlignment(Qt::AlignCenter);
        m_lblsellen = new QLabel(QStringLiteral("0 - 0x0"), this);
        m_status->addWidget(l);
        m_status->addWidget(m_lblsellen);
    }

    layout->addWidget(m_status);

    buildUpContent(cw);

    m_scriptDialog = new ScriptingDialog(this);

    setEditModeEnabled(false);
    m_toolBtneditors.value(ToolButtonIndex::EDITOR_VIEWS)->setEnabled(false);

    // ok, preparing for starting...
    this->setWindowTitle(tr("WingHexExplorer"));
    this->setWindowIcon(Utilities::isRoot()
                            ? ICONRES(QStringLiteral("iconroot"))
                            : ICONRES(QStringLiteral("icon")));
    this->setMinimumSize(800, 600);

    // launch logging system
    connect(Logger::instance(), &Logger::log, m_logbrowser,
            &QTextBrowser::append);

    // launch plugin system
    auto &plg = PluginSystem::instance();
    plg.setMainWindow(this);
    plg.LoadPlugin();

    auto plgview = m_toolBtneditors.value(PLUGIN_VIEWS);
    plgview->setEnabled(!plgview->menu()->isEmpty());

    // ok, build up the dialog of setting
    buildUpSettingDialog();

    // Don't call show(WindowState::Maximized) diretly.
    // I don't know why it doesn't work.
    QTimer::singleShot(0, this, [this] { this->show(WindowState::Maximized); });
}

MainWindow::~MainWindow() { Logger::instance()->disconnect(); }

void MainWindow::buildUpRibbonBar() {
    m_ribbon = new Ribbon(this);

    loadCacheIcon();

    using RibbonCatagories = WingHex::WingRibbonToolBoxInfo::RibbonCatagories;
    RibbonCatagories catagories;

    m_ribbonMaps[catagories.FILE] = buildFilePage(m_ribbon->addTab(tr("File")));
    m_editStateWidgets << (m_ribbonMaps[catagories.EDIT] =
                               buildEditPage(m_ribbon->addTab(tr("Edit"))));
    m_ribbonMaps[catagories.VIEW] = buildViewPage(m_ribbon->addTab(tr("View")));
    m_ribbonMaps[catagories.SCRIPT] =
        buildScriptPage(m_ribbon->addTab(tr("Script")));
    m_ribbonMaps[catagories.PLUGIN] =
        buildPluginPage(m_ribbon->addTab(tr("Plugin")));
    m_ribbonMaps[catagories.SETTING] =
        buildSettingPage(m_ribbon->addTab(tr("Setting")));
    m_ribbonMaps[catagories.ABOUT] =
        buildAboutPage(m_ribbon->addTab(tr("About")));
}

void MainWindow::buildUpDockSystem(QWidget *container) {
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
                auto editview = qobject_cast<EditorView *>(now);
                if (editview) {
                    swapEditor(m_curEditor, editview);
                } else {
                    setEditModeEnabled(false);
                }
                m_curEditor = editview;
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
    auto rightArea = buildUpLogDock(m_dock, ads::RightDockWidgetArea);
    m_rightViewArea = rightArea;
    auto bottomLeftArea =
        buildUpFindResultDock(m_dock, ads::BottomDockWidgetArea);
    buildUpNumberShowDock(m_dock, ads::CenterDockWidgetArea, rightArea);
    buildUpHexBookMarkDock(m_dock, ads::CenterDockWidgetArea, rightArea);
    buildUpDecodingStrShowDock(m_dock, ads::CenterDockWidgetArea, rightArea);
    auto bottomRightArea = buildUpScriptConsoleDock(
        m_dock, ads::RightDockWidgetArea, bottomLeftArea);
    buildUpVisualDataDock(m_dock, ads::CenterDockWidgetArea, bottomLeftArea);
    buildUpScriptVarShowDock(m_dock, ads::CenterDockWidgetArea,
                             bottomRightArea);
    m_bottomViewArea = bottomRightArea;

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

ads::CDockAreaWidget *MainWindow::buildUpLogDock(ads::CDockManager *dock,
                                                 ads::DockWidgetArea area,
                                                 ads::CDockAreaWidget *areaw) {
    m_logbrowser = new QTextBrowser(this);
    m_logbrowser->setFocusPolicy(Qt::StrongFocus);
    m_logbrowser->setOpenExternalLinks(true);
    m_logbrowser->setUndoRedoEnabled(false);

    auto dw =
        buildDockWidget(dock, QStringLiteral("Log"), tr("Log"), m_logbrowser);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpFindResultDock(ads::CDockManager *dock,
                                  ads::DockWidgetArea area,
                                  ads::CDockAreaWidget *areaw) {
    auto findresultMenu = new QMenu(this);
    findresultMenu->addAction(newAction(QStringLiteral("export"),
                                        tr("ExportFindResult"),
                                        &MainWindow::on_exportfindresult));
    findresultMenu->addAction(newAction(QStringLiteral("del"),
                                        tr("ClearFindResult"),
                                        &MainWindow::on_clearfindresult));

    m_findresult = new QTableWidget(0, 2, this);
    m_findresult->setProperty("EditorView", quintptr(0));
    m_findresult->setEditTriggers(QTableWidget::EditTrigger::NoEditTriggers);
    m_findresult->setSelectionBehavior(
        QAbstractItemView::SelectionBehavior::SelectRows);
    m_findresult->setHorizontalHeaderLabels(
        QStringList({tr("addr"), tr("value")}));
    m_findresult->horizontalHeader()->setStretchLastSection(true);
    m_findresult->setContextMenuPolicy(
        Qt::ContextMenuPolicy::CustomContextMenu);
    connect(m_findresult, &QTableWidget::customContextMenuRequested, this,
            [=]() { findresultMenu->popup(cursor().pos()); });
    connect(m_findresult, &QTableWidget::itemDoubleClicked, this, [=] {
        auto editor =
            m_findresult->property("EditorView").value<EditorView *>();
        auto item = m_findresult->item(m_findresult->currentRow(), 0);
        auto hexeditor = m_curEditor.loadAcquire();
        if (hexeditor != editor) {
            editor->raise();
            editor->setFocus();
        }
        editor->hexEditor()->document()->cursor()->moveTo(
            item->data(Qt::UserRole).toLongLong());
    });

    auto dw = buildDockWidget(dock, QStringLiteral("FindResult"),
                              tr("FindResult"), m_findresult);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpNumberShowDock(ads::CDockManager *dock,
                                  ads::DockWidgetArea area,
                                  ads::CDockAreaWidget *areaw) {
    m_numshowtable = new QTableWidget(8, 1, this);
    m_numshowtable->setEditTriggers(QTableWidget::EditTrigger::NoEditTriggers);
    m_numshowtable->setSelectionBehavior(
        QAbstractItemView::SelectionBehavior::SelectRows);
    m_numshowtable->setHorizontalHeaderLabels(QStringList(tr("Value")));
    m_numshowtable->setColumnWidth(0, 350);
    m_numshowtable->setFocusPolicy(Qt::StrongFocus);
    m_numshowtable->setVerticalHeaderLabels(
        QStringList({"byte", "char", "ushort", "short", "uint32", "int32",
                     "uint64", "int64"}));
    m_numshowtable->horizontalHeader()->setStretchLastSection(true);

    auto numtableMenu = new QMenu(m_numshowtable);
    auto a = new QAction(numtableMenu);
    a->setText(tr("Copy"));
    connect(a, &QAction::triggered, this, [=] {
        auto r = m_numshowtable->currentRow();
        qApp->clipboard()->setText(_numsitem[r]->text());
        Toast::toast(this, NAMEICONRES(QStringLiteral("copy")),
                     tr("CopyToClipBoard"));
    });
    numtableMenu->addAction(a);
    numtableMenu->addSeparator();

    auto actionGroup = new QActionGroup(this);
    actionGroup->setExclusive(true);

    auto le = Utilities::checkIsLittleEndian();
    m_littleEndian = new QAction(actionGroup);
    m_littleEndian->setText(tr("LittleEndian"));
    m_littleEndian->setCheckable(true);
    m_littleEndian->setChecked(le);
    connect(m_littleEndian, &QAction::triggered, this, [=] {
        m_islittle = true;
        this->on_locChanged();
    });
    numtableMenu->addAction(m_littleEndian);

    m_bigEndian = new QAction(actionGroup);
    m_bigEndian->setText(tr("BigEndian"));
    m_bigEndian->setCheckable(true);
    m_bigEndian->setChecked(!le);
    connect(m_bigEndian, &QAction::triggered, this, [=] {
        m_islittle = false;
        this->on_locChanged();
    });
    numtableMenu->addAction(m_bigEndian);

    m_numshowtable->setContextMenuPolicy(
        Qt::ContextMenuPolicy::CustomContextMenu);
    connect(m_numshowtable, &QTableWidget::customContextMenuRequested, this,
            [=] { numtableMenu->popup(cursor().pos()); });

    _numsitem.fill(nullptr, NumTableIndexCount);
    for (int i = 0; i < NumTableIndexCount; i++) {
        auto item = new QTableWidgetItem;
        item->setText(QStringLiteral("-"));
        item->setTextAlignment(Qt::AlignCenter);
        _numsitem[i] = item;
        m_numshowtable->setItem(i, 0, item);
    }

    auto dw = buildDockWidget(dock, QStringLiteral("Number"), tr("Number"),
                              m_numshowtable);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpHexBookMarkDock(ads::CDockManager *dock,
                                   ads::DockWidgetArea area,
                                   ads::CDockAreaWidget *areaw) {
    m_bookmarks = new QListWidget(this);
    auto menu = new QMenu(m_bookmarks);
    auto a = new QAction(ICONRES(QStringLiteral("bookmarkdel")),
                         tr("DeleteBookMark"), menu);
    connect(a, &QAction::triggered, this, [=] {
        if (m_curEditor == nullptr) {
            return;
        }

        auto s = m_bookmarks->selectedItems();
        QList<qsizetype> pos;
        for (auto &item : s) {
            pos.push_back(item->data(Qt::UserRole).toLongLong());
        }

        auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
        hexeditor->document()->RemoveBookMarks(pos);
    });
    menu->addAction(a);
    a = new QAction(ICONRES(QStringLiteral("bookmarkcls")), tr("ClearBookMark"),
                    menu);
    connect(a, &QAction::triggered, this, &MainWindow::on_bookmarkcls);
    menu->addAction(a);

    m_bookmarks->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    m_bookmarks->setSelectionMode(
        QListWidget::SelectionMode::ExtendedSelection);
    m_bookmarks->setFocusPolicy(Qt::StrongFocus);
    connect(m_bookmarks, &QListWidget::itemDoubleClicked, this, [=]() {
        if (m_curEditor == nullptr) {
            return;
        }
        auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
        hexeditor->renderer()->enableCursor(true);
        hexeditor->document()->gotoBookMark(m_bookmarks->currentRow());
    });
    connect(m_bookmarks, &QListWidget::customContextMenuRequested, this,
            [=] { menu->popup(cursor().pos()); });

    auto dw = buildDockWidget(dock, QStringLiteral("BookMark"), tr("BookMark"),
                              m_bookmarks);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpDecodingStrShowDock(ads::CDockManager *dock,
                                       ads::DockWidgetArea area,
                                       ads::CDockAreaWidget *areaw) {
    m_txtDecode = new QTextBrowser(this);
    m_txtDecode->setUndoRedoEnabled(false);

    auto dw = buildDockWidget(dock, QStringLiteral("DecodeText"),
                              tr("DecodeText"), m_txtDecode);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpScriptConsoleDock(ads::CDockManager *dock,
                                     ads::DockWidgetArea area,
                                     ads::CDockAreaWidget *areaw) {
    m_scriptConsole = new ScriptingConsole(this);

    auto dw = buildDockWidget(dock, QStringLiteral("ScriptConsole"),
                              tr("ScriptConsole"), m_scriptConsole);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpScriptVarShowDock(ads::CDockManager *dock,
                                     ads::DockWidgetArea area,
                                     ads::CDockAreaWidget *areaw) {
    m_varshowtable = new QTableWidget(0, 2, this);
    m_varshowtable->setEditTriggers(QTableWidget::EditTrigger::DoubleClicked);
    m_varshowtable->setSelectionBehavior(
        QAbstractItemView::SelectionBehavior::SelectRows);
    m_varshowtable->setHorizontalHeaderLabels(
        QStringList({tr("Name"), tr("Value")}));
    m_varshowtable->horizontalHeader()->setStretchLastSection(true);

    auto dw = buildDockWidget(dock, QStringLiteral("ScriptVarShow"),
                              tr("ScriptVarShow"), m_varshowtable);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpVisualDataDock(ads::CDockManager *dock,
                                  ads::DockWidgetArea area,
                                  ads::CDockAreaWidget *areaw) {
    using namespace ads;

    m_infolist = new QListWidget(this);
    auto dw = buildDockWidget(dock, QStringLiteral("DVList"), tr("DVList"),
                              m_infolist);
    auto ar = dock->addDockWidget(area, dw, areaw);

    auto m_infotree = new QTreeWidget(this);
    m_infotree->setHeaderLabel(QString());
    dw = buildDockWidget(dock, QStringLiteral("DVTree"), tr("DVTree"),
                         m_infotree);
    dock->addDockWidget(CenterDockWidgetArea, dw, ar);

    auto m_infotable = new QTableWidget(this);
    dw = buildDockWidget(dock, QStringLiteral("DVTable"), tr("DVTable"),
                         m_infotable);
    dock->addDockWidget(CenterDockWidgetArea, dw, ar);

    auto m_infotxt = new QTextBrowser(this);
    dw = buildDockWidget(dock, QStringLiteral("DVText"), tr("DVText"),
                         m_infotxt);
    dock->addDockWidget(CenterDockWidgetArea, dw, ar);

    return ar;
}

RibbonTabContent *MainWindow::buildFilePage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();
    {
        auto pannel = tab->addGroup(tr("Basic"));
        addPannelAction(pannel, QStringLiteral("new"), tr("New"),
                        &MainWindow::on_newfile, QKeySequence::New);

        addPannelAction(pannel, QStringLiteral("open"), tr("OpenF"),
                        &MainWindow::on_openfile, QKeySequence::Open);

        addPannelAction(
            pannel, QStringLiteral("openr"), tr("OpenFR"),
            &MainWindow::on_openregion,
            shortcuts.keySequence(QKeySequences::Key::OPEN_REGION_FILE));

        addPannelAction(
            pannel, QStringLiteral("workspace"), tr("OpenWorkSpace"),
            &MainWindow::on_openworkspace,
            shortcuts.keySequence(QKeySequences::Key::OPEN_WORKSPACE));

        addPannelAction(pannel, QStringLiteral("opendriver"), tr("OpenD"),
                        &MainWindow::on_opendriver);

        addPannelAction(pannel, QStringLiteral("recent"), tr("RecentFiles"),
                        EMPTY_FUNC, {}, m_recentMenu);

        m_editStateWidgets << addPannelAction(pannel, QStringLiteral("reload"),
                                              tr("Reload"),
                                              &MainWindow::on_reload);
    }

    {
        auto pannel = tab->addGroup(tr("Save"));

        auto a = addPannelAction(pannel, QStringLiteral("save"), tr("Save"),
                                 &MainWindow::on_save, QKeySequence::Save);
        m_driverStateWidgets << a;
        m_editStateWidgets << a;

        a = addPannelAction(pannel, QStringLiteral("saveas"), tr("SaveAs"),
                            &MainWindow::on_saveas,
                            shortcuts.keySequence(QKeySequences::Key::SAVE_AS));
        m_driverStateWidgets << a;
        m_editStateWidgets << a;

        a = addPannelAction(pannel, QStringLiteral("export"), tr("Export"),
                            &MainWindow::on_exportfile,
                            shortcuts.keySequence(QKeySequences::Key::EXPORT));
        m_driverStateWidgets << a;
        m_editStateWidgets << a;

        a = addPannelAction(pannel, QStringLiteral("savesel"), tr("SaveSel"),
                            &MainWindow::on_savesel);
        m_driverStateWidgets << a;
        m_editStateWidgets << a;
    }

    return tab;
}

RibbonTabContent *MainWindow::buildEditPage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();
    {
        auto pannel = tab->addGroup(tr("General"));
        m_toolBtneditors.insert(
            ToolButtonIndex::UNDO_ACTION,
            addPannelAction(pannel, QStringLiteral("undo"), tr("Undo"),
                            &MainWindow::on_undofile, QKeySequence::Undo));
        m_toolBtneditors.insert(
            ToolButtonIndex::REDO_ACTION,
            addPannelAction(pannel, QStringLiteral("redo"), tr("Redo"),
                            &MainWindow::on_redofile, QKeySequence::Redo));
        addPannelAction(pannel, QStringLiteral("cut"), tr("Cut"),
                        &MainWindow::on_cutfile, QKeySequence::Cut);
        addPannelAction(pannel, QStringLiteral("copy"), tr("Copy"),
                        &MainWindow::on_copyfile, QKeySequence::Copy);
        addPannelAction(pannel, QStringLiteral("paste"), tr("Paste"),
                        &MainWindow::on_pastefile, QKeySequence::Paste);
        addPannelAction(pannel, QStringLiteral("del"), tr("Delete"),
                        &MainWindow::on_delete, QKeySequence::Delete);
        addPannelAction(pannel, QStringLiteral("clone"), tr("Clone"),
                        &MainWindow::on_clone);
    }

    {
        auto pannel = tab->addGroup(tr("Lookup"));
        addPannelAction(pannel, QStringLiteral("find"), tr("Find"),
                        &MainWindow::on_findfile, QKeySequence::Find);

        addPannelAction(pannel, QStringLiteral("jmp"), tr("Goto"),
                        &MainWindow::on_gotoline,
                        shortcuts.keySequence(QKeySequences::Key::GOTO));

        addPannelAction(pannel, QStringLiteral("encoding"), tr("Encoding"),
                        &MainWindow::on_encoding,
                        shortcuts.keySequence(QKeySequences::Key::ENCODING));
        addPannelAction(pannel, QStringLiteral("info"), tr("FileInfo"),
                        &MainWindow::on_fileInfo,
                        shortcuts.keySequence(QKeySequences::Key::FILE_INFO));
    }

    {
        auto pannel = tab->addGroup(tr("Hex"));
        addPannelAction(pannel, QStringLiteral("cuthex"), tr("CutHex"),
                        &MainWindow::on_cuthex,
                        shortcuts.keySequence(QKeySequences::Key::CUT_HEX));

        addPannelAction(pannel, QStringLiteral("copyhex"), tr("CopyHex"),
                        &MainWindow::on_copyhex,
                        shortcuts.keySequence(QKeySequences::Key::COPY_HEX));
        addPannelAction(pannel, QStringLiteral("pastehex"), tr("PasteHex"),
                        &MainWindow::on_pastehex,
                        shortcuts.keySequence(QKeySequences::Key::PASTE_HEX));
        addPannelAction(pannel, QStringLiteral("fill"), tr("Fill"),
                        &MainWindow::on_fill,
                        shortcuts.keySequence(QKeySequences::Key::HEX_FILL));
        addPannelAction(pannel, QStringLiteral("fillZero"), tr("FillZero"),
                        &MainWindow::on_fillzero,
                        shortcuts.keySequence(QKeySequences::Key::HEX_FILL0));
    }

    {
        auto pannel = tab->addGroup(tr("MetaData"));
        addPannelAction(pannel, QStringLiteral("bookmark"), tr("BookMark"),
                        &MainWindow::on_bookmark,
                        shortcuts.keySequence(QKeySequences::Key::BOOKMARK));
        addPannelAction(
            pannel, QStringLiteral("bookmarkdel"), tr("DeleteBookMark"),
            &MainWindow::on_bookmarkdel,
            shortcuts.keySequence(QKeySequences::Key::BOOKMARK_DEL));
        addPannelAction(
            pannel, QStringLiteral("bookmarkcls"), tr("ClearBookMark"),
            &MainWindow::on_bookmarkcls,
            shortcuts.keySequence(QKeySequences::Key::BOOKMARK_CLS));
        addPannelAction(pannel, QStringLiteral("metadata"), tr("MetaData"),
                        &MainWindow::on_metadata,
                        shortcuts.keySequence(QKeySequences::Key::METADATA));
        addPannelAction(
            pannel, QStringLiteral("metadataedit"), tr("MetaDataEdit"),
            &MainWindow::on_metadataedit,
            shortcuts.keySequence(QKeySequences::Key::METADATA_EDIT));
        addPannelAction(
            pannel, QStringLiteral("metadatadel"), tr("DeleteMetaData"),
            &MainWindow::on_metadatadel,
            shortcuts.keySequence(QKeySequences::Key::METADATA_DEL));
        addPannelAction(
            pannel, QStringLiteral("metadatacls"), tr("ClearMetaData"),
            &MainWindow::on_metadatacls,
            shortcuts.keySequence(QKeySequences::Key::METADATA_CLS));
    }

    return tab;
}

RibbonTabContent *MainWindow::buildViewPage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();
    {
        auto pannel = tab->addGroup(tr("Display"));
        auto menu = new QMenu(this);
        menu->addAction(newAction(QStringLiteral("80%"), [this] {
            this->setCurrentHexEditorScale(0.8);
        }));
        menu->addAction(newAction(QStringLiteral("90%"), [this] {
            this->setCurrentHexEditorScale(0.9);
        }));
        menu->addAction(newAction(QStringLiteral("100%"), [this] {
            this->setCurrentHexEditorScale(1.0);
        }));
        menu->addSeparator();
        menu->addAction(newAction(QStringLiteral("120%"), [this] {
            this->setCurrentHexEditorScale(1.2);
        }));
        menu->addAction(newAction(QStringLiteral("150%"), [this] {
            this->setCurrentHexEditorScale(1.5);
        }));
        menu->addAction(newAction(QStringLiteral("200%"), [this] {
            this->setCurrentHexEditorScale(2.0);
        }));
        menu->addAction(newAction(QStringLiteral("250%"), [this] {
            this->setCurrentHexEditorScale(2.5);
        }));
        menu->addAction(newAction(QStringLiteral("300%"), [this] {
            this->setCurrentHexEditorScale(3.0);
        }));
        addPannelAction(pannel, QStringLiteral("scale"), tr("Scale"),
                        EMPTY_FUNC, {}, menu);
        addPannelAction(pannel, QStringLiteral("scalereset"), tr("ResetScale"),
                        [this] { this->setCurrentHexEditorScale(1.0); });
        m_editStateWidgets << pannel;
    }

    {
        auto pannel = tab->addGroup(tr("MetaData"));

        auto menu = new QMenu(this);
        menu->addAction(
            m_aShowMetafg = newCheckableAction(
                menu, tr("ShowMetafg"), &MainWindow::on_metadatafg,
                shortcuts.keySequence(QKeySequences::Key::METADATA_FG)));
        menu->addAction(
            m_aShowMetabg = newCheckableAction(
                menu, tr("ShowMetabg"), &MainWindow::on_metadatabg,
                shortcuts.keySequence(QKeySequences::Key::METADATA_BG)));
        menu->addAction(
            m_aShowMetaComment = newCheckableAction(
                menu, tr("ShowMetaComment"), &MainWindow::on_metadatacomment,
                shortcuts.keySequence(QKeySequences::Key::METADATA_COMMENT)));

        addPannelAction(pannel, QStringLiteral("metadata"), tr("MetaData"),
                        EMPTY_FUNC, {}, menu);

        addPannelAction(
            pannel, QStringLiteral("metashow"), tr("MetaShowAll"),
            &MainWindow::on_metashowall,
            shortcuts.keySequence(QKeySequences::Key::METADATA_SHOW));
        addPannelAction(
            pannel, QStringLiteral("metahide"), tr("MetaHideAll"),
            &MainWindow::on_metahideall,
            shortcuts.keySequence(QKeySequences::Key::METADATA_HIDE));
        m_editStateWidgets << pannel;
    }

    {
        auto pannel = tab->addGroup(tr("FileStatus"));
        auto disableStyle =
            QStringLiteral("border:none;background:transparent;");
        m_iSaved =
            addPannelAction(pannel, _infoSaved, tr("InfoSave"), EMPTY_FUNC);
        m_iSaved->setStyleSheet(disableStyle);
        m_iReadWrite =
            addPannelAction(pannel, _infoReadonly, tr("ReadOnly"), EMPTY_FUNC);
        m_iReadWrite->setStyleSheet(disableStyle);
        m_iWorkSpace =
            addPannelAction(pannel, _infouw, tr("IsWorkSpace"), [this]() {
                if (m_curEditor == nullptr) {
                    return;
                }
                auto editor = m_curEditor.loadAcquire();
                if (editor->isWorkSpace()) {
                    WingMessageBox::information(
                        this, qAppName(),
                        tr("AlreadyWorkSpace") + QStringLiteral("\n") +
                            tr("Path:") + m_views[editor]);
                    return;
                }
                auto ret = WingMessageBox::question(this, qAppName(),
                                                    tr("Convert2WorkSpace?"));
                if (ret == QMessageBox::Yes) {
                    editor->setIsWorkSpace(true);
                }
            });
        m_iLocked =
            addPannelAction(pannel, _infoLock, tr("SetLocked"), [this]() {
                if (m_curEditor == nullptr) {
                    return;
                }
                auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
                if (!hexeditor->setLockedFile(!hexeditor->isLocked())) {
                    Toast::toast(this, _pixLock, tr("ErrUnLock"));
                }
            });
        m_iCanOver =
            addPannelAction(pannel, _infoCanOver, tr("SetOver"), [this]() {
                if (m_curEditor == nullptr) {
                    return;
                }

                auto editor = m_curEditor.loadAcquire();
                auto hexeditor = editor->hexEditor();

                auto b = !hexeditor->isKeepSize();
                if ((!b && editor->documentType() ==
                               EditorView::DocumentType::RegionFile) ||
                    !hexeditor->setKeepSize(b)) {
                    Toast::toast(this, _pixCannotOver, tr("ErrUnOver"));
                } else {
                    if (hexeditor->document()->metadata()->hasMetadata()) {
                        Toast::toast(this, _pixCanOver, tr("InfoCanOverLimit"));
                    }
                }
            });
        m_editStateWidgets << pannel;
    }

    {
        auto pannel = tab->addGroup(tr("Window"));
        m_toolBtneditors.insert(ToolButtonIndex::EDITOR_VIEWS,
                                addPannelAction(pannel, QStringLiteral("file"),
                                                tr("Editor"), EMPTY_FUNC, {},
                                                new QMenu(this)));
        auto edwins = new QMenu(this);
        edwins->addAction(newAction(QStringLiteral("file"), tr("Hex"), [this] {
            if (m_curEditor == nullptr) {
                return;
            }
            auto editor = m_curEditor.loadAcquire();
            editor->switchView(-1);
        }));
        m_toolBtneditors.insert(ToolButtonIndex::EDITOR_WINS,
                                addPannelAction(pannel, QStringLiteral("win"),
                                                tr("View"), EMPTY_FUNC, {},
                                                edwins));
        m_toolBtneditors.insert(
            ToolButtonIndex::TOOL_VIEWS,
            addPannelAction(pannel, QStringLiteral("general"), tr("Tools"),
                            EMPTY_FUNC, {}, new QMenu(this)));

        m_toolBtneditors.insert(ToolButtonIndex::PLUGIN_VIEWS,
                                addPannelAction(pannel, QStringLiteral("edit"),
                                                tr("Plugin"), EMPTY_FUNC, {},
                                                new QMenu(this)));
        m_editStateWidgets << m_toolBtneditors.value(
            ToolButtonIndex::EDITOR_WINS);
    }

    {
        auto pannel = tab->addGroup(tr("HexEditorLayout"));
        addPannelAction(
            pannel, QStringLiteral("mAddr"), tr("SetBaseAddr"), [this]() {
                if (m_curEditor == nullptr) {
                    return;
                }
                auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
                bool b;
                auto num = WingInputDialog::getText(
                    this, tr("addressBase"), tr("inputAddressBase"),
                    QLineEdit::Normal, QString(), &b);
                if (b) {
                    quintptr qnum = num.toULongLong(&b, 0);
                    if (b) {
                        auto r = qnum + quintptr(hexeditor->documentBytes());
                        if (qnum > r ||
                            quintptr(hexeditor->documentBytes()) > r) {
                            Toast::toast(this,
                                         NAMEICONRES(QStringLiteral("mAddr")),
                                         tr("WarnBigBaseAddress"));
                        }
                        hexeditor->setAddressBase(qnum);
                    } else {
                        Toast::toast(this, NAMEICONRES(QStringLiteral("mAddr")),
                                     tr("ErrBaseAddress"));
                    }
                }
            });
        addPannelAction(
            pannel, QStringLiteral("mColInfo"), tr("SetColInfo"), [this]() {
                if (m_curEditor == nullptr) {
                    return;
                }
                auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
                hexeditor->setAddressVisible(!hexeditor->addressVisible());
            });
        addPannelAction(
            pannel, QStringLiteral("mLineInfo"), tr("SetHeaderInfo"), [this]() {
                if (m_curEditor == nullptr) {
                    return;
                }
                auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
                hexeditor->setHeaderVisible(!hexeditor->headerVisible());
            });
        addPannelAction(
            pannel, QStringLiteral("mStr"), tr("SetAsciiString"), [this]() {
                if (m_curEditor == nullptr) {
                    return;
                }
                auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
                hexeditor->setAsciiVisible(!hexeditor->asciiVisible());
            });
        m_editStateWidgets << pannel;
    }

    {
        auto pannel = tab->addGroup(tr("Layout"));
        addPannelAction(pannel, QStringLiteral("fullscreen"), tr("Fullscreen"),
                        &MainWindow::on_fullScreen);
        addPannelAction(pannel, QStringLiteral("layout"), tr("RestoreLayout"),
                        &MainWindow::on_restoreLayout);
    }

    {
        auto pannel = tab->addGroup(tr("Log"));
        addPannelAction(pannel, QStringLiteral("log"), tr("ExportLog"),
                        &MainWindow::on_exportlog);
        addPannelAction(pannel, QStringLiteral("clearhis"), tr("ClearLog"),
                        &MainWindow::on_clslog);
    }

    return tab;
}

RibbonTabContent *MainWindow::buildScriptPage(RibbonTabContent *tab) {
    {
        auto pannel = tab->addGroup(tr("Basic"));
        addPannelAction(pannel, QStringLiteral("script"), tr("ScriptEditor"),
                        &MainWindow::on_scriptwindow);
    }

    {
        auto pannel = tab->addGroup(tr("Scripts"));
        pannel->setVisible(false);
        connect(pannel, &RibbonButtonGroup::emptyStatusChanged, this,
                [pannel](bool isEmpty) { pannel->setVisible(!isEmpty); });
        m_scriptDBGroup = pannel;
    }

    return tab;
}

RibbonTabContent *MainWindow::buildPluginPage(RibbonTabContent *tab) {
    {
        auto pannel = tab->addGroup(tr("General"));
        addPannelAction(pannel, QStringLiteral("loadplg"), tr("LoadPlugin"),
                        &MainWindow::on_loadplg);
    }

    {
        auto pannel = tab->addGroup(tr("PluginFunctions"));
        pannel->setVisible(false);
        connect(pannel, &RibbonButtonGroup::emptyStatusChanged, this,
                [pannel](bool isEmpty) { pannel->setVisible(!isEmpty); });
    }
    return tab;
}

RibbonTabContent *MainWindow::buildSettingPage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();
    {
        auto pannel = tab->addGroup(tr("General"));
        addPannelAction(
            pannel, QStringLiteral("general"), tr("General"),
            &MainWindow::on_setting_general,
            shortcuts.keySequence(QKeySequences::Key::SETTING_GENERAL));
        addPannelAction(
            pannel, QStringLiteral("settingplugin"), tr("Plugin"),
            &MainWindow::on_setting_plugin,
            shortcuts.keySequence(QKeySequences::Key::SETTING_PLUGIN));
    }

    {
        auto pannel = tab->addGroup(tr("PluginSettings"));
        pannel->setVisible(false);
        connect(pannel, &RibbonButtonGroup::emptyStatusChanged, this,
                [pannel](bool isEmpty) { pannel->setVisible(!isEmpty); });
    }

    return tab;
}

RibbonTabContent *MainWindow::buildAboutPage(RibbonTabContent *tab) {
    auto pannel = tab->addGroup(tr("Info"));

    addPannelAction(pannel, QStringLiteral("soft"), tr("Software"),
                    &MainWindow::on_about);

    addPannelAction(pannel, QStringLiteral("sponsor"), tr("Sponsor"),
                    &MainWindow::on_sponsor);

    addPannelAction(pannel, QStringLiteral("wiki"), tr("Wiki"),
                    &MainWindow::on_wiki);

    addPannelAction(pannel, QStringLiteral("qt"), tr("AboutQT"),
                    [this] { WingMessageBox::aboutQt(this); });

    return tab;
}

void MainWindow::buildUpSettingDialog() {
    m_setdialog = new SettingDialog(this);
    auto generalPage = new GeneralSettingDialog(m_setdialog);
    m_setdialog->addPage(generalPage);
    auto editorPage = new EditorSettingDialog(m_setdialog);
    m_setdialog->addPage(editorPage);
    auto plgPage = new PluginSettingDialog(m_setdialog);
    m_setdialog->addPage(plgPage);

    auto scriptPage = new ScriptSettingDialog(m_setdialog);
    // TODO
    for (auto &cat : scriptPage->sysDisplayCats()) {
        addPannelAction(m_scriptDBGroup,
                        ICONRES(QStringLiteral("scriptfolder")), cat,
                        EMPTY_FUNC, {}, new QMenu(this));
    }
    for (auto &cat : scriptPage->usrDisplayCats()) {
        addPannelAction(m_scriptDBGroup,
                        ICONRES(QStringLiteral("scriptfolderusr")), cat,
                        EMPTY_FUNC, {}, new QMenu(this));
    }
    m_setdialog->addPage(scriptPage);

    m_setdialog->build();
}

EditorView *MainWindow::newfileGUI() {
    if (!newOpenFileSafeCheck()) {
        return nullptr;
    }
    auto editor = new EditorView(m_enablePlugin);
    auto index = m_newIndex++;
    editor->newFile(index);
    m_openedFileNames << editor->fileName();
    registerEditorView(editor);
    m_dock->addDockWidget(ads::CenterDockWidgetArea, editor, m_editorViewArea);
    return editor;
}

void MainWindow::on_newfile() { newfileGUI(); }

void MainWindow::on_openfile() {
    auto filename =
        WingFileDialog::getOpenFileName(this, tr("ChooseFile"), m_lastusedpath);
    if (!filename.isEmpty()) {
        m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();
        EditorView *editor;
        auto res = openFile(filename, &editor);
        if (res == ErrFile::NotExist) {
            WingMessageBox::critical(this, tr("Error"), tr("FileNotExist"));
            return;
        }
        if (res == ErrFile::Permission) {
            WingMessageBox::critical(this, tr("Error"), tr("FilePermission"));
            return;
        }
        if (res == ErrFile::AlreadyOpened) {
            Q_ASSERT(editor);
            editor->raise();
            editor->setFocus();
            return;
        }
    }
    m_recentmanager->addRecentFile(filename);
}

void MainWindow::on_openregion() {
    OpenRegionDialog d(m_lastusedpath);
    if (d.exec()) {
        auto res = d.getResult();
        EditorView *editor;
        auto ret = openRegionFile(res.filename, &editor, res.start, res.length);
        if (ret == ErrFile::NotExist) {
            QMessageBox::critical(this, tr("Error"), tr("FileNotExist"));
            return;
        }
        if (ret == ErrFile::Permission) {
            QMessageBox::critical(this, tr("Error"), tr("FilePermission"));
            return;
        }
        if (ret == ErrFile::AlreadyOpened) {
            Q_ASSERT(editor);
            editor->raise();
            editor->setFocus();
            return;
        }
    }
}

void MainWindow::on_openworkspace() {
    auto filename = WingFileDialog::getOpenFileName(
        this, tr("ChooseFile"), m_lastusedpath, tr("ProjectFile (*.wingpro)"));
    if (filename.isEmpty())
        return;
    m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();
    EditorView *editor;
    auto res = openWorkSpace(filename, &editor);
    if (res == ErrFile::NotExist) {
        WingMessageBox::critical(this, tr("Error"), tr("FileNotExist"));
        return;
    }
    if (res == ErrFile::Permission) {
        WingMessageBox::critical(this, tr("Error"), tr("FilePermission"));
        return;
    }
    if (res == ErrFile::AlreadyOpened) {
        Q_ASSERT(editor);
        editor->raise();
        editor->setFocus();
        return;
    }
    m_recentmanager->addRecentFile(filename);
}

void MainWindow::on_opendriver() {
    if (!Utilities::isRoot()) {
        WingMessageBox::critical(this, tr("Error"), tr("Root Required!"));
        return;
    }

    DriverSelectorDialog ds;
    if (ds.exec()) {
        EditorView *editor;
        auto res = openDriver(ds.GetResult().device(), &editor);
        if (res == ErrFile::NotExist) {
            WingMessageBox::critical(this, tr("Error"), tr("FileNotExist"));
            return;
        }
        if (res == ErrFile::Permission) {
            WingMessageBox::critical(this, tr("Error"), tr("FilePermission"));
            return;
        }
        if (res == ErrFile::AlreadyOpened) {
            Q_ASSERT(editor);
            editor->raise();
            editor->setFocus();
            return;
        }
    }
}

void MainWindow::on_reload() {
    if (m_curEditor == nullptr) {
        return;
    }
    m_curEditor.loadAcquire()->reload();
}

void MainWindow::on_save() {
    if (m_curEditor == nullptr) {
        return;
    }

    auto editor = m_curEditor.loadAcquire();
    auto isNewFile = editor->isNewFile();
    if (isNewFile) {
        on_saveas();
        return;
    }

    QString workspace = m_views.value(editor);
    if (editor->change2WorkSpace()) {
        workspace = editor->fileName() + PROEXT;
    }

    auto res = editor->save(workspace);
restart:
    if (res == ErrFile::Permission) {
        WingMessageBox::critical(this, tr("Error"), tr("FilePermission"));
        return;
    }
    if (res == ErrFile::SourceFileChanged) {
        if (WingMessageBox::warning(this, tr("Warn"), tr("SourceChanged"),
                                    QMessageBox::Yes | QMessageBox::No) ==
            QMessageBox::Yes) {
            res = editor->save(workspace);
            goto restart;
        } else {
            Toast::toast(this, NAMEICONRES(QStringLiteral("save")),
                         tr("SaveSourceFileError"));
        }
    }
    if (res == ErrFile::WorkSpaceUnSaved) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("save")),
                     tr("SaveWSError"));
        return;
    }

    m_views[editor] = workspace;
    Toast::toast(this, NAMEICONRES(QStringLiteral("save")),
                 tr("SaveSuccessfully"));
}

void MainWindow::on_saveas() {
    if (m_curEditor == nullptr) {
        return;
    }

    auto filename = QFileDialog::getSaveFileName(this, tr("ChooseSaveFile"),
                                                 m_lastusedpath);
    if (filename.isEmpty())
        return;
    m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();

    auto editor = m_curEditor.loadAcquire();
    auto oldFileName = editor->fileName();
    QString workspace = m_views.value(editor);
    if (editor->change2WorkSpace()) {
        workspace = editor->fileName() + PROEXT;
    }
    auto res = editor->save(workspace, filename);

restart:
    switch (res) {
    case ErrFile::Success: {
        Toast::toast(this, NAMEICONRES(QStringLiteral("saveas")),
                     tr("SaveSuccessfully"));
        m_views[editor] = workspace;
        auto namep = m_openedFileNames.indexOf(oldFileName);
        m_openedFileNames.replace(namep, filename);
        break;
    }
    case ErrFile::WorkSpaceUnSaved: {
        Toast::toast(this, NAMEICONRES(QStringLiteral("saveas")),
                     tr("SaveWSError"));
        break;
    }
    case ErrFile::SourceFileChanged: {
        if (WingMessageBox::warning(this, tr("Warn"), tr("SourceChanged"),
                                    QMessageBox::Yes | QMessageBox::No) ==
            QMessageBox::Yes) {
            res = editor->save(workspace, filename);
            goto restart;
        } else {
            Toast::toast(this, NAMEICONRES(QStringLiteral("save")),
                         tr("SaveSourceFileError"));
        }
        break;
    }
    default: {
        Toast::toast(this, NAMEICONRES(QStringLiteral("saveas")),
                     tr("SaveUnSuccessfully"));
        break;
    }
    }
}

void MainWindow::on_exportfile() {
    if (m_curEditor == nullptr) {
        return;
    }

    auto filename = QFileDialog::getSaveFileName(this, tr("ChooseExportFile"),
                                                 m_lastusedpath);
    if (filename.isEmpty())
        return;
    m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();

    auto editor = m_curEditor.loadAcquire();
    QString workspace = m_views.value(editor);
    if (editor->change2WorkSpace()) {
        workspace = editor->fileName() + PROEXT;
    }
    auto res = editor->save(workspace, filename, true);

restart:
    switch (res) {
    case ErrFile::Success: {
        Toast::toast(this, NAMEICONRES(QStringLiteral("export")),
                     tr("ExportSuccessfully"));
        break;
    }
    case ErrFile::SourceFileChanged: {
        if (QMessageBox::warning(this, tr("Warn"), tr("SourceChanged"),
                                 QMessageBox::Yes | QMessageBox::No) ==
            QMessageBox::Yes) {
            res = editor->save(workspace, filename, true);
            goto restart;
        } else {
            Toast::toast(this, NAMEICONRES(QStringLiteral("export")),
                         tr("ExportSourceFileError"));
        }
        break;
    }
    default: {
        Toast::toast(this, NAMEICONRES(QStringLiteral("export")),
                     tr("ExportUnSuccessfully"));
        break;
    }
    }
}

void MainWindow::on_savesel() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    auto filename = WingFileDialog::getSaveFileName(this, tr("ChooseSaveFile"),
                                                    m_lastusedpath);
    if (filename.isEmpty())
        return;
    m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();
    QFile qfile(filename);
    if (qfile.open(QFile::WriteOnly)) {
        auto buffer = hexeditor->document()->selectedBytes();
        qfile.write(buffer);
        qfile.close();
        Toast::toast(this, NAMEICONRES(QStringLiteral("savesel")),
                     tr("SaveSelSuccess"));
    } else {
        Toast::toast(this, NAMEICONRES(QStringLiteral("savesel")),
                     tr("SaveSelError"));
    }
}

void MainWindow::on_undofile() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    hexeditor->document()->undo();
}

void MainWindow::on_redofile() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    hexeditor->document()->redo();
}

void MainWindow::on_cutfile() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    if (Q_LIKELY(hexeditor->document()->Cut())) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("cut")),
                     tr("CutToClipBoard"));
    } else {
        Toast::toast(this, NAMEICONRES(QStringLiteral("cut")),
                     tr("UnCutToClipBoard"));
    }
}

void MainWindow::on_copyfile() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    if (Q_LIKELY(hexeditor->document()->copy())) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("copy")),
                     tr("CopyToClipBoard"));
    } else {
        Toast::toast(this, NAMEICONRES(QStringLiteral("copy")),
                     tr("UnCopyToClipBoard"));
    }
}

void MainWindow::on_pastefile() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    hexeditor->document()->Paste();
}

void MainWindow::on_delete() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    hexeditor->document()->RemoveSelection();
}

void MainWindow::on_clone() {}

void MainWindow::on_findfile() {
    if (m_curEditor == nullptr) {
        return;
    }

    auto editor = m_curEditor.loadAcquire();
    auto hexeditor = editor->hexEditor();
    FindDialog fd(editor->isBigFile(), 0, int(hexeditor->documentBytes()),
                  hexeditor->selectlength() > 1, this);
    if (fd.exec()) {
        FindDialog::Result r;
        auto res = fd.getResult(r);
        ExecAsync<EditorView::FindError>(
            [this, r, res]() -> EditorView::FindError {
                return m_curEditor.loadAcquire()->find(res, r, this->_findmax);
            },
            [this](EditorView::FindError err) {
                switch (err) {
                case EditorView::FindError::Success:
                    Toast::toast(this, NAMEICONRES(QStringLiteral("find")),
                                 tr("FindFininish"));
                    break;
                case EditorView::FindError::Busy:
                    Toast::toast(this, NAMEICONRES(QStringLiteral("find")),
                                 tr("FindFininishBusy"));
                    break;
                case EditorView::FindError::MayOutOfRange:
                    Toast::toast(this, NAMEICONRES(QStringLiteral("find")),
                                 tr("MayTooMuchFindResult"));
                    break;
                }

                loadFindResult(m_curEditor.loadAcquire());
            });
    }
}

void MainWindow::on_gotoline() {
    if (m_curEditor == nullptr) {
        return;
    }
    m_curEditor.loadAcquire()->triggerGoto();
}

void MainWindow::on_encoding() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    EncodingDialog d;
    if (d.exec()) {
        auto res = d.getResult();
        hexeditor->renderer()->SetEncoding(res);
    }
}

void MainWindow::on_fileInfo() {
    if (m_curEditor == nullptr) {
        return;
    }

    auto editor = m_curEditor.loadAcquire();
    FileInfoDialog d(editor->fileName(),
                     editor->documentType() ==
                         EditorView::DocumentType::RegionFile);
    d.exec();
}

void MainWindow::on_cuthex() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    if (hexeditor->document()->Cut(true)) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("cut")),
                     tr("CutToClipBoard"));
    } else {
        Toast::toast(this, NAMEICONRES(QStringLiteral("cut")),
                     tr("UnCutToClipBoard"));
    }
}

void MainWindow::on_copyhex() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    if (hexeditor->document()->copy(true)) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("copyhex")),
                     tr("CopyToClipBoard"));
    } else {
        Toast::toast(this, NAMEICONRES(QStringLiteral("copyhex")),
                     tr("UnCopyToClipBoard"));
    }
}

void MainWindow::on_pastehex() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    hexeditor->document()->Paste();
}

void MainWindow::on_fill() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    bool b;
    auto in = WingInputDialog::getText(this, tr("Fill"), tr("PleaseInputFill"),
                                       QLineEdit::Normal, QString(), &b);
    if (b) {
        auto ch = char(in.toULongLong(&b, 0));
        if (b) {
            auto doc = hexeditor->document();
            if (doc->isEmpty() || hexeditor->selectlength() == 0)
                return;
            auto pos = doc->cursor()->selectionStart().offset();
            doc->Replace(pos,
                         QByteArray(int(hexeditor->selectlength()), char(ch)));
        } else {
            Toast::toast(this, NAMEICONRES(QStringLiteral("fill")),
                         tr("FillInputError"));
        }
    }
}

void MainWindow::on_fillzero() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    auto doc = hexeditor->document();
    if (doc->isEmpty() || hexeditor->selectlength() == 0)
        return;
    auto pos = doc->cursor()->selectionStart().offset();
    doc->Replace(pos, QByteArray(int(hexeditor->selectlength()), char(0)));
}

void MainWindow::on_bookmark() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    auto doc = hexeditor->document();
    if (!doc->isKeepSize()) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("bookmark")),
                     tr("CheckKeepSize"));
        return;
    }
    qindextype index = -1;
    if (doc->existBookMarkByIndex(index)) {
        auto b = doc->bookMarkByIndex(index);
        bool ok;
        hexeditor->renderer()->enableCursor();
        auto comment =
            WingInputDialog::getText(this, tr("BookMark"), tr("InputComment"),
                                     QLineEdit::Normal, b.comment, &ok);
        if (ok) {
            doc->ModBookMark(b.pos, comment);
        }
    } else {
        bool ok;
        auto comment =
            WingInputDialog::getText(this, tr("BookMark"), tr("InputComment"),
                                     QLineEdit::Normal, QString(), &ok);
        if (ok) {
            auto pos = hexeditor->currentOffset();
            doc->AddBookMark(pos, comment);
        }
    }
}

void MainWindow::on_bookmarkdel() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    auto doc = hexeditor->document();
    if (!doc->isKeepSize()) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("bookmarkdel")),
                     tr("CheckKeepSize"));
        return;
    }
    qindextype index = -1;
    if (doc->existBookMarkByIndex(index)) {
        doc->removeBookMarkByIndex(index);
    }
}

void MainWindow::on_bookmarkcls() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    auto doc = hexeditor->document();
    if (!doc->isKeepSize()) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("bookmarkcls")),
                     tr("CheckKeepSize"));
        return;
    }
    doc->ClearBookMark();
}

void MainWindow::on_metadata() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    auto doc = hexeditor->document();
    if (!doc->isKeepSize()) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("metadata")),
                     tr("CheckKeepSize"));
        return;
    }
    if (hexeditor->documentBytes() > 0) {
        MetaDialog m;
        auto cur = doc->cursor();
        if (cur->selectionLength() > 0) {
            auto begin = cur->selectionStart().offset();
            auto end = cur->selectionEnd().offset() + 1;
            if (m.exec()) {
                doc->metadata()->Metadata(begin, end, m.foreGroundColor(),
                                          m.backGroundColor(), m.comment());
            }
        } else {
            Toast::toast(this, NAMEICONRES(QStringLiteral("metadata")),
                         tr("NoSelection"));
        }
    }
}

void MainWindow::on_metadataedit() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    auto doc = hexeditor->document();
    if (!doc->isKeepSize()) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("metadataedit")),
                     tr("CheckKeepSize"));
        return;
    }
    if (hexeditor->documentBytes() > 0) {
        MetaDialog m;
        auto cur = doc->cursor();
        if (cur->selectionLength() > 0) {
            auto mc = doc->metadata()->gets(cur->position().offset());

            if (mc.length() > 0) {
                auto meta = mc.last();
                auto begin = meta.begin;
                auto end = meta.end;
                m.setForeGroundColor(meta.foreground);
                m.setBackGroundColor(meta.background);
                m.setComment(meta.comment);
                if (m.exec()) {
                    auto mi = hexeditor->document()->metadata();
                    QHexMetadataAbsoluteItem o;
                    o.begin = begin;
                    o.end = end;
                    o.foreground = m.foreGroundColor();
                    o.background = m.backGroundColor();
                    o.comment = m.comment();
                    mi->ModifyMetadata(meta, o);
                }
            } else {
                Toast::toast(this, NAMEICONRES(QStringLiteral("metadata")),
                             tr("NoMetaData"));
            }
        } else {
            Toast::toast(this, NAMEICONRES(QStringLiteral("metadata")),
                         tr("NoSelection"));
        }
    }
}

void MainWindow::on_metadatadel() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    auto doc = hexeditor->document();
    if (!doc->isKeepSize()) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("metadatadel")),
                     tr("CheckKeepSize"));
        return;
    }
    auto meta = doc->metadata();
    auto pos = doc->cursor()->position().offset();
    meta->RemoveMetadata(pos);
}

void MainWindow::on_metadatacls() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    auto doc = hexeditor->document();
    if (!doc->isKeepSize()) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("metadatacls")),
                     tr("CheckKeepSize"));
        return;
    }
    doc->metadata()->Clear();
}

void MainWindow::on_bookmarkChanged(BookMarkModEnum flag, int index, qint64 pos,
                                    QString comment) {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    auto doc = hexeditor->document();
    switch (flag) {
    case BookMarkModEnum::Insert: {
        QListWidgetItem *litem = new QListWidgetItem;
        litem->setIcon(ICONRES(QStringLiteral("bookmark")));
        litem->setData(Qt::UserRole, pos);
        litem->setText(comment);
        litem->setToolTip(QString(tr("Addr : 0x%1")).arg(pos, 0, 16));
        m_bookmarks->addItem(litem);
    } break;
    case BookMarkModEnum::Modify: {
        m_bookmarks->item(index)->setText(comment);
    } break;
    case BookMarkModEnum::Remove: {
        auto item = m_bookmarks->item(index);
        m_bookmarks->removeItemWidget(item);
        delete item; // let item disappear
    } break;
    case BookMarkModEnum::Apply: {
        QList<BookMarkStruct> bookmaps;
        m_bookmarks->clear();
        doc->getBookMarks(bookmaps);
        for (auto &item : bookmaps) {
            QListWidgetItem *litem = new QListWidgetItem;
            litem->setIcon(ICONRES(QStringLiteral("bookmark")));
            litem->setData(Qt::UserRole, item.pos);
            litem->setText(item.comment);
            litem->setToolTip(QString(tr("Addr : 0x%1")).arg(item.pos, 0, 16));
            m_bookmarks->addItem(litem);
        }
    } break;
    case BookMarkModEnum::Clear: {
        m_bookmarks->clear();
    } break;
    }
}

void MainWindow::on_metadatafg(bool checked) {
    if (m_curEditor == nullptr) {
        return;
    }
    auto editor = m_curEditor.loadAcquire();
    auto hexeditor = editor->hexEditor();
    auto doc = hexeditor->document();
    if (editor->isWorkSpace()) {
        doc->SetMetafgVisible(checked);
    } else {
        doc->setMetafgVisible(checked);
    }
}

void MainWindow::on_metadatabg(bool checked) {
    if (m_curEditor == nullptr) {
        return;
    }
    auto editor = m_curEditor.loadAcquire();
    auto hexeditor = editor->hexEditor();
    auto doc = hexeditor->document();
    if (editor->isWorkSpace()) {
        doc->SetMetabgVisible(checked);
    } else {
        doc->setMetabgVisible(checked);
    }
}

void MainWindow::on_metadatacomment(bool checked) {
    if (m_curEditor == nullptr) {
        return;
    }
    auto editor = m_curEditor.loadAcquire();
    auto hexeditor = editor->hexEditor();
    auto doc = hexeditor->document();
    if (editor->isWorkSpace()) {
        doc->SetMetaCommentVisible(checked);
    } else {
        doc->setMetaCommentVisible(checked);
    }
}

void MainWindow::on_metashowall() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto editor = m_curEditor.loadAcquire();
    auto hexeditor = editor->hexEditor();
    auto doc = hexeditor->document();
    if (editor->isWorkSpace()) {
        doc->SetMetaVisible(true);
    } else {
        doc->setMetafgVisible(true);
        doc->setMetabgVisible(true);
        doc->setMetaCommentVisible(true);
    }
}

void MainWindow::on_metahideall() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto editor = m_curEditor.loadAcquire();
    auto hexeditor = editor->hexEditor();
    auto doc = hexeditor->document();
    if (editor->isWorkSpace()) {
        doc->SetMetaVisible(false);
    } else {
        doc->setMetafgVisible(false);
        doc->setMetabgVisible(false);
        doc->setMetaCommentVisible(false);
    }
}

void MainWindow::on_clearfindresult() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto editor = m_curEditor.loadAcquire();
    editor->clearFindResult();
    m_findresult->setRowCount(0);
}

void MainWindow::on_exportfindresult() {
    if (m_curEditor == nullptr) {
        return;
    }

    auto c = m_findresult->rowCount();
    if (c == 0) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("export")),
                     tr("EmptyFindResult"));
        return;
    }
    auto filename = QFileDialog::getSaveFileName(this, tr("ChooseSaveFile"),
                                                 m_lastusedpath);
    if (filename.isEmpty())
        return;
    m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();

    auto editor = m_curEditor.loadAcquire();
    auto findresitem = editor->findResult();

    QFile f(filename);
    if (f.open(QFile::WriteOnly)) {
        QJsonObject fobj;
        fobj.insert(QStringLiteral("file"), editor->fileName());
        QJsonArray arr;
        for (int i = 0; i < c; i++) {
            QJsonObject jobj;
            jobj.insert(QStringLiteral("offset"),
                        m_findresult->item(i, 0)->text());
            jobj.insert(QStringLiteral("value"),
                        m_findresult->item(i, 1)->text());
            arr.append(jobj);
        }
        fobj.insert(QStringLiteral("data"), arr);

        QJsonDocument doc(fobj);
        if (f.write(doc.toJson(QJsonDocument::JsonFormat::Indented)) >= 0) {
            f.close();
            Toast::toast(this, NAMEICONRES(QStringLiteral("export")),
                         tr("SaveFindResult"));
        }
    } else {
        Toast::toast(this, NAMEICONRES(QStringLiteral("export")),
                     tr("SaveFindResultError"));
    }
}

void MainWindow::on_locChanged() {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    m_lblloc->setText(QStringLiteral("(%1,%2)")
                          .arg(hexeditor->currentRow())
                          .arg(hexeditor->currentColumn()));
    auto sellen = hexeditor->selectlength();
    m_lblsellen->setText(QStringLiteral("%1 - 0x%2")
                             .arg(sellen)
                             .arg(QString::number(sellen, 16).toUpper()));

    // number analyse
    auto off = qint64(hexeditor->currentOffset());
    auto d = hexeditor->document();

    auto tmp = d->read(off, sizeof(quint64));
    quint64 n = *reinterpret_cast<const quint64 *>(tmp.constData());

    auto len = tmp.length();

    static auto UNKNOWN_NUM = QStringLiteral("-");

    if (len == sizeof(quint64)) {
        auto s = processEndian(n);
        _numsitem[NumTableIndex::Uint64]->setText(
            QStringLiteral("0x%1").arg(QString::number(s, 16).toUpper()));
        auto s1 = processEndian(qint64(n));
        _numsitem[NumTableIndex::Int64]->setText(QString::number(s1));
    } else {
        _numsitem[NumTableIndex::Uint64]->setText(UNKNOWN_NUM);
        _numsitem[NumTableIndex::Int64]->setText(UNKNOWN_NUM);
    }

    if (len > sizeof(quint32)) {
        auto s = processEndian(quint32(n));
        _numsitem[NumTableIndex::Uint32]->setText(
            QStringLiteral("0x%1").arg(QString::number(s, 16).toUpper()));
        auto s1 = processEndian(qint32(n));
        _numsitem[NumTableIndex::Int32]->setText(QString::number(s1));
    } else {
        _numsitem[NumTableIndex::Uint32]->setText(UNKNOWN_NUM);
        _numsitem[NumTableIndex::Int32]->setText(UNKNOWN_NUM);
    }

    if (len > sizeof(quint16)) {
        auto s = processEndian(quint16(n));
        _numsitem[NumTableIndex::Ushort]->setText(
            QStringLiteral("0x%1").arg(QString::number(s, 16).toUpper()));
        auto s1 = processEndian(qint16(n));
        _numsitem[NumTableIndex::Short]->setText(QString::number(s1));
    } else {
        _numsitem[NumTableIndex::Ushort]->setText(UNKNOWN_NUM);
        _numsitem[NumTableIndex::Short]->setText(UNKNOWN_NUM);
    }
    if (len > sizeof(uchar)) {
        auto s1 = tmp.at(0);
        auto s = uchar(s1);
        _numsitem[NumTableIndex::Byte]->setText(
            QStringLiteral("0x%1").arg(QString::number(s, 16).toUpper()));
        _numsitem[NumTableIndex::Char]->setText(QString::number(s1));
    } else {
        _numsitem[NumTableIndex::Byte]->setText(UNKNOWN_NUM);
        _numsitem[NumTableIndex::Char]->setText(UNKNOWN_NUM);
    }

    //解码字符串
    if (sellen > 1) {
        // 如果不超过 10KB （默认）那么解码，防止太多卡死
        if (sellen <= 1024 * _decstrlim) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            auto enc = QStringConverter::encodingForName(
                hexeditor->renderer()->encoding().toUtf8());
            Q_ASSERT(enc.has_value());
            QStringDecoder dec(enc.value());
            m_txtDecode->setText(dec.decode(d->selectedBytes()));
#else
            auto enc = QTextCodec::codecForName(
                hexeditor->renderer()->encoding().toUtf8());
            auto dec = enc->makeDecoder();
            m_txtDecode->setText(dec->toUnicode(d->selectedBytes()));
#endif
        } else {
            m_txtDecode->setHtml(QStringLiteral("<font color=\"red\">%1</font>")
                                     .arg(tr("TooManyBytesDecode")));
        }
    } else {
        m_txtDecode->clear();
    }
}

void MainWindow::on_fullScreen() { this->show(WindowState::FullScreen); }

void MainWindow::on_restoreLayout() {}

void MainWindow::on_exportlog() {
    auto nfile = saveLog();
    if (nfile.isEmpty()) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("log")),
                     tr("ExportLogError"));
    } else {
        Toast::toast(this, NAMEICONRES(QStringLiteral("log")),
                     tr("ExportLogSuccess") + nfile);
    }
}

void MainWindow::on_clslog() {
    Q_ASSERT(m_logbrowser);
    m_logbrowser->clear();
    Toast::toast(this, NAMEICONRES(QStringLiteral("clearhis")),
                 tr("ClearLogSuccess"));
}

void MainWindow::on_scriptwindow() {
    Q_ASSERT(m_scriptDialog);
    m_scriptDialog->show();
    m_scriptDialog->raise();
}

void MainWindow::on_loadplg() {
    if (!m_enablePlugin)
        return;
#ifdef QT_DEBUG
#ifdef Q_OS_WIN
    auto filename = WingFileDialog::getOpenFileName(
        this, tr("ChoosePlugin"), m_lastusedpath, tr("PluginFile (*.dll)"));
#else
    auto filename = WingFileDialog::getOpenFileName(
        this, tr("ChoosePlugin"), m_lastusedpath, tr("PluginFile (*.so)"));
#endif
#else
    auto filename = WingFileDialog::getOpenFileName(
        this, tr("ChoosePlugin"), m_lastusedpath, tr("PluginFile (*.wingplg)"));
#endif

    if (!filename.isEmpty()) {
        auto info = QFileInfo(filename);
        m_lastusedpath = info.absoluteDir().absolutePath();
        PluginSystem::instance().loadPlugin(info);
    }
}

void MainWindow::on_setting_general() { m_setdialog->showConfig(); }

void MainWindow::on_setting_plugin() { m_setdialog->showConfig(2); }

void MainWindow::on_about() { AboutSoftwareDialog().exec(); }

void MainWindow::on_sponsor() { SponsorDialog().exec(); }

void MainWindow::on_wiki() {
    QDesktopServices::openUrl(
        QUrl(QStringLiteral("https://www.cnblogs.com/wingsummer/p/18286419")));
}

QString MainWindow::saveLog() {
    QDir ndir(
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    ndir.mkdir(QStringLiteral("log")); // 确保日志存放目录存在

    QFile lfile(ndir.absolutePath() + QStringLiteral("/log/") +
                QDateTime::currentDateTime().toString(
                    QStringLiteral("yyyyMMddhhmmsszzz")) +
                ".log");
    if (lfile.open(QFile::WriteOnly)) {
        lfile.write(m_logbrowser->toPlainText().toUtf8());
        lfile.close();
        return lfile.fileName();
    }
    return QString();
}

ads::CDockWidget *MainWindow::buildDockWidget(ads::CDockManager *dock,
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
    m_toolBtneditors.value(index)->menu()->addAction(dw->toggleViewAction());
    return dw;
}

EditorView *MainWindow::findEditorView(const QString &filename) {
    for (auto it = m_views.keyBegin(); it != m_views.keyEnd(); ++it) {
        if ((*it)->fileName() == filename) {
            return *it;
        }
    }
    return nullptr;
}

bool MainWindow::newOpenFileSafeCheck() {
    if (m_views.size() >= std::numeric_limits<qindextype>::max() - 1) {
        WingMessageBox::critical(this, tr("Error"),
                                 tr("Too much opened files"));
        return false;
    }
    return true;
}

void MainWindow::registerEditorView(EditorView *editor) {
    connectEditorView(editor);
    m_views.insert(editor, {});
    auto ev = m_toolBtneditors.value(ToolButtonIndex::EDITOR_VIEWS);
    auto menu = ev->menu();
    Q_ASSERT(menu);
    menu->addAction(editor->toggleViewAction());
    ev->setEnabled(true);
}

void MainWindow::connectEditorView(EditorView *editor) {
    Q_ASSERT(editor);
    connect(editor, &EditorView::sigOnFill, this, &MainWindow::on_fill);
    connect(editor, &EditorView::sigOnCutHex, this, &MainWindow::on_cuthex);
    connect(editor, &EditorView::sigOnDelete, this, &MainWindow::on_delete);
    connect(editor, &EditorView::sigOnCopyHex, this, &MainWindow::on_copyhex);
    connect(editor, &EditorView::sigOnCutFile, this, &MainWindow::on_cutfile);
    connect(editor, &EditorView::sigOnBookMark, this, &MainWindow::on_bookmark);
    connect(editor, &EditorView::sigOnCopyFile, this, &MainWindow::on_copyfile);
    connect(editor, &EditorView::sigOnEncoding, this, &MainWindow::on_encoding);
    connect(editor, &EditorView::sigOnFindFile, this, &MainWindow::on_findfile);
    connect(editor, &EditorView::sigOnGoToLine, this, &MainWindow::on_gotoline);
    connect(editor, &EditorView::sigOnMetadata, this, &MainWindow::on_metadata);
    connect(editor, &EditorView::sigOnPasteHex, this, &MainWindow::on_pastehex);
    connect(editor, &EditorView::sigOnPasteFile, this,
            &MainWindow::on_pastefile);

    connect(editor, &EditorView::closeRequested, this, [this] {
        auto editor = qobject_cast<EditorView *>(sender());
        Q_ASSERT(editor);
        Q_ASSERT(m_views.contains(editor));

        auto hexeditor = editor->hexEditor();
        if (!hexeditor->isSaved()) {
            auto ret = m_isOnClosing ? QMessageBox::Yes : this->saveRequest();
            if (ret == QMessageBox::Cancel) {
                return;
            } else if (ret == QMessageBox::Yes) {
                this->on_save();
                if (!hexeditor->isSaved()) {
                    return;
                }
            }
        }

        auto file = editor->fileName();
        if (!file.isEmpty()) {
            m_openedFileNames.removeOne(file);
        }
        m_views.remove(editor);
        if (m_curEditor == editor) {
            m_curEditor = nullptr;
        }
        editor->deleteDockWidget();
        m_toolBtneditors.value(ToolButtonIndex::EDITOR_VIEWS)
            ->setEnabled(m_views.size() != 0);
    });
}

void MainWindow::swapEditor(EditorView *old, EditorView *cur) {
    if (old != nullptr) {
        auto hexeditor = old->hexEditor();
        hexeditor->disconnect(SLOT(cursorLocationChanged()));
        hexeditor->disconnect(SLOT(
            documentBookMarkChanged(BookMarkModEnum, int, qint64, QString)));
        hexeditor->disconnect(SLOT(canUndoChanged(bool)));
        hexeditor->disconnect(SLOT(canRedoChanged(bool)));
        hexeditor->disconnect(SLOT(documentSaved(bool)));
        hexeditor->disconnect(SLOT(documentKeepSize(bool)));
        hexeditor->disconnect(SLOT(documentLockedFile(bool)));
        hexeditor->disconnect(SLOT(copyLimitRaised()));
        hexeditor->disconnect(SLOT(scaleRateChanged()));
        hexeditor->document()->disconnect();
    }

    Q_ASSERT(cur);
    auto hexeditor = cur->hexEditor();
    connect(hexeditor, &QHexView::cursorLocationChanged, this,
            &MainWindow::on_locChanged);
    connect(hexeditor, &QHexView::documentBookMarkChanged, this,
            &MainWindow::on_bookmarkChanged);
    connect(hexeditor, &QHexView::canUndoChanged, this, [this](bool b) {
        m_toolBtneditors[ToolButtonIndex::UNDO_ACTION]->setEnabled(b);
    });
    connect(hexeditor, &QHexView::canRedoChanged, this, [this](bool b) {
        m_toolBtneditors[ToolButtonIndex::REDO_ACTION]->setEnabled(b);
    });
    connect(hexeditor, &QHexView::documentSaved, this, [this](bool b) {
        m_iSaved->setIcon(b ? _infoSaved : _infoUnsaved);
    });
    connect(hexeditor, &QHexView::documentKeepSize, this, [this](bool b) {
        m_iCanOver->setIcon(b ? _infoCannotOver : _infoCanOver);
    });
    connect(hexeditor, &QHexView::documentLockedFile, this, [this](bool b) {
        m_iLocked->setIcon(b ? _infoLock : _infoUnLock);
    });
    connect(hexeditor, &QHexView::copyLimitRaised, this, [this] {
        Toast::toast(this, NAMEICONRES(QStringLiteral("copy")),
                     tr("CopyLimit"));
    });
    connect(hexeditor, &QHexView::scaleRateChanged, this, [this] {
        auto hexeditor = qobject_cast<QHexView *>(QObject::sender());
        Q_ASSERT(hexeditor);
        Toast::toast(this, NAMEICONRES(QStringLiteral("scale")),
                     QString::number(hexeditor->scaleRate() * 100) +
                         QStringLiteral("%"));
    });

    auto doc = hexeditor->document().get();
    m_aShowMetabg->setChecked(doc->metabgVisible());
    m_aShowMetafg->setChecked(doc->metafgVisible());
    m_aShowMetaComment->setChecked(doc->metaCommentVisible());
    connect(doc, &QHexDocument::metabgVisibleChanged, m_aShowMetabg,
            &QAction::setChecked);
    connect(doc, &QHexDocument::metafgVisibleChanged, m_aShowMetafg,
            &QAction::setChecked);
    connect(doc, &QHexDocument::metaCommentVisibleChanged, m_aShowMetaComment,
            &QAction::setChecked);

    setEditModeEnabled(true,
                       cur->documentType() == EditorView::DocumentType::Driver);
    hexeditor->getStatus();
    m_iReadWrite->setIcon(hexeditor->isReadOnly() ? _infoReadonly
                                                  : _infoWriteable);
    m_iWorkSpace->setIcon(cur->isWorkSpace() ? _infow : _infouw);

    loadFindResult(cur);
}

void MainWindow::loadFindResult(EditorView *view) {

    auto result = view->findResult();
    auto data = view->lastFindData();
    auto len = m_findresult->rowCount();
    for (int i = 0; i < len; ++i) {
        m_findresult->setItem(i, 0, nullptr);
        m_findresult->setItem(i, 1, nullptr);
    }
    len = view->findResultCount();
    m_findresult->setRowCount(len);
    m_findresult->setProperty("EditorView", QVariant::fromValue(view));
    if (len > 0) {
        ExecAsync_VOID(
            [=] {
                for (auto i = 0; i < len; i++) {
                    auto tab0 = new QTableWidgetItem(
                        QStringLiteral("0x") +
                        QString::number(quintptr(result.at(i)) +
                                            view->hexEditor()->addressBase(),
                                        16)
                            .toUpper());
                    tab0->setData(Qt::UserRole, result.at(i));
                    m_findresult->setItem(i, 0, tab0);
                    m_findresult->setItem(
                        i, 1, new QTableWidgetItem(data.toHex(' ').toUpper()));
                }
            },
            EMPTY_FUNC);
    }
}

void MainWindow::openFiles(const QStringList &files) {
    bool err = false;
    QStringList errof;
    for (auto &file : files) {
        if (openFile(file, nullptr)) {
            err = true;
            errof << file;
        }
    }

    show();
    raise();
    activateWindow();

    if (err) {
        WingMessageBox::critical(this, tr("Error"),
                                 tr("ErrOpenFileBelow") + QStringLiteral("\n") +
                                     errof.join('\n'));
    }
}

ErrFile MainWindow::openFile(const QString &file, EditorView **editor) {
    if (m_openedFileNames.contains(file)) {
        // find the result
        if (editor) {
            *editor = findEditorView(file);
        }
        return ErrFile::AlreadyOpened;
    }

    // ok, going on
    if (!newOpenFileSafeCheck()) {
        return ErrFile::Error;
    }

    QFileInfo finfo(file);
    auto filename = finfo.absoluteFilePath();

    auto ev = new EditorView(m_enablePlugin);
    auto res = ev->openFile(filename);

    if (res != ErrFile::Success) {
        delete ev;
        return res;
    }

    m_openedFileNames << filename;
    m_views.insert(ev, QString());
    registerEditorView(ev);
    m_dock->addDockWidget(ads::CenterDockWidgetArea, ev, m_editorViewArea);
    return ErrFile::Success;
}

ErrFile MainWindow::openDriver(const QString &driver, EditorView **editor) {
    if (m_openedFileNames.contains(driver)) {
        // find the result
        if (editor) {
            *editor = findEditorView(driver);
        }
        return ErrFile::AlreadyOpened;
    }

    // ok, going on
    if (!newOpenFileSafeCheck()) {
        return ErrFile::Error;
    }

    auto ev = new EditorView(m_enablePlugin);
    auto res = ev->openDriver(driver);

    if (res != ErrFile::Success) {
        delete ev;
        return res;
    }

    m_openedFileNames << driver;
    m_views.insert(ev, QString());
    registerEditorView(ev);
    m_dock->addDockWidget(ads::CenterDockWidgetArea, ev, m_editorViewArea);
    return ErrFile::Success;
}

ErrFile MainWindow::openWorkSpace(const QString &file, EditorView **editor) {
    if (m_openedFileNames.contains(file)) {
        // find the result
        if (editor) {
            *editor = findEditorView(file);
        }
        return ErrFile::AlreadyOpened;
    }

    // ok, going on
    if (!newOpenFileSafeCheck()) {
        return ErrFile::Error;
    }

    QFileInfo finfo(file);
    auto filename = finfo.absoluteFilePath();

    auto ev = new EditorView(m_enablePlugin);
    auto res = ev->openWorkSpace(filename);

    if (res != ErrFile::Success) {
        delete ev;
        return res;
    }

    m_openedFileNames << ev->fileName();
    m_views.insert(ev, file);
    registerEditorView(ev);
    m_dock->addDockWidget(ads::CenterDockWidgetArea, ev, m_editorViewArea);
    return ErrFile::Success;
}

ErrFile MainWindow::openRegionFile(QString file, EditorView **editor,
                                   qsizetype start, qsizetype length) {
    if (m_openedFileNames.contains(file)) {
        // find the result
        if (editor) {
            *editor = findEditorView(file);
        }
        return ErrFile::AlreadyOpened;
    }

    // ok, going on
    if (!newOpenFileSafeCheck()) {
        return ErrFile::Error;
    }

    QFileInfo finfo(file);
    auto filename = finfo.absoluteFilePath();

    auto ev = new EditorView(m_enablePlugin);
    auto res = ev->openRegionFile(filename, start, length);

    if (res != ErrFile::Success) {
        delete ev;
        return res;
    }

    m_openedFileNames << filename;
    registerEditorView(ev);
    m_dock->addDockWidget(ads::CenterDockWidgetArea, ev, m_editorViewArea);
    return ErrFile::Success;
}

void MainWindow::setEditModeEnabled(bool b, bool isdriver) {
    for (auto &item : m_editStateWidgets) {
        item->setEnabled(b);
    }

    if (b) {
        if (m_curEditor == nullptr) {
            return;
        }

        auto editor = m_curEditor.loadAcquire();
        auto hexeditor = editor->hexEditor();
        enableDirverLimit(isdriver);

        auto dm = editor->isWorkSpace();
        m_iWorkSpace->setIcon(dm ? _infow : _infouw);
        auto doc = hexeditor->document();
        emit doc->canRedoChanged(doc->canRedo());
        emit doc->canUndoChanged(doc->canUndo());
    } else {
        m_iWorkSpace->setIcon(_infouw);
    }

    if (!b) {
        m_lblloc->setText(QStringLiteral("(0,0)"));
        m_lblsellen->setText(QStringLiteral("0 - 0x0"));
        for (auto i = 0; i < NumTableIndexCount; i++) {
            _numsitem[i]->setText(QStringLiteral("-"));
        }
        m_bookmarks->clear();
    }
}

void MainWindow::enableDirverLimit(bool isdriver) {
    auto e = !isdriver;
    for (auto &item : m_driverStateWidgets) {
        item->setEnabled(e);
    }
}

void MainWindow::setCurrentHexEditorScale(qreal rate) {
    if (m_curEditor == nullptr) {
        return;
    }
    auto hexeditor = m_curEditor.loadAcquire()->hexEditor();
    hexeditor->setScaleRate(rate);
}

void MainWindow::loadCacheIcon() {
    _infoSaved = ICONRES(QStringLiteral("saved"));

    _infoUnsaved = ICONRES(QStringLiteral("unsaved"));

    _infoWriteable = ICONRES(QStringLiteral("writable"));
    _infoReadonly = ICONRES(QStringLiteral("readonly"));

    _infow = ICONRES(QStringLiteral("works"));
    _infouw = ICONRES(QStringLiteral("unworks"));

    _infoUnLock = ICONRES(QStringLiteral("unlock"));
    _infoLock = ICONRES(QStringLiteral("lock"));

    _infoCanOver = ICONRES(QStringLiteral("canover"));
    _infoCannotOver = ICONRES(QStringLiteral("unover"));

    _pixLock = QPixmap(NAMEICONRES("lock"));
    _pixCanOver = QPixmap(NAMEICONRES("canover"));
    _pixCannotOver = QPixmap(NAMEICONRES("unover"));
}

QMessageBox::StandardButton MainWindow::saveRequest() {
    auto ret = WingMessageBox::warning(this, qAppName(), tr("ConfirmSave"),
                                       QMessageBox::Yes | QMessageBox::No |
                                           QMessageBox::Cancel);
    return ret;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    m_isOnClosing = true;
    if (!m_views.isEmpty()) {
        QStringList unSavedFiles;
        QList<EditorView *> need2CloseView;
        for (auto p = m_views.keyBegin(); p != m_views.keyEnd(); p++) {
            auto editor = *p;
            bool saved = editor->hexEditor()->isSaved();
            if (saved) {
                need2CloseView << editor;
            } else {
                unSavedFiles << editor->fileName();
            }
        }

        for (auto &view : need2CloseView) {
            emit view->closeRequested();
        }

        auto ret =
            unSavedFiles.isEmpty()
                ? QMessageBox::No
                : WingMessageBox::warning(
                      this, qAppName(), tr("ConfirmAPPSave"),
                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (ret == QMessageBox::Yes) {
            for (auto &p : m_views.keys()) {
                emit p->closeRequested();
            }
            m_isOnClosing = false;
            if (!m_views.isEmpty()) {
                event->ignore();
                return;
            }
        } else if (ret == QMessageBox::No) {
            for (auto p = m_views.keyBegin(); p != m_views.keyEnd(); p++) {
                auto editor = *p;
                editor->closeDockWidget(); // force close
            }
            m_isOnClosing = false;
        } else {
            event->ignore();
            m_isOnClosing = false;
            return;
        }
    }

    m_dock->saveState(1);

    FramelessMainWindow::closeEvent(event);
}
