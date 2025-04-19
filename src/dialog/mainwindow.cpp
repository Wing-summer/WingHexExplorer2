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
#include "class/dockcomponentsfactory.h"
#include "class/eventfilter.h"
#include "class/inspectqtloghelper.h"
#include "class/languagemanager.h"
#include "class/layoutmanager.h"
#include "class/logger.h"
#include "class/pluginsystem.h"
#include "class/qkeysequences.h"
#include "class/richtextitemdelegate.h"
#include "class/scriptmachine.h"
#include "class/settingmanager.h"
#include "class/wingfiledialog.h"
#include "class/winginputdialog.h"
#include "class/wingmessagebox.h"
#include "class/wingupdater.h"
#include "control/toast.h"
#include "encodingdialog.h"
#include "fileinfodialog.h"
#include "finddialog.h"
#include "metadialog.h"
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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QPainter>
#include <QPicture>
#include <QPushButton>
#include <QRadioButton>
#include <QScopeGuard>
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
    this->setUpdatesEnabled(false);
    this->setMinimumSize(900, 800);

    // recent file manager init
    if (splash)
        splash->setInfoText(tr("SetupRecent"));
    m_recentMenu = new QMenu(this);
    m_recentmanager = new RecentFileManager(m_recentMenu, false);
    connect(m_recentmanager, &RecentFileManager::triggered, this,
            [=](const RecentFileManager::RecentInfo &rinfo) {
                AppManager::instance()->openFile(rinfo.fileName,
                                                 rinfo.isWorkSpace);
            });
    m_recentmanager->apply(this, SettingManager::instance().recentHexFiles());

    if (splash)
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

    if (splash)
        splash->setInfoText(tr("SetupDocking"));
    buildUpDockSystem(cw);

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

        _status = new QLabel(m_status);
        m_status->addPermanentWidget(_status);

        auto separator = new QFrame(m_status);
        separator->setFrameShape(QFrame::VLine);
        separator->setFrameShadow(QFrame::Sunken);
        m_status->addPermanentWidget(separator);

        auto disableStyle =
            QStringLiteral("border:none;background:transparent;");

        m_sSaved = new QToolButton(this);
        m_sSaved->setStyleSheet(disableStyle);
        m_sSaved->setToolTip(tr("InfoSave"));
        m_sSaved->setIcon(_infoSaved);
        m_sSaved->setToolButtonStyle(Qt::ToolButtonIconOnly);
        m_status->addPermanentWidget(m_sSaved);
        m_editStateWidgets << m_sSaved;

        m_sReadWrite = new QToolButton(this);
        m_sReadWrite->setStyleSheet(disableStyle);
        m_sReadWrite->setToolTip(tr("ReadOnly"));
        m_sReadWrite->setIcon(_infoWriteable);
        m_sReadWrite->setToolButtonStyle(Qt::ToolButtonIconOnly);
        m_status->addPermanentWidget(m_sReadWrite);
        m_editStateWidgets << m_sReadWrite;

        auto ev = new EventFilter(QEvent::MouseButtonDblClick, m_status);
        connect(ev, &EventFilter::eventTriggered, this,
                [this](QObject *obj, QEvent *) {
                    if (obj == m_sLocked) {
                        if (m_sLocked->isEnabled()) {
                            m_iLocked->click();
                        }
                    } else if (obj == m_sCanOver) {
                        if (m_sCanOver->isEnabled()) {
                            m_iCanOver->click();
                        }
                    }
                });

        m_sLocked = new QToolButton(this);
        m_sLocked->setStyleSheet(disableStyle);
        m_sLocked->setToolTip(tr("SetLocked"));
        m_sLocked->setIcon(_infoUnLock);
        m_sLocked->setToolButtonStyle(Qt::ToolButtonIconOnly);
        m_sLocked->installEventFilter(ev);
        m_status->addPermanentWidget(m_sLocked);
        m_editStateWidgets << m_sLocked;

        m_sCanOver = new QToolButton(this);
        m_sCanOver->setStyleSheet(disableStyle);
        m_sCanOver->setToolTip(tr("SetOver"));
        m_sCanOver->setIcon(_infoCannotOver);
        m_sCanOver->setToolButtonStyle(Qt::ToolButtonIconOnly);
        m_sCanOver->installEventFilter(ev);
        m_status->addPermanentWidget(m_sCanOver);
        m_editStateWidgets << m_sCanOver;
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
    if (splash)
        splash->setInfoText(tr("LaunchingLog"));
    auto &log = Logger::instance();
    log.setLogLevel(Logger::Level(set.logLevel()));
    connect(&log, &Logger::log, m_logbrowser, &QTextBrowser::append);

    // check config writable
    set.checkWriteableAndWarn();

    // launch plugin system
    if (splash)
        splash->setInfoText(tr("SetupPluginSystem"));
    auto &plg = PluginSystem::instance();
    connect(&plg, &PluginSystem::pluginLoading, this,
            [=](const QString &plgName) {
                if (splash)
                    splash->setInfoText(tr("LoadingPlg:") + plgName);
            });

    plg.setMainWindow(this);
    plg.loadAllPlugin();

    // Don't setup it too early, because the plugin can register script
    // functions. Code completions of them will be not worked out.
    if (set.scriptEnabled()) {
        auto &sm = ScriptMachine::instance();
        auto smr = sm.init();
        if (smr) {
            ScriptMachine::RegCallBacks callbacks;
            callbacks.getInputFn = [this]() -> QString {
                return m_scriptConsole->getInput();
            };
            callbacks.clearFn = [this]() { m_scriptConsole->clearConsole(); };
            callbacks.printMsgFn =
                [this](const ScriptMachine::MessageInfo &message) {
                    m_scriptConsole->onOutput(message);
                };
            sm.registerCallBack(ScriptMachine::Interactive, callbacks);

            callbacks.getInputFn = [this]() -> QString {
                return WingInputDialog::getText(this, tr(""), tr(""));
            };
            callbacks.clearFn = [this]() { m_bgScriptOutput->clear(); };
            callbacks.printMsgFn =
                std::bind(&MainWindow::onOutputBgScriptOutput, this,
                          std::placeholders::_1);
            sm.registerCallBack(ScriptMachine::Background, callbacks);
        } else {
            // TODO
        }

        // At this time, AngelScript service plugin has started
        if (splash)
            splash->setInfoText(tr("SetupConsole"));

        m_scriptConsole->init();
        if (splash)
            splash->setInfoText(tr("SetupScriptManager"));

        if (splash)
            splash->setInfoText(tr("SetupScriptService"));

        m_scriptConsole->initOutput();
        m_scriptConsole->setMode(QConsoleWidget::Input);

        if (splash)
            splash->setInfoText(tr("SetupScriptEditor"));
        m_scriptDialog = new ScriptingDialog(this);
        m_scriptDialog->initConsole();
    }

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
    if (splash)
        splash->setInfoText(tr("SetupSetDialog"));
    buildUpSettingDialog();

    // we start to installing plugin widgets
    if (splash)
        splash->setInfoText(tr("SetupPlgWidgets"));
    installPluginEditorWidgets();
    auto plgview = m_toolBtneditors.value(PLUGIN_VIEWS);
    plgview->setEnabled(!plgview->menu()->isEmpty());

    finishBuildDockSystem();

    // load saved docking layout
    if (splash)
        splash->setInfoText(tr("SetupDockingLayout"));

    _defaultLayout = m_dock->saveState();

    m_leftViewArea = nullptr;
    m_rightViewArea = nullptr;
    m_topViewArea = nullptr;
    m_bottomViewArea = nullptr;
    m_dock->restoreState(set.dockLayout());

    m_lastusedpath = set.lastUsedPath();

    if (splash)
        splash->setInfoText(tr("SetupWaiting"));

    // others
    _showtxt = new ShowTextDialog(this);
    qApp->processEvents();

    // update status
    updateEditModeEnabled();

    qApp->installEventFilter(this);

    this->setUpdatesEnabled(true);

    plg.dispatchEvent(IWingPlugin::RegisteredEvent::AppReady, {});

    if (splash)
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
    m_ribbonMaps[catagories.EDIT] = buildEditPage(m_ribbon->addTab(tr("Edit")));
    qApp->processEvents();
    m_ribbonMaps[catagories.VIEW] = buildViewPage(m_ribbon->addTab(tr("View")));
    qApp->processEvents();

    auto &set = SettingManager::instance();
    if (set.scriptEnabled()) {
        m_ribbonMaps[catagories.SCRIPT] =
            buildScriptPage(m_ribbon->addTab(tr("Script")));
        qApp->processEvents();
    }

    if (set.enablePlugin()) {
        m_ribbonMaps[catagories.PLUGIN] =
            buildPluginPage(m_ribbon->addTab(tr("Plugin")));
        qApp->processEvents();
    }

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

    // check update if set
    if (set.checkUpdate()) {
        ExecAsync<int>(
            []() -> int {
                bool ok = false;
                auto newest = WingUpdater::checkUpdate(&ok);
                if (ok) {
                    return newest ? 1 : 0;
                } else {
                    return -1;
                }
            },
            [this](int status) {
                if (status == 0) {
                    WingMessageBox::warning(this, qAppName(),
                                            tr("OlderVersion"));
                }
            });
    }
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

    ads::CDockComponentsFactory::setFactory(new DockComponentsFactory);

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
    connect(m_dock, &CDockManager::stateRestored, this, [this]() {
        // no allowing floating widgets
        QList<CDockWidget *> _dws;
        for (auto &dw : m_dock->dockWidgetsMap()) {
            if (dw->dockAreaWidget() == nullptr) {
                _dws.append(dw);
            }
        }

        if (_dws.isEmpty()) {
            return;
        }

        auto replaceDW = [this](CDockWidget *dw,
                                CDockAreaWidget *darea) -> CDockAreaWidget * {
            m_dock->blockSignals(true);
            m_dock->removeDockWidget(dw);

            auto area =
                m_dock->addDockWidget(ads::BottomDockWidgetArea, dw, darea);
            m_dock->blockSignals(false);
            dw->toggleView(false);
            return area;
        };

        auto darea = m_dock->centralWidget()->dockAreaWidget();
        auto dw = _dws.takeFirst();
        darea = replaceDW(dw, darea);
        for (auto &dw : _dws) {
            replaceDW(dw, darea);
        }
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
        m_dock->createDockWidget(QStringLiteral("CentralWidget"));
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

    ads::CDockAreaWidget *bottomRightArea;
    if (SettingManager::instance().scriptEnabled()) {
        bottomRightArea = buildUpScriptConsoleDock(
            m_dock, ads::RightDockWidgetArea, bottomLeftArea);
        qApp->processEvents();
        buildUpScriptBgOutputDock(m_dock, ads::CenterDockWidgetArea,
                                  bottomRightArea);
        qApp->processEvents();
        buildUpHashResultDock(m_dock, ads::CenterDockWidgetArea,
                              bottomRightArea);
        qApp->processEvents();
    } else {
        bottomRightArea = buildUpHashResultDock(
            m_dock, ads::RightDockWidgetArea, bottomLeftArea);
        qApp->processEvents();
    }

    qApp->processEvents();

    m_bottomViewArea = bottomRightArea;
}

void MainWindow::finishBuildDockSystem() {
    // add empty action to ext
    auto menu = m_toolBtneditors[MainWindow::ToolButtonIndex::OPEN_EXT]->menu();
    if (menu->isEmpty()) {
        auto a = newAction(tr("NoExtension"), EMPTY_FUNC);
        a->setEnabled(false);
        menu->addAction(a);
    }

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
        Qt::ContextMenuPolicy::CustomContextMenu);

    auto se = [=]() {
        auto model = qobject_cast<FindResultModel *>(m_findresult->model());
        if (model) {
            m_find->setWindowTitle(tr("FindResult") + QStringLiteral(" (") +
                                   model->encoding() + QStringLiteral(")"));
        }
    };

    auto menu = new QMenu(tr("Encoding"), this);
    menu->setIcon(ICONRES(QStringLiteral("encoding")));
    auto aGroup = new QActionGroup(this);
    auto langs = Utilities::getEncodings();
    for (auto &l : langs) {
        auto a = newCheckableAction(menu, l, [=]() {
            auto model = qobject_cast<FindResultModel *>(m_findresult->model());
            if (model) {
                model->setEncoding(l);
                se();
            }
        });
        aGroup->addAction(a);
        menu->addAction(a);
        m_findEncoding.insert(l, a);
    }

    m_menuFind = new QMenu(m_findresult);
    m_menuFind->addMenu(menu);
    m_menuFind->addAction(
        newAction(QStringLiteral("copy"), tr("Copy"), [this]() {
            auto idx = m_findresult->currentIndex();
            if (idx.isValid()) {
                auto model =
                    qobject_cast<FindResultModel *>(m_findresult->model());
                if (model) {
                    auto content = model->copyContent(idx);
                    qApp->clipboard()->setText(content);
                    Toast::toast(this, NAMEICONRES(QStringLiteral("copy")),
                                 tr("CopyToClipBoard"));
                }
            }
        }));
    m_menuFind->addAction(newAction(QStringLiteral("export"),
                                    tr("ExportFindResult"),
                                    &MainWindow::on_exportfindresult));
    m_menuFind->addAction(newAction(QStringLiteral("del"),
                                    tr("ClearFindResult"),
                                    &MainWindow::on_clearfindresult));

    connect(m_findresult, &QTableViewExt::customContextMenuRequested, this,
            [=](const QPoint &pos) {
                m_menuFind->popup(m_findresult->viewport()->mapToGlobal(pos));
            });

    m_findresult->setItemDelegate(new RichTextItemDelegate(m_findresult));

    m_findresult->setModel(_findEmptyResult);
    m_findEncoding.value(_findEmptyResult->encoding())->setChecked(true);

    connect(m_findresult, &QTableView::doubleClicked, this,
            [=](const QModelIndex &index) {
                auto editor =
                    m_findresult->property("EditorView").value<EditorView *>();
                auto hexeditor = currentEditor();
                if (hexeditor != editor) {
                    editor->raise();
                    editor->setFocus();
                }

                auto e = editor->hexEditor();
                auto fm = editor->findResultModel();
                auto cursor = e->cursor();

                cursor->moveTo(fm->resultAt(index.row()).offset);
                if (cursor->selectionCount() <= 1 && index.column() >= 3) {
                    cursor->select(fm->lastFindData().length());
                }
            });

    auto dw = buildDockWidget(dock, QStringLiteral("FindResult"),
                              tr("FindResult") + QStringLiteral(" (ASCII)"),
                              m_findresult);
    m_find = dw;
    connect(m_findresult, &QTableViewExt::modelChanged, this, se);

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
    a->setIcon(ICONRES("copy"));
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
    auto aLittleEndian = new QAction(actionGroup);
    aLittleEndian->setText(tr("LittleEndian"));
    aLittleEndian->setCheckable(true);
    aLittleEndian->setChecked(le);
    connect(aLittleEndian, &QAction::triggered, this, [=] {
        m_islittle = true;
        this->on_locChanged();
    });
    m_numshowtable->addAction(aLittleEndian);

    auto aBigEndian = new QAction(actionGroup);
    aBigEndian->setText(tr("BigEndian"));
    aBigEndian->setCheckable(true);
    aBigEndian->setChecked(!le);
    connect(aBigEndian, &QAction::triggered, this, [=] {
        m_islittle = false;
        this->on_locChanged();
    });
    m_numshowtable->addAction(aBigEndian);

    a = new QAction(this);
    a->setSeparator(true);
    m_numshowtable->addAction(a);

    auto aUnsignedHex = new QAction(this);
    aUnsignedHex->setText(tr("UnsignedHex"));
    aUnsignedHex->setCheckable(true);
    aUnsignedHex->setChecked(false);
    connect(aUnsignedHex, &QAction::toggled, this, [=](bool b) {
        m_unsignedHex = b;
        this->on_locChanged();
    });
    m_numshowtable->addAction(aUnsignedHex);

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
    m_bookmarks->setSelectionMode(QTableViewExt::ExtendedSelection);

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

    m_bookmarks->addAction(
        newAction(QStringLiteral("export"), tr("ExportResult"), [this]() {
            auto model = m_bookmarks->model();
            saveTableContent(model);
        }));

    auto sep = new QAction(m_bookmarks);
    sep->setSeparator(true);
    m_bookmarks->addAction(sep);

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

        m_bookmarks->clearSelection();
        doc->RemoveBookMarks(pos);
        emit model->layoutChanged();
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
    m_metadatas->setSelectionMode(QTableViewExt::ExtendedSelection);

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

    m_metadatas->addAction(
        newAction(QStringLiteral("export"), tr("ExportResult"), [this]() {
            auto model = m_metadatas->model();
            saveTableContent(model);
        }));

    auto sep = new QAction(m_metadatas);
    sep->setSeparator(true);
    m_metadatas->addAction(sep);

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

        m_metadatas->clearSelection();
        hexeditor->document()->metadata()->RemoveMetadatas(pmetas);
        emit m_metadatas->model()->layoutChanged();
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
    m_txtDecode->setProperty("__NAME__", tr("DecodeText"));
    auto dw = buildDockWidget(dock, QStringLiteral("DecodeText"),
                              tr("DecodeText") + QStringLiteral(" (ASCII)"),
                              m_txtDecode);

    auto menu = m_txtDecode->createStandardContextMenu();
    menu->addSeparator();
    auto a = new QAction(tr("Encoding"), this);
    a->setIcon(ICONRES(QStringLiteral("encoding")));
    connect(a, &QAction::triggered, this, &MainWindow::on_encoding);
    menu->addAction(a);

    m_txtDecode->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_txtDecode, &QTextBrowser::customContextMenuRequested, this,
            [=](const QPoint &pos) {
                menu->popup(m_txtDecode->viewport()->mapToGlobal(pos));
            });

    connect(m_txtDecode, &QTextBrowser::windowTitleChanged, dw,
            &QDockWidget::setWindowTitle);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpScriptConsoleDock(ads::CDockManager *dock,
                                     ads::DockWidgetArea area,
                                     ads::CDockAreaWidget *areaw) {
    m_scriptConsole = new ScriptingConsole(this);

    connect(m_scriptConsole, &ScriptingConsole::consoleCommand, this,
            [this] { showStatus({}); });
    connect(m_scriptConsole, &ScriptingConsole::onFunctionTip, this,
            [this](const QString &content) {
                showStatus(QStringLiteral("<b><font color=\"gold\">") +
                           content + QStringLiteral("</font></b>"));
            });

    auto dw = buildDockWidget(dock, QStringLiteral("ScriptConsole"),
                              tr("ScriptConsole"), m_scriptConsole);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpScriptBgOutputDock(ads::CDockManager *dock,
                                      ads::DockWidgetArea area,
                                      ads::CDockAreaWidget *areaw) {
    m_bgScriptOutput = new QPlainTextEdit(this);
    m_bgScriptOutput->setReadOnly(true);

    auto dw = buildDockWidget(dock, QStringLiteral("BgScriptOutput"),
                              tr("BgScriptOutput"), m_bgScriptOutput);
    return dock->addDockWidget(area, dw, areaw);
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
            pannel, QStringLiteral("workspace"), tr("OpenWorkSpace"),
            &MainWindow::on_openworkspace,
            shortcuts.keySequence(QKeySequences::Key::OPEN_WORKSPACE));

        auto menu = new QMenu(this);
        m_toolBtneditors.insert(
            ToolButtonIndex::OPEN_EXT,
            addPannelAction(pannel, QStringLiteral("openother"), tr("OpenExt"),
                            EMPTY_FUNC, {}, menu));

        addPannelAction(pannel, QStringLiteral("recent"), tr("RecentFiles"),
                        EMPTY_FUNC, {}, m_recentMenu);

        auto a = addPannelAction(pannel, QStringLiteral("reload"), tr("Reload"),
                                 &MainWindow::on_reload);
        m_editStateWidgets << a;
    }

    {
        auto pannel = tab->addGroup(tr("Save"));

        auto a = addPannelAction(pannel, QStringLiteral("save"), tr("Save"),
                                 &MainWindow::on_save, QKeySequence::Save);
        m_editStateWidgets << a;

        a = addPannelAction(pannel, QStringLiteral("saveas"), tr("SaveAs"),
                            &MainWindow::on_saveas,
                            shortcuts.keySequence(QKeySequences::Key::SAVE_AS));
        m_driverStateWidgets << a;
        m_editStateWidgets << a;

        a = addPannelAction(pannel, QStringLiteral("export"), tr("Export"),
                            &MainWindow::on_exportfile,
                            shortcuts.keySequence(QKeySequences::Key::EXPORT));
        m_editStateWidgets << a;

        a = addPannelAction(pannel, QStringLiteral("convpro"), tr("ConvertWS"),
                            &MainWindow::on_convpro);
        m_editStateWidgets << a;

        a = addPannelAction(pannel, QStringLiteral("savesel"), tr("SaveSel"),
                            &MainWindow::on_savesel);
        m_editStateWidgets << a;
    }

    return tab;
}

