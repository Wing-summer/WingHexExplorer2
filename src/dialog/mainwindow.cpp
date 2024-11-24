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

#include "mainwindow.h"

#include "Qt-Advanced-Docking-System/src/DockAreaWidget.h"
#include "Qt-Advanced-Docking-System/src/DockFocusController.h"
#include "Qt-Advanced-Docking-System/src/DockSplitter.h"
#include "Qt-Advanced-Docking-System/src/DockWidgetTab.h"
#include "aboutsoftwaredialog.h"
#include "checksumdialog.h"
#include "class/appmanager.h"
#include "class/langservice.h"
#include "class/languagemanager.h"
#include "class/logger.h"
#include "class/qkeysequences.h"
#include "class/scriptconsolemachine.h"
#include "class/settingmanager.h"
#include "class/wingfiledialog.h"
#include "class/winginputdialog.h"
#include "class/wingmessagebox.h"
#include "control/toast.h"
#include "driverselectordialog.h"
#include "encodingdialog.h"
#include "fileinfodialog.h"
#include "finddialog.h"
#include "metadialog.h"
#include "openregiondialog.h"
#include "plugin/pluginsystem.h"
#include "settings/editorsettingdialog.h"
#include "settings/generalsettingdialog.h"
#include "settings/othersettingsdialog.h"
#include "settings/pluginsettingdialog.h"
#include "settings/scriptsettingdialog.h"

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
#include <QScrollBar>
#include <QStatusBar>
#include <QTimer>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringDecoder>
#else
#include <QTextCodec>
#endif

constexpr auto EMPTY_FUNC = [] {};

