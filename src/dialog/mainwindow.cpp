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
#include "class/angellsp.h"
#include "class/appmanager.h"
#include "class/dockcomponentsfactory.h"
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
#include "class/workspacemanager.h"
#include "control/toast.h"
#include "define.h"
#include "dialog/layoutdeldialog.h"
#include "encodingdialog.h"
#include "fileinfodialog.h"
#include "finddialog.h"
#include "metadialog.h"
#include "metaheaderdialog.h"
#include "settings/editorsettingdialog.h"
#include "settings/generalsettingdialog.h"
#include "settings/lspsettingdialog.h"
#include "settings/othersettingsdialog.h"
#include "settings/pluginsettingdialog.h"
#include "settings/qeditconfig.h"
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

#include <QStringDecoder>

constexpr auto EMPTY_FUNC = [] {};

MainWindow::MainWindow(SplashDialog *splash) : FramelessMainWindow() {
    this->setUpdatesEnabled(false);
    this->setMinimumSize(900, 800);

    if (splash) {
        splash->setInfoText(tr("SetupUI"));
    }

    // build up UI
    m_recentMenu = new QMenu(this); // used in buildUpRibbonBar
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
        _editArea = new QLabel(this);
        auto font = _editArea->font();
        auto fm = QFontMetrics(font);
        _editArea->setAlignment(Qt::AlignCenter);
        // ASCII or HEX or NONE
        _editArea->setFixedWidth(fm.horizontalAdvance('A') * 5);
        m_status->addWidget(_editArea);

        auto separator = new QFrame(m_status);
        separator->setFrameShape(QFrame::VLine);
        separator->setFrameShadow(QFrame::Sunken);
        m_status->addWidget(separator);

        m_lblloc = new QLabel(QStringLiteral("(0,0)"), this);
        auto l = new QLabel(tr("loc"), this);
        m_status->addWidget(l);
        m_status->addWidget(m_lblloc);

        separator = new QFrame(m_status);
        separator->setFrameShape(QFrame::VLine);
        separator->setFrameShadow(QFrame::Sunken);
        m_status->addWidget(separator);

        l = new QLabel(tr("sel"), this);
        m_lblsellen = new QLabel(QStringLiteral("0 - 0x0"), this);
        m_status->addWidget(l);
        m_status->addWidget(m_lblsellen);

        _status = new QLabel(m_status);
        m_status->addPermanentWidget(_status);

        separator = new QFrame(m_status);
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

        m_sLocked = new QToolButton(this);
        m_sLocked->setStyleSheet(disableStyle);
        m_sLocked->setToolTip(tr("SetLocked"));
        m_sLocked->setIcon(_infoUnLock);
        m_sLocked->setToolButtonStyle(Qt::ToolButtonIconOnly);
        m_sLocked->installEventFilter(this);
        m_status->addPermanentWidget(m_sLocked);
        m_editStateWidgets << m_sLocked;

        m_sCanOver = new QToolButton(this);
        m_sCanOver->setStyleSheet(disableStyle);
        m_sCanOver->setToolTip(tr("SetOver"));
        m_sCanOver->setIcon(_infoCannotOver);
        m_sCanOver->setToolButtonStyle(Qt::ToolButtonIconOnly);
        m_sCanOver->installEventFilter(this);
        m_status->addPermanentWidget(m_sCanOver);
        m_editStateWidgets << m_sCanOver;
    }

    layout->addWidget(m_status);

    buildUpContent(cw);

    m_toolBtneditors.value(ToolButtonIndex::EDITOR_VIEWS)->setEnabled(false);

    // ok, preparing for starting...
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
                if (splash) {
                    splash->setInfoText(tr("LoadingPlg:") + plgName);
                }
            });

    plg.setMainWindow(this);
    plg.loadAllPlugins();

    // recent file manager init
    if (splash) {
        splash->setInfoText(tr("SetupRecent"));
    }

    m_recentmanager = new RecentFileManager(m_recentMenu, false);
    connect(m_recentmanager, &RecentFileManager::triggered, this,
            [=](const RecentFileManager::RecentInfo &rinfo) {
                EditorView *editor = nullptr;
                ErrFile ret;
                const auto &url = rinfo.url;

                if (url.isLocalFile()) {
                    auto fileName = rinfo.url.toLocalFile();
                    if (rinfo.isWorkSpace) {
                        ret = openWorkSpace(fileName, &editor);
                    } else {
                        ret = openFile(fileName, &editor);
                    }
                } else {
                    // plugin extension
                    auto ext = url.authority();
                    auto file = url.path();
                    if (file.front() == '/') {
                        file.removeFirst();
                    }
                    ret = openExtFile(ext, file, &editor);
                }

                if (ret == ErrFile::AlreadyOpened && editor != m_curEditor) {
                    if (editor) {
                        editor->raise();
                        editor->setFocus();
                    }
                } else {
                    reportErrFileError(ret, {}, {}, {});
                }
            });
    m_recentmanager->apply(this, SettingManager::instance().recentHexFiles());

    // Don't setup it too early, because the plugin can register script
    // functions. Code completions of them will be not worked out.
    if (set.scriptEnabled()) {
        auto &sm = ScriptMachine::instance();
        auto smr = sm.init();
        if (smr) {
            plg.doneRegisterScriptObj();

            // register the callback for output error message
            // when configuring error
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

            m_scriptConsole->setMode(QConsoleWidget::Output);
            auto cfgerr = sm.checkEngineConfigError();

            // At this time, AngelScript service plugin has started
            if (cfgerr) {
                ScriptMachine::MessageInfo msg;
                msg.type = ScriptMachine::MessageType::Error;
                msg.mode = ScriptMachine::Interactive;
                msg.message = tr("EngineConfigError");
                m_scriptConsole->onOutput(msg);

                m_scriptConsole->setReadOnly(true);
                m_scriptConsole->verticalScrollBar()->setValue(0);
                m_scriptConsole->horizontalScrollBar()->setValue(0);

                // configure error, so disable all script feature
                m_ribbonMaps[WingHex::WingRibbonCatagories::SCRIPT]->setEnabled(
                    false);
            } else {
                ScriptMachine::RegCallBacks callbacks;
                callbacks.getInputFn = [this]() -> QString {
                    return WingInputDialog::getText(this, tr("InputRequest"),
                                                    tr("PleaseInput"));
                };
                callbacks.clearFn = [this]() { m_bgScriptOutput->clear(); };
                callbacks.printMsgFn =
                    std::bind(&MainWindow::onOutputBgScriptOutput, this,
                              std::placeholders::_1);
                sm.registerCallBack(ScriptMachine::Background, callbacks);

                if (splash)
                    splash->setInfoText(tr("SetupConsole"));

                m_scriptConsole->init();
                if (splash)
                    splash->setInfoText(tr("SetupScriptManager"));

                if (splash)
                    splash->setInfoText(tr("SetupScriptService"));

                m_scriptConsole->initOutput();
                m_scriptConsole->setMode(QConsoleWidget::Input);

                auto &lsp = AngelLsp::instance();
                if (lsp.start()) {
                    auto ret = lsp.initializeSync();
                    if (!ret.isNull()) {
                        lsp.initialized();
                        connect(&lsp, &AngelLsp::serverExited, this, [this]() {
                            Toast::toast(
                                this, NAMEICONRES(QStringLiteral("angellsp")),
                                tr("AngelLspExited"));
                        });
                    } else {
                        _showEvents = [this]() {
                            Toast::toast(
                                this, NAMEICONRES(QStringLiteral("angellsp")),
                                tr("AngelLspInitFailed"), 3000);
                        };
                    }
                } else {
                    _showEvents = [this]() {
                        Toast::toast(this,
                                     NAMEICONRES(QStringLiteral("angellsp")),
                                     tr("AngelLspInitFailed"), 3000);
                    };
                }
                m_scriptConsole->enableLSP();
            }
        } else {
            WingMessageBox::critical(nullptr, qAppName(),
                                     tr("ScriptEngineInitFailed"));
            set.setScriptEnabled(false);
            throw CrashCode::ScriptInitFailed;
        }
    }

    // connect settings signals
    connect(&set, &SettingManager::sigEditorfontSizeChanged, this,
            [this](int v) {
                for (auto &p : m_views) {
                    p->setFontSize(qreal(v));
                }
            });
    connect(&set, &SettingManager::sigCopylimitChanged, this, [this](int v) {
        for (auto &p : m_views) {
            p->setCopyLimit(v);
        }
    });
    connect(&set, &SettingManager::sigDecodeStrlimitChanged, this,
            [this](int v) {
                _decstrlim = v;
                on_selectionChanged();
            });

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

    registerGlobalSwitchViewLoop();

    // update status
    updateEditModeEnabled();
    qApp->installEventFilter(this);

    this->setUpdatesEnabled(true);

    if (splash)
        splash->setInfoText(tr("SetupFinished"));
}

MainWindow::~MainWindow() {
    qDeleteAll(m_hexContextMenu);
    Logger::instance().disconnect();
}