RibbonTabContent *MainWindow::buildEditPage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();
    {
        auto pannel = tab->addGroup(tr("General"));

        auto a = addPannelAction(pannel, QStringLiteral("undo"), tr("Undo"),
                                 &MainWindow::on_undofile, QKeySequence::Undo);
        m_toolBtneditors.insert(ToolButtonIndex::UNDO_ACTION, a);
        m_editStateWidgets << a;

        a = addPannelAction(pannel, QStringLiteral("redo"), tr("Redo"),
                            &MainWindow::on_redofile,
                            shortcuts.keySequence(QKeySequences::Key::REDO));
        m_toolBtneditors.insert(ToolButtonIndex::REDO_ACTION, a);
        m_editStateWidgets << a;

        a = addPannelAction(pannel, QStringLiteral("cut"), tr("Cut"),
                            &MainWindow::on_cutfile, QKeySequence::Cut);
        m_editStateWidgets << a;
        a = addPannelAction(pannel, QStringLiteral("copy"), tr("Copy"),
                            &MainWindow::on_copyfile, QKeySequence::Copy);
        m_editStateWidgets << a;
        a = addPannelAction(pannel, QStringLiteral("paste"), tr("Paste"),
                            &MainWindow::on_pastefile, QKeySequence::Paste);
        m_editStateWidgets << a;
        a = addPannelAction(pannel, QStringLiteral("del"), tr("Delete"),
                            &MainWindow::on_delete, QKeySequence::Delete);
        m_editStateWidgets << a;
        a = addPannelAction(pannel, QStringLiteral("clone"), tr("Clone"),
                            &MainWindow::on_clone);
        m_editStateWidgets << a;
    }

    {
        auto pannel = tab->addGroup(tr("Lookup"));
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("find"), tr("Find"),
            &MainWindow::on_findfile, QKeySequence::Find);

        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("jmp"), tr("Goto"), &MainWindow::on_gotoline,
            shortcuts.keySequence(QKeySequences::Key::GOTO));

        m_editStateWidgets << addPannelAction(pannel, QStringLiteral("sum"),
                                              tr("CheckSum"),
                                              &MainWindow::on_checksum);
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("info"), tr("FileInfo"),
            &MainWindow::on_fileInfo,
            shortcuts.keySequence(QKeySequences::Key::FILE_INFO));
    }

    {
        auto pannel = tab->addGroup(tr("Hex"));
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("cuthex"), tr("CutHex"),
            &MainWindow::on_cuthex,
            shortcuts.keySequence(QKeySequences::Key::CUT_HEX));

        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("copyhex"), tr("CopyHex"),
            &MainWindow::on_copyhex,
            shortcuts.keySequence(QKeySequences::Key::COPY_HEX));
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("pastehex"), tr("PasteHex"),
            &MainWindow::on_pastehex,
            shortcuts.keySequence(QKeySequences::Key::PASTE_HEX));
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("fill"), tr("Fill"), &MainWindow::on_fill,
            shortcuts.keySequence(QKeySequences::Key::HEX_FILL));
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("fillZero"), tr("FillZero"),
            &MainWindow::on_fillzero,
            shortcuts.keySequence(QKeySequences::Key::HEX_FILL0));
    }

    {
        auto pannel = tab->addGroup(tr("MetaData"));
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("bookmark"), tr("BookMark"),
            &MainWindow::on_bookmark,
            shortcuts.keySequence(QKeySequences::Key::BOOKMARK));
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("bookmarkdel"), tr("DeleteBookMark"),
            &MainWindow::on_bookmarkdel,
            shortcuts.keySequence(QKeySequences::Key::BOOKMARK_DEL));
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("bookmarkcls"), tr("ClearBookMark"),
            &MainWindow::on_bookmarkcls,
            shortcuts.keySequence(QKeySequences::Key::BOOKMARK_CLS));
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("metadata"), tr("MetaData"),
            &MainWindow::on_metadata,
            shortcuts.keySequence(QKeySequences::Key::METADATA));
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("metadataedit"), tr("MetaDataEdit"),
            &MainWindow::on_metadataedit,
            shortcuts.keySequence(QKeySequences::Key::METADATA_EDIT));
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("metadatadel"), tr("DeleteMetaData"),
            &MainWindow::on_metadatadel,
            shortcuts.keySequence(QKeySequences::Key::METADATA_DEL));
        m_editStateWidgets << addPannelAction(
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
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("scale"), tr("Scale"), EMPTY_FUNC, {}, menu);
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("scalereset"), tr("ResetScale"),
            [this] { this->setCurrentHexEditorScale(1.0); });
        m_editStateWidgets << addPannelAction(pannel, QStringLiteral("viewtxt"),
                                              tr("ViewText"),
                                              &MainWindow::on_viewtxt);
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

        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("metadata"), tr("MetaData"), EMPTY_FUNC, {},
            menu);

        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("metashow"), tr("MetaShowAll"),
            &MainWindow::on_metashowall,
            shortcuts.keySequence(QKeySequences::Key::METADATA_SHOW));
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("metahide"), tr("MetaHideAll"),
            &MainWindow::on_metahideall,
            shortcuts.keySequence(QKeySequences::Key::METADATA_HIDE));
    }

    {
        auto pannel = tab->addGroup(tr("FileStatus"));
        auto disableStyle =
            QStringLiteral("border:none;background:transparent;");
        m_iSaved =
            addPannelAction(pannel, _infoSaved, tr("InfoSave"), EMPTY_FUNC);
        m_iSaved->setStyleSheet(disableStyle);
        m_editStateWidgets << m_iSaved;

        m_iReadWrite =
            addPannelAction(pannel, _infoReadonly, tr("ReadOnly"), EMPTY_FUNC);
        m_iReadWrite->setStyleSheet(disableStyle);
        m_editStateWidgets << m_iReadWrite;

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
        m_editStateWidgets << m_iLocked;

        m_iCanOver =
            addPannelAction(pannel, _infoCanOver, tr("SetOver"), [this]() {
                auto editor = currentEditor();
                if (editor == nullptr) {
                    return;
                }
                auto hexeditor = editor->hexEditor();
                auto doc = hexeditor->document();

                auto b = !hexeditor->isKeepSize();
                if (!hexeditor->setKeepSize(b)) {
                    Toast::toast(this, _pixCannotOver, tr("ErrUnOver"));
                }
            });
        m_editStateWidgets << m_iCanOver;
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
            editor->switchView({});
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
        m_editStateWidgets << addPannelAction(
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
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("mColInfo"), tr("SetColInfo"), [this]() {
                auto hexeditor = currentHexView();
                if (hexeditor == nullptr) {
                    return;
                }
                hexeditor->setAddressVisible(!hexeditor->addressVisible());
            });
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("mLineInfo"), tr("SetHeaderInfo"), [this]() {
                auto hexeditor = currentHexView();
                if (hexeditor == nullptr) {
                    return;
                }
                hexeditor->setHeaderVisible(!hexeditor->headerVisible());
            });
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("mStr"), tr("SetAsciiString"), [this]() {
                auto hexeditor = currentHexView();
                if (hexeditor == nullptr) {
                    return;
                }
                hexeditor->setAsciiVisible(!hexeditor->asciiVisible());
            });
    }

    {
        auto pannel = tab->addGroup(tr("Layout"));
        addPannelAction(pannel, QStringLiteral("fullscreen"), tr("Fullscreen"),
                        &MainWindow::on_fullScreen);

        auto &l = LayoutManager::instance().layouts();

        auto menu = new QMenu(this);
        menu->addAction(newAction(tr("Default"), [this]() {
            showStatus(tr("LayoutRestoring..."));
            qApp->processEvents();
            m_dock->restoreState(_defaultLayout);
            showStatus({});
        }));

        if (!l.isEmpty()) {
            menu->addSeparator();
        }

        for (auto p = l.constKeyValueBegin(); p != l.constKeyValueEnd(); ++p) {
            auto layout = p->second;
            menu->addAction(newAction(p->first, [this, layout]() {
                showStatus(tr("LayoutRestoring..."));
                qApp->processEvents();
                m_dock->restoreState(layout);
                showStatus({});
            }));
        }

        m_toolBtneditors.insert(
            ToolButtonIndex::LAYOUT_ACTION,
            addPannelAction(pannel, QStringLiteral("layout"),
                            tr("RestoreLayout"), EMPTY_FUNC, {}, menu));

        addPannelAction(pannel, QStringLiteral("layoutexport"),
                        tr("SaveLayout"), &MainWindow::on_saveLayout);
    }

    {
        auto pannel = tab->addGroup(tr("Log"));
        addPannelAction(pannel, QStringLiteral("log"), tr("ExportLog"),
                        &MainWindow::on_exportlog);
        addPannelAction(pannel, QStringLiteral("clearhis"), tr("ClearLog"),
                        &MainWindow::on_clslog);
        addPannelAction(pannel, QStringLiteral("qtloginspect"), tr("InsepctQt"),
                        &MainWindow::on_inspectQt);
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
            &MainWindow::on_settingPlugin,
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

    auto &set = SettingManager::instance();

    {
        auto pannel = tab->addGroup(tr("General"));
        addPannelAction(
            pannel, QStringLiteral("general"), tr("General"),
            &MainWindow::on_settingGeneral,
            shortcuts.keySequence(QKeySequences::Key::SETTING_GENERAL));

        if (set.scriptEnabled()) {
            addPannelAction(pannel, QStringLiteral("scriptset"),
                            tr("ScriptSetting"), &MainWindow::on_settingScript);
        }
    }

    if (set.enablePlugin()) {
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

    addPannelAction(pannel, QStringLiteral("update"), tr("CheckUpdate"),
                    &MainWindow::on_update);

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

    auto menu = m_toolBtneditors.value(EDITOR_WINS)->menu();

    decltype(m_editorViewWidgets) newEditorViewWidgets;
    for (auto p = m_editorViewWidgets.constKeyValueBegin();
         p != m_editorViewWidgets.constKeyValueEnd(); ++p) {
        decltype(newEditorViewWidgets)::mapped_type newCreatorList;
        for (auto &wctor : p->second) {
            qApp->processEvents();

            auto id = wctor->id();
            if (names.contains(id)) {
                log.critical(tr("Plugin %1 contains a duplicate ID (%2) that "
                                "is already registered by plugin %3")
                                 .arg(p->first->pluginName(), id,
                                      names.value(id)->pluginName()));
                continue;
            }

            auto a = newAction(wctor->icon(), wctor->name(), [this, id] {
                auto editor = currentEditor();
                if (editor == nullptr) {
                    return;
                }
                editor->switchView(id);
            });
            a->setProperty("__ID__", id);
            menu->addAction(a);

            names.insert(id, p->first);
            newCreatorList.append(wctor);
        }
        newEditorViewWidgets.insert(p->first, newCreatorList);
        qApp->processEvents();
    }

    m_editorViewWidgets = newEditorViewWidgets;
}

void MainWindow::showStatus(const QString &status) {
    if (status.isEmpty() && m_isfinding) {
        _status->setText(tr("Finding..."));
    } else {
        _status->setText(status);
    }
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
    editor->setFocus();
    return editor;
}

void MainWindow::on_newfile() { newfileGUI(); }

void MainWindow::on_openfile() {
    showStatus(tr("Opening..."));
    QScopeGuard g([this]() { showStatus({}); });

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

        RecentFileManager::RecentInfo info;
        info.fileName = filename;
        m_recentmanager->addRecentFile(info);
    }
}