MainWindow::MainWindow(SplashDialog *splash) : FramelessMainWindow() {
    Q_ASSERT(splash);

    this->setUpdatesEnabled(false);
    this->setMinimumSize(900, 800);

    // recent file manager init
    splash->setInfoText(tr("SetupRecent"));
    m_recentMenu = new QMenu(this);
    m_recentmanager = new RecentFileManager(m_recentMenu, false);
    connect(m_recentmanager, &RecentFileManager::triggered, this,
            [=](const RecentFileManager::RecentInfo &rinfo) {
                AppManager::instance()->openFile(
                    rinfo.fileName, rinfo.isWorkSpace, rinfo.start, rinfo.stop);
            });
    m_recentmanager->apply(this, SettingManager::instance().recentHexFiles());

    splash->setInfoText(tr("SetupUI"));
    // build up UI
    buildUpRibbonBar();

    auto cw = new QWidget(this);
    cw->setSizePolicy(
        QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    auto layout = new QVBoxLayout(cw);
    layout->setContentsMargins(1, 0, 1, 0);
    layout->setSpacing(0);
    layout->addWidget(q_check_ptr(m_ribbon));

    splash->setInfoText(tr("SetupDocking"));
    buildUpDockSystem(cw);
    _defaultLayout = m_dock->saveState();

    layout->addWidget(m_dock, 1);

    m_status = new QStatusBar(this);

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

    m_toolBtneditors.value(ToolButtonIndex::EDITOR_VIEWS)->setEnabled(false);

    // ok, preparing for starting...
    this->setWindowTitle(tr("WingHexExplorer"));
    this->setWindowIcon(Utilities::isRoot()
                            ? ICONRES(QStringLiteral("iconroot"))
                            : ICONRES(QStringLiteral("icon")));

    auto &set = SettingManager::instance();

    // launch logging system
    splash->setInfoText(tr("LaunchingLog"));
    auto &log = Logger::instance();
    log.setLogLevel(Logger::Level(set.logLevel()));
    connect(&log, &Logger::log, m_logbrowser, &QTextBrowser::append);

    // launch plugin system
    splash->setInfoText(tr("SetupPluginSystem"));
    auto &plg = PluginSystem::instance();
    connect(&plg, &PluginSystem::pluginLoading, this,
            [=](const QString &plgName) {
                splash->setInfoText(tr("LoadingPlg:") + plgName);
            });
    plg.setMainWindow(this);
    plg.LoadPlugin();

    // At this time, AngelScript service plugin has started
    splash->setInfoText(tr("SetupConsole"));
    m_scriptConsole->init();
    splash->setInfoText(tr("SetupScriptManager"));
    ScriptManager::instance().attach(m_scriptConsole);

    splash->setInfoText(tr("SetupScriptService"));
    auto &langins = LangService::instance();
    langins.init(m_scriptConsole->machine()->engine());
    langins.applyLanguageSerivce(m_scriptConsole);

    m_scriptConsole->initOutput();

    splash->setInfoText(tr("SetupScriptEditor"));
    m_scriptDialog = new ScriptingDialog(this);
    m_scriptDialog->initConsole();

    // load the model
    Q_ASSERT(m_scriptConsole && m_scriptConsole->machine());
    m_varshowtable->setModel(m_scriptConsole->consoleMachine()->model());

    // connect settings signals
    connect(&set, &SettingManager::sigEditorfontSizeChanged, this,
            [this](int v) {
                auto views = m_views.keys();
                for (auto &p : views) {
                    p->setFontSize(qreal(v));
                }
            });
    connect(&set, &SettingManager::sigCopylimitChanged, this, [this](int v) {
        auto views = m_views.keys();
        for (auto &p : views) {
            p->setCopyLimit(v);
        }
    });
    connect(&set, &SettingManager::sigDecodeStrlimitChanged, this,
            [this](int v) { _decstrlim = v; });

    // ok, build up the dialog of setting
    splash->setInfoText(tr("SetupSetDialog"));
    buildUpSettingDialog();

    // we start to installing plugin widgets
    splash->setInfoText(tr("SetupPlgWidgets"));
    installPluginEditorWidgets();
    auto plgview = m_toolBtneditors.value(PLUGIN_VIEWS);
    plgview->setEnabled(!plgview->menu()->isEmpty());

    // load saved docking layout
    splash->setInfoText(tr("SetupDockingLayout"));
    m_dock->restoreState(set.dockLayout());

    m_lastusedpath = set.lastUsedPath();

    splash->setInfoText(tr("SetupWaiting"));
    // others
    _showtxt = new ShowTextDialog(this);
    qApp->processEvents();

    // update status
    updateEditModeEnabled();

    this->setUpdatesEnabled(true);

    splash->setInfoText(tr("SetupFinished"));
}

MainWindow::~MainWindow() { Logger::instance().disconnect(); }

void MainWindow::buildUpRibbonBar() {
    m_ribbon = new Ribbon(this);

    loadCacheIcon();

    using RibbonCatagories = WingHex::WingRibbonToolBoxInfo::RibbonCatagories;
    RibbonCatagories catagories;

    m_ribbonMaps[catagories.FILE] = buildFilePage(m_ribbon->addTab(tr("File")));
    qApp->processEvents();
    m_editStateWidgets << (m_ribbonMaps[catagories.EDIT] =
                               buildEditPage(m_ribbon->addTab(tr("Edit"))));
    qApp->processEvents();
    m_ribbonMaps[catagories.VIEW] = buildViewPage(m_ribbon->addTab(tr("View")));
    qApp->processEvents();
    m_ribbonMaps[catagories.SCRIPT] =
        buildScriptPage(m_ribbon->addTab(tr("Script")));
    qApp->processEvents();
    m_ribbonMaps[catagories.PLUGIN] =
        buildPluginPage(m_ribbon->addTab(tr("Plugin")));
    qApp->processEvents();
    m_ribbonMaps[catagories.SETTING] =
        buildSettingPage(m_ribbon->addTab(tr("Setting")));
    qApp->processEvents();
    m_ribbonMaps[catagories.ABOUT] =
        buildAboutPage(m_ribbon->addTab(tr("About")));
    qApp->processEvents();

    connect(m_ribbon, &Ribbon::onDragDropFiles, this,
            [=](const QStringList &files) {
                auto app = AppManager::instance();
                for (auto &f : files) {
                    app->openFile(f);
                }
            });
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

    qApp->processEvents();

    m_dock = new CDockManager;
    m_dock->setStyleSheet(QString());
    m_dock->setParent(this);
    connect(m_dock, &CDockManager::focusedDockWidgetChanged, this,
            [this](CDockWidget *old, CDockWidget *now) {
                Q_UNUSED(old);
                auto editview = qobject_cast<EditorView *>(now);
                if (editview) {
                    _editorLock.lockForWrite();
                    swapEditor(m_curEditor, editview);
                    _editorLock.unlock();
                } else {
                    m_findresult->setModel(_findEmptyResult);
                    m_bookmarks->setModel(_bookMarkEmpty);
                    m_metadatas->setModel(_metadataEmpty);
                    m_aDelBookMark->setEnabled(false);
                    m_aDelMetaData->setEnabled(false);
                }
                updateEditModeEnabled();
            });

    qApp->processEvents();

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

    qApp->processEvents();

    CDockWidget *CentralDockWidget =
        new CDockWidget(QStringLiteral("CentralWidget"));
    CentralDockWidget->setWidget(label);
    CentralDockWidget->setFeature(ads::CDockWidget::DockWidgetFocusable, false);
    CentralDockWidget->setFeature(ads::CDockWidget::NoTab, true);
    auto editorViewArea = m_dock->setCentralWidget(CentralDockWidget);

    qApp->processEvents();

    // build up basic docking widgets
    auto bottomLeftArea =
        buildUpFindResultDock(m_dock, ads::BottomDockWidgetArea);

    qApp->processEvents();

    auto splitter =
        ads::internal::findParent<ads::CDockSplitter *>(editorViewArea);
    if (splitter) {
        constexpr auto bottomHeight = 300;
        splitter->setSizes({height() - bottomHeight, bottomHeight});
    }

    qApp->processEvents();

    auto rightArea =
        buildUpLogDock(m_dock, ads::RightDockWidgetArea, editorViewArea);

    qApp->processEvents();

    splitter = ads::internal::findParent<ads::CDockSplitter *>(editorViewArea);
    if (splitter) {
        constexpr auto rightHeight = 450;
        splitter->setSizes({width() - rightHeight, rightHeight});
    }

    m_rightViewArea = rightArea;

    qApp->processEvents();

    buildUpNumberShowDock(m_dock, ads::CenterDockWidgetArea, rightArea);
    qApp->processEvents();
    buildUpHexBookMarkDock(m_dock, ads::CenterDockWidgetArea, rightArea);
    qApp->processEvents();
    buildUpHexMetaDataDock(m_dock, ads::CenterDockWidgetArea, rightArea);
    qApp->processEvents();
    buildUpDecodingStrShowDock(m_dock, ads::CenterDockWidgetArea, rightArea);
    qApp->processEvents();
    auto bottomRightArea = buildUpScriptConsoleDock(
        m_dock, ads::RightDockWidgetArea, bottomLeftArea);
    qApp->processEvents();
    buildUpHashResultDock(m_dock, ads::CenterDockWidgetArea, bottomRightArea);
    qApp->processEvents();
    buildUpVisualDataDock(m_dock, ads::CenterDockWidgetArea, bottomLeftArea);
    qApp->processEvents();
    buildUpScriptObjShowDock(m_dock, ads::CenterDockWidgetArea,
                             bottomRightArea);
    qApp->processEvents();
    m_bottomViewArea = bottomRightArea;

    // set the first tab visible
    for (auto &item : m_dock->openedDockAreas()) {
        for (int i = 0; i < item->dockWidgetsCount(); ++i) {
            qApp->processEvents();
            auto d = item->dockWidget(i);
            if (d->features().testFlag(ads::CDockWidget::NoTab)) {
                continue;
            }
            item->setCurrentIndex(i);
            break;
        }
        qApp->processEvents();
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
    _findEmptyResult = new FindResultModel(this);
    m_findresult = new QTableViewExt(this);
    m_findresult->setProperty("EditorView", quintptr(0));

    Utilities::applyTableViewProperty(m_findresult);

    m_findresult->setContextMenuPolicy(
        Qt::ContextMenuPolicy::ActionsContextMenu);
    m_findresult->addAction(newAction(QStringLiteral("export"),
                                      tr("ExportFindResult"),
                                      &MainWindow::on_exportfindresult));
    m_findresult->addAction(newAction(QStringLiteral("del"),
                                      tr("ClearFindResult"),
                                      &MainWindow::on_clearfindresult));
    m_findresult->setModel(_findEmptyResult);

    connect(m_findresult, &QTableView::doubleClicked, this,
            [=](const QModelIndex &index) {
                auto editor =
                    m_findresult->property("EditorView").value<EditorView *>();
                auto hexeditor = currentEditor();
                if (hexeditor != editor) {
                    editor->raise();
                    editor->setFocus();
                }
                editor->hexEditor()->cursor()->moveTo(
                    editor->findResultModel()->resultAt(index.row()).offset);
            });

    auto dw = buildDockWidget(dock, QStringLiteral("FindResult"),
                              tr("FindResult"), m_findresult);
    m_find = dw;

    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpNumberShowDock(ads::CDockManager *dock,
                                  ads::DockWidgetArea area,
                                  ads::CDockAreaWidget *areaw) {
    _numsitem = new NumShowModel(this);
    m_numshowtable = new QTableViewExt(this);
    Utilities::applyTableViewProperty(m_numshowtable);

    m_numshowtable->setModel(_numsitem);

    auto a = new QAction(this);
    a->setText(tr("Copy"));
    connect(a, &QAction::triggered, this, [=] {
        auto r = m_numshowtable->currentIndex().row();
        qApp->clipboard()->setText(
            _numsitem->numData(NumShowModel::NumTableIndex(r)));
        Toast::toast(this, NAMEICONRES(QStringLiteral("copy")),
                     tr("CopyToClipBoard"));
    });
    m_numshowtable->addAction(a);

    connect(m_numshowtable->selectionModel(),
            &QItemSelectionModel::currentRowChanged, a,
            [=](const QModelIndex &current, const QModelIndex &) {
                a->setEnabled(current.isValid());
            });

    a = new QAction(this);
    a->setSeparator(true);
    m_numshowtable->addAction(a);

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
    m_numshowtable->addAction(m_littleEndian);

    m_bigEndian = new QAction(actionGroup);
    m_bigEndian->setText(tr("BigEndian"));
    m_bigEndian->setCheckable(true);
    m_bigEndian->setChecked(!le);
    connect(m_bigEndian, &QAction::triggered, this, [=] {
        m_islittle = false;
        this->on_locChanged();
    });
    m_numshowtable->addAction(m_bigEndian);
    m_numshowtable->setContextMenuPolicy(
        Qt::ContextMenuPolicy::ActionsContextMenu);

    auto dw = buildDockWidget(dock, QStringLiteral("Number"), tr("Number"),
                              m_numshowtable);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpHashResultDock(ads::CDockManager *dock,
                                  ads::DockWidgetArea area,
                                  ads::CDockAreaWidget *areaw) {
    m_hashtable = new QTableViewExt(this);
    Utilities::applyTableViewProperty(m_hashtable);
    m_hashtable->setColumnWidth(0, 350);

    _hashModel = new CheckSumModel(this);
    m_hashtable->setModel(_hashModel);

    m_hashtable->setContextMenuPolicy(
        Qt::ContextMenuPolicy::ActionsContextMenu);

    auto a = new QAction(m_hashtable);
    a->setText(tr("Copy"));
    connect(a, &QAction::triggered, this, [=] {
        auto r = m_hashtable->currentIndex();
        qApp->clipboard()->setText(
            _hashModel->checkSumData(QCryptographicHash::Algorithm(r.row())));
        Toast::toast(this, NAMEICONRES(QStringLiteral("copy")),
                     tr("CopyToClipBoard"));
    });
    m_hashtable->addAction(a);
    connect(m_hashtable->selectionModel(),
            &QItemSelectionModel::currentRowChanged, a,
            [=](const QModelIndex &current, const QModelIndex &) {
                a->setEnabled(current.isValid());
            });

    auto dw = buildDockWidget(dock, QStringLiteral("CheckSum"), tr("CheckSum"),
                              m_hashtable);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpHexBookMarkDock(ads::CDockManager *dock,
                                   ads::DockWidgetArea area,
                                   ads::CDockAreaWidget *areaw) {
    _bookMarkEmpty = new BookMarksModel(nullptr);

    m_bookmarks = new QTableViewExt(this);
    Utilities::applyTableViewProperty(m_bookmarks);

    m_bookmarks->setModel(_bookMarkEmpty);

    connect(m_bookmarks, &QTableView::doubleClicked, this,
            [=](const QModelIndex &index) {
                auto hexeditor = currentHexView();
                if (hexeditor == nullptr) {
                    return;
                }
                hexeditor->renderer()->enableCursor(true);

                auto model = m_bookmarks->model();

                auto offIndex = model->index(index.row(), 0);
                auto offset = model->data(offIndex).value<qsizetype>();
                hexeditor->cursor()->moveTo(offset);
            });

    m_aDelBookMark = new QAction(ICONRES(QStringLiteral("bookmarkdel")),
                                 tr("DeleteBookMark"), m_bookmarks);
    connect(m_aDelBookMark, &QAction::triggered, this, [=] {
        auto hexeditor = currentHexView();
        if (hexeditor == nullptr) {
            return;
        }
        auto s = m_bookmarks->selectionModel()->selectedRows();
        auto doc = hexeditor->document();

        auto model = m_bookmarks->model();

        QList<qsizetype> pos;
        for (auto &item : s) {
            auto offIndex = model->index(item.row(), 0);
            auto offset = model->data(offIndex).value<qsizetype>();
            pos.append(offset);
        }

        doc->RemoveBookMarks(pos);
    });
    m_aDelBookMark->setEnabled(false);
    m_bookmarks->addAction(m_aDelBookMark);

    auto a = new QAction(ICONRES(QStringLiteral("bookmarkcls")),
                         tr("ClearBookMark"), m_bookmarks);
    connect(a, &QAction::triggered, this, &MainWindow::on_bookmarkcls);
    m_bookmarks->addAction(a);

    m_bookmarks->setContextMenuPolicy(
        Qt::ContextMenuPolicy::ActionsContextMenu);

    auto dw = buildDockWidget(dock, QStringLiteral("BookMark"), tr("BookMark"),
                              m_bookmarks);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpHexMetaDataDock(ads::CDockManager *dock,
                                   ads::DockWidgetArea area,
                                   ads::CDockAreaWidget *areaw) {
    _metadataEmpty = new MetaDataModel(nullptr);

    m_metadatas = new QTableViewExt(this);
    Utilities::applyTableViewProperty(m_metadatas);

    m_metadatas->setModel(_metadataEmpty);

    connect(m_metadatas, &QTableView::doubleClicked, this,
            [=](const QModelIndex &index) {
                auto hexeditor = currentHexView();
                if (hexeditor == nullptr) {
                    return;
                }
                hexeditor->renderer()->enableCursor(true);

                auto model = m_metadatas->model();
                auto offIndex = model->index(index.row(), 0);
                auto offset =
                    model->data(offIndex, Qt::UserRole).value<qsizetype>();
                hexeditor->cursor()->moveTo(offset);
            });

    m_aDelMetaData = new QAction(ICONRES(QStringLiteral("metadatadel")),
                                 tr("DeleteMetadata"), m_metadatas);
    connect(m_aDelMetaData, &QAction::triggered, this, [=] {
        auto hexeditor = currentHexView();
        if (hexeditor == nullptr) {
            return;
        }
        auto s = m_metadatas->selectionModel()->selectedRows();
        auto doc = hexeditor->document();

        const auto &mds = doc->metadata()->getAllMetadata();

        QList<QHexMetadataItem> pmetas;
        for (auto &item : s) {
            pmetas.push_back(mds.at(item.row()));
        }

        hexeditor->document()->metadata()->RemoveMetadatas(pmetas);
    });
    m_aDelMetaData->setEnabled(false);
    m_metadatas->addAction(m_aDelMetaData);

    auto a = new QAction(ICONRES(QStringLiteral("metadatacls")),
                         tr("ClearMetadata"), m_metadatas);
    connect(a, &QAction::triggered, this, &MainWindow::on_metadatacls);
    m_metadatas->addAction(a);

    m_metadatas->setContextMenuPolicy(
        Qt::ContextMenuPolicy::ActionsContextMenu);

    auto dw = buildDockWidget(dock, QStringLiteral("MetaData"), tr("MetaData"),
                              m_metadatas);
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
MainWindow::buildUpScriptObjShowDock(ads::CDockManager *dock,
                                     ads::DockWidgetArea area,
                                     ads::CDockAreaWidget *areaw) {
    m_varshowtable = new QTableViewExt(this);
    m_varshowtable->setEditTriggers(QTableView::EditTrigger::DoubleClicked);
    m_varshowtable->setSelectionBehavior(
        QAbstractItemView::SelectionBehavior::SelectRows);
    m_varshowtable->horizontalHeader()->setStretchLastSection(true);

    auto dw = buildDockWidget(dock, QStringLiteral("ScriptObjShow"),
                              tr("ScriptObjShow"), m_varshowtable);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpVisualDataDock(ads::CDockManager *dock,
                                  ads::DockWidgetArea area,
                                  ads::CDockAreaWidget *areaw) {
    using namespace ads;

    m_infolist = new QListView(this);
    auto dw = buildDockWidget(dock, QStringLiteral("DVList"), tr("DVList"),
                              m_infolist);
    auto ar = dock->addDockWidget(area, dw, areaw);

    m_infotree = new QTreeView(this);

    dw = buildDockWidget(dock, QStringLiteral("DVTree"), tr("DVTree"),
                         m_infotree);
    dock->addDockWidget(CenterDockWidgetArea, dw, ar);

    m_infotable = new QTableView(this);
    dw = buildDockWidget(dock, QStringLiteral("DVTable"), tr("DVTable"),
                         m_infotable);
    dock->addDockWidget(CenterDockWidgetArea, dw, ar);

    m_infotxt = new QTextBrowser(this);
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

        auto a = addPannelAction(pannel, QStringLiteral("reload"), tr("Reload"),
                                 &MainWindow::on_reload);
        m_editStateWidgets << a;
        m_cloneFileStateWidgets << a;
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
                            &MainWindow::on_redofile,
                            shortcuts.keySequence(QKeySequences::Key::REDO)));
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
        addPannelAction(pannel, QStringLiteral("sum"), tr("CheckSum"),
                        &MainWindow::on_checksum);
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
        addPannelAction(pannel, QStringLiteral("viewtxt"), tr("ViewText"),
                        &MainWindow::on_viewtxt);
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
        m_iLocked =
            addPannelAction(pannel, _infoLock, tr("SetLocked"), [this]() {
                auto hexeditor = currentHexView();
                if (hexeditor == nullptr) {
                    return;
                }
                if (!hexeditor->setLockedFile(!hexeditor->isLocked())) {
                    Toast::toast(this, _pixLock, tr("ErrUnLock"));
                }
            });
        m_iCanOver =
            addPannelAction(pannel, _infoCanOver, tr("SetOver"), [this]() {
                auto editor = currentEditor();
                if (editor == nullptr) {
                    return;
                }
                auto hexeditor = editor->hexEditor();
                auto doc = hexeditor->document();

                auto b = !hexeditor->isKeepSize();
                if ((!b && editor->documentType() ==
                               EditorView::DocumentType::RegionFile) ||
                    !hexeditor->setKeepSize(b)) {
                    Toast::toast(this, _pixCannotOver, tr("ErrUnOver"));
                } else {
                    if (b) {
                        Toast::toast(this, _pixCannotOver,
                                     tr("InfoCanOverLimit"));
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
            auto editor = currentEditor();
            if (editor == nullptr) {
                return;
            }
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
                auto hexeditor = currentHexView();
                if (hexeditor == nullptr) {
                    return;
                }
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
                auto hexeditor = currentHexView();
                if (hexeditor == nullptr) {
                    return;
                }
                hexeditor->setAddressVisible(!hexeditor->addressVisible());
            });
        addPannelAction(
            pannel, QStringLiteral("mLineInfo"), tr("SetHeaderInfo"), [this]() {
                auto hexeditor = currentHexView();
                if (hexeditor == nullptr) {
                    return;
                }
                hexeditor->setHeaderVisible(!hexeditor->headerVisible());
            });
        addPannelAction(
            pannel, QStringLiteral("mStr"), tr("SetAsciiString"), [this]() {
                auto hexeditor = currentHexView();
                if (hexeditor == nullptr) {
                    return;
                }
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
        _scriptMaps = ScriptManager::buildUpRibbonScriptRunner(pannel);
        m_scriptDBGroup = pannel;
    }

    return tab;
}

RibbonTabContent *MainWindow::buildPluginPage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();
    {
        auto pannel = tab->addGroup(tr("General"));
        addPannelAction(
            pannel, QStringLiteral("settingplugin"), tr("Plugin"),
            &MainWindow::on_setting_plugin,
            shortcuts.keySequence(QKeySequences::Key::SETTING_PLUGIN));
    }

    {
        auto pannel = tab->addGroup(tr("PluginFunctions"));
        pannel->setVisible(false);
        connect(pannel, &RibbonButtonGroup::emptyStatusChanged, this,
                [pannel](bool isEmpty) { pannel->setVisible(!isEmpty); });
        m_pluginSettingsGroup = pannel;
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
    QStringList usedIDs;
    QString id;

    m_setdialog = new SettingDialog(this);
    qApp->processEvents();

    auto generalPage = new GeneralSettingDialog(m_setdialog);
    connect(generalPage, &SettingPage::optionNeedRestartChanged, m_setdialog,
            &SettingDialog::toastTakeEffectReboot);
    m_setdialog->addPage(generalPage);
    id = generalPage->id();
    Q_ASSERT(!id.isEmpty());
    Q_ASSERT(usedIDs.indexOf(id) < 0);
    usedIDs.append(id);
    qApp->processEvents();

    auto editorPage = new EditorSettingDialog(m_setdialog);
    connect(editorPage, &SettingPage::optionNeedRestartChanged, m_setdialog,
            &SettingDialog::toastTakeEffectReboot);
    m_setdialog->addPage(editorPage);
    id = editorPage->id();
    Q_ASSERT(!id.isEmpty());
    Q_ASSERT(usedIDs.indexOf(id) < 0);
    usedIDs.append(id);
    qApp->processEvents();

    auto plgPage = new PluginSettingDialog(m_setdialog);
    connect(plgPage, &SettingPage::optionNeedRestartChanged, m_setdialog,
            &SettingDialog::toastTakeEffectReboot);
    plgPage->buildUp(m_plgPages);
    m_setdialog->addPage(plgPage);
    id = plgPage->id();
    Q_ASSERT(!id.isEmpty());
    Q_ASSERT(usedIDs.indexOf(id) < 0);
    usedIDs.append(id);
    qApp->processEvents();

    auto scriptPage = new ScriptSettingDialog(m_setdialog);
    connect(scriptPage, &SettingPage::optionNeedRestartChanged, m_setdialog,
            &SettingDialog::toastTakeEffectReboot);
    m_setdialog->addPage(scriptPage);
    id = scriptPage->id();
    Q_ASSERT(!id.isEmpty());
    Q_ASSERT(usedIDs.indexOf(id) < 0);
    usedIDs.append(id);
    qApp->processEvents();

    auto otherPage = new OtherSettingsDialog(m_setdialog);
    id = otherPage->id();
    Q_ASSERT(!id.isEmpty());
    Q_ASSERT(usedIDs.indexOf(id) < 0);
    usedIDs.append(id);
    qApp->processEvents();

    for (auto page_p = m_settingPages.constKeyValueBegin();
         page_p != m_settingPages.constKeyValueEnd(); ++page_p) {
        auto page = page_p->first;
        auto name = page->name();
        auto id = page->id();

        // check
        if (id.isEmpty()) {
            id = name;
            auto plg = page->property("__plg__").value<IWingPlugin *>();
            Logger::warning(
                QStringLiteral("[") + plg->metaObject()->className() +
                QStringLiteral("::") + name + QStringLiteral("] ") +
                QStringLiteral(":") + tr("SetPageIDEmptyTryUseName"));
        }

        if (usedIDs.contains(id)) {
            auto plg = page->property("__plg__").value<IWingPlugin *>();
            Logger::critical(
                QStringLiteral("[") + plg->metaObject()->className() +
                QStringLiteral("::") + name + QStringLiteral("] ") +
                QStringLiteral(":") + tr("SetPageDupNameIgnored"));
            continue;
        }

        connect(page, &SettingPage::optionNeedRestartChanged, m_setdialog,
                &SettingDialog::toastTakeEffectReboot);
        m_setdialog->addPage(page);
        if (page_p->second) {
            auto icon = page->categoryIcon();
            addPannelAction(m_pluginSettingsGroup, icon, name,
                            [=] { m_setdialog->showConfig(id); });
        }
        usedIDs.append(id);
        qApp->processEvents();
    }

    connect(otherPage, &SettingPage::optionNeedRestartChanged, m_setdialog,
            &SettingDialog::toastTakeEffectReboot);
    m_setdialog->addPage(otherPage);
    qApp->processEvents();

    m_setdialog->build();
}

void MainWindow::installPluginEditorWidgets() {
    QHash<QString, IWingPlugin *> names;
    auto &log = Logger::instance();

    auto menu = m_toolBtneditors.value(EDITOR_WINS);
    for (auto p = m_editorViewWidgets.begin(); p != m_editorViewWidgets.end();
         ++p) {
        for (auto &w : p.value()) {
            qApp->processEvents();
            if (names.contains(w->id())) {
                log.critical(tr("Plugin %1 contains a duplicate ID (%2) that "
                                "is already registered by plugin %3")
                                 .arg(p.key()->pluginName(), w->id(),
                                      names.value(w->id())->pluginName()));
                continue;
            }

            menu->addAction(newAction(w->icon(), w->name(), [this] {
                auto editor = currentEditor();
                if (editor == nullptr) {
                    return;
                }
                editor->switchView(m_editorViewWidgets.size());
            }));

            names.insert(w->id(), p.key());
            m_editorViewWidgetsBuffer.append(w);
        }
        qApp->processEvents();
    }

    // clear for no using
    m_editorViewWidgets.clear();
}

EditorView *MainWindow::newfileGUI() {
    if (!newOpenFileSafeCheck()) {
        return nullptr;
    }
    auto editor = new EditorView(this);
    auto index = m_newIndex++;
    editor->newFile(index);
    registerEditorView(editor);
    m_dock->addDockWidget(ads::CenterDockWidgetArea, editor, editorViewArea());
    return editor;
}

void MainWindow::on_newfile() { newfileGUI(); }

void MainWindow::on_openfile() {
    auto filename =
        WingFileDialog::getOpenFileName(this, tr("ChooseFile"), m_lastusedpath);
    if (!filename.isEmpty()) {
        m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();
        EditorView *editor = nullptr;
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

    RecentFileManager::RecentInfo info;
    info.fileName = filename;
    m_recentmanager->addRecentFile(info);
}

void MainWindow::on_openregion() {
    OpenRegionDialog d(m_lastusedpath);
    if (d.exec()) {
        auto res = d.getResult();
        EditorView *editor = nullptr;
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
    EditorView *editor = nullptr;
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

    RecentFileManager::RecentInfo info;
    info.fileName = filename;
    info.isWorkSpace = true;
    m_recentmanager->addRecentFile(info);
}

void MainWindow::on_opendriver() {
    DriverSelectorDialog ds;
    if (ds.exec()) {
        if (!Utilities::isRoot()) {
            WingMessageBox::critical(this, tr("Error"), tr("Root Required!"));
            return;
        }
        EditorView *editor = nullptr;
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
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    auto res = editor->reload();
    switch (res) {
    case ErrFile::Success: {
        Toast::toast(this, NAMEICONRES(QStringLiteral("reload")),
                     tr("ReloadSuccessfully"));
        break;
    }
    default: {
        WingMessageBox::critical(this, tr("Error"), tr("ReloadUnSuccessfully"));
        break;
    }
    }
}

void MainWindow::on_save() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }

    auto res = saveEditor(editor, {}, false);

    auto isNewFile = editor->isNewFile();
    if (isNewFile) {
        on_saveas();
        return;
    }

restart:
    if (res == ErrFile::IsNewFile) {
        on_saveas();
        return;
    }
    if (res == ErrFile::Permission) {
        WingMessageBox::critical(this, tr("Error"), tr("FilePermission"));
        return;
    }
    if (res == ErrFile::SourceFileChanged) {
        if (WingMessageBox::warning(this, tr("Warn"), tr("SourceChanged"),
                                    QMessageBox::Yes | QMessageBox::No) ==
            QMessageBox::Yes) {
            res = saveEditor(editor, {}, true);
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

    Toast::toast(this, NAMEICONRES(QStringLiteral("save")),
                 tr("SaveSuccessfully"));
}

void MainWindow::on_saveas() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }

    auto filename = WingFileDialog::getSaveFileName(this, tr("ChooseSaveFile"),
                                                    m_lastusedpath);
    if (filename.isEmpty())
        return;
    m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();

    auto res = saveEditor(editor, filename, false);

restart:
    switch (res) {
    case ErrFile::Success: {
        Toast::toast(this, NAMEICONRES(QStringLiteral("saveas")),
                     tr("SaveSuccessfully"));
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
            res = saveEditor(editor, filename, true);
            goto restart;
        } else {
            Toast::toast(this, NAMEICONRES(QStringLiteral("save")),
                         tr("SaveSourceFileError"));
        }
        break;
    }
    default: {
        WingMessageBox::critical(this, tr("Error"), tr("SaveUnSuccessfully"));
        break;
    }
    }
}

void MainWindow::on_exportfile() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }

    auto filename = WingFileDialog::getSaveFileName(
        this, tr("ChooseExportFile"), m_lastusedpath);
    if (filename.isEmpty())
        return;
    m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();

    auto res = saveEditor(editor, filename, false, true);
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
            res = saveEditor(editor, filename, true, true);
            goto restart;
        } else {
            Toast::toast(this, NAMEICONRES(QStringLiteral("export")),
                         tr("ExportSourceFileError"));
        }
        break;
    }
    default: {
        WingMessageBox::critical(this, tr("Error"), tr("ExportUnSuccessfully"));
        break;
    }
    }
}

void MainWindow::on_savesel() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    auto filename = WingFileDialog::getSaveFileName(this, tr("ChooseSaveFile"),
                                                    m_lastusedpath);
    if (filename.isEmpty())
        return;
    m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();
    QFile qfile(filename);
    if (qfile.open(QFile::WriteOnly)) {
        auto buffer = hexeditor->selectedBytes();
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
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    hexeditor->document()->undo();
}

void MainWindow::on_redofile() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    hexeditor->document()->redo();
}

void MainWindow::on_cutfile() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    if (Q_LIKELY(hexeditor->Cut())) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("cut")),
                     tr("CutToClipBoard"));
    } else {
        Toast::toast(this, NAMEICONRES(QStringLiteral("cut")),
                     tr("UnCutToClipBoard"));
    }
}