void MainWindow::buildUpRibbonBar() {
    m_ribbon = new Ribbon(this);

    loadCacheIcon();

    using RibbonCatagories = WingHex::WingRibbonCatagories;

    m_ribbonMaps[RibbonCatagories::FILE] =
        buildFilePage(m_ribbon->addTab(tr("File")));
    m_ribbonMaps[RibbonCatagories::EDIT] =
        buildEditPage(m_ribbon->addTab(tr("Edit")));
    m_ribbonMaps[RibbonCatagories::VIEW] =
        buildViewPage(m_ribbon->addTab(tr("View")));

    auto &set = SettingManager::instance();
    if (set.scriptEnabled()) {
        m_ribbonMaps[RibbonCatagories::SCRIPT] =
            buildScriptPage(m_ribbon->addTab(tr("Script")));
    }

    if (set.enablePlugin()) {
        m_ribbonMaps[RibbonCatagories::PLUGIN] =
            buildPluginPage(m_ribbon->addTab(tr("Plugin")));
    }

    m_ribbonMaps[RibbonCatagories::SETTING] =
        buildSettingPage(m_ribbon->addTab(tr("Setting")));
    m_ribbonMaps[RibbonCatagories::ABOUT] =
        buildAboutPage(m_ribbon->addTab(tr("About")));

    connect(m_ribbon, &Ribbon::onDragDropFiles, this, &MainWindow::openFiles);

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

    ads::CDockComponentsFactory::setFactory(new DockComponentsFactory);

    m_dock = new CDockManager;
    m_dock->setStyleSheet(QString());
    m_dock->setParent(this);
    connect(m_dock, &CDockManager::focusedDockWidgetChanged, this,
            [this](CDockWidget *old, CDockWidget *now) {
                Q_UNUSED(old);
                auto editview = qobject_cast<EditorView *>(now);
                swapEditor(m_curEditor, editview);
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
    auto editorViewArea = m_dock->setCentralWidget(CentralDockWidget);

    // build up basic docking widgets
    auto bottomLeftArea =
        buildUpFindResultDock(m_dock, ads::BottomDockWidgetArea);
    bottomLeftArea = buildUpHashResultDock(m_dock, ads::CenterDockWidgetArea,
                                           bottomLeftArea);
    bottomLeftArea =
        buildUpUndoStackDock(m_dock, ads::CenterDockWidgetArea, bottomLeftArea);
    buildUpLogDock(m_dock, ads::CenterDockWidgetArea, bottomLeftArea);

    auto splitter =
        ads::internal::findParent<ads::CDockSplitter *>(editorViewArea);
    if (splitter) {
        constexpr auto bottomHeight = 300;
        splitter->setSizes({height() - bottomHeight, bottomHeight});
    }

    auto rightArea =
        buildUpNumberShowDock(m_dock, ads::RightDockWidgetArea, editorViewArea);

    splitter = ads::internal::findParent<ads::CDockSplitter *>(editorViewArea);
    if (splitter) {
        constexpr auto rightHeight = 450;
        splitter->setSizes({width() - rightHeight, rightHeight});
    }

    m_rightViewArea = rightArea;

    buildUpHexBookMarkDock(m_dock, ads::CenterDockWidgetArea, rightArea);
    buildUpHexMetaDataDock(m_dock, ads::CenterDockWidgetArea, rightArea);
    buildUpDecodingStrShowDock(m_dock, ads::CenterDockWidgetArea, rightArea);

    if (SettingManager::instance().scriptEnabled()) {
        ads::CDockAreaWidget *bottomRightArea;
        bottomRightArea = buildUpScriptConsoleDock(
            m_dock, ads::RightDockWidgetArea, bottomLeftArea);
        bottomRightArea = buildUpScriptBgOutputDock(
            m_dock, ads::CenterDockWidgetArea, bottomRightArea);
        buildUpScriptObjDock(m_dock, ads::CenterDockWidgetArea,
                             bottomRightArea);
        m_bottomViewArea = bottomRightArea;
    }
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

ads::CDockAreaWidget *MainWindow::buildUpLogDock(ads::CDockManager *dock,
                                                 ads::DockWidgetArea area,
                                                 ads::CDockAreaWidget *areaw) {
    m_logbrowser = new QTextBrowser(this);
    m_logbrowser->setFocusPolicy(Qt::StrongFocus);
    m_logbrowser->setOpenExternalLinks(true);
    m_logbrowser->setUndoRedoEnabled(false);

    auto a = newAction(
        ICONRES("copy"), tr("Copy"), [=]() { m_logbrowser->copy(); },
        QKeySequence::Copy);
    a->setShortcutContext(Qt::WidgetShortcut);
    m_logbrowser->addAction(a);

    a = new QAction(this);
    a->setSeparator(true);
    m_logbrowser->addAction(a);

    m_logbrowser->addAction(newAction(ICONRES(QStringLiteral("log")),
                                      tr("ExportLog"),
                                      &MainWindow::on_exportlog));
    m_logbrowser->addAction(newAction(ICONRES(QStringLiteral("clearhis")),
                                      tr("ClearLog"), &MainWindow::on_clslog));

    m_logbrowser->setContextMenuPolicy(Qt::ActionsContextMenu);

    auto dw =
        buildDockWidget(dock, QStringLiteral("Log"), tr("Log"), m_logbrowser);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpFindResultDock(ads::CDockManager *dock,
                                  ads::DockWidgetArea area,
                                  ads::CDockAreaWidget *areaw) {
    _findResultModel = new FindResultModel(this);
    m_findresult = new QTableViewExt(this);
    m_findresult->setProperty("EditorView", quintptr(0));

    Utilities::applyTableViewProperty(m_findresult);
    m_findresult->setContextMenuPolicy(
        Qt::ContextMenuPolicy::CustomContextMenu);

    auto se = [=]() {
        if (_findResultModel->isValid()) {
            m_find->setWindowTitle(tr("FindResult") + QStringLiteral(" (") +
                                   _findResultModel->encoding() +
                                   QStringLiteral(")"));
        } else {
            m_find->setWindowTitle(tr("FindResult"));
        }
    };

    auto menu = new QMenu(tr("Encoding"), this);
    menu->setIcon(ICONRES(QStringLiteral("encoding")));
    auto aGroup = new QActionGroup(menu);
    aGroup->setParent(menu);
    auto langs = Utilities::getEncodings();
    for (auto &l : langs) {
        auto a = newCheckableAction(menu, l, [=]() {
            _findResultModel->setEncoding(l);
            se();
        });
        a->setActionGroup(aGroup);
        menu->addAction(a);
        m_findEncoding.insert(l, a);
    }
    if (langs.isEmpty()) {
        aGroup->deleteLater();
    }

    connect(_findResultModel, &FindResultModel::modelReset, menu, [=]() {
        auto r = _findResultModel->isValid();
        menu->setEnabled(r);
        if (r) {
            m_findEncoding.value(_findResultModel->encoding())
                ->setChecked(true);
        }
        se();
    });

    m_menuFind = new QMenu(m_findresult);
    auto em = new QAction(tr("ShowDec"), this);
    em->setCheckable(true);
    auto fsds = SettingManager::instance().findStrDecShow();
    em->setChecked(fsds);
    connect(em, &QAction::toggled, this, [this](bool b) {
        m_findresult->horizontalHeader()->setSectionHidden(4, !b);
        SettingManager::instance().setFindStrDecShow(b);
    });

    m_menuFind->addAction(em);
    m_menuFind->addSeparator();
    m_menuFind->addMenu(menu);
    menu->setEnabled(false);
    m_menuFind->addAction(
        newAction(QStringLiteral("copy"), tr("Copy"), [this]() {
            auto idx = m_findresult->currentIndex();
            if (idx.isValid()) {
                auto content = _findResultModel->copyContent(idx);
                qApp->clipboard()->setText(content);
                Toast::toast(this, NAMEICONRES(QStringLiteral("copy")),
                             tr("CopyToClipBoard"));
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

    m_findresult->setModel(_findResultModel);
    m_findresult->horizontalHeader()->setSectionHidden(4, !fsds);

    connect(m_findresult, &QTableView::doubleClicked, this,
            [=](const QModelIndex &index) {
                auto editor = currentEditor();
                auto e = editor->hexEditor();
                auto fm = editor->findResult();
                auto cursor = e->cursor();

                cursor->moveTo(fm.results.at(index.row()).offset);
                if (cursor->selectionCount() <= 1 && index.column() >= 3) {
                    cursor->select(fm.lastFindData.second);
                }
            });

    auto header = m_findresult->horizontalHeader();
    header->setSectionResizeMode(3, QHeaderView::Stretch);
    header->setSectionResizeMode(4, QHeaderView::Stretch);
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
        updateNumberTable(true);
    });
    m_numshowtable->addAction(aLittleEndian);

    auto aBigEndian = new QAction(actionGroup);
    aBigEndian->setText(tr("BigEndian"));
    aBigEndian->setCheckable(true);
    aBigEndian->setChecked(!le);
    connect(aBigEndian, &QAction::triggered, this, [=] {
        m_islittle = false;
        updateNumberTable(true);
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
        updateNumberTable(true);
    });
    m_numshowtable->addAction(aUnsignedHex);

    m_numshowtable->setContextMenuPolicy(
        Qt::ContextMenuPolicy::ActionsContextMenu);

    auto dw = buildDockWidget(dock, QStringLiteral("Number"), tr("Number"),
                              m_numshowtable);
    m_numshowtable->installEventFilter(this);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpHashResultDock(ads::CDockManager *dock,
                                  ads::DockWidgetArea area,
                                  ads::CDockAreaWidget *areaw) {
    m_hash = new QTableViewExt(this);
    Utilities::applyTableViewProperty(m_hash);
    m_hash->setColumnWidth(0, 350);

    _hashModel = new CheckSumModel(this);
    m_hash->setModel(_hashModel);

    m_hash->setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);

    auto a = newAction(ICONRES(QStringLiteral("copy")), tr("Copy"), [=] {
        auto r = m_hash->currentIndex();
        qApp->clipboard()->setText(
            _hashModel->checkSumData(CryptographicHash::Algorithm(r.row())));
        Toast::toast(this, NAMEICONRES(QStringLiteral("copy")),
                     tr("CopyToClipBoard"));
    });
    m_hash->addAction(a);
    a = newAction(QStringLiteral("del"), tr("Clear"), [=]() {
        auto cur = currentEditor();
        if (cur) {
            auto &cs = cur->checkSumResult();
            for (auto &d : cs) {
                d.clear();
            }
            cur->resetCheckSumVisible();
        }

        _hashModel->clearData();
        auto header = m_hash->verticalHeader();
        auto total = int(CryptographicHash::Algorithm::NumAlgorithms);
        for (int i = 0; i < total; ++i) {
            header->showSection(i);
        }
    });
    m_hash->addAction(a);
    connect(m_hash->selectionModel(), &QItemSelectionModel::currentRowChanged,
            a, [=](const QModelIndex &current, const QModelIndex &) {
                a->setEnabled(current.isValid());
            });

    auto dw = buildDockWidget(dock, QStringLiteral("CheckSum"), tr("CheckSum"),
                              m_hash);
    m_hashtable = dw;
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpHexBookMarkDock(ads::CDockManager *dock,
                                   ads::DockWidgetArea area,
                                   ads::CDockAreaWidget *areaw) {
    _bookMarkModel = new BookMarksModel(nullptr);
    m_bookMark = new QTableViewExt(this);
    Utilities::applyTableViewProperty(m_bookMark);
    m_bookMark->horizontalHeader()->setDefaultSectionSize(150);
    m_bookMark->setSelectionMode(QTableViewExt::ExtendedSelection);
    m_bookMark->setModel(_bookMarkModel);

    connect(m_bookMark, &QTableView::doubleClicked, this,
            [=](const QModelIndex &index) {
                auto hexeditor = currentHexView();
                if (hexeditor == nullptr) {
                    return;
                }
                hexeditor->renderer()->enableCursor(true);
                QAbstractItemModel *model = _bookMarkModel;
                auto offIndex = model->index(index.row(), 0);
                auto offset = model->data(offIndex).value<qsizetype>();
                hexeditor->cursor()->moveTo(offset);
            });

    m_bookMark->addAction(
        newAction(QStringLiteral("export"), tr("ExportResult"),
                  [this]() { saveTableContent(_bookMarkModel); }));

    auto sep = new QAction(m_bookMark);
    sep->setSeparator(true);
    m_bookMark->addAction(sep);

    m_aDelBookMark = new QAction(ICONRES(QStringLiteral("bookmarkdel")),
                                 tr("DeleteBookMark"), m_bookMark);
    connect(m_aDelBookMark, &QAction::triggered, this, [=] {
        auto hexeditor = currentHexView();
        if (hexeditor == nullptr) {
            return;
        }
        auto s = m_bookMark->selectionModel()->selectedRows();
        auto doc = hexeditor->document();

        auto model = m_bookMark->model();

        QList<qsizetype> pos;
        for (auto &item : s) {
            auto offIndex = model->index(item.row(), 0);
            auto offset = model->data(offIndex).value<qsizetype>();
            pos.append(offset);
        }

        m_bookMark->clearSelection();
        doc->RemoveBookMarks(pos);
        Q_EMIT model->layoutChanged();
    });
    m_aDelBookMark->setEnabled(false);
    m_bookMark->addAction(m_aDelBookMark);

    auto a = new QAction(ICONRES(QStringLiteral("bookmarkcls")),
                         tr("ClearBookMark"), m_bookMark);
    connect(a, &QAction::triggered, this, &MainWindow::on_bookmarkcls);
    m_bookMark->addAction(a);

    m_bookMark->setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);

    connect(m_bookMark->selectionModel(),
            &QItemSelectionModel::selectionChanged, m_aDelBookMark,
            [=](const QItemSelection &, const QItemSelection &) {
                m_aDelBookMark->setEnabled(
                    m_bookMark->selectionModel()->hasSelection());
            });

    auto dw = buildDockWidget(dock, QStringLiteral("BookMark"), tr("BookMark"),
                              m_bookMark);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpHexMetaDataDock(ads::CDockManager *dock,
                                   ads::DockWidgetArea area,
                                   ads::CDockAreaWidget *areaw) {
    _metadataModel = new MetaDataModel(nullptr);
    m_metadata = new QTableViewExt(this);
    Utilities::applyTableViewProperty(m_metadata);
    m_metadata->setSelectionMode(QTableViewExt::ExtendedSelection);

    m_metadata->setModel(_metadataModel);

    connect(m_metadata, &QTableView::doubleClicked, this,
            [=](const QModelIndex &index) {
                auto hexeditor = currentHexView();
                if (hexeditor == nullptr) {
                    return;
                }
                hexeditor->renderer()->enableCursor(true);

                auto model = m_metadata->model();
                auto offIndex = model->index(index.row(), 0);
                auto offset =
                    model->data(offIndex, Qt::UserRole).value<qsizetype>();
                hexeditor->cursor()->moveTo(offset);
            });

    m_metadata->addAction(
        newAction(QStringLiteral("mLineInfo"), tr("CustomHeader"), [=]() {
            QVector<bool> ov(5);
            auto total = 5;
            auto header = m_metadata->horizontalHeader();
            for (qsizetype i = 0; i < total; ++i) {
                ov[i] = header->isSectionHidden(i);
            }
            MetaHeaderDialog d(ov);
            if (d.exec()) {
                auto data = d.getResults();
                for (qsizetype i = 0; i < total; ++i) {
                    header->setSectionHidden(i, data.at(i));
                }
                SettingManager::instance().setMetaHeaderHidden(data);
            }
        }));

    auto sep = new QAction(m_metadata);
    sep->setSeparator(true);
    m_metadata->addAction(sep);

    m_metadata->addAction(
        newAction(QStringLiteral("export"), tr("ExportResult"),
                  [this]() { saveTableContent(_metadataModel); }));

    sep = new QAction(m_metadata);
    sep->setSeparator(true);
    m_metadata->addAction(sep);

    m_aDelMetaData = new QAction(ICONRES(QStringLiteral("metadatadel")),
                                 tr("DeleteMetadata"), m_metadata);
    connect(m_aDelMetaData, &QAction::triggered, this, [=] {
        auto hexeditor = currentHexView();
        if (hexeditor == nullptr) {
            return;
        }
        auto s = m_metadata->selectionModel()->selectedRows();
        auto doc = hexeditor->document();

        const auto &mds = doc->metadata()->getAllMetadata();

        QList<QHexMetadataItem> pmetas;
        for (auto &item : s) {
            pmetas.push_back(mds.at(item.row()));
        }

        m_metadata->clearSelection();
        hexeditor->document()->metadata()->RemoveMetadatas(pmetas);
        Q_EMIT m_metadata->model()->layoutChanged();
    });
    m_aDelMetaData->setEnabled(false);
    m_metadata->addAction(m_aDelMetaData);

    auto a = new QAction(ICONRES(QStringLiteral("metadatacls")),
                         tr("ClearMetadata"), m_metadata);
    connect(a, &QAction::triggered, this, &MainWindow::on_metadatacls);
    m_metadata->addAction(a);
    m_metadata->setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);

    connect(m_metadata->selectionModel(),
            &QItemSelectionModel::selectionChanged, m_aDelMetaData,
            [=](const QItemSelection &, const QItemSelection &) {
                m_aDelMetaData->setEnabled(
                    m_metadata->selectionModel()->hasSelection());
            });

    auto ovdata = SettingManager::instance().metaHeaderHidden();
    auto header = m_metadata->horizontalHeader();
    for (qsizetype i = 0; i < 5; ++i) {
        header->setSectionHidden(i, ovdata.at(i));
    }

    auto dw = buildDockWidget(dock, QStringLiteral("MetaData"), tr("MetaData"),
                              m_metadata);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpDecodingStrShowDock(ads::CDockManager *dock,
                                       ads::DockWidgetArea area,
                                       ads::CDockAreaWidget *areaw) {
    m_txtDecode = new QTextBrowser(this);
    m_txtDecode->setUndoRedoEnabled(false);
    m_txtDecode->setPlaceholderText(tr("PleaseSelectBytes"));
    m_txtDecode->setProperty("__NAME__", tr("DecodeText"));
    auto dw = buildDockWidget(dock, QStringLiteral("DecodeText"),
                              tr("DecodeText") + QStringLiteral(" (ASCII)"),
                              m_txtDecode);

    auto a = newAction(
        ICONRES("copy"), tr("Copy"), [=]() { m_logbrowser->copy(); },
        QKeySequence::Copy);
    a->setShortcutContext(Qt::WidgetShortcut);
    m_txtDecode->addAction(a);
    a = new QAction(tr("Encoding"), this);
    a->setIcon(ICONRES(QStringLiteral("encoding")));
    connect(a, &QAction::triggered, this, &MainWindow::on_encoding);
    m_txtDecode->addAction(a);

    m_txtDecode->setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(m_txtDecode, &QTextBrowser::windowTitleChanged, dw,
            &QDockWidget::setWindowTitle);
    m_txtDecode->installEventFilter(this);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpScriptConsoleDock(ads::CDockManager *dock,
                                     ads::DockWidgetArea area,
                                     ads::CDockAreaWidget *areaw) {
    m_scriptConsole = new ScriptingConsole(this);
    m_scriptConsole->setIsTerminal(true);
    connect(m_scriptConsole, &ScriptingConsole::abortEvaluation, this,
            [this]() {
                auto &sm = ScriptMachine::instance();
                if (sm.isRunning(ScriptMachine::Interactive)) {
                    sm.abortScript(ScriptMachine::Interactive);
                } else {
                    m_scriptConsole->abortCurrentCode();
                }
            });
    m_scriptConsole->installEventFilter(this);

    auto dw = buildDockWidget(dock, QStringLiteral("ScriptConsole"),
                              tr("ScriptConsole"), m_scriptConsole);
    m_console = dw;

    connect(m_scriptConsole, &ScriptingConsole::consoleCommand, m_console,
            [this]() {
                if (m_scriptConsole->isVisible()) {
                    return;
                }
                m_console->setIcon(ICONRES(QStringLiteral("dbgrun")));
            });
    connect(m_scriptConsole, &ScriptingConsole::consoleScriptRunFinished,
            m_console, [this]() { m_console->setIcon({}); });
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpScriptBgOutputDock(ads::CDockManager *dock,
                                      ads::DockWidgetArea area,
                                      ads::CDockAreaWidget *areaw) {

    m_bgScriptOutput = new QPlainTextEdit(this);
    m_bgScriptOutput->setPlaceholderText(tr("BgScriptOutputHere"));
    m_bgScriptOutput->setReadOnly(true);

    _hlAnim = new ConsoleHighlighAnim(m_bgScriptOutput);

    auto a = newAction(
        ICONRES(QStringLiteral("mStr")), tr("SelectAll"),
        [this]() { m_bgScriptOutput->selectAll(); }, QKeySequence::SelectAll);
    a->setShortcutContext(Qt::WidgetShortcut);
    m_bgScriptOutput->addAction(a);
    a = newAction(
        ICONRES(QStringLiteral("copy")), tr("Copy"),
        [this]() { m_bgScriptOutput->copy(); }, QKeySequence::Copy);
    a->setShortcutContext(Qt::WidgetShortcut);
    m_bgScriptOutput->addAction(a);
    a = newAction(ICONRES(QStringLiteral("del")), tr("Clear"),
                  [this]() { m_bgScriptOutput->clear(); });
    a->setShortcutContext(Qt::WidgetShortcut);
    m_bgScriptOutput->addAction(a);
    a = new QAction(this);
    a->setSeparator(true);
    m_bgScriptOutput->addAction(a);
    a = newAction(
        ICONRES(QStringLiteral("dbgstop")), tr("AbortScript"),
        []() {
            ScriptMachine::instance().abortScript(ScriptMachine::Background);
        },
        QKeySequence(Qt::ControlModifier | Qt::Key_Q));
    a->setShortcutContext(Qt::WidgetShortcut);
    m_bgScriptOutput->addAction(a);
    m_bgScriptOutput->setContextMenuPolicy(Qt::ActionsContextMenu);

    auto dw = buildDockWidget(dock, QStringLiteral("BgScriptOutput"),
                              tr("BgScriptOutput"), m_bgScriptOutput);
    auto tab = dw->tabWidget();
    _hlAnim->setWidget(tab);
    ScriptManager::instance().setIndicator(tab);

    m_bgScriptOutput->installEventFilter(this);

    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpScriptObjDock(ads::CDockManager *dock,
                                 ads::DockWidgetArea area,
                                 ads::CDockAreaWidget *areaw) {
    _scriptObjView = new asIDBTreeView(this);
    _scriptObjView->header()->setDefaultSectionSize(200);

    connect(
        m_scriptConsole, &ScriptingConsole::consoleScriptRunFinished, this,
        [this]() {
            auto &m = ScriptMachine::instance();
            auto mod = m.module(ScriptMachine::Interactive);
            if (mod == nullptr) {
                return;
            }
            auto globals = std::make_shared<asIDBVariable>(*m.debugger());
            globals->ptr = globals;

            // copy from asIDBCache
            auto typeNameFromType = [](const asIDBTypeId &id) -> std::string {
                static asIDBTypeNameMap type_names; // cached name
                auto ret = type_names.find(id);
                if (ret != type_names.end()) {
                    return ret->second;
                }

                auto engine = ScriptMachine::instance().engine();
                auto type = engine->GetTypeInfoById(id.typeId);
                const char *name;

                if (!type) {
                    // a primitive
                    switch (id.typeId & asTYPEID_MASK_SEQNBR) {
                    case asTYPEID_BOOL:
                        name = "bool";
                        break;
                    case asTYPEID_INT8:
                        name = "int8";
                        break;
                    case asTYPEID_INT16:
                        name = "int16";
                        break;
                    case asTYPEID_INT32:
                        name = "int32";
                        break;
                    case asTYPEID_INT64:
                        name = "int64";
                        break;
                    case asTYPEID_UINT8:
                        name = "uint8";
                        break;
                    case asTYPEID_UINT16:
                        name = "uint16";
                        break;
                    case asTYPEID_UINT32:
                        name = "uint32";
                        break;
                    case asTYPEID_UINT64:
                        name = "uint64";
                        break;
                    case asTYPEID_FLOAT:
                        name = "float";
                        break;
                    case asTYPEID_DOUBLE:
                        name = "double";
                        break;
                    default:
                        name = "???";
                        break;
                    }
                } else {
                    name = type->GetName();
                }

                type_names.emplace(id, name);
                return name;
            };

            auto total = mod->GetGlobalVarCount();
            for (asUINT n = 0; n < total; n++) {
                const char *name;
                const char *nameSpace;
                int typeId;
                void *ptr;
                bool isConst;

                mod->GetGlobalVar(n, &name, &nameSpace, &typeId, &isConst);
                ptr = mod->GetAddressOfGlobalVar(n);

                asIDBTypeId typeKey{typeId, isConst ? asTM_CONST : asTM_NONE};
                const auto viewType = typeNameFromType(typeKey);

                asIDBVarAddr idKey{typeId, isConst, ptr};

                globals->CreateChildVariable(
                    asIDBVarName((nameSpace && nameSpace[0]) ? nameSpace : "",
                                 name),
                    idKey, viewType);
            }
            auto engine = mod->GetEngine();
            total = engine->GetGlobalPropertyCount();
            for (asUINT n = 0; n < total; n++) {
                const char *name;
                const char *nameSpace;
                int typeId;
                void *ptr;
                bool isConst;

                engine->GetGlobalPropertyByIndex(n, &name, &nameSpace, &typeId,
                                                 &isConst, nullptr, &ptr);
                asIDBTypeId typeKey{typeId, isConst ? asTM_CONST : asTM_NONE};
                const auto viewType = typeNameFromType(typeKey);

                asIDBVarAddr idKey{typeId, isConst, ptr};

                std::string localName =
                    (nameSpace && nameSpace[0])
                        ? fmt::format("{}::{}", nameSpace, name)
                        : name;

                globals->CreateChildVariable(std::move(localName), idKey,
                                             viewType);
            }

            globals->evaluated = globals->expanded = true;

            if (!globals->namedProps.empty() || !globals->indexedProps.empty())
                globals->SetRefId();

            _scriptObjView->refreshWithNewRoot(globals);
        });

    auto dw = buildDockWidget(dock, QStringLiteral("ConsoleObj"),
                              tr("ConsoleObj"), _scriptObjView);
    return dock->addDockWidget(area, dw, areaw);
}

ads::CDockAreaWidget *
MainWindow::buildUpUndoStackDock(ads::CDockManager *dock,
                                 ads::DockWidgetArea area,
                                 ads::CDockAreaWidget *areaw) {
    _undoView = new QUndoView(this);
    auto dw = buildDockWidget(dock, QStringLiteral("UndoStack"),
                              tr("UndoStack"), _undoView);
    return dock->addDockWidget(area, dw, areaw);
}

RibbonTabContent *MainWindow::buildFilePage(RibbonTabContent *tab) {
    updateUI();
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
    updateUI();
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
                            &MainWindow::on_delete);
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
    updateUI();
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
        menu->addAction(newAction(QStringLiteral("icon"), tr("Default"),
                                  [this]() { restoreLayout(_defaultLayout); }));

        if (!l.isEmpty()) {
            menu->addSeparator();
        }

        for (auto p = l.constKeyValueBegin(); p != l.constKeyValueEnd(); ++p) {
            auto layout = p->second;
            menu->addAction(
                newAction(QStringLiteral("layout"), p->first,
                          [this, layout]() { restoreLayout(layout); }));
        }

        m_toolBtneditors.insert(
            ToolButtonIndex::LAYOUT_ACTION,
            addPannelAction(pannel, QStringLiteral("layout"),
                            tr("RestoreLayout"), EMPTY_FUNC, {}, menu));

        addPannelAction(pannel, QStringLiteral("layoutexport"),
                        tr("SaveLayout"), &MainWindow::on_saveLayout);

        addPannelAction(pannel, QStringLiteral("layoutdel"), tr("DelLayout"),
                        &MainWindow::on_delLayout);
    }

    return tab;
}

RibbonTabContent *MainWindow::buildScriptPage(RibbonTabContent *tab) {
    updateUI();
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
        _scriptContexts =
            ScriptManager::buildUpScriptRunnerContext(pannel, this);
        m_scriptDBGroup = pannel;
    }

    return tab;
}

RibbonTabContent *MainWindow::buildPluginPage(RibbonTabContent *tab) {
    updateUI();
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
    updateUI();
    auto shortcuts = QKeySequences::instance();

    auto &set = SettingManager::instance();

    {
        auto pannel = tab->addGroup(tr("General"));
        addPannelAction(
            pannel, QStringLiteral("general"), tr("Setting"),
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
    updateUI();

    {
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

void MainWindow::buildUpSettingDialog() {
    QStringList usedIDs;
    QString id;
    auto &set = SettingManager::instance();

    m_setdialog = new SettingDialog(ICONRES(QStringLiteral("general")),
                                    tr("Setting"), this);
    updateUI();

    auto generalPage = new GeneralSettingDialog(m_setdialog);
    connect(generalPage, &SettingPage::optionNeedRestartChanged, m_setdialog,
            QOverload<>::of(&SettingDialog::toastTakeEffectReboot));
    m_setdialog->addPage(generalPage);
    id = generalPage->id();
    Q_ASSERT(!id.isEmpty());
    Q_ASSERT(usedIDs.indexOf(id) < 0);
    usedIDs.append(id);
    updateUI();

    auto editorPage = new EditorSettingDialog(m_setdialog);
    connect(editorPage, &SettingPage::optionNeedRestartChanged, m_setdialog,
            QOverload<>::of(&SettingDialog::toastTakeEffectReboot));
    m_setdialog->addPage(editorPage);
    id = editorPage->id();
    Q_ASSERT(!id.isEmpty());
    Q_ASSERT(usedIDs.indexOf(id) < 0);
    usedIDs.append(id);
    updateUI();

    auto plgPage = new PluginSettingDialog(m_setdialog);
    connect(plgPage, &SettingPage::optionNeedRestartChanged, m_setdialog,
            QOverload<>::of(&SettingDialog::toastTakeEffectReboot));
    m_setdialog->addPage(plgPage);
    id = plgPage->id();
    Q_ASSERT(!id.isEmpty());
    Q_ASSERT(usedIDs.indexOf(id) < 0);
    usedIDs.append(id);
    updateUI();

    auto scriptPage = new ScriptSettingDialog(m_setdialog);
    connect(scriptPage, &SettingPage::optionNeedRestartChanged, m_setdialog,
            QOverload<>::of(&SettingDialog::toastTakeEffectReboot));
    m_setdialog->addPage(scriptPage);
    id = scriptPage->id();
    Q_ASSERT(!id.isEmpty());
    Q_ASSERT(usedIDs.indexOf(id) < 0);
    usedIDs.append(id);
    updateUI();

    auto otherPage = new OtherSettingsDialog(m_setdialog);
    id = otherPage->id();
    Q_ASSERT(!id.isEmpty());
    Q_ASSERT(usedIDs.indexOf(id) < 0);
    usedIDs.append(id);
    updateUI();

    for (auto &page : m_settingPages) {
        updateUI();

        auto name = page->name();
        auto id = page->id();

        // check
        if (id.isEmpty()) {
            id = name;
            auto plg = page->property("__plg__").value<IWingPlugin *>();
            Logger::warning(QStringLiteral("[") +
                            QString::fromUtf8(plg->metaObject()->className()) +
                            QStringLiteral("::") + name + QStringLiteral("] ") +
                            QStringLiteral(":") +
                            tr("SetPageIDEmptyTryUseName"));
        }

        if (usedIDs.contains(id, Qt::CaseInsensitive)) {
            auto plg = page->property("__plg__").value<IWingPlugin *>();
            Logger::critical(QStringLiteral("[") +
                             QString::fromUtf8(plg->metaObject()->className()) +
                             QStringLiteral("::") + name +
                             QStringLiteral("] ") + QStringLiteral(":") +
                             tr("SetPageDupNameIgnored"));
            continue;
        }

        connect(page, &SettingPage::optionNeedRestartChanged, m_setdialog,
                QOverload<>::of(&SettingDialog::toastTakeEffectReboot));
        m_setdialog->addPage(page);
        if (page->showInRibbon()) {
            auto icon = page->categoryIcon();
            addPannelAction(m_pluginSettingsGroup, icon, name,
                            [=] { m_setdialog->showConfig(id); });
        }
        usedIDs.append(id);
    }
    m_settingPages.clear();
    m_settingPages.squeeze();

    connect(otherPage, &SettingPage::optionNeedRestartChanged, m_setdialog,
            QOverload<>::of(&SettingDialog::toastTakeEffectReboot));
    m_setdialog->addPage(otherPage);
    updateUI();

    m_setdialog->build();
    auto ge = set.settingsLayout();
    if (!ge.isEmpty()) {
        m_setdialog->restoreLayout(ge);
    }
    connect(m_setdialog, &SettingDialog::onClosing, this, [this]() {
        SettingManager::instance().setSettingsLayout(m_setdialog->saveLayout());
    });

    // script settings
    m_scriptsetdlg = new SettingDialog(ICONRES(QStringLiteral("scriptset")),
                                       tr("ScriptSetting"), this);
    auto edit = new QEditConfig(false, m_scriptsetdlg);
    m_scriptsetdlg->addPage(edit);
    connect(edit, &SettingPage::optionNeedRestartChanged, m_scriptsetdlg,
            QOverload<>::of(&SettingDialog::toastTakeEffectReboot));
    edit = new QEditConfig(true, m_scriptsetdlg);
    m_scriptsetdlg->addPage(edit);
    connect(edit, &SettingPage::optionNeedRestartChanged, m_scriptsetdlg,
            QOverload<>::of(&SettingDialog::toastTakeEffectReboot));
    auto lspset = new LspSettingDialog(this);
    m_scriptsetdlg->addPage(lspset);
    connect(lspset, &SettingPage::optionNeedRestartChanged, m_scriptsetdlg,
            QOverload<>::of(&SettingDialog::toastTakeEffectReboot));
    m_scriptsetdlg->build();

    ge = set.settingsScriptLayout();
    if (!ge.isEmpty()) {
        m_scriptsetdlg->restoreLayout(ge);
    }
    connect(m_scriptsetdlg, &SettingDialog::onClosing, this, [this]() {
        SettingManager::instance().setSettingsScriptLayout(
            m_scriptsetdlg->saveLayout());
    });
}

void MainWindow::installPluginEditorWidgets() {
    QMap<QString, QPair<IWingPlugin *, QAction *>> names;

    auto &log = Logger::instance();
    auto menu = m_toolBtneditors.value(EDITOR_WINS)->menu();

    decltype(m_editorViewWidgets) newEditorViewWidgets;
    for (auto p = m_editorViewWidgets.constKeyValueBegin();
         p != m_editorViewWidgets.constKeyValueEnd(); ++p) {
        decltype(newEditorViewWidgets)::mapped_type newCreatorList;
        for (auto &wctor : p->second) {
            updateUI();

            auto id = wctor->id();
            if (names.contains(id)) {
                log.critical(tr("Plugin %1 contains a duplicate ID (%2) that "
                                "is already registered by plugin %3")
                                 .arg(p->first->pluginName(), id,
                                      names[id].first->pluginName()));
                continue;
            }

            auto icon = wctor->icon();
            if (icon.isNull()) {
                icon = ICONRES(QStringLiteral("win"));
            }
            auto name = wctor->name().trimmed();
            if (name.isEmpty()) {
                name = p->first->pluginName();
            }
            auto a = newAction(icon, name, [this, id] {
                auto editor = currentEditor();
                if (editor == nullptr) {
                    return;
                }
                editor->switchView(id);
            });
            a->setProperty("__ID__", id);

            names.insert(id, qMakePair(p->first, a));
            newCreatorList.append(wctor);
        }

        for (auto &item : names) {
            menu->addAction(item.second);
        }

        newEditorViewWidgets.insert(p->first, newCreatorList);
        updateUI();
    }

    m_editorViewWidgets = newEditorViewWidgets;
}

void MainWindow::registerGlobalSwitchViewLoop() {
    auto sc =
        new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Left), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [this]() {
        auto view = currentEditor();
        if (view) {
            view->switchViewStackLoop(false);
        }
    });

    sc = new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Right), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [this]() {
        auto view = currentEditor();
        if (view) {
            view->switchViewStackLoop(true);
        }
    });
}

void MainWindow::showStatus(const QString &status) {
    auto editor = currentEditor();
    if (status.isEmpty() && editor && editor->isFindBusy()) {
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
        m_lastusedpath = Utilities::getAbsoluteDirPath(filename);
        EditorView *editor = nullptr;
        auto res = openFile(filename, &editor);
        if (res != ErrFile::Success) {
            if (res == ErrFile::NotExist) {
                WingMessageBox::critical(this, tr("Error"), tr("FileNotExist"));
            } else if (res == ErrFile::Permission) {
                WingMessageBox::critical(this, tr("Error"),
                                         tr("FilePermission"));
            } else if (res == ErrFile::AlreadyOpened && editor != m_curEditor) {
                if (editor) {
                    editor->raise();
                    editor->setFocus();
                }
            } else {
                auto e = QMetaEnum::fromType<ErrFile>();
                WingMessageBox::critical(
                    this, tr("Error"),
                    tr("UnkownError") + QStringLiteral(" - ") +
                        QString::fromLatin1(e.valueToKey(int(res))));
            }
            return;
        }

        addRecentFile(filename);
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

    m_lastusedpath = Utilities::getAbsoluteDirPath(filename);
    EditorView *editor = nullptr;
    auto res = openWorkSpace(filename, &editor);
    if (res == ErrFile::AlreadyOpened && editor != m_curEditor) {
        if (editor) {
            editor->raise();
            editor->setFocus();
        }
    } else {
        if (reportErrFileError(res, {}, {}, {})) {
            addRecentFile(filename, true);
        }
    }
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
    reportErrFileError(res, NAMEICONRES(QStringLiteral("reload")),
                       tr("ReloadSuccessfully"), tr("ReloadUnSuccessfully"));
    updateEditModeEnabled();
}

void MainWindow::on_save() {
    showStatus(tr("Saving..."));
    QScopeGuard g([this]() { showStatus({}); });

    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }

    auto isNewFile = editor->isNewFile();
    if (isNewFile) {
        on_saveas();
        return;
    }

    auto changedTo = editor->change2WorkSpace();
    QString ws;
    auto res = saveEditor(editor, {}, false, false, &ws);
    if (reportErrFileError(res, NAMEICONRES(QStringLiteral("save")),
                           tr("SaveSuccessfully"), tr("SaveUnSuccessfully"))) {
        if (changedTo) {
            addRecentFile(ws, true);
        }
    }
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

    if (editor->isWorkSpace()) {
        Toast::toast(this, NAMEICONRES("convpro"), tr("AlreadyWorkSpace"));
        return;
    }

    QString workspace;
    auto ret = saveEditor(editor, {}, false, true, &workspace);
    if (ret == ErrFile::WorkSpaceUnSaved) {
        WingMessageBox::critical(this, tr("Error"), tr("ConvWorkSpaceFailed"));
    } else {
        if (reportErrFileError(ret, NAMEICONRES("convpro"),
                               tr("ConvWorkSpaceSuccess"),
                               tr("ConvWorkSpaceFailed"))) {
            // add to history
            addRecentFile(workspace, true);
        }
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
        lastpath = editor->fileNameUrl().toLocalFile();
    }

    auto filename =
        WingFileDialog::getSaveFileName(this, tr("ChooseSaveFile"), lastpath);
    if (filename.isEmpty())
        return;
    m_lastusedpath = Utilities::getAbsoluteDirPath(filename);

    bool isWorkspace = editor->isWorkSpace();
    QString ws;
    auto res = saveEditor(editor, filename, false, isWorkspace, &ws);
    if (reportErrFileError(res, NAMEICONRES(QStringLiteral("saveas")),
                           tr("SaveSuccessfully"), tr("SaveUnSuccessfully"))) {
        if (ws.isEmpty()) {
            addRecentFile(filename, false);
        } else {
            addRecentFile(ws, true);
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

    QString lastpath;
    if (editor->isNewFile() || editor->isExtensionFile()) {
        lastpath = m_lastusedpath;
    } else {
        lastpath = editor->fileNameUrl().toLocalFile();
    }

    auto filename =
        WingFileDialog::getSaveFileName(this, tr("ChooseExportFile"), lastpath);
    if (filename.isEmpty())
        return;
    m_lastusedpath = Utilities::getAbsoluteDirPath(filename);

    bool isWorkspace = editor->isWorkSpace();
    auto res = saveEditor(editor, filename, true, isWorkspace);
    reportErrFileError(res, NAMEICONRES(QStringLiteral("export")),
                       tr("ExportSuccessfully"), tr("ExportUnSuccessfully"));
}

void MainWindow::on_savesel() {
    showStatus(tr("SavingSel..."));
    QScopeGuard g([this]() { showStatus({}); });

    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }

    auto hexeditor = editor->hexEditor();

    QString lastpath;
    if (editor->isNewFile() || editor->isExtensionFile()) {
        lastpath = m_lastusedpath;
    } else {
        QFileInfo finfo(editor->fileNameUrl().toLocalFile());
        auto dir = finfo.absoluteDir();
        lastpath = dir.absoluteFilePath(
            QStringLiteral("sel_%1_%2.bin")
                .arg(finfo.baseName(),
                     QDateTime::currentDateTime().toString(
                         QStringLiteral("yyyyMMdd_hhmmsszzz"))));
    }

    auto filename =
        WingFileDialog::getSaveFileName(this, tr("ChooseSaveFile"), lastpath);
    if (filename.isEmpty())
        return;
    m_lastusedpath = Utilities::getAbsoluteDirPath(filename);
    QFile qfile(filename);
    if (qfile.open(QFile::WriteOnly)) {
        auto buffer = hexeditor->selectedBytes().join();
        qfile.write(buffer);
        qfile.close();
        Toast::toast(this, NAMEICONRES(QStringLiteral("savesel")),
                     tr("SaveSelSuccess"));
    } else {
        WingMessageBox::critical(this, tr("Error"),
                                 tr("SaveSelError") + QStringLiteral(" - ") +
                                     qfile.errorString());
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
    if (!hexeditor->Paste()) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("paste")),
                     tr("PasteFailedNote"));
    }
}

void MainWindow::on_delete() {
    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }

    if (!hexeditor->RemoveSelection()) {
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
    info.isSel = hexeditor->selectionCount() == 1;

    FindDialog fd(info, this);
    if (fd.exec()) {
        auto r = fd.getResult();
        info.isStringFind = r.isStringFind;
        info.encoding = r.encoding;
        info.findValue = r.value;

        showStatus(tr("Finding..."));
        ExecAsync<EditorView::FindError>(
            [this, r, editor]() -> EditorView::FindError {
                return editor->find(r);
            },
            [this, editor](EditorView::FindError err) {
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

                auto r = editor->findResult();
                m_findEncoding.value(r.encoding)->setChecked(true);
                _findResultModel->refresh();
                auto header = m_findresult->horizontalHeader();
                auto font = QFontMetrics(m_findresult->font());
                auto len = font.horizontalAdvance('F') * (15 + 16 * 2);
                if (header->sectionSize(3) < len) {
                    header->resizeSection(3, len);
                }

                m_find->raise();
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

    if (editor->isFindBusy()) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("sum")),
                     tr("FindFininishBusy"));
        return;
    }

    static QVector<int> cs([]() -> QVector<int> {
        QVector<int> ret;
        auto total = int(CryptographicHash::Algorithm::NumAlgorithms);
        ret.reserve(total);
        for (int i = 0; i < total; ++i) {
            ret.append(i);
        }
        return ret;
    }());

    CheckSumDialog d(cs);
    if (d.exec()) {
        cs = d.getResults();
        editor->getCheckSum(cs);
        _hashModel->updateCheckSumData(editor->checkSumResult());

        auto header = m_hash->verticalHeader();
        auto total = int(CryptographicHash::Algorithm::NumAlgorithms);
        for (int i = 0; i < total; ++i) {
            header->hideSection(i);
        }
        for (auto &c : cs) {
            header->showSection(c);
        }

        m_hashtable->raise();
    }
}

void MainWindow::on_fileInfo() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    FileInfoDialog d(editor);
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
    if (!hexeditor->Paste(true)) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("copyhex")),
                     tr("PasteFailedNote"));
    }
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
            doc->beginMarco(QStringLiteral("[H*?G] {cnt: %1}").arg(total));
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
    doc->beginMarco(QStringLiteral("[H*0G] {cnt: %1}").arg(total));
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
        rmOff.reserve(total);
        for (int i = 0; i < total; ++i) {
            rmOff.append(doc->bookMarkRange(cursor->selectionStart(i).offset(),
                                            cursor->selectionEnd(i).offset()));
        }

        if (!rmOff.isEmpty()) {
            doc->beginMarco(QStringLiteral("[B-G] {cnt: %1}").arg(total));
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
                if (total == 1) {
                    auto begin = cur->selectionStart(0).offset();
                    auto end = cur->selectionEnd(0).offset();
                    meta->Metadata(begin, end, m.foreGroundColor(),
                                   m.backGroundColor(), m.comment());
                } else {
                    meta->beginMarco(
                        QStringLiteral("[M+G] {cnt: %1}").arg(total));
                    for (int i = 0; i < total; ++i) {
                        auto begin = cur->selectionStart(i).offset();
                        auto end = cur->selectionEnd(i).offset();
                        meta->Metadata(begin, end, m.foreGroundColor(),
                                       m.backGroundColor(), m.comment());
                    }
                    meta->endMarco();
                }
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
    _findResultModel->clear();
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

    auto findresitem = editor->findResult();

    auto c = findresitem.results.size();
    if (c == 0) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("export")),
                     tr("EmptyFindResult"));
        return;
    }
    auto filename = WingFileDialog::getSaveFileName(
        this, tr("ChooseSaveFile"), m_lastusedpath, {"Json (*.json)"});
    if (filename.isEmpty())
        return;
    m_lastusedpath = Utilities::getAbsoluteDirPath(filename);

    QFile f(filename);
    if (f.open(QFile::WriteOnly)) {
        QJsonObject fobj;
        fobj.insert(QStringLiteral("file"), editor->fileNameUrl().url());
        fobj.insert(QStringLiteral("encoding"), findresitem.encoding);

        auto d = findresitem.lastFindData;

        fobj.insert(QStringLiteral("find"), d.first);
        QJsonArray arr;
        for (int i = 0; i < c; i++) {
            auto data = findresitem.results.at(i);
            QJsonObject jobj;
            jobj.insert(QStringLiteral("line"), QString::number(data.line));
            jobj.insert(QStringLiteral("col"), QString::number(data.col));
            jobj.insert(QStringLiteral("offset"), QString::number(data.offset));
            jobj.insert(QStringLiteral("range"), findresitem.generateRange(i));
            jobj.insert(QStringLiteral("decoding"),
                        findresitem.generateDecoding(i));
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

    updateNumberTable(false);

    auto cursor = hexeditor->cursor();

    PluginSystem::instance().dispatchEvent(
        IWingPlugin::RegisteredEvent::CursorPositionChanged,
        {QVariant::fromValue(cursor->position())});
}

void MainWindow::on_selectionChanged() {
    on_locChanged();
    auto pair = updateStringDec();
    PluginSystem::instance().dispatchEvent(
        IWingPlugin::RegisteredEvent::SelectionChanged,
        {QVariant::fromValue(pair.first), pair.second});
}

void MainWindow::on_editableAreaClicked(int area) {
    if (area == QHexRenderer::AsciiArea) {
        _editArea->setText(QStringLiteral("ASCII"));
    } else {
        _editArea->setText(QStringLiteral("HEX"));
    }
}

void MainWindow::on_viewtxt() {
    auto editor = currentEditor();
    if (editor == nullptr) {
        return;
    }
    auto hexeditor = editor->hexEditor();
    QMimeDatabase db;
    auto mime = db.mimeTypeForData(hexeditor->document()->buffer()->ioDevice());
    auto ret = Utilities::isTextFile(mime);
    if (!ret) {
        auto ret = WingMessageBox::warning(
            this, tr("Warn"), tr("NoTextFileMayInvalid"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret == QMessageBox::No) {
            return;
        }
    }
    if (_showtxt == nullptr) {
        _showtxt = new ShowTextDialog(this);
        connect(_showtxt, &ShowTextDialog::destroyed, this,
                [this]() { _showtxt = nullptr; });
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
    while (true) {
        bool ok;
        auto fileID =
            WingInputDialog::getText(this, tr("SaveLayout"), tr("PleaseInput"),
                                     QLineEdit::Normal, {}, &ok);
        if (ok) {
            fileID = fileID.trimmed();
            auto &lm = LayoutManager::instance();
            if (lm.contains(fileID)) {
                WingMessageBox::critical(this, tr("SaveLayout"),
                                         tr("DupLayoutID"));
                continue;
            }

            auto layoutDir = LayoutManager::layoutDir();
            QFile f(layoutDir.absoluteFilePath(fileID + suffix));
            if (f.open(QFile::WriteOnly)) {
                auto layout = m_dock->saveState();
                f.write(layout);
                f.close();

                // append saved layout
                lm.addLayout(fileID, layout);

                auto menu =
                    m_toolBtneditors[ToolButtonIndex::LAYOUT_ACTION]->menu();
                Q_ASSERT(menu);
                auto actions = menu->actions();
                if (actions.size() <= 1) {
                    menu->addSeparator();
                }

                menu->addAction(newAction(
                    QStringLiteral("layout"), fileID, [this, layout]() {
                        showStatus(tr("LayoutRestoring..."));
                        updateUI();
                        restoreLayout(layout);
                        showStatus({});
                    }));

                Toast::toast(this, NAMEICONRES(QStringLiteral("layoutexport")),
                             tr("SaveLayoutSuccess"));
            } else {
                Toast::toast(this, NAMEICONRES(QStringLiteral("layoutexport")),
                             tr("SaveLayoutError"));
            }
        }
        break;
    }
}

void MainWindow::on_delLayout() {
    auto &lm = LayoutManager::instance();
    if (lm.layoutCount() == 0) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("save")),
                     tr("NothingToRemove"));
        return;
    }

    LayoutDelDialog d(lm.layoutNames(), this);
    bool ok = true;
    if (d.exec()) {
        auto dir = lm.layoutDir();
        auto menu = m_toolBtneditors[ToolButtonIndex::LAYOUT_ACTION]->menu();
        Q_ASSERT(menu);
        auto actions = menu->actions();
        for (auto &l : d.getResult()) {
            auto r = dir.remove(l + QStringLiteral(".wing-layout"));
            if (r) {
                auto r = std::find_if(
                    actions.begin(), actions.end(),
                    [l](const QAction *a) { return a->text() == l; });
                if (r != actions.end()) {
                    auto a = *r;
                    menu->removeAction(a);
                    a->deleteLater();
                }
            }
            ok &= r;
        }
        if (ok) {
            Toast::toast(this, NAMEICONRES(QStringLiteral("layoutdel")),
                         tr("DelLayoutSuccess"));
        } else {
            Toast::toast(this, NAMEICONRES(QStringLiteral("layoutdel")),
                         tr("DelLayoutError"));
        }
    }
}

void MainWindow::on_exportlog() {
    showStatus(tr("LogExporting..."));
    QScopeGuard g([this]() { showStatus({}); });

    auto fileName = WingFileDialog::getSaveFileName(
        this, tr("ExportLog"), m_lastusedpath, tr("Logs (*.log)"));
    if (!fileName.isEmpty()) {
        m_lastusedpath = Utilities::getAbsoluteDirPath(fileName);
        QFile lfile(fileName);
        if (lfile.open(QFile::WriteOnly | QFile::NewOnly | QFile::Text)) {
            auto data = m_logbrowser->toPlainText();
            lfile.write(data.toUtf8());
            lfile.close();
            Toast::toast(this, NAMEICONRES(QStringLiteral("log")),
                         tr("ExportLogSuccess"));
        } else {
            Toast::toast(this, NAMEICONRES(QStringLiteral("log")),
                         tr("ExportLogError"));
        }
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

void MainWindow::on_scriptwindow() { createScriptDialog(nullptr); }

void MainWindow::on_settingGeneral() { m_setdialog->showConfig(0); }

void MainWindow::on_settingScript() { m_scriptsetdlg->showConfig(); }

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

ads::CDockWidget *MainWindow::buildDockWidget(ads::CDockManager *dock,
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
    m_toolBtneditors.value(index)->menu()->addAction(dw->toggleViewAction());
    return dw;
}

EditorView *MainWindow::findEditorView(const QUrl &filename) {
    for (auto &p : m_views) {
        if (p->fileNameUrl() == filename) {
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
            auto icon = w->icon();
            if (icon.isNull()) {
                icon = ICONRES(QStringLiteral("win"));
            }
            editor->registerView(id, v, icon);
        }
    }

    for (auto &m : _scriptContexts) {
        editor->registerQMenu(m);
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
                if (ret == WingHex::Success) {
                    closeEditor(editor, false);
                } else {
                    if (ret == WingHex::WorkSpaceUnSaved) {
                        auto btn = WingMessageBox::critical(
                            this, tr("Error"), tr("UnknownErrorSure2Quit"),
                            QMessageBox::Yes | QMessageBox::No);
                        if (btn == QMessageBox::Yes) {
                            closeEditor(editor, true);
                        }
                    } else {
                        auto btn = WingMessageBox::critical(
                            this, tr("Error"), tr("FilePermissionSure2Quit"),
                            QMessageBox::Yes | QMessageBox::No);
                        if (btn == QMessageBox::Yes) {
                            closeEditor(editor, true);
                        }
                    }
                }
            } else {
                closeEditor(editor, true);
            }
        }

        if (m_views.isEmpty()) {
            updateEditModeEnabled();
        }
    });

    m_views.append(editor);
    auto ev = m_toolBtneditors.value(ToolButtonIndex::EDITOR_VIEWS);
    auto menu = ev->menu();
    Q_ASSERT(menu);
    auto ta = editor->toggleViewAction();
    menu->addAction(ta);
    ev->setEnabled(true);

    PluginSystem::instance().dispatchEvent(
        IWingPlugin::RegisteredEvent::FileOpened,
        {editor->fileNameUrl(),
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

    connect(editor, &EditorView::closeRequested, this, [this, editor]() {
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
    connect(editor, &EditorView::sigOnCheckSum, this, &MainWindow::on_checksum);
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
                    if (total == 1) {
                        auto begin = cur->selectionStart(0).offset();
                        auto end = cur->selectionEnd(0).offset();
                        meta->Metadata(begin, end, m.foreGroundColor(),
                                       m.backGroundColor(), m.comment());
                    } else {
                        meta->beginMarco(
                            QStringLiteral("[M+G] {cnt: %1}").arg(total));
                        for (int i = 0; i < total; ++i) {
                            auto begin = cur->selectionStart(i).offset();
                            auto end = cur->selectionEnd(i).offset();
                            meta->Metadata(begin, end, m.foreGroundColor(),
                                           m.backGroundColor(), m.comment());
                        }
                        meta->endMarco();
                    }
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

    if (!m_curConnections.isEmpty()) {
        for (auto &c : m_curConnections) {
            disconnect(c);
        }
        m_curConnections.clear();
    }

    if (old) {
        auto &dbp = old->scrollPoints();
        dbp[int(EditorView::ScrollPoint::FindResult)] =
            QPoint(m_findresult->horizontalScrollBar()->value(),
                   m_findresult->verticalScrollBar()->value());
        dbp[int(EditorView::ScrollPoint::CheckSum)] =
            QPoint(m_hash->horizontalScrollBar()->value(),
                   m_hash->verticalScrollBar()->value());
        dbp[int(EditorView::ScrollPoint::UndoStack)] =
            QPoint(_undoView->horizontalScrollBar()->value(),
                   _undoView->verticalScrollBar()->value());
        dbp[int(EditorView::ScrollPoint::BookMark)] =
            QPoint(m_bookMark->horizontalScrollBar()->value(),
                   m_bookMark->verticalScrollBar()->value());
        dbp[int(EditorView::ScrollPoint::MetaData)] =
            QPoint(m_metadata->horizontalScrollBar()->value(),
                   m_metadata->verticalScrollBar()->value());
        dbp[int(EditorView::ScrollPoint::DecodeStr)] =
            QPoint(m_txtDecode->horizontalScrollBar()->value(),
                   m_txtDecode->verticalScrollBar()->value());
    }

    if (cur == nullptr) {
        return;
    }

    _editorLock.lockForWrite();
    auto hexeditor = cur->hexEditor();
    m_curConnections << connect(hexeditor, &QHexView::cursorLocationChanged,
                                this, &MainWindow::on_locChanged);
    m_curConnections << connect(hexeditor, &QHexView::cursorSelectionChanged,
                                this, &MainWindow::on_selectionChanged);
    m_curConnections << connect(hexeditor, &QHexView::editableAreaClicked, this,
                                &MainWindow::on_editableAreaClicked);
    m_curConnections << connect(
        hexeditor, &QHexView::canUndoChanged, this, [this](bool b) {
            m_toolBtneditors[ToolButtonIndex::UNDO_ACTION]->setEnabled(b);
        });
    m_curConnections << connect(
        hexeditor, &QHexView::canRedoChanged, this, [this](bool b) {
            m_toolBtneditors[ToolButtonIndex::REDO_ACTION]->setEnabled(b);
        });
    m_curConnections << connect(
        hexeditor, &QHexView::documentSaved, this, [this](bool b) {
            m_iSaved->setIcon(b ? _infoSaved : _infoUnsaved);
            m_sSaved->setIcon(b ? _infoSaved : _infoUnsaved);
            if (b) {
                auto cur = currentEditor();
                if (cur) {
                    auto fn = cur->fileNameUrl();
                    m_sSaved->setToolTip(fn.url());
                    updateWindowTitle(cur);
                }
            }
        });
    m_curConnections << connect(
        hexeditor, &QHexView::documentKeepSize, this, [this](bool b) {
            m_iCanOver->setIcon(b ? _infoCannotOver : _infoCanOver);
            m_sCanOver->setIcon(b ? _infoCannotOver : _infoCanOver);
        });
    m_curConnections << connect(
        hexeditor, &QHexView::documentLockedFile, this, [this](bool b) {
            m_iLocked->setIcon(b ? _infoLock : _infoUnLock);
            m_sLocked->setIcon(b ? _infoLock : _infoUnLock);
        });
    m_curConnections << connect(
        hexeditor, &QHexView::copyLimitRaised, this, [this] {
            Toast::toast(this, NAMEICONRES(QStringLiteral("copy")),
                         tr("CopyLimit"));
        });
    m_curConnections << connect(
        hexeditor, &QHexView::scaleRateChanged, this, [this] {
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
    m_curConnections << connect(doc, &QHexDocument::metabgVisibleChanged,
                                m_aShowMetabg, &QAction::setChecked);
    m_curConnections << connect(doc, &QHexDocument::metafgVisibleChanged,
                                m_aShowMetafg, &QAction::setChecked);
    m_curConnections << connect(doc, &QHexDocument::metaCommentVisibleChanged,
                                m_aShowMetaComment, &QAction::setChecked);

    auto icon = hexeditor->isReadOnly() ? _infoReadonly : _infoWriteable;
    m_iReadWrite->setIcon(icon);
    m_sReadWrite->setIcon(icon);

    loadFindResult(cur);
    _bookMarkModel->setDocument(doc);
    _metadataModel->setDocument(doc);

    auto header = m_hash->verticalHeader();
    const auto &headerV = cur->checkSumVisible();
    auto total = headerV.size();
    for (int i = 0; i < total; ++i) {
        header->setSectionHidden(i, !headerV[i]);
    }
    _hashModel->updateCheckSumData(cur->checkSumResult());

    for (auto &menu : m_hexContextMenu) {
        menu->setProperty("__CONTEXT__", quintptr(cur->editorContext()));
    }

    _undoView->setStack(doc->undoStack());

    m_curEditor = cur;
    hexeditor->getStatus();
    updateNumberTable(true);
    updateStringDec();

    // reload new scroll offset
    QTimer::singleShot(0, [this, cur]() {
        auto &dbp = cur->scrollPoints();
        auto p = dbp[int(EditorView::ScrollPoint::FindResult)];
        m_findresult->horizontalScrollBar()->setValue(p.x());
        m_findresult->verticalScrollBar()->setValue(p.y());

        p = dbp[int(EditorView::ScrollPoint::CheckSum)];
        m_hash->horizontalScrollBar()->setValue(p.x());
        m_hash->verticalScrollBar()->setValue(p.y());

        p = dbp[int(EditorView::ScrollPoint::UndoStack)];
        _undoView->horizontalScrollBar()->setValue(p.x());
        _undoView->verticalScrollBar()->setValue(p.y());

        p = dbp[int(EditorView::ScrollPoint::BookMark)];
        m_bookMark->horizontalScrollBar()->setValue(p.x());
        m_bookMark->verticalScrollBar()->setValue(p.y());

        p = dbp[int(EditorView::ScrollPoint::MetaData)];
        m_metadata->horizontalScrollBar()->setValue(p.x());
        m_metadata->verticalScrollBar()->setValue(p.y());

        p = dbp[int(EditorView::ScrollPoint::DecodeStr)];
        m_txtDecode->horizontalScrollBar()->setValue(p.x());
        m_txtDecode->verticalScrollBar()->setValue(p.y());
    });

    PluginSystem::instance().dispatchEvent(
        IWingPlugin::RegisteredEvent::FileSwitched,
        {cur->fileNameUrl(), (old ? old->fileNameUrl() : QUrl())});

    _editorLock.unlock();
}

void MainWindow::updateWindowTitle(EditorView *view) {
    static auto title = tr("WingHexExplorer");
    QString path;
    if (view) {
        auto fn = view->fileNameUrl();
        if (view->isExtensionFile()) {
            path = fn.authority() + fn.path();
        } else {
            path = fn.fileName();
        }

        auto idx = view->cloneIndex();
        if (idx >= 0) {
            path += QString::number(idx + 1)
                        .prepend(QStringLiteral(" ["))
                        .append(']');
        }
    }
    if (path.isEmpty()) {
        this->setWindowTitle(title);
    } else {
        this->setWindowTitle(title + QStringLiteral(" - ") + path);
    }
}

void MainWindow::loadFindResult(EditorView *view) {
    auto &result = view->findResult();
    _findResultModel->updateData(result);
    m_findEncoding.value(result.encoding)->setChecked(true);
}

void MainWindow::openFiles(const QStringList &files) {
    QStringList errof;
    for (auto &file : files) {
        bool isWs;
        if (AppManager::instance()->openFile(file, true, true, &isWs)) {
            errof.append(file);
        } else {
            addRecentFile(file, isWs);
            m_lastusedpath = Utilities::getAbsoluteDirPath(file);
        }
    }

    show();
    raise();
    activateWindow();

    if (!errof.isEmpty()) {
        WingMessageBox::critical(this, tr("Error"),
                                 tr("ErrOpenFileBelow") + QStringLiteral("\n") +
                                     errof.join('\n'));
    }
}

ErrFile MainWindow::openFile(const QString &file, EditorView **editor) {
    auto e = findEditorView(QUrl::fromLocalFile(file));
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
    auto e = findEditorView(Utilities::getDeviceFileName(ext, file));
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
    auto wsDoc = WorkSpaceManager::loadWorkSpace(file);
    if (wsDoc.isEmpty()) {
        return ErrFile::InvalidFormat;
    }
    auto fName = WorkSpaceManager::loadWorkSpaceDocFile(file, wsDoc);
    if (!fName.isValid()) {
        return ErrFile::InvalidFormat;
    }
    for (auto &p : m_views) {
        if (p->fileNameUrl() == fName) {
            if (editor) {
                *editor = p;
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
    auto res = ev->openWorkSpace(filename, wsDoc);

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

    auto url = editor->fileNameUrl();
    QString newName;
    if (filename.isEmpty()) {
        if (url.isLocalFile()) {
            if (!EditorView::isNewFileUrl(url)) {
                auto oldName = url.toLocalFile();
                newName = oldName;
            }
        } else {
            static QRegularExpression regex(QStringLiteral("[/*?:<>|\"\\\\]"));
            newName = (url.authority() + url.path())
                          .replace(regex, QStringLiteral("_"));
            if (!m_lastusedpath.isEmpty()) {
                newName.prepend(QDir::separator()).prepend(m_lastusedpath);
            }
        }
    } else {
        newName = filename;
    }

    QString workspace = editor->workSpaceName();
    if (forceWorkspace || (workspace.isEmpty() && editor->change2WorkSpace())) {
        QString curFile = newName + PROEXT;
        auto wsfile = getWorkSpaceFileName(curFile);
        if (wsfile.isEmpty()) {
            return ErrFile::WorkSpaceUnSaved;
        }
        workspace = wsfile;
    }

    if (ws) {
        *ws = workspace;
    }

    auto ret = editor->save(workspace, filename, isExport,
                            forceWorkspace
                                ? EditorView::SaveWorkSpaceAttr::ForceWorkSpace
                                : EditorView::SaveWorkSpaceAttr::AutoWorkSpace);
    if (ret == ErrFile::Success) {
        if (editor->isCloneFile()) {
            editor = editor->cloneParent();
        }
        PluginSystem::instance().dispatchEvent(
            IWingPlugin::RegisteredEvent::FileSaved,
            {QUrl::fromLocalFile(newName), url, isExport,
             QVariant::fromValue(getEditorViewFileType(editor))});
    }
    return ret;
}

ErrFile MainWindow::closeEditor(EditorView *editor, bool force) {
    if (editor == nullptr) {
        return ErrFile::Error;
    }
    if (editor->isCloneFile()) {
        editor->requestCloseDockWidget(); // must be success
        return ErrFile::Success;
    }

    if (!force) {
        if (!editor->isSaved()) {
            return ErrFile::UnSaved;
        }
    }
    auto cret = editor->closeFile();
    if (cret != ErrFile::Success && !force) {
        return cret;
    }

    m_views.remove(editor);
    if (currentEditor() == editor) {
        _editorLock.lockForWrite();
        m_curEditor = nullptr;
        _editorLock.unlock();
    }

    auto fileName = editor->fileNameUrl();
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

void MainWindow::openScriptFile(const QString &filename, SplashDialog *splash) {
    createScriptDialog(splash);
    m_scriptDialog->openFile(filename);
    QTimer::singleShot(100, this, [this] { m_scriptDialog->raise(); });
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
        updateWindowTitle(editor);
    } else {
        for (auto &menu : m_hexContextMenu) {
            menu->setProperty("__CONTEXT__", {});
        }
        _findResultModel->reset();
        _bookMarkModel->setDocument(nullptr);
        _metadataModel->setDocument(nullptr);
        _hashModel->clearData();
        m_txtDecode->clear();
        m_aDelBookMark->setEnabled(false);
        m_aDelMetaData->setEnabled(false);
        _undoView->setStack(nullptr);

        m_lblloc->setText(QStringLiteral("(0,0)"));
        m_lblsellen->setText(QStringLiteral("0 - 0x0"));
        _numsitem->clear();
        on_editableAreaClicked(-1);
        updateWindowTitle(nullptr);
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
            for (auto &ev : m_views) {
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
        QJsonArray jsonData;

        // Add header row
        QStringList headers;
        for (int col = 0; col < model->columnCount(); ++col) {
            headers.append(model->headerData(col, Qt::Horizontal, Qt::UserRole)
                               .toString());
        }

        // Add data rows
        for (int row = 0; row < model->rowCount(); ++row) {
            QJsonObject rowData;
            for (int col = 0; col < model->columnCount(); ++col) {
                QModelIndex index = model->index(row, col);
                rowData.insert(headers.at(col), model->data(index).toString());
            }
            jsonData.append(rowData);
        }

        // Create JSON document
        QJsonDocument jsonDocument(jsonData);

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
            auto content =
                model->headerData(col, Qt::Horizontal, Qt::UserRole).toString();
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

void MainWindow::updateNumberTable(bool force) {
    if (!m_numshowtable->isVisible()) {
        return;
    }

    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return;
    }

    // lazy loading
    static QHexView *lastView = nullptr;
    static qsizetype lastOff = -1;
    auto off = hexeditor->currentOffset();
    if (!force && lastView == hexeditor && lastOff == off) {
        return;
    } else {
        lastView = hexeditor;
        lastOff = off;
    }

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
        if (m_islittle) {
            { // utf16
                QStringDecoder dec(QStringDecoder::Encoding::Utf16LE);
                QString data = dec.decode(tmp);
                _numsitem->setNumData(NumShowModel::NumTableIndex::UTF16_STR,
                                      data);
            }

            { // utf32
                QStringDecoder dec(QStringDecoder::Encoding::Utf32LE);
                QString data = dec.decode(tmp);
                _numsitem->setNumData(NumShowModel::NumTableIndex::UTF32_STR,
                                      data);
            }
        } else {
            { // utf16
                QStringDecoder dec(QStringDecoder::Encoding::Utf16BE);
                QString data = dec.decode(tmp);
                _numsitem->setNumData(NumShowModel::NumTableIndex::UTF16_STR,
                                      data);
            }

            { // utf32
                QStringDecoder dec(QStringDecoder::Encoding::Utf32BE);
                QString data = dec.decode(tmp);
                _numsitem->setNumData(NumShowModel::NumTableIndex::UTF32_STR,
                                      data);
            }
        }
    } else {
        _numsitem->setNumData(NumShowModel::NumTableIndex::ASCII_STR,
                              QString());
        _numsitem->setNumData(NumShowModel::NumTableIndex::UTF8_STR, QString());
        _numsitem->setNumData(NumShowModel::NumTableIndex::UTF16_STR,
                              QString());
        _numsitem->setNumData(NumShowModel::NumTableIndex::UTF32_STR,
                              QString());
    }
}

QPair<QByteArrayList, bool> MainWindow::updateStringDec() {
    if (!m_txtDecode->isVisible()) {
        return {};
    }

    auto hexeditor = currentHexView();
    if (hexeditor == nullptr) {
        return {};
    }

    m_txtDecode->clear();

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
    for (int i = 0; i < total; i++) {
        auto b = buffer.at(i);
        if (!isPreview) {
            m_txtDecode->insertHtml(
                QStringLiteral("<font color=\"gold\">[ %1 / %2 ]</font><br />")
                    .arg(i + 1)
                    .arg(total));
        }

        if (buffer.length() <= 1024 * 1024 * _decstrlim) {
            m_txtDecode->insertPlainText(
                Utilities::decodingString(b, m_encoding));
            m_txtDecode->insertPlainText(QStringLiteral("\n"));
        } else {
            m_txtDecode->insertHtml(
                QStringLiteral("<font color=\"red\">%1</font><br />")
                    .arg(tr("TooManyBytesDecode")));
        }
    }

    return qMakePair(buffer, isPreview);
}

void MainWindow::updateUI() { QApplication::processEvents(); }

void MainWindow::createScriptDialog(SplashDialog *d) {
    if (m_scriptDialog == nullptr) {
        auto splash = d ? d : new SplashDialog(this);
        splash->setInfoText(tr("SetupScriptEditor"));

        m_scriptDialog = new ScriptingDialog(m_scriptsetdlg, this);
        connect(m_scriptDialog, &ScriptingDialog::destroyed, this,
                [this]() { m_scriptDialog = nullptr; });
        m_scriptDialog->initConsole();

        splash->close();
    }
    m_scriptDialog->show();
    m_scriptDialog->raise();
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

bool MainWindow::reportErrFileError(ErrFile err, const QPixmap &toastIcon,
                                    const QString &okMsg,
                                    const QString &errMsg) {
    switch (err) {
    case WingHex::UnSaved:
    case ErrFile::WorkSpaceUnSaved:
        // these flags are no need to report with GUI
        break;
    case ErrFile::Success: {
        if (!okMsg.isEmpty()) {
            Toast::toast(this, toastIcon, okMsg);
        }
    } break;
    case WingHex::AlreadyOpened: {
        Toast::toast(this, NAMEICONRES(QStringLiteral("file")),
                     tr("AlreadyOpened"));
    } break;
    case WingHex::DevNotFound: {
        WingMessageBox::critical(this, tr("Error"), tr("DevNotFound"));
    } break;
    case WingHex::Permission: {
        WingMessageBox::critical(this, tr("Error"), tr("FilePermission"));
    } break;
    case WingHex::NotExist: {
        WingMessageBox::critical(this, tr("Error"), tr("FileNotExist"));
    } break;
    case WingHex::IsNewFile: {
        WingMessageBox::critical(this, tr("Error"), tr("SaveWSError"));
    } break;
    case WingHex::InvalidFormat: {
        WingMessageBox::critical(this, tr("Error"), tr("FileInvalidFmt"));
    } break;
    case WingHex::TooManyOpenedFile: {
        WingMessageBox::critical(this, tr("Error"), tr("TooManyOpenedFile"));
    } break;
    case WingHex::NotAllowedInNoneGUIThread:
    case WingHex::ClonedFile:
    case WingHex::Error: {
        auto e = QMetaEnum::fromType<ErrFile>();
        if (errMsg.isEmpty()) {
            WingMessageBox::critical(
                this, tr("Error"),
                tr("UnknownError") + QStringLiteral(" - ") +
                    QString::fromLatin1(e.valueToKey(err)));
        } else {
            WingMessageBox::critical(
                this, tr("Error"),
                errMsg + QStringLiteral(" - ") +
                    QString::fromLatin1(e.valueToKey(err)));
        }
        break;
    } break;
    }

    return err == WingHex::Success;
}

void MainWindow::addRecentFile(const QString &fileName, bool isWorkspace) {
    RecentFileManager::RecentInfo info;
    info.url = QUrl::fromLocalFile(fileName);
    info.isWorkSpace = isWorkspace;
    m_recentmanager->addRecentFile(info);
}

ads::CDockAreaWidget *MainWindow::editorViewArea() const {
    return m_dock->centralWidget()->dockAreaWidget();
}

void MainWindow::onOutputBgScriptOutput(
    const ScriptMachine::MessageInfo &message) {
    auto doc = m_bgScriptOutput->document();
    auto lastLine = doc->lastBlock();
    // each empty block has a QChar::ParagraphSeparator
    auto isNotBlockStart = lastLine.length() > 1;

    auto cursor = m_bgScriptOutput->textCursor();
    cursor.movePosition(QTextCursor::End);

    auto fmtMsg = [](const ScriptMachine::MessageInfo &message) -> QString {
        if (message.row <= 0 || message.col <= 0) {
            return message.message;
        } else {
            return QStringLiteral("(") + QString::number(message.row) +
                   QStringLiteral(", ") + QString::number(message.col) +
                   QStringLiteral(") ") + message.message;
        }
    };

    switch (message.type) {
    case ScriptMachine::MessageType::ExecInfo:
    case ScriptMachine::MessageType::Info:
        if (isNotBlockStart) {
            cursor.insertBlock();
        }
        cursor.insertText(tr("[Info]") + fmtMsg(message), {});
        break;
    case ScriptMachine::MessageType::Warn: {
        if (isNotBlockStart) {
            m_bgScriptOutput->appendPlainText({});
        }
        auto fmt = cursor.charFormat();
        fmt.setForeground(QColorConstants::Svg::gold);
        cursor.insertText(tr("[Warn]") + fmtMsg(message), fmt);
    } break;
    case ScriptMachine::MessageType::Error: {
        if (isNotBlockStart) {
            cursor.insertBlock();
        }
        auto fmt = cursor.charFormat();
        fmt.setForeground(Qt::red);
        cursor.insertText(tr("[Error]") + fmtMsg(message), fmt);
    } break;
    case ScriptMachine::MessageType::Print:
        if (isNotBlockStart && _lastOutputType != message.type) {
            cursor.insertBlock();
        }
        cursor.insertText(message.message, {});
        break;
    case ScriptMachine::MessageType::Unknown:
        break;
    }

    _lastOutputType = message.type;
    if (message.type != ScriptMachine::MessageType::ExecInfo) {
        if (!m_bgScriptOutput->isVisible()) {
            _hlAnim->start();
        }
    }
}

void MainWindow::restoreLayout(const QByteArray &layout) {
    showStatus(tr("LayoutRestoring..."));
    updateUI();

    if (m_views.isEmpty()) {
        m_dock->restoreState(layout);
    } else {
        auto curEditor = m_curEditor;
        if (m_views.size() > 1) {
            auto notSameContainer = std::any_of(
                m_views.begin(), m_views.end(), [curEditor](EditorView *view) {
                    return curEditor->dockAreaWidget() !=
                           view->dockAreaWidget();
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
        QVector<EditorView *> hiddenView;
        for (auto &view : m_views) {
            if (view->isClosed()) {
                hiddenView.append(view);
            }
            m_dock->removeDockWidget(view);
        }

        m_dock->restoreState(layout);

        // add back
        auto centeralWidget = m_dock->centralWidget();
        auto area = centeralWidget->dockAreaWidget();
        for (auto &view : m_views) {
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

    showStatus({});
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // plugin first checking
    auto closing = PluginSystem::instance().dispatchEvent(
        IWingPlugin::RegisteredEvent::AppClosing, {});
    if (!closing) {
        event->ignore();
        return;
    }

    auto &sm = ScriptMachine::instance();

    // then checking the scripting dialog
    if (m_scriptDialog) {
        if (!m_scriptDialog->about2Close()) {
            event->ignore();
            return;
        }

        sm.abortDbgScript();
        m_scriptDialog->close();
    }

    if (sm.isRunning(ScriptMachine::Interactive) ||
        sm.isRunning(ScriptMachine::Background)) {
        auto ret =
            WingMessageBox::warning(this, qAppName(), tr("ScriptStillRunning"),
                                    QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::No) {
            event->ignore();
            return;
        }
        sm.abortScript(ScriptMachine::Interactive);
        sm.abortScript(ScriptMachine::Background);
    }

    // then checking itself
    if (!m_views.isEmpty()) {
        bool unSavedFiles = false;
        QList<EditorView *> need2CloseView;
        for (auto &editor : m_views) {
            bool saved = editor->isSaved();
            if (saved) {
                need2CloseView << editor;
            } else {
                unSavedFiles = true;
            }
        }

        for (auto &view : need2CloseView) {
            closeEditor(view, true);
        }

        if (unSavedFiles) {
            auto ret = WingMessageBox::warning(
                this, qAppName(), tr("ConfirmSave"),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            if (ret == QMessageBox::Yes) {
                for (auto &view : m_views) {
                    view->save(view->workSpaceName());
                    closeEditor(view, true);
                }

                if (!m_views.isEmpty()) {
                    event->ignore();
                    return;
                }
            } else if (ret == QMessageBox::No) {
                for (auto &p : m_views) {
                    closeEditor(p, true);
                }
            } else {
                event->ignore();
                return;
            }
        }
    }

    auto &lsp = AngelLsp::instance();
    lsp.blockSignals(true);
    lsp.shutdownAndExit();

    auto &set = SettingManager::instance();
    set.setDockLayout(m_dock->saveState());
    set.setRecentFiles(m_recentmanager->saveRecent());
    set.setLastUsedPath(m_lastusedpath);

    PluginSystem::instance().destory();

    FramelessMainWindow::closeEvent(event);
    Q_EMIT closed();
}

void MainWindow::showEvent(QShowEvent *event) {
    FramelessMainWindow::showEvent(event);
    static bool firstInit = true;
    if (firstInit) {
        if (_showEvents) {
            _showEvents();
            _showEvents = {};
        }

        connect(
            m_findresult->horizontalHeader(), &QHeaderView::sectionResized,
            this,
            [this]() {
                QTimer::singleShot(0, [this]() {
                    auto header = m_findresult->horizontalHeader();
                    header->setSectionResizeMode(QHeaderView::Interactive);
                });
            },
            Qt::SingleShotConnection);

        PluginSystem::instance().dispatchEvent(
            IWingPlugin::RegisteredEvent::AppReady, {});
        firstInit = false;
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->modifiers() == Qt::NoModifier) {
        auto key = event->button();
        if (key == Qt::XButton1 || key == Qt::XButton2) {
            auto view = currentEditor();
            if (view) {
                view->switchViewStackLoop(key == Qt::XButton2);
            }
            event->ignore();
        }
    }
    FramelessMainWindow::mousePressEvent(event);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    Q_UNUSED(watched);
    if (isVisible()) {
        switch (event->type()) {
        case QEvent::Shortcut: {
            auto e = reinterpret_cast<QShortcutEvent *>(event);
            if (m_scriptConsole) {
                if (m_scriptConsole->hasFocus()) {
                    auto k = e->key()[0];
                    QKeyEvent ev(QEvent::KeyPress, k.key(),
                                 k.keyboardModifiers());
                    m_scriptConsole->processKeyEvent(&ev);
                    return true;
                }
            }
        } break;
        case QEvent::Show: {
            if (watched == m_bgScriptOutput) {
                _hlAnim->stop();
            } else if (watched == m_scriptConsole) {
                m_console->setIcon({});
            } else if (watched == m_numshowtable) {
                updateNumberTable(false);
            } else if (watched == m_txtDecode) {
                updateStringDec();
            }
        } break;
        case QEvent::Hide: {
            if (watched == m_scriptConsole) {
                if (ScriptMachine::instance().isRunning(
                        ScriptMachine::Interactive)) {
                    m_console->setIcon(ICONRES(QStringLiteral("dbgrun")));
                }
            }
        } break;
        case QEvent::MouseButtonDblClick: {
            if (watched == m_sLocked) {
                if (m_sLocked->isEnabled()) {
                    m_iLocked->click();
                }
            } else if (watched == m_sCanOver) {
                if (m_sCanOver->isEnabled()) {
                    m_iCanOver->click();
                }
            }
        } break;
        default:
            break;
        }
    }
    return FramelessMainWindow::eventFilter(watched, event);
}