void MainWindow::on_openworkspace() {
    showStatus(tr("WorkSpaceOpening..."));
    QScopeGuard g([this]() { showStatus({}); });

    auto filename = WingFileDialog::getOpenFileName(
        this, tr("ChooseFile"), m_lastusedpath, tr("ProjectFile (*.wingpro)"));
    if (filename.isEmpty()) {
        return;
    }

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

    showStatus({});
}

void MainWindow::on_reload() {
    showStatus(tr("Reloading..."));
    QScopeGuard g([this]() { showStatus({}); });

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
    showStatus(tr("Saving..."));
    QScopeGuard g([this]() { showStatus({}); });

    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }

    auto changedTo = editor->change2WorkSpace();
    QString ws;
    auto res = saveEditor(editor, {}, false, false, &ws);

    auto isNewFile = editor->isNewFile();
    if (isNewFile) {
        on_saveas();
        return;
    }

    if (res == ErrFile::Permission) {
        WingMessageBox::critical(this, tr("Error"), tr("FilePermission"));
        return;
    }
    if (res == ErrFile::WorkSpaceUnSaved) {
        WingMessageBox::critical(this, tr("Error"), tr("SaveWSError"));
        return;
    }

    if (changedTo) {
        RecentFileManager::RecentInfo info;
        info.fileName = ws;
        info.isWorkSpace = true;
        m_recentmanager->addRecentFile(info);
    }
    Toast::toast(this, NAMEICONRES(QStringLiteral("save")),
                 tr("SaveSuccessfully"));
}