void MainWindow::on_copyfile() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    if (Q_LIKELY(hexeditor->copy())) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("copy")),
                     tr("CopyToClipBoard"));
    } else {
        Toast::toast(this, NAMEICONRES(QStringLiteral("copy")),
                     tr("UnCopyToClipBoard"));
    }
}

void MainWindow::on_pastefile() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    hexeditor->Paste();
}

void MainWindow::on_delete() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    hexeditor->RemoveSelection();
}

void MainWindow::on_clone() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    auto hexeditor = editor->clone();
    if (hexeditor == nullptr) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("clone")),
                     tr("NoMoreClone"));
        return;
    }

    registerEditorView(hexeditor);
    m_dock->addDockWidget(ads::CenterDockWidgetArea, hexeditor,
                          editorViewArea());
}

void MainWindow::on_findfile() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    auto hexeditor = editor->hexEditor();
    FindDialog fd(editor->isBigFile(), 0, int(hexeditor->documentBytes()),
                  hexeditor->selectlength() > 1, this);
    if (fd.exec()) {
        FindDialog::Result r;
        auto res = fd.getResult(r);
        ExecAsync<EditorView::FindError>(
            [this, r, res]() -> EditorView::FindError {
                return currentEditor()->find(res, r);
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
                m_find->raise();
            });
    }
}

void MainWindow::on_gotoline() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    editor->triggerGoto();
}

void MainWindow::on_encoding() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    EncodingDialog d;
    if (d.exec()) {
        auto res = d.getResult();
        hexeditor->renderer()->SetEncoding(res);
    }
}

void MainWindow::on_checksum() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    CheckSumDialog d;
    if (d.exec()) {
        auto cs = d.getResults();
        auto hashes = Utilities::supportedHashAlgorithms();

        if (editor->hexEditor()->selectlength() > 1) {
            auto data = editor->hexEditor()->selectedBytes();
            for (auto &c : cs) {
                auto h = hashes.at(c);
                QCryptographicHash hash(h);
                hash.addData(data);
                _hashModel->setCheckSumData(h, hash.result().toHex().toUpper());
            }
        } else {
            if (editor->isNewFile()) {
                auto bytes = editor->hexEditor()->document()->read(0);
                for (auto &c : cs) {
                    auto h = hashes.at(c);
                    QCryptographicHash hash(h);
                    hash.addData(bytes);
                    _hashModel->setCheckSumData(
                        h, hash.result().toHex().toUpper());
                }
            } else {
                QFile f(editor->fileName());
                for (auto &c : cs) {
                    auto h = hashes.at(c);
                    QCryptographicHash hash(h);
                    hash.addData(&f);
                    _hashModel->setCheckSumData(
                        h, hash.result().toHex().toUpper());
                }
            }
        }
    }
}