void MainWindow::on_convpro() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }

    if (editor->isNewFile()) {
        Toast::toast(this, NAMEICONRES("convpro"), tr("SaveNewFirst"));
        return;
    }

    if (editor->isOriginWorkSpace()) {
        Toast::toast(this, NAMEICONRES("convpro"), tr("AlreadyWorkSpace"));
        return;
    }

    QString workspace;
    auto ret = saveEditor(editor, {}, false, true, &workspace);
    if (ret == ErrFile::WorkSpaceUnSaved) {
        WingMessageBox::critical(this, tr("Error"), tr("ConvWorkSpaceFailed"));
    } else if (ret == ErrFile::Success) {
        // add to history
        RecentFileManager::RecentInfo info;
        info.fileName = workspace;
        info.isWorkSpace = true;
        m_recentmanager->addRecentFile(info);
        Toast::toast(this, NAMEICONRES("convpro"), tr("ConvWorkSpaceSuccess"));
    } else {
        // should not go there
        Q_ASSERT(false);
    }
}

void MainWindow::on_saveas() {
    showStatus(tr("SavingAs..."));
    QScopeGuard g([this]() { showStatus({}); });

    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }

    QString lastpath;
    if (editor->isNewFile() || editor->isExtensionFile()) {
        lastpath = m_lastusedpath;
    } else {
        lastpath = editor->fileName();
    }

    auto filename =
        WingFileDialog::getSaveFileName(this, tr("ChooseSaveFile"), lastpath);
    if (filename.isEmpty())
        return;
    m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();

    bool isWorkspace = editor->isOriginWorkSpace();
    auto res = saveEditor(editor, filename, false, isWorkspace);

    switch (res) {
    case ErrFile::Success: {
        Toast::toast(this, NAMEICONRES(QStringLiteral("saveas")),
                     tr("SaveSuccessfully"));
        RecentFileManager::RecentInfo info;
        info.fileName = filename;
        info.isWorkSpace = isWorkspace;
        m_recentmanager->addRecentFile(info);
        break;
    }
    case ErrFile::WorkSpaceUnSaved: {
        WingMessageBox::critical(this, tr("Error"), tr("SaveWSError"));
        break;
    }
    default: {
        WingMessageBox::critical(this, tr("Error"), tr("SaveUnSuccessfully"));
        break;
    }
    }
}

void MainWindow::on_exportfile() {
    showStatus(tr("Exporting..."));
    QScopeGuard g([this]() { showStatus({}); });

    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }

    auto filename = WingFileDialog::getSaveFileName(
        this, tr("ChooseExportFile"), m_lastusedpath);
    if (filename.isEmpty())
        return;
    m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();

    bool isWorkspace = editor->isOriginWorkSpace();
    auto res = saveEditor(editor, filename, true, isWorkspace);
    switch (res) {
    case ErrFile::Success: {
        Toast::toast(this, NAMEICONRES(QStringLiteral("export")),
                     tr("ExportSuccessfully"));
        break;
    }
    default: {
        WingMessageBox::critical(this, tr("Error"), tr("ExportUnSuccessfully"));
        break;
    }
    }
}

void MainWindow::on_savesel() {
    showStatus(tr("SavingSel..."));
    QScopeGuard g([this]() { showStatus({}); });

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
        auto buffer = hexeditor->selectedBytes().join();
        qfile.write(buffer);
        qfile.close();
        Toast::toast(this, NAMEICONRES(QStringLiteral("savesel")),
                     tr("SaveSelSuccess"));
    } else {
        WingMessageBox::critical(this, tr("Error"), tr("SaveSelError"));
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
    if (hexeditor->RemoveSelection()) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("del")),
                     tr("DeleteSuccess"));
    } else {
        Toast::toast(this, NAMEICONRES(QStringLiteral("del")),
                     tr("DeleteFailed"));
    }
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

    registerClonedEditorView(hexeditor);
    m_dock->addDockWidget(ads::CenterDockWidgetArea, hexeditor,
                          editorViewArea());
}

void MainWindow::on_findfile() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    auto hexeditor = editor->hexEditor();

    static FindDialog::FindInfo info;
    info.isBigFile = editor->isBigFile();
    info.start = 0;
    info.stop = hexeditor->documentBytes();
    info.isSel = hexeditor->selectionCount() == 1;

    FindDialog fd(info, this);
    if (fd.exec()) {
        auto r = fd.getResult();
        info.isStringFind = r.isStringFind;
        info.encoding = r.encoding;
        info.str = r.str;

        showStatus(tr("Finding..."));
        ExecAsync<EditorView::FindError>(
            [this, r]() -> EditorView::FindError {
                m_isfinding = true;

                return currentEditor()->find(r);
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

                auto result =
                    qobject_cast<FindResultModel *>(m_findresult->model());
                if (result) {
                    m_findEncoding.value(result->encoding())->setChecked(true);
                }
                m_find->raise();

                m_isfinding = false;
                this->showStatus({});
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
    EncodingDialog d;
    if (d.exec()) {
        m_encoding = d.getResult();
        m_txtDecode->setWindowTitle(
            m_txtDecode->property("__NAME__").toString() +
            QStringLiteral(" (") + m_encoding + QStringLiteral(")"));
        on_selectionChanged();
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

        if (editor->hexEditor()->hasSelection()) {
            auto data = editor->hexEditor()->selectedBytes();
            for (auto &c : cs) {
                auto h = hashes.at(c);
                QCryptographicHash hash(h);
                hash.addData(data.join());
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
    FileInfoDialog d(editor->fileName());
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
    hexeditor->Paste(true);
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
        auto v = in.toUInt(&b, 0);
        auto limit = std::numeric_limits<uchar>().max();
        if (v > limit) {
            Toast::toast(this, NAMEICONRES(QStringLiteral("fill")),
                         tr("FillInputTruncWarn"));
        }
        auto ch = uchar(v);
        if (b) {
            auto doc = hexeditor->document();
            if (doc->isEmpty() || !hexeditor->hasSelection())
                return;

            auto total = hexeditor->selectionCount();
            doc->beginMarco(QStringLiteral("FillBytes"));
            for (int i = 0; i < total; ++i) {
                auto cursor = hexeditor->cursor();
                auto pos = cursor->selectionStart(i).offset();
                hexeditor->Replace(
                    pos, QByteArray(cursor->selectionLength(i), char(ch)));
            }
            doc->endMarco();
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
    if (doc->isEmpty() || !hexeditor->hasSelection())
        return;

    auto cur = hexeditor->cursor();
    auto total = cur->selectionCount();
    doc->beginMarco(QStringLiteral("FillZero"));
    for (int i = 0; i < total; ++i) {
        auto pos = cur->selectionStart(i).offset();
        hexeditor->Replace(pos, QByteArray(cur->selectionLength(i), char(0)));
    }
    doc->endMarco();
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
    auto cursor = hexeditor->cursor();
    if (cursor->hasSelection()) {
        QList<qsizetype> rmOff;

        auto total = cursor->selectionCount();
        for (int i = 0; i < total; ++i) {
            rmOff.append(doc->bookMarkRange(cursor->selectionStart(i).offset(),
                                            cursor->selectionEnd(i).offset()));
        }

        if (!rmOff.isEmpty()) {
            doc->beginMarco(QStringLiteral("BookMarkRmRange"));
            for (auto &pos : rmOff) {
                doc->RemoveBookMark(pos);
            }
            doc->endMarco();
            Toast::toast(this, NAMEICONRES("bookmarkdel"),
                         tr("BookmarkDelSuccess"));
            return;
        }
    } else {
        auto pos = cursor->position().offset();
        if (doc->bookMarkExists(pos)) {
            if (doc->RemoveBookMark(pos)) {
                Toast::toast(this, NAMEICONRES("bookmarkdel"),
                             tr("BookmarkDelSuccess"));
                return;
            }
        }
    }

    Toast::toast(this, NAMEICONRES("bookmarkdel"), tr("BookmarkDelNoItem"));
}

void MainWindow::on_bookmarkcls() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    auto doc = hexeditor->document();
    doc->ClearBookMark();
    Toast::toast(this, NAMEICONRES("bookmarkdel"), tr("BookmarkClearSuccess"));
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
        if (cur->hasSelection()) {
            if (m.exec()) {
                auto total = cur->selectionCount();
                auto meta = doc->metadata();
                meta->beginMarco(QStringLiteral("MetaData"));
                for (int i = 0; i < total; ++i) {
                    auto begin = cur->selectionStart(i).offset();
                    auto end = cur->selectionEnd(i).offset();
                    meta->Metadata(begin, end, m.foreGroundColor(),
                                   m.backGroundColor(), m.comment());
                }
                meta->endMarco();
                cur->clearSelection();
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
        if (cur->currentSelectionLength() > 0) {
            auto mc = doc->metadata()->get(cur->position().offset());

            if (mc.has_value()) {
                auto meta = mc.value();
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
    auto cursor = hexeditor->cursor();
    if (cursor->hasSelection()) {
        Toast::toast(this, NAMEICONRES("metadatadel"), tr("PleaseClearSel"));
        return;
    }
    auto pos = cursor->position().offset();
    if (meta->RemoveMetadata(pos)) {
        Toast::toast(this, NAMEICONRES("metadatadel"), tr("MetaDelSuccess"));
    } else {
        Toast::toast(this, NAMEICONRES("metadatadel"), tr("MetaDelNoItem"));
    }
}

void MainWindow::on_metadatacls() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }
    auto doc = hexeditor->document();
    doc->metadata()->Clear();
    Toast::toast(this, NAMEICONRES("metadatacls"), tr("MetaClearSuccess"));
}

void MainWindow::on_metadatafg(bool checked) {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    auto hexeditor = editor->hexEditor();
    auto doc = hexeditor->document();
    doc->setMetafgVisible(checked);
}

void MainWindow::on_metadatabg(bool checked) {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    auto hexeditor = editor->hexEditor();
    auto doc = hexeditor->document();
    doc->setMetabgVisible(checked);
}

void MainWindow::on_metadatacomment(bool checked) {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    auto hexeditor = editor->hexEditor();
    auto doc = hexeditor->document();
    doc->setMetaCommentVisible(checked);
}

void MainWindow::on_metashowall() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    auto hexeditor = editor->hexEditor();
    auto doc = hexeditor->document();
    doc->setMetafgVisible(true);
    doc->setMetabgVisible(true);
    doc->setMetaCommentVisible(true);
}

void MainWindow::on_metahideall() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    auto hexeditor = editor->hexEditor();
    auto doc = hexeditor->document();
    doc->setMetafgVisible(false);
    doc->setMetabgVisible(false);
    doc->setMetaCommentVisible(false);
}

void MainWindow::on_clearfindresult() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    editor->clearFindResult();
}

void MainWindow::on_exportfindresult() {
    showStatus(tr("FindResultExporting..."));
    QScopeGuard g([this]() { showStatus({}); });

    auto editor = currentEditor();
    if (editor == nullptr) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("export")),
                     tr("EmptyFindResult"));
        return;
    }

    auto findresitem = editor->findResultModel();

    auto c = findresitem->size();
    if (c == 0) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("export")),
                     tr("EmptyFindResult"));
        return;
    }
    auto filename = WingFileDialog::getSaveFileName(
        this, tr("ChooseSaveFile"), m_lastusedpath, {"Json (*.json)"});
    if (filename.isEmpty())
        return;
    m_lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();

    QFile f(filename);
    if (f.open(QFile::WriteOnly)) {
        QJsonObject fobj;
        fobj.insert(QStringLiteral("file"), editor->fileName());

        auto d = findresitem->lastFindData();

        fobj.insert(QStringLiteral("find"), d);
        QJsonArray arr;
        for (int i = 0; i < c; i++) {
            auto data = findresitem->resultAt(i);
            QJsonObject jobj;
            jobj.insert(QStringLiteral("line"), QString::number(data.line));
            jobj.insert(QStringLiteral("col"), QString::number(data.col));
            jobj.insert(QStringLiteral("offset"), QString::number(data.offset));

            QTextDocument doc;
            doc.setHtml(
                findresitem->data(findresitem->index(i, 3), Qt::DisplayRole)
                    .toString());
            jobj.insert(QStringLiteral("range"), doc.toPlainText());

            doc.setHtml(
                findresitem->data(findresitem->index(i, 4), Qt::DisplayRole)
                    .toString());
            jobj.insert(QStringLiteral("encoding"), doc.toPlainText());

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
    auto sellen = hexeditor->currentSelectionLength();
    m_lblsellen->setText(QStringLiteral("%1 - 0x%2")
                             .arg(sellen)
                             .arg(QString::number(sellen, 16).toUpper()));

    // number analyse
    auto off = hexeditor->currentOffset();
    auto d = hexeditor->document();

    constexpr auto maxlen = 32;

    auto tmp = d->read(off, maxlen);
    quint64 n = *reinterpret_cast<const quint64 *>(tmp.constData());

    auto len = size_t(tmp.length());

    if (len >= sizeof(quint64)) {
        auto s = processEndian(n);
        _numsitem->setNumData(
            NumShowModel::NumTableIndex::Uint64,
            m_unsignedHex
                ? QStringLiteral("0x%1").arg(QString::number(s, 16).toUpper())
                : QString::number(s));
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

    if (len >= sizeof(quint32)) {
        auto s = processEndian(quint32(n));
        _numsitem->setNumData(
            NumShowModel::NumTableIndex::Uint32,
            m_unsignedHex
                ? QStringLiteral("0x%1").arg(QString::number(s, 16).toUpper())
                : QString::number(s));
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

    if (len >= sizeof(quint16)) {
        auto s = processEndian(quint16(n));
        _numsitem->setNumData(
            NumShowModel::NumTableIndex::Ushort,
            m_unsignedHex
                ? QStringLiteral("0x%1").arg(QString::number(s, 16).toUpper())
                : QString::number(s));
        auto s1 = processEndian(qint16(n));
        _numsitem->setNumData(NumShowModel::NumTableIndex::Short,
                              QString::number(s1));
    } else {
        _numsitem->setNumData(NumShowModel::NumTableIndex::Ushort, QString());
        _numsitem->setNumData(NumShowModel::NumTableIndex::Short, QString());
    }
    if (len >= sizeof(uchar)) {
        auto s1 = tmp.at(0);
        auto s = uchar(s1);
        _numsitem->setNumData(
            NumShowModel::NumTableIndex::Byte,
            m_unsignedHex
                ? QStringLiteral("0x%1").arg(QString::number(s, 16).toUpper())
                : QString::number(s));
        _numsitem->setNumData(NumShowModel::NumTableIndex::Char,
                              QString::number(s1));
    } else {
        _numsitem->setNumData(NumShowModel::NumTableIndex::Byte, QString());
        _numsitem->setNumData(NumShowModel::NumTableIndex::Char, QString());
    }

    // str
    if (len > 0) {
        _numsitem->setNumData(NumShowModel::NumTableIndex::ASCII_STR,
                              QString::fromLatin1(tmp));
        _numsitem->setNumData(NumShowModel::NumTableIndex::UTF8_STR,
                              QString::fromUtf8(tmp));
        auto re = processEndian(tmp);
        _numsitem->setNumData(
            NumShowModel::NumTableIndex::UTF16_STR,
            QString::fromUtf16(reinterpret_cast<const char16_t *>(re.data()),
                               re.length() / sizeof(char16_t)));
        _numsitem->setNumData(
            NumShowModel::NumTableIndex::UTF32_STR,
            QString::fromUcs4(reinterpret_cast<const char32_t *>(re.data()),
                              re.length() / sizeof(char32_t)));
    } else {
        _numsitem->setNumData(NumShowModel::NumTableIndex::ASCII_STR,
                              QString());
        _numsitem->setNumData(NumShowModel::NumTableIndex::UTF8_STR, QString());
        _numsitem->setNumData(NumShowModel::NumTableIndex::UTF16_STR,
                              QString());
        _numsitem->setNumData(NumShowModel::NumTableIndex::UTF32_STR,
                              QString());
    }

    auto cursor = hexeditor->cursor();

    PluginSystem::instance().dispatchEvent(
        IWingPlugin::RegisteredEvent::CursorPositionChanged,
        {QVariant::fromValue(cursor->position())});
}

void MainWindow::on_selectionChanged() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }

    on_locChanged();

    auto cursor = hexeditor->cursor();
    QByteArrayList buffer;
    bool isPreview = false;
    if (cursor->previewSelectionMode() != QHexCursor::SelectionRemove &&
        cursor->hasPreviewSelection()) {
        buffer.append(hexeditor->previewSelectedBytes());
        isPreview = true;
    }

    if (buffer.isEmpty()) {
        if (hexeditor->selectionCount() > 0) {
            buffer = hexeditor->selectedBytes();
        }
    }

    auto total = buffer.size();
    m_txtDecode->clear();

    for (int i = 0; i < total; i++) {
        auto b = buffer.at(i);

        if (!isPreview) {
            m_txtDecode->insertHtml(
                QStringLiteral("<font color=\"gold\">[ %1 / %2 ]</font><br />")
                    .arg(i + 1)
                    .arg(total));
        }

        if (buffer.length() <= 1024 * _decstrlim) {
            m_txtDecode->insertPlainText(
                Utilities::decodingString(b, m_encoding));
            m_txtDecode->insertPlainText(QStringLiteral("\n"));
        } else {
            m_txtDecode->insertHtml(
                QStringLiteral("<font color=\"red\">%1</font><br />")
                    .arg(tr("TooManyBytesDecode")));
        }
    }

    PluginSystem::instance().dispatchEvent(
        IWingPlugin::RegisteredEvent::SelectionChanged,
        {QVariant::fromValue(buffer), isPreview});
}

void MainWindow::on_viewtxt() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    auto hexeditor = editor->hexEditor();
    auto filename = editor->fileName();
    QMimeDatabase db;
    auto mime = db.mimeTypeForFile(filename);
    auto ret = Utilities::isTextFile(mime);
    if (!ret) {
        auto ret = WingMessageBox::warning(
            this, tr("Warn"), tr("NoTextFileMayInvalid"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret == QMessageBox::No) {
            return;
        }
    }
    _showtxt->load(hexeditor->document()->buffer(), mime.name());
}

void MainWindow::on_fullScreen() {
    if (this->isFullScreen()) {
        this->showMaximized();
    } else {
        this->showFullScreen();
    }
}

void MainWindow::on_saveLayout() {
    showStatus(tr("LayoutSaving..."));
    QScopeGuard g([this]() { showStatus({}); });

    static auto suffix = QStringLiteral(".wing-layout");
    bool ok;
    auto fileID = WingInputDialog::getText(
        this, tr("SaveLayout"), tr("PleaseInput"), QLineEdit::Normal, {}, &ok);
    if (ok) {
        auto layoutDir = LayoutManager::layoutDir();
        QFile f(layoutDir.absoluteFilePath(fileID + suffix));
        if (f.open(QFile::WriteOnly)) {
            auto layout = m_dock->saveState();
            f.write(layout);
            f.close();

            // append saved layout
            auto &lm = LayoutManager::instance();
            auto menu =
                m_toolBtneditors[ToolButtonIndex::LAYOUT_ACTION]->menu();
            Q_ASSERT(menu);
            menu->addAction(
                newAction(lm.getSavedLayoutName(fileID), [this, layout]() {
                    showStatus(tr("LayoutRestoring..."));
                    qApp->processEvents();
                    m_dock->restoreState(layout);
                    showStatus({});
                }));

            Toast::toast(this, NAMEICONRES(QStringLiteral("layoutexport")),
                         tr("SaveLayoutSuccess"));
        } else {
            Toast::toast(this, NAMEICONRES(QStringLiteral("layoutexport")),
                         tr("SaveLayoutError"));
        }
    }
}

void MainWindow::on_exportlog() {
    showStatus(tr("LogExporting..."));
    QScopeGuard g([this]() { showStatus({}); });
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

void MainWindow::on_inspectQt() {
    InspectQtLogHelper::instance().showLogWidget();
}

void MainWindow::on_scriptwindow() {
    Q_ASSERT(m_scriptDialog);
    m_scriptDialog->show();
    m_scriptDialog->raise();
}

void MainWindow::on_settingGeneral() { m_setdialog->showConfig(0); }

void MainWindow::on_settingScript() {
    m_scriptDialog->settingDialog()->showConfig();
}

void MainWindow::on_settingPlugin() { m_setdialog->showConfig(2); }

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

void MainWindow::on_update() {
    showStatus(tr("CheckingUpdate"));
    ExecAsync<int>(
        []() -> int {
            bool ok = false;
            auto newest = WingUpdater::checkUpdate(&ok);
            if (ok) {
                return newest ? 1 : 0;
            } else {
                return -1;
            }
        },
        [this](int status) {
            if (status < 0) {
                WingMessageBox::critical(this, qAppName(), tr("BadNetwork"));
            } else if (status == 0) {
                WingMessageBox::warning(this, qAppName(), tr("OlderVersion"));
            } else {
                WingMessageBox::information(this, qAppName(),
                                            tr("NewestVersion"));
            }
            this->showStatus({});
        });
}

QString MainWindow::saveLog() {
    QDir ndir(Utilities::getAppDataPath());
    ndir.mkpath(QStringLiteral("log"));
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
    auto dw = dock->createDockWidget(displayName, dock);
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

void MainWindow::registerEditorView(EditorView *editor, const QString &ws) {
    for (auto p = m_editorViewWidgets.constKeyValueBegin();
         p != m_editorViewWidgets.constKeyValueEnd(); p++) {
        for (auto &w : p->second) {
            auto v = w->create(editor);
            auto id = w->id();
            editor->registerView(id, v);
            connect(v, &WingEditorViewWidget::savedStateChanged, this,
                    [editor](bool saved) {
                        editor->hexEditor()->document()->setDocSaved(saved);
                    });
        }
    }
    for (auto &m : m_hexContextMenu) {
        editor->registerQMenu(m);
    }

    auto &set = SettingManager::instance();
    editor->setCopyLimit(set.copylimit());
    editor->setFontSize(set.editorfontSize());

    connectEditorView(editor);
    connect(editor, &EditorView::closeRequested, this, [this] {
        auto editor = qobject_cast<EditorView *>(sender());
        Q_ASSERT(editor);
        Q_ASSERT(m_views.contains(editor));

        // the plugin view should explain itself why preventing closing
        if (!editor->processWingEditorViewClosing()) {
            return;
        }

        if (editor->hasCloneChildren()) {
            auto ret =
                WingMessageBox::question(this, tr("Warn"), tr("HasClonedView"));
            if (ret == QMessageBox::Yes) {
                editor->closeAllClonedChildren();
            } else {
                return;
            }
        }

        auto ret = closeEditor(editor, false);
        if (ret == ErrFile::UnSaved || ret == ErrFile::WorkSpaceUnSaved) {
            auto ret = this->saveRequest();
            if (ret == QMessageBox::Cancel) {
                return;
            } else if (ret == QMessageBox::Yes) {
                auto ret = saveEditor(editor, {});
                switch (ret) {
                case WingHex::Success:
                    // ok, no need to report
                    closeEditor(editor, false);
                    break;
                case WingHex::Permission: {
                    auto btn = WingMessageBox::critical(
                        this, tr("Error"), tr("FilePermissionSure2Quit"),
                        QMessageBox::Yes | QMessageBox::No);
                    if (btn == QMessageBox::Yes) {
                        closeEditor(editor, true);
                    }
                } break;
                case WingHex::Error:
                case WingHex::UnSaved:
                case WingHex::NotExist:
                case WingHex::AlreadyOpened:
                case WingHex::IsNewFile:
                case WingHex::ClonedFile:
                case WingHex::InvalidFormat:
                case WingHex::TooManyOpenedFile:
                case WingHex::DevNotFound:
                case WingHex::NotAllowedInNoneGUIThread: {
                    // unknown error
                    auto btn = WingMessageBox::critical(
                        this, tr("Error"), tr("UnknownErrorSure2Quit"),
                        QMessageBox::Yes | QMessageBox::No);
                    if (btn == QMessageBox::Yes) {
                        closeEditor(editor, true);
                    }
                } break;
                case WingHex::WorkSpaceUnSaved: {
                    auto btn = WingMessageBox::critical(
                        this, tr("Error"), tr("WorkSpaceUnSavedSure2Quit"),
                        QMessageBox::Yes | QMessageBox::No);
                    if (btn == QMessageBox::Yes) {
                        closeEditor(editor, true);
                    }
                } break;
                    break;
                }
            } else {
                closeEditor(editor, true);
            }
        }

        if (m_views.isEmpty()) {
            updateEditModeEnabled();
        }
    });

    m_views.insert(editor, ws);
    auto ev = m_toolBtneditors.value(ToolButtonIndex::EDITOR_VIEWS);
    auto menu = ev->menu();
    Q_ASSERT(menu);
    auto ta = editor->toggleViewAction();
    menu->addAction(ta);
    ev->setEnabled(true);

    PluginSystem::instance().dispatchEvent(
        IWingPlugin::RegisteredEvent::FileOpened,
        {editor->fileName(),
         QVariant::fromValue(getEditorViewFileType(editor))});
}

void MainWindow::registerClonedEditorView(EditorView *editor) {
    for (auto &m : m_hexContextMenu) {
        editor->registerQMenu(m);
    }

    auto ev = m_toolBtneditors.value(ToolButtonIndex::EDITOR_VIEWS);
    auto menu = ev->menu();
    Q_ASSERT(menu);
    auto ta = editor->toggleViewAction();
    menu->addAction(ta);
    ev->setEnabled(true);

    connect(editor, &EditorView::closeRequested, this, [=]() {
        adjustEditorFocus(editor);
        editor->closeDockWidget();
    });
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
            if (cur->hasSelection()) {
                auto total = hexeditor->selectionCount();
                if (m.exec()) {
                    auto meta = doc->metadata();
                    meta->beginMarco(QStringLiteral("OnMetaData"));
                    for (int i = 0; i < total; ++i) {
                        auto begin = cur->selectionStart(i).offset();
                        auto end = cur->selectionEnd(i).offset();
                        meta->Metadata(begin, end, m.foreGroundColor(),
                                       m.backGroundColor(), m.comment());
                    }
                    meta->endMarco();
                    cur->clearSelection();
                }
            } else {
                auto md = doc->metadata()->get(cur->position().offset());
                if (md.has_value()) {
                    auto meta = md.value();
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
                        mi->ModifyMetadata(o, meta);
                    }
                } else {
                    Toast::toast(this, NAMEICONRES(QStringLiteral("metadata")),
                                 tr("NoSelection"));
                }
            }
        }
    });
    connect(editor, &EditorView::sigOnPasteHex, this, &MainWindow::on_pastehex);
    connect(editor, &EditorView::sigOnPasteFile, this,
            &MainWindow::on_pastefile);
}