void MainWindow::on_fileInfo() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    FileInfoDialog d(editor->fileName(),
                     editor->documentType() ==
                         EditorView::DocumentType::RegionFile);
    d.exec();
}

void MainWindow::on_cuthex() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    if (hexeditor->Cut(true)) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("cut")),
                     tr("CutToClipBoard"));
    } else {
        Toast::toast(this, NAMEICONRES(QStringLiteral("cut")),
                     tr("UnCutToClipBoard"));
    }
}

void MainWindow::on_copyhex() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    if (hexeditor->copy(true)) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("copyhex")),
                     tr("CopyToClipBoard"));
    } else {
        Toast::toast(this, NAMEICONRES(QStringLiteral("copyhex")),
                     tr("UnCopyToClipBoard"));
    }
}

void MainWindow::on_pastehex() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    hexeditor->Paste();
}

void MainWindow::on_fill() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    bool b;
    auto in = WingInputDialog::getText(this, tr("Fill"), tr("PleaseInputFill"),
                                       QLineEdit::Normal, QString(), &b);
    if (b) {
        auto ch = char(in.toULongLong(&b, 0));
        if (b) {
            auto doc = hexeditor->document();
            if (doc->isEmpty() || hexeditor->selectlength() == 0)
                return;
            auto pos = hexeditor->cursor()->selectionStart().offset();
            hexeditor->Replace(
                pos, QByteArray(int(hexeditor->selectlength()), char(ch)));
        } else {
            Toast::toast(this, NAMEICONRES(QStringLiteral("fill")),
                         tr("FillInputError"));
        }
    }
}