void MainWindow::swapEditor(EditorView *old, EditorView *cur) {
    if (old == cur) {
        return;
    }

    if (old != nullptr) {
        auto hexeditor = old->hexEditor();
        hexeditor->disconnect(SIGNAL(cursorLocationChanged()), this);
        hexeditor->disconnect(SIGNAL(cursorSelectionChanged()), this);
        hexeditor->disconnect(SIGNAL(canUndoChanged(bool)), this);
        hexeditor->disconnect(SIGNAL(canRedoChanged(bool)), this);
        hexeditor->disconnect(SIGNAL(documentSaved(bool)), this);
        hexeditor->disconnect(SIGNAL(documentKeepSize(bool)), this);
        hexeditor->disconnect(SIGNAL(documentLockedFile(bool)), this);
        hexeditor->disconnect(SIGNAL(copyLimitRaised()), this);
        hexeditor->disconnect(SIGNAL(scaleRateChanged()), this);
        auto doc = hexeditor->document();
        doc->disconnect(m_aShowMetabg);
        doc->disconnect(m_aShowMetafg);
        doc->disconnect(m_aShowMetaComment);
    }

    Q_ASSERT(cur);
    auto hexeditor = cur->hexEditor();
    connect(hexeditor, &QHexView::cursorLocationChanged, this,
            &MainWindow::on_locChanged);
    connect(hexeditor, &QHexView::cursorSelectionChanged, this,
            &MainWindow::on_selectionChanged);
    connect(hexeditor, &QHexView::canUndoChanged, this, [this](bool b) {
        m_toolBtneditors[ToolButtonIndex::UNDO_ACTION]->setEnabled(b);
    });
    connect(hexeditor, &QHexView::canRedoChanged, this, [this](bool b) {
        m_toolBtneditors[ToolButtonIndex::REDO_ACTION]->setEnabled(b);
    });
    connect(hexeditor, &QHexView::documentSaved, this, [this](bool b) {
        m_iSaved->setIcon(b ? _infoSaved : _infoUnsaved);
        m_sSaved->setIcon(b ? _infoSaved : _infoUnsaved);
    });
    connect(hexeditor, &QHexView::documentKeepSize, this, [this](bool b) {
        m_iCanOver->setIcon(b ? _infoCannotOver : _infoCanOver);
        m_sCanOver->setIcon(b ? _infoCannotOver : _infoCanOver);
    });
    connect(hexeditor, &QHexView::documentLockedFile, this, [this](bool b) {
        m_iLocked->setIcon(b ? _infoLock : _infoUnLock);
        m_sLocked->setIcon(b ? _infoLock : _infoUnLock);
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

    auto menu = m_toolBtneditors.value(EDITOR_WINS)->menu();
    for (auto &a : menu->actions()) {
        auto id = a->property("__ID__").toString();
        if (id.isEmpty()) {
            continue;
        }
        a->setEnabled(cur->isWingEditorViewEnabled(id));
    }

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

    auto icon = hexeditor->isReadOnly() ? _infoReadonly : _infoWriteable;
    m_iReadWrite->setIcon(icon);
    m_sReadWrite->setIcon(icon);

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

    PluginSystem::instance().dispatchEvent(
        IWingPlugin::RegisteredEvent::FileSwitched,
        {cur->fileName(), (old ? old->fileName() : QString())});
}

void MainWindow::loadFindResult(EditorView *view) {
    auto result = view->findResultModel();
    m_findresult->setModel(result);
    m_findEncoding.value(result->encoding())->setChecked(true);
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
    if (e) {
        if (editor) {
            *editor = e;
        }
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

    registerEditorView(ev);
    if (editor) {
        *editor = ev;
    }
    m_dock->addDockWidget(ads::CenterDockWidgetArea, ev, editorViewArea());
    ev->setFocus();
    return ErrFile::Success;
}

ErrFile MainWindow::openExtFile(const QString &ext, const QString &file,
                                EditorView **editor) {
    auto e = findEditorView(EditorView::getDeviceFileName(ext, file));
    if (e) {
        if (editor) {
            *editor = e;
        }
        return ErrFile::AlreadyOpened;
    }

    auto ev = new EditorView(this);
    auto res = ev->openExtFile(ext, file);

    if (res != ErrFile::Success) {
        delete ev;
        return res;
    }

    registerEditorView(ev);
    if (editor) {
        *editor = ev;
    }

    m_dock->addDockWidget(ads::CenterDockWidgetArea, ev, editorViewArea());
    ev->setFocus();
    return ErrFile::Success;
}

ErrFile MainWindow::openWorkSpace(const QString &file, EditorView **editor) {
    // different from other common files
    for (auto p = m_views.constKeyValueBegin(); p != m_views.constKeyValueEnd();
         ++p) {
#ifdef Q_OS_WIN
        if (p->second.compare(file, Qt::CaseInsensitive) == 0) {
#else
        if (p->second == file) {
#endif
            if (editor) {
                *editor = p->first;
            }
            return ErrFile::AlreadyOpened;
        }
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

    registerEditorView(ev, file);
    if (editor) {
        *editor = ev;
    }
    m_dock->addDockWidget(ads::CenterDockWidgetArea, ev, editorViewArea());
    ev->setFocus();
    return ErrFile::Success;
}

ErrFile MainWindow::saveEditor(EditorView *editor, const QString &filename,
                               bool isExport, bool forceWorkspace,
                               QString *ws) {
    if (editor == nullptr) {
        return ErrFile::Error;
    }

    auto isNewFile = editor->isNewFile();
    if (isNewFile && filename.isEmpty()) {
        return ErrFile::IsNewFile;
    }

    auto oldName = editor->fileName();
    auto newName = filename.isEmpty() ? oldName : filename;

    QString workspace = m_views.value(editor);
    if (forceWorkspace || workspace.isEmpty()) {
        if (forceWorkspace || editor->change2WorkSpace()) {
            QString curFile;
            if (!editor->isExtensionFile()) {
                curFile = newName + PROEXT;
            }

            auto wsfile = getWorkSpaceFileName(curFile);
            if (wsfile.isEmpty()) {
                return ErrFile::WorkSpaceUnSaved;
            }
            workspace = wsfile;
        }
    }

    if (ws) {
        *ws = workspace;
    }

    auto ret = editor->save(workspace, filename, isExport,
                            forceWorkspace
                                ? EditorView::SaveWorkSpaceAttr::ForceWorkSpace
                                : EditorView::SaveWorkSpaceAttr::AutoWorkSpace);
    if (ret == ErrFile::Success) {
        m_views[editor] = workspace;

        PluginSystem::instance().dispatchEvent(
            IWingPlugin::RegisteredEvent::FileSaved,
            {newName, oldName, isExport,
             QVariant::fromValue(getEditorViewFileType(editor))});
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

    auto cret = editor->closeFile();
    if (cret != ErrFile::Success) {
        return cret;
    }

    m_views.remove(editor);
    if (currentEditor() == editor) {
        _editorLock.lockForWrite();
        m_curEditor = nullptr;
        _editorLock.unlock();
    }

    auto fileName = editor->fileName();
    auto &plgsys = PluginSystem::instance();
    plgsys.cleanUpEditorViewHandle(editor);
    plgsys.dispatchEvent(
        IWingPlugin::RegisteredEvent::FileClosed,
        {fileName, QVariant::fromValue(getEditorViewFileType(editor))});
    editor->closeDockWidget();

    m_toolBtneditors.value(ToolButtonIndex::EDITOR_VIEWS)
        ->setEnabled(m_views.size() != 0);

    adjustEditorFocus(editor);
    return ErrFile::Success;
}

IWingPlugin::FileType MainWindow::getEditorViewFileType(EditorView *view) {
    Q_ASSERT(view);

    switch (view->documentType()) {
    case EditorView::DocumentType::File:
        return IWingPlugin::FileType::File;
    case EditorView::DocumentType::Extension:
        return IWingPlugin::FileType::Extension;
    case EditorView::DocumentType::Cloned:
        return getEditorViewFileType(view->cloneParent());
        break;
    default:
        Q_ASSERT(false);
        break;
    }

    return IWingPlugin::FileType::Invalid;
}

void MainWindow::updateEditModeEnabled() {
    auto editor = currentEditor();
    auto b = (editor != nullptr);

    for (auto &item : m_editStateWidgets) {
        item->setEnabled(b);
    }

    if (b) {
        auto hexeditor = editor->hexEditor();
        auto doc = hexeditor->document();
        m_toolBtneditors[ToolButtonIndex::REDO_ACTION]->setEnabled(
            doc->canRedo());
        m_toolBtneditors[ToolButtonIndex::UNDO_ACTION]->setEnabled(
            doc->canUndo());
    } else {
        m_lblloc->setText(QStringLiteral("(0,0)"));
        m_lblsellen->setText(QStringLiteral("0 - 0x0"));
        _numsitem->clear();
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

void MainWindow::adjustEditorFocus(EditorView *closedEditor) {
    if (m_dock->focusedDockWidget() == closedEditor) {
        if (!m_views.isEmpty()) {
            for (auto p = m_views.keyBegin(); p != m_views.keyEnd(); ++p) {
                auto ev = *p;
                if (ev != closedEditor && ev->isCurrentTab()) {
                    ev->setFocus();
                }
            }
        }
    }
}

QString MainWindow::getWorkSpaceFileName(const QString &curFile) {
    return WingFileDialog::getSaveFileName(this, tr("SaveWorkSpace"), curFile,
                                           tr("WingHexWorkSpace (*.wingpro)"));
}

void MainWindow::saveTableContent(QAbstractItemModel *model) {
    if (!model || model->rowCount() == 0) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("save")),
                     tr("NothingToSave"));
        return;
    }

    QString selFilter;
    auto filename = WingFileDialog::getSaveFileName(
        this, tr("ChooseSaveFile"), m_lastusedpath,
        QStringLiteral("Json (*.json);;CSV (*.csv)"), &selFilter);
    if (filename.isEmpty()) {
        return;
    }

    if (selFilter.startsWith(QStringLiteral("Json"))) {
        QJsonArray tableData;

        // Add header row
        QJsonArray headers;
        for (int col = 0; col < model->columnCount(); ++col) {
            headers.append(model->headerData(col, Qt::Horizontal).toString());
        }
        tableData.append(headers);

        // Add data rows
        for (int row = 0; row < model->rowCount(); ++row) {
            QJsonArray rowData;
            for (int col = 0; col < model->columnCount(); ++col) {
                QModelIndex index = model->index(row, col);
                rowData.append(model->data(index).toString());
            }
            tableData.append(rowData);
        }

        // Create JSON document
        QJsonDocument jsonDocument(tableData);

        // Write to file
        QFile file(filename);
        if (!file.open(QFile::WriteOnly | QFile::Text)) {
            WingMessageBox::critical(this, tr("Error"), tr("FilePermission"));
            return;
        }

        file.write(jsonDocument.toJson(QJsonDocument::Indented));
        file.close();
    } else {
        QFile file(filename);
        if (!file.open(QFile::WriteOnly | QFile::Text)) {
            WingMessageBox::critical(this, tr("Error"), tr("FilePermission"));
            return;
        }

        QTextStream stream(&file);

        // Write headers
        QStringList headers;
        for (int col = 0; col < model->columnCount(); ++col) {
            auto content = model->headerData(col, Qt::Horizontal).toString();
            content.prepend('"').append('"');
            headers << content;
        }
        stream << headers.join(',') << Qt::endl;

        // Write data rows
        for (int row = 0; row < model->rowCount(); ++row) {
            QStringList rowData;
            for (int col = 0; col < model->columnCount(); ++col) {
                QModelIndex index = model->index(row, col);
                auto content = model->data(index).toString();
                content.prepend('"').append('"');
                rowData << content;
            }
            stream << rowData.join(',') << Qt::endl;
        }

        file.close();
    }

    Toast::toast(this, NAMEICONRES(QStringLiteral("save")),
                 tr("SaveSuccessfully"));
}

QHexView *MainWindow::currentHexView() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return nullptr;
    }
    return editor->hexEditor();
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

void MainWindow::onOutputBgScriptOutput(
    const ScriptMachine::MessageInfo &message) {
    static QPair<ScriptMachine::MessageType, QPair<int, int>> lastInfo{
        ScriptMachine::MessageType::Print, {-1, -1}};

    auto doc = m_bgScriptOutput->document();
    auto lastLine = doc->lastBlock();
    auto isNotBlockStart = !lastLine.text().isEmpty();

    auto cursor = m_bgScriptOutput->textCursor();
    cursor.movePosition(QTextCursor::End);

    auto fmtMsg = [](const ScriptMachine::MessageInfo &message) -> QString {
        if (message.row <= 0 || message.col <= 0) {
            return message.message;
        } else {
            return QStringLiteral("(") + QString::number(message.row) +
                   QStringLiteral(", ") + QString::number(message.col) +
                   QStringLiteral(")") + message.message;
        }
    };

    auto isMatchLast = [](const ScriptMachine::MessageInfo &message) -> bool {
        if (message.row < 0 || message.col < 0) {
            return false;
        }
        return lastInfo.first == message.type &&
               lastInfo.second.first == message.row &&
               lastInfo.second.second == message.col;
    };

    switch (message.type) {
    case ScriptMachine::MessageType::Info:
        if (isMatchLast(message)) {
            cursor.insertText(message.message);
        } else {
            if (isNotBlockStart) {
                cursor.insertBlock();
            }
            cursor.insertText(tr("[Info]") + fmtMsg(message));
        }
        break;
    case ScriptMachine::MessageType::Warn:
        if (isMatchLast(message)) {
            auto fmt = cursor.charFormat();
            fmt.setForeground(QColorConstants::Svg::gold);
            cursor.insertText(message.message, fmt);
        } else {
            if (isNotBlockStart) {
                m_bgScriptOutput->appendPlainText({});
            }
            auto fmt = cursor.charFormat();
            fmt.setForeground(QColorConstants::Svg::gold);
            cursor.insertText(tr("[Warn]") + fmtMsg(message), fmt);
        }
        break;
    case ScriptMachine::MessageType::Error:
        if (isMatchLast(message)) {
            auto fmt = cursor.charFormat();
            fmt.setForeground(Qt::red);
            cursor.insertText(message.message, fmt);
        } else {
            if (isNotBlockStart) {
                cursor.insertBlock();
            }
            auto fmt = cursor.charFormat();
            fmt.setForeground(Qt::red);
            cursor.insertText(tr("[Error]") + fmtMsg(message), fmt);
        }
        break;
    case ScriptMachine::MessageType::Print:
        if (lastInfo.first != message.type) {
            cursor.insertBlock();
        }
        cursor.insertText(message.message);
        break;
    }

    lastInfo.first = message.type;
    lastInfo.second = qMakePair(message.row, message.col);
}

void MainWindow::closeEvent(QCloseEvent *event) {

    // plugin first checking
    auto closing = PluginSystem::instance().dispatchEvent(
        IWingPlugin::RegisteredEvent::AppClosing, {});
    if (!closing) {
        event->ignore();
        return;
    }

    // then checking the scripting dialog
    if (m_scriptDialog) {
        if (!m_scriptDialog->about2Close()) {
            event->ignore();
            return;
        }

        // then abort all script running
        ScriptMachine::instance().abortScript();
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
            view->requestCloseDockWidget();
        }

        auto ret =
            unSavedFiles.isEmpty()
                ? QMessageBox::No
                : WingMessageBox::warning(
                      this, qAppName(), tr("ConfirmAPPSave"),
                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (ret == QMessageBox::Yes) {
            for (auto pview = m_views.constKeyValueBegin();
                 pview != m_views.constKeyValueEnd(); ++pview) {
                pview->first->save(pview->second, {});
                pview->first->requestCloseDockWidget();
            }

            if (!m_views.isEmpty()) {
                event->ignore();
                return;
            }
        } else if (ret == QMessageBox::No) {
            auto views = m_views.keys();
            for (auto &p : views) {
                p->closeDockWidget(); // force close
            }
        } else {
            event->ignore();
            return;
        }
    }

    auto &set = SettingManager::instance();
    set.setDockLayout(m_dock->saveState());

    if (m_scriptDialog) {
        m_scriptDialog->saveDockLayout();
        set.setRecentFiles(m_recentmanager->saveRecent());
        set.save();
    }

    PluginSystem::instance().destory();

    FramelessMainWindow::closeEvent(event);
    emit closed();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    Q_UNUSED(watched);
    if (isVisible()) {
        if (event->type() == QEvent::Shortcut) {
            auto e = reinterpret_cast<QShortcutEvent *>(event);
            if (m_scriptConsole) {
                if (m_scriptConsole->hasFocus()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                    auto k = e->key()[0];
                    QKeyEvent ev(QEvent::KeyPress, k.key(),
                                 k.keyboardModifiers());
                    m_scriptConsole->processKeyEvent(&ev);
#else
                    auto key = e->key()[0];
                    Qt::KeyboardModifiers modifiers = Qt::NoModifier;

                    // Determine modifiers (simplified logic)
                    if (key & Qt::SHIFT)
                        modifiers |= Qt::ShiftModifier;
                    if (key & Qt::CTRL)
                        modifiers |= Qt::ControlModifier;
                    if (key & Qt::ALT)
                        modifiers |= Qt::AltModifier;
                    if (key & Qt::META)
                        modifiers |= Qt::MetaModifier;

                    // Filter out the modifiers from the key itself
                    key &= ~Qt::KeyboardModifierMask;
                    QKeyEvent ev(QEvent::KeyPress, key, modifiers);
                    m_scriptConsole->processKeyEvent(&ev);
#endif
                    return true;
                }
            }
        }
    }
    return FramelessMainWindow::eventFilter(watched, event);
}