void MainWindow::on_fillzero() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    auto doc = hexeditor->document();
    if (doc->isEmpty() || hexeditor->selectlength() == 0)
        return;
    auto pos = hexeditor->cursor()->selectionStart().offset();
    hexeditor->Replace(pos,
                       QByteArray(int(hexeditor->selectlength()), char(0)));
}

void MainWindow::on_bookmark() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    auto doc = hexeditor->document();
    auto pos = hexeditor->currentOffset();

    if (doc->existBookMark(pos)) {
        auto bcomment = doc->bookMark(pos);
        bool ok;
        hexeditor->renderer()->enableCursor();
        auto comment =
            WingInputDialog::getText(this, tr("BookMark"), tr("InputComment"),
                                     QLineEdit::Normal, bcomment, &ok);
        if (ok) {
            doc->ModBookMark(pos, comment);
        }
    } else {
        bool ok;
        auto comment =
            WingInputDialog::getText(this, tr("BookMark"), tr("InputComment"),
                                     QLineEdit::Normal, QString(), &ok);
        if (ok) {
            doc->AddBookMark(pos, comment);
        }
    }
}

void MainWindow::on_bookmarkdel() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    auto doc = hexeditor->document();
    auto pos = hexeditor->currentOffset();

    if (doc->bookMarkExists(pos)) {
        doc->RemoveBookMark(pos);
    }
}

void MainWindow::on_bookmarkcls() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    auto doc = hexeditor->document();
    doc->ClearBookMark();
}

void MainWindow::on_metadata() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    auto doc = hexeditor->document();
    if (hexeditor->documentBytes() > 0) {
        MetaDialog m(this);
        auto cur = hexeditor->cursor();
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
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    auto doc = hexeditor->document();

    if (hexeditor->documentBytes() > 0) {
        MetaDialog m(this);
        auto cur = hexeditor->cursor();
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
                    QHexMetadataItem o;
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
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    auto doc = hexeditor->document();
    auto meta = doc->metadata();
    auto pos = hexeditor->cursor()->position().offset();
    meta->RemoveMetadata(pos);
}

void MainWindow::on_metadatacls() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    auto doc = hexeditor->document();
    doc->metadata()->Clear();
}

void MainWindow::on_metadatafg(bool checked) {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    auto hexeditor = editor->hexEditor();
    auto doc = hexeditor->document();
    if (editor->isOriginWorkSpace()) {
        doc->SetMetafgVisible(checked);
    } else {
        doc->setMetafgVisible(checked);
    }
}

void MainWindow::on_metadatabg(bool checked) {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    auto hexeditor = editor->hexEditor();
    auto doc = hexeditor->document();
    if (editor->isOriginWorkSpace()) {
        doc->SetMetabgVisible(checked);
    } else {
        doc->setMetabgVisible(checked);
    }
}

void MainWindow::on_metadatacomment(bool checked) {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    auto hexeditor = editor->hexEditor();
    auto doc = hexeditor->document();
    if (editor->isOriginWorkSpace()) {
        doc->SetMetaCommentVisible(checked);
    } else {
        doc->setMetaCommentVisible(checked);
    }
}

void MainWindow::on_metashowall() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    auto hexeditor = editor->hexEditor();
    auto doc = hexeditor->document();
    if (editor->isOriginWorkSpace()) {
        doc->SetMetaVisible(true);
    } else {
        doc->setMetafgVisible(true);
        doc->setMetabgVisible(true);
        doc->setMetaCommentVisible(true);
    }
}

void MainWindow::on_metahideall() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    auto hexeditor = editor->hexEditor();
    auto doc = hexeditor->document();
    if (editor->isOriginWorkSpace()) {
        doc->SetMetaVisible(false);
    } else {
        doc->setMetafgVisible(false);
        doc->setMetabgVisible(false);
        doc->setMetaCommentVisible(false);
    }
}

void MainWindow::on_clearfindresult() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    editor->clearFindResult();
}

void MainWindow::on_exportfindresult() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }

    auto findresitem = editor->findResultModel();

    auto c = findresitem->size();
    if (c == 0) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("export")),
                     tr("EmptyFindResult"));
        return;
    }
    auto filename = WingFileDialog::getSaveFileName(this, tr("ChooseSaveFile"),
                                                    m_lastusedpath);
    if (filename.isEmpty())
        return;
    m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();

    QFile f(filename);
    if (f.open(QFile::WriteOnly)) {
        QJsonObject fobj;
        fobj.insert(QStringLiteral("file"), editor->fileName());
        fobj.insert(QStringLiteral("data"), findresitem->lastFindData());
        QJsonArray arr;
        for (int i = 0; i < c; i++) {
            auto data = findresitem->resultAt(i);
            QJsonObject jobj;
            jobj.insert(QStringLiteral("line"), QString::number(data.line));
            jobj.insert(QStringLiteral("col"), QString::number(data.col));
            jobj.insert(QStringLiteral("offset"), QString::number(data.offset));
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
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    m_lblloc->setText(QStringLiteral("(%1,%2)")
                          .arg(hexeditor->currentRow())
                          .arg(hexeditor->currentColumn()));
    auto sellen = hexeditor->selectlength();
    m_lblsellen->setText(QStringLiteral("%1 - 0x%2")
                             .arg(sellen)
                             .arg(QString::number(sellen, 16).toUpper()));

    // number analyse
    auto off = hexeditor->currentOffset();
    auto d = hexeditor->document();

    auto tmp = d->read(off, sizeof(quint64));
    quint64 n = *reinterpret_cast<const quint64 *>(tmp.constData());

    auto len = size_t(tmp.length());

    if (len == sizeof(quint64)) {
        auto s = processEndian(n);
        _numsitem->setNumData(
            NumShowModel::NumTableIndex::Uint64,
            QStringLiteral("0x%1").arg(QString::number(s, 16).toUpper()));
        auto s1 = processEndian(qsizetype(n));
        _numsitem->setNumData(NumShowModel::NumTableIndex::Int64,
                              QString::number(s1));
        double s2 = *(double *)(&n);
        auto s3 = processEndian(s2);
        _numsitem->setNumData(NumShowModel::NumTableIndex::Double64,
                              qIsNaN(s3) ? QStringLiteral("NAN")
                                         : QString::number(s3));
    } else {
        _numsitem->setNumData(NumShowModel::NumTableIndex::Uint64, QString());
        _numsitem->setNumData(NumShowModel::NumTableIndex::Int64, QString());
        _numsitem->setNumData(NumShowModel::NumTableIndex::Double64, QString());
    }

    if (len > sizeof(quint32)) {
        auto s = processEndian(quint32(n));
        _numsitem->setNumData(
            NumShowModel::NumTableIndex::Uint32,
            QStringLiteral("0x%1").arg(QString::number(s, 16).toUpper()));
        auto s1 = processEndian(qint32(n));
        _numsitem->setNumData(NumShowModel::NumTableIndex::Int32,
                              QString::number(s1));
        float s2 = *(float *)(&n);
        auto s3 = processEndian(s2);
        _numsitem->setNumData(NumShowModel::NumTableIndex::Float32,
                              qIsNaN(s3) ? QStringLiteral("NAN")
                                         : QString::number(s3));
    } else {
        _numsitem->setNumData(NumShowModel::NumTableIndex::Uint32, QString());
        _numsitem->setNumData(NumShowModel::NumTableIndex::Int32, QString());
        _numsitem->setNumData(NumShowModel::NumTableIndex::Float32, QString());
    }

    if (len > sizeof(quint16)) {
        auto s = processEndian(quint16(n));
        _numsitem->setNumData(
            NumShowModel::NumTableIndex::Ushort,
            QStringLiteral("0x%1").arg(QString::number(s, 16).toUpper()));
        auto s1 = processEndian(qint16(n));
        _numsitem->setNumData(NumShowModel::NumTableIndex::Short,
                              QString::number(s1));
    } else {
        _numsitem->setNumData(NumShowModel::NumTableIndex::Ushort, QString());
        _numsitem->setNumData(NumShowModel::NumTableIndex::Short, QString());
    }
    if (len > sizeof(uchar)) {
        auto s1 = tmp.at(0);
        auto s = uchar(s1);
        _numsitem->setNumData(
            NumShowModel::NumTableIndex::Byte,
            QStringLiteral("0x%1").arg(QString::number(s, 16).toUpper()));
        _numsitem->setNumData(NumShowModel::NumTableIndex::Char,
                              QString::number(s1));
    } else {
        _numsitem->setNumData(NumShowModel::NumTableIndex::Byte, QString());
        _numsitem->setNumData(NumShowModel::NumTableIndex::Char, QString());
    }

    // 解码字符串
    if (sellen > 1) {
        // 如果不超过 10KB （默认）那么解码，防止太多卡死
        if (sellen <= 1024 * _decstrlim) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            auto enc = QStringConverter::encodingForName(
                hexeditor->renderer()->encoding().toUtf8());
            Q_ASSERT(enc.has_value());
            QStringDecoder dec(enc.value());
            m_txtDecode->setText(dec.decode(hexeditor->selectedBytes()));
#else
            auto enc = QTextCodec::codecForName(
                hexeditor->renderer()->encoding().toUtf8());
            auto dec = enc->makeDecoder();
            m_txtDecode->setText(dec->toUnicode(hexeditor->selectedBytes()));
#endif
        } else {
            m_txtDecode->setHtml(QStringLiteral("<font color=\"red\">%1</font>")
                                     .arg(tr("TooManyBytesDecode")));
        }
    } else {
        m_txtDecode->clear();
    }
}

void MainWindow::on_viewtxt() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    _showtxt->load(hexeditor->document()->buffer());
}

void MainWindow::on_fullScreen() {
    if (this->isFullScreen()) {
        this->showMaximized();
    } else {
        this->showFullScreen();
    }
}

void MainWindow::on_restoreLayout() { m_dock->restoreState(_defaultLayout); }

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

void MainWindow::on_setting_general() { m_setdialog->showConfig(); }

void MainWindow::on_setting_plugin() { m_setdialog->showConfig(2); }

void MainWindow::on_about() { AboutSoftwareDialog().exec(); }

void MainWindow::on_sponsor() {
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

void MainWindow::on_wiki() {
    QDesktopServices::openUrl(
        QUrl(QStringLiteral("https://www.cnblogs.com/wingsummer/p/18286419")));
}

QString MainWindow::saveLog() {
    QDir ndir(Utilities::getAppDataPath());
    ndir.mkpath(QStringLiteral("log")); // 确保日志存放目录存在

    QFile lfile(ndir.absolutePath() + QDir::separator() +
                QStringLiteral("log") + QDir::separator() +
                QDateTime::currentDateTime().toString(
                    QStringLiteral("yyyyMMddhhmmsszzz")) +
                ".log");
    if (lfile.open(QFile::WriteOnly | QFile::NewOnly)) {
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
    auto views = m_views.keys();
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

bool MainWindow::newOpenFileSafeCheck() {
    if (m_views.size() >=
        std::numeric_limits<decltype(m_views)::size_type>::max() - 1) {
        WingMessageBox::critical(this, tr("Error"),
                                 tr("Too much opened files"));
        return false;
    }
    return true;
}

void MainWindow::registerEditorView(EditorView *editor) {
    for (auto &w : m_editorViewWidgetsBuffer) {
        editor->registerView(w);
    }
    for (auto &m : m_hexContextMenu) {
        editor->registerQMenu(m);
    }

    auto &set = SettingManager::instance();
    editor->setCopyLimit(set.copylimit());
    editor->setFontSize(set.editorfontSize());

    connectEditorView(editor);
    m_views.insert(editor, {});
    auto ev = m_toolBtneditors.value(ToolButtonIndex::EDITOR_VIEWS);
    auto menu = ev->menu();
    Q_ASSERT(menu);
    auto ta = editor->toggleViewAction();
    menu->addAction(ta);
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
    connect(editor, &EditorView::sigOnMetadata, this, [=] {
        auto hexeditor = currentHexView();
        if (hexeditor == nullptr) {
            return;
        }
        auto doc = hexeditor->document();
        if (hexeditor->documentBytes() > 0) {
            MetaDialog m(this);
            auto cur = hexeditor->cursor();
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
                        QHexMetadataItem o;
                        o.begin = begin;
                        o.end = end;
                        o.foreground = m.foreGroundColor();
                        o.background = m.backGroundColor();
                        o.comment = m.comment();
                        mi->ModifyMetadata(meta, o);
                    }
                } else {
                    auto begin = cur->selectionStart().offset();
                    auto end = cur->selectionEnd().offset() + 1;
                    if (m.exec()) {
                        doc->metadata()->Metadata(
                            begin, end, m.foreGroundColor(),
                            m.backGroundColor(), m.comment());
                    }
                }
            } else {
                Toast::toast(this, NAMEICONRES(QStringLiteral("metadata")),
                             tr("NoSelection"));
            }
        }
    });
    connect(editor, &EditorView::sigOnPasteHex, this, &MainWindow::on_pastehex);
    connect(editor, &EditorView::sigOnPasteFile, this,
            &MainWindow::on_pastefile);

    connect(editor, &EditorView::closeRequested, this, [this] {
        auto editor = qobject_cast<EditorView *>(sender());
        Q_ASSERT(editor);
        Q_ASSERT(m_views.contains(editor));

        if (closeEditor(editor, m_isOnClosing) == ErrFile::UnSaved) {
            auto ret = this->saveRequest();
            if (ret == QMessageBox::Cancel) {
                return;
            } else if (ret == QMessageBox::Yes) {
                if (saveEditor(editor, {}, false) == ErrFile::Success) {
                    closeEditor(editor, m_isOnClosing);
                }
            } else {
                closeEditor(editor, true);
            }
        }
    });
}

void MainWindow::swapEditor(EditorView *old, EditorView *cur) {
    if (old == cur) {
        return;
    }

    if (old != nullptr) {
        auto hexeditor = old->hexEditor();
        hexeditor->disconnect(SLOT(cursorLocationChanged()));
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

    m_iReadWrite->setIcon(hexeditor->isReadOnly() ? _infoReadonly
                                                  : _infoWriteable);

    loadFindResult(cur);

    auto bm = cur->bookmarksModel();
    m_bookmarks->setModel(bm);
    connect(m_bookmarks->selectionModel(),
            &QItemSelectionModel::selectionChanged, m_aDelBookMark,
            [=](const QItemSelection &, const QItemSelection &) {
                m_aDelBookMark->setEnabled(
                    m_bookmarks->selectionModel()->hasSelection());
            });
    auto md = cur->metadataModel();
    m_metadatas->setModel(md);
    connect(m_metadatas->selectionModel(),
            &QItemSelectionModel::selectionChanged, m_aDelMetaData,
            [=](const QItemSelection &, const QItemSelection &) {
                m_aDelMetaData->setEnabled(
                    m_metadatas->selectionModel()->hasSelection());
            });

    m_curEditor = cur;
    hexeditor->getStatus();
}

void MainWindow::loadFindResult(EditorView *view) {
    auto result = view->findResultModel();
    m_findresult->setModel(result);
    m_findresult->setProperty("EditorView", QVariant::fromValue(view));
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
    auto e = findEditorView(file);
    if (editor) {
        *editor = e;
    }
    if (e) {
        return ErrFile::AlreadyOpened;
    }

    // ok, going on
    if (!newOpenFileSafeCheck()) {
        return ErrFile::Error;
    }

    QFileInfo finfo(file);
    auto filename = finfo.absoluteFilePath();

    auto ev = new EditorView(this);
    auto res = ev->openFile(filename);

    if (res != ErrFile::Success) {
        delete ev;
        return res;
    }

    m_views.insert(ev, QString());
    registerEditorView(ev);
    m_dock->addDockWidget(ads::CenterDockWidgetArea, ev, editorViewArea());
    return ErrFile::Success;
}

ErrFile MainWindow::openDriver(const QString &driver, EditorView **editor) {
    auto e = findEditorView(driver);
    if (editor) {
        *editor = e;
    }
    if (e) {
        return ErrFile::AlreadyOpened;
    }

    // ok, going on
    if (!newOpenFileSafeCheck()) {
        return ErrFile::Error;
    }

    auto ev = new EditorView(this);
    auto res = ev->openDriver(driver);

    if (res != ErrFile::Success) {
        delete ev;
        return res;
    }

    m_views.insert(ev, QString());
    registerEditorView(ev);
    m_dock->addDockWidget(ads::CenterDockWidgetArea, ev, editorViewArea());
    return ErrFile::Success;
}

ErrFile MainWindow::openWorkSpace(const QString &file, EditorView **editor) {
    auto e = findEditorView(file);
    if (editor) {
        *editor = e;
    }
    if (e) {
        return ErrFile::AlreadyOpened;
    }

    // ok, going on
    if (!newOpenFileSafeCheck()) {
        return ErrFile::Error;
    }

    QFileInfo finfo(file);
    auto filename = finfo.absoluteFilePath();

    auto ev = new EditorView(this);
    auto res = ev->openWorkSpace(filename);

    if (res != ErrFile::Success) {
        delete ev;
        return res;
    }

    m_views.insert(ev, file);
    registerEditorView(ev);
    m_dock->addDockWidget(ads::CenterDockWidgetArea, ev, editorViewArea());
    return ErrFile::Success;
}

ErrFile MainWindow::openRegionFile(QString file, EditorView **editor,
                                   qsizetype start, qsizetype length) {
    auto e = findEditorView(file);
    if (editor) {
        *editor = e;
    }
    if (e) {
        return ErrFile::AlreadyOpened;
    }

    // ok, going on
    if (!newOpenFileSafeCheck()) {
        return ErrFile::Error;
    }

    QFileInfo finfo(file);
    auto filename = finfo.absoluteFilePath();

    auto ev = new EditorView(this);
    auto res = ev->openRegionFile(filename, start, length);

    if (res != ErrFile::Success) {
        delete ev;
        return res;
    }

    registerEditorView(ev);
    m_dock->addDockWidget(ads::CenterDockWidgetArea, ev, editorViewArea());
    return ErrFile::Success;
}

ErrFile MainWindow::saveEditor(EditorView *editor, const QString &filename,
                               bool ignoreMd5, bool isExport) {
    if (editor == nullptr) {
        return ErrFile::Error;
    }

    auto isNewFile = editor->isNewFile();
    if (isNewFile && filename.isEmpty()) {
        return ErrFile::IsNewFile;
    }

    if (!writeSafeCheck(false, {})) {
        return ErrFile::Permission;
    }

    QString workspace = m_views.value(editor);
    if (editor->change2WorkSpace()) {
        workspace = editor->fileName() + PROEXT;
    }

    auto ret = editor->save(workspace, filename, ignoreMd5, isExport);
    if (ret == ErrFile::Success) {
        m_views[editor] = workspace;
    }
    return ret;
}

ErrFile MainWindow::closeEditor(EditorView *editor, bool force) {
    if (editor == nullptr) {
        return ErrFile::Error;
    }
    if (!editor->isCloneFile()) {
        auto hexeditor = editor->hexEditor();
        if (!force) {
            if (!hexeditor->isSaved()) {
                return ErrFile::UnSaved;
            }
        }
    }

    m_views.remove(editor);
    if (currentEditor() == editor) {
        _editorLock.lockForWrite();
        m_curEditor = nullptr;
        _editorLock.unlock();
    }
    PluginSystem::instance().cleanUpEditorViewHandle(editor);
    editor->deleteDockWidget();
    m_toolBtneditors.value(ToolButtonIndex::EDITOR_VIEWS)
        ->setEnabled(m_views.size() != 0);

    if (m_dock->focusedDockWidget() == editor) {
        if (!m_views.isEmpty()) {
            for (auto p = m_views.keyBegin(); p != m_views.keyEnd(); ++p) {
                auto ev = *p;
                if (ev != editor && ev->isCurrentTab()) {
                    ev->setFocus();
                }
            }
        }
    }
    return ErrFile::Success;
}

void MainWindow::updateEditModeEnabled() {
    auto editor = currentEditor();
    auto b = (editor != nullptr);

    for (auto &item : m_editStateWidgets) {
        item->setEnabled(b);
    }

    if (b) {
        auto hexeditor = editor->hexEditor();
        enableDirverLimit(editor->isDriver());
        enableCloneFileLimit(editor->isCloneFile());
        auto doc = hexeditor->document();
        emit doc->canRedoChanged(doc->canRedo());
        emit doc->canUndoChanged(doc->canUndo());
    } else {
        m_lblloc->setText(QStringLiteral("(0,0)"));
        m_lblsellen->setText(QStringLiteral("0 - 0x0"));
        _numsitem->clear();
    }
}

void MainWindow::enableDirverLimit(bool isDriver) {
    auto e = !isDriver;
    for (auto &item : m_driverStateWidgets) {
        item->setEnabled(e);
    }
}

void MainWindow::enableCloneFileLimit(bool isCloneFile) {
    auto e = !isCloneFile;
    for (auto &item : m_cloneFileStateWidgets) {
        item->setEnabled(e);
    }
}

void MainWindow::setCurrentHexEditorScale(qreal rate) {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    hexeditor->setScaleRate(rate);
}

EditorView *MainWindow::currentEditor() {
    if (QThread::currentThread() != this->thread()) {
        _editorLock.lockForRead();
        auto ret = m_curEditor;
        _editorLock.unlock();
        return ret;
    }
    return m_curEditor;
}

QHexView *MainWindow::currentHexView() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return nullptr;
    }
    return editor->hexEditor();
}

bool MainWindow::writeSafeCheck(bool isNewFile, const QString &savePath) {
    if (Utilities::isRoot()) {
        if (isNewFile || savePath.isEmpty()) {
            return false;
        }

        if (Utilities::isStorageDevice(savePath)) {
            return false;
        }

        QFileInfo finfo(savePath);
        if (!finfo.exists()) {
            return false;
        }
    }
    return true;
}

void MainWindow::loadCacheIcon() {
    _infoSaved = ICONRES(QStringLiteral("saved"));

    _infoUnsaved = ICONRES(QStringLiteral("unsaved"));

    _infoWriteable = ICONRES(QStringLiteral("writable"));
    _infoReadonly = ICONRES(QStringLiteral("readonly"));

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

ads::CDockAreaWidget *MainWindow::editorViewArea() const {
    return m_dock->centralWidget()->dockAreaWidget();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    m_isOnClosing = true;
    // first checking the scripting dialog
    if (!m_scriptDialog->about2Close()) {
        event->ignore();
        m_isOnClosing = false;
        return;
    }

    // then checking itself
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
            auto views = m_views.keys();
            for (auto &p : views) {
                emit p->closeRequested();
            }
            m_isOnClosing = false;
            if (!m_views.isEmpty()) {
                event->ignore();
                return;
            }
        } else if (ret == QMessageBox::No) {
            auto views = m_views.keys();
            for (auto &p : views) {
                p->closeDockWidget(); // force close
            }
            m_isOnClosing = false;
        } else {
            event->ignore();
            m_isOnClosing = false;
            return;
        }
    }

    auto &set = SettingManager::instance();
    set.setDockLayout(m_dock->saveState());

    m_scriptDialog->saveDockLayout();
    set.setRecentFiles(m_recentmanager->saveRecent());
    set.save();

    FramelessMainWindow::closeEvent(event);
}
