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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dialog/splashdialog.h"
#include "framelessmainwindow.h"

#include <QListView>
#include <QMainWindow>
#include <QMap>
#include <QPixmap>
#include <QReadWriteLock>
#include <QShortcut>
#include <QTableView>
#include <QTextBrowser>
#include <QToolButton>
#include <QTreeView>
#include <QtConcurrent/QtConcurrentRun>

#include "QWingRibbon/ribbon.h"
#include "QWingRibbon/ribbonbuttongroup.h"
#include "QWingRibbon/ribbontabcontent.h"
#include "Qt-Advanced-Docking-System/src/DockManager.h"
#include "Qt-Advanced-Docking-System/src/DockWidget.h"
#include "class/clickcallback.h"
#include "class/recentfilemanager.h"
#include "class/scriptmanager.h"
#include "class/wingprogressdialog.h"
#include "control/editorview.h"
#include "control/qtableviewext.h"
#include "control/scriptingconsole.h"
#include "model/bookmarksmodel.h"
#include "model/checksummodel.h"
#include "model/metadatamodel.h"
#include "model/numshowmodel.h"
#include "plugin/iwingplugin.h"
#include "scriptingdialog.h"
#include "settingdialog.h"
#include "showtextdialog.h"
#include "utilities.h"

class PluginSystem;

class MainWindow : public FramelessMainWindow {
    Q_OBJECT

    friend class PluginSystem;

    using ClickedCallBack = ClickCallBack;
    using DblClickedCallBack = ClickCallBack;

public:
    explicit MainWindow(SplashDialog *splash);
    virtual ~MainWindow() override;

private:
    enum PageIndex {
        PAGE_FILE,
        PAGE_EDIT,
        PAGE_VIEW,
        PAGE_PLUGIN,
        PAGE_SETTING,
        PAGE_ABOUT
    };

    enum ToolButtonIndex : uint {
        EDITOR_VIEWS,
        TOOL_VIEWS,
        PLUGIN_VIEWS,
        EDITOR_WINS,
        UNDO_ACTION,
        REDO_ACTION,
        OPEN_EXT,
        LAYOUT_ACTION,
    };

private:
    void buildUpRibbonBar();
    void buildUpDockSystem(QWidget *container);
    void finishBuildDockSystem();

    ads::CDockAreaWidget *buildUpLogDock(ads::CDockManager *dock,
                                         ads::DockWidgetArea area,
                                         ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpFindResultDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                          ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpNumberShowDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                          ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpHashResultDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                          ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpHexBookMarkDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                           ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpHexMetaDataDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                           ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpDecodingStrShowDock(ads::CDockManager *dock,
                               ads::DockWidgetArea area,
                               ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpScriptConsoleDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                             ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpScriptObjShowDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                             ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpVisualDataDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                          ads::CDockAreaWidget *areaw = nullptr);

    RibbonTabContent *buildFilePage(RibbonTabContent *tab);
    RibbonTabContent *buildEditPage(RibbonTabContent *tab);
    RibbonTabContent *buildViewPage(RibbonTabContent *tab);
    RibbonTabContent *buildScriptPage(RibbonTabContent *tab);
    RibbonTabContent *buildPluginPage(RibbonTabContent *tab);
    RibbonTabContent *buildSettingPage(RibbonTabContent *tab);
    RibbonTabContent *buildAboutPage(RibbonTabContent *tab);

    void buildUpSettingDialog();
    void installPluginEditorWidgets();

    void showStatus(const QString &status);

private:
    EditorView *newfileGUI();

private slots:
    void on_newfile();
    void on_openfile();
    void on_openworkspace();
    void on_reload();

    void on_save();
    void on_convpro();
    void on_saveas();
    void on_exportfile();
    void on_savesel();

    void on_undofile();
    void on_redofile();
    void on_cutfile();
    void on_copyfile();
    void on_pastefile();
    void on_delete();
    void on_clone();

    void on_findfile();
    void on_gotoline();
    void on_encoding();
    void on_checksum();
    void on_fileInfo();

    void on_cuthex();
    void on_copyhex();
    void on_pastehex();
    void on_fill();
    void on_fillzero();

    void on_bookmark();
    void on_bookmarkdel();
    void on_bookmarkcls();

    void on_metadata();
    void on_metadataedit();
    void on_metadatadel();
    void on_metadatacls();

    void on_metadatafg(bool checked);
    void on_metadatabg(bool checked);
    void on_metadatacomment(bool checked);
    void on_metashowall();
    void on_metahideall();

    void on_clearfindresult();
    void on_exportfindresult();
    void on_locChanged();
    void on_selectionChanged();

    void on_viewtxt();
    void on_fullScreen();
    void on_saveLayout();
    void on_exportlog();
    void on_clslog();

    void on_scriptwindow();
    void on_setting_general();
    void on_setting_script();
    void on_setting_plugin();

    void on_about();
    void on_sponsor();
    void on_wiki();
    void on_update();

public:
    ErrFile openFile(const QString &file, EditorView **editor);
    ErrFile openExtFile(const QString &ext, const QString &file,
                        EditorView **editor);
    ErrFile openWorkSpace(const QString &file, EditorView **editor);

    ErrFile saveEditor(EditorView *editor, const QString &filename,
                       bool isExport = false, bool forceWorkspace = false,
                       QString *ws = nullptr);
    ErrFile closeEditor(EditorView *editor, bool force);

    EditorView *currentEditor();
    void adjustEditorFocus(EditorView *closedEditor);

    QString getWorkSpaceFileName(const QString &curFile);

    void saveTableContent(QAbstractItemModel *model);

private:
    IWingPlugin::FileType getEditorViewFileType(EditorView *view);

    QString saveLog();

    ads::CDockWidget *buildDockWidget(ads::CDockManager *dock,
                                      const QString &widgetName,
                                      const QString &displayName,
                                      QWidget *content,
                                      ToolButtonIndex index = TOOL_VIEWS);
    EditorView *findEditorView(const QString &filename);

    bool newOpenFileSafeCheck();
    void registerEditorView(EditorView *editor, const QString &ws = {});
    void registerClonedEditorView(EditorView *editor);

    void connectEditorView(EditorView *editor);
    void swapEditor(EditorView *old, EditorView *cur);

    void loadFindResult(EditorView *view);

    void openFiles(const QStringList &files);

    void updateEditModeEnabled();

    void setCurrentHexEditorScale(qreal rate);

    QHexView *currentHexView();

    void loadCacheIcon();
    QMessageBox::StandardButton saveRequest();

    inline ads::CDockAreaWidget *editorViewArea() const;

private:
    QJsonObject extractModelData(const QAbstractItemModel *model,
                                 const QModelIndex &parent = QModelIndex());

protected:
    virtual void closeEvent(QCloseEvent *event) override;

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

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
#if QT_VERSION <= QT_VERSION_CHECK(6, 6, 0)
        if (menu) {
            a->setText(title + QStringLiteral(" ▼"));
        } else
#endif
        {
            a->setText(title);
        }

        a->setIcon(icon);
        a->setToolTip(
            shortcut.isEmpty()
                ? QStringLiteral("<p align=\"center\">%1</p>").arg(title)
                : QStringLiteral(
                      "<p align=\"center\">%1</p><p align=\"center\">%2</p>")
                      .arg(title, shortcut.toString()));

        if (!shortcut.isEmpty()) {
            auto shortCut = new QShortcut(shortcut, this);
            shortCut->setContext(Qt::WindowShortcut);
            connect(shortCut, &QShortcut::activated, a, &QToolButton::click);
        }

        a->setMenu(menu);
        if (menu) {
#if QT_VERSION > QT_VERSION_CHECK(6, 6, 0)
            a->setArrowType(Qt::DownArrow);
#endif
            a->setPopupMode(QToolButton::InstantPopup);
        }
        connect(a, &QToolButton::clicked, this, slot);
        pannel->addButton(a);
        return a;
    }

    template <typename Func>
    inline QToolButton *
    addCheckablePannelAction(RibbonButtonGroup *pannel, const QString &iconName,
                             const QString &title, Func &&slot,
                             const QKeySequence &shortcut = QKeySequence()) {
        Q_ASSERT(pannel);
        auto a = new QToolButton(pannel);
        a->setText(title);
        a->setIcon(ICONRES(iconName));
        a->setCheckable(true);
        a->setChecked(false);

        if (!shortcut.isEmpty()) {
            auto shortCut = new QShortcut(shortcut, this);
            shortCut->setContext(Qt::WindowShortcut);
            connect(shortCut, &QShortcut::activated, a, &QToolButton::click);
        }

        connect(a, &QToolButton::toggled, this, slot);
        pannel->addButton(a);
        return a;
    }

    template <typename Func>
    inline QAction *
    newCheckableAction(QObject *parent, const QString &title, Func &&slot,
                       const QKeySequence &shortcut = QKeySequence()) {
        auto a = new QAction(parent);
        a->setText(title);
        a->setShortcutVisibleInContextMenu(true);
        a->setShortcut(shortcut);
        a->setCheckable(true);
        connect(a, &QAction::triggered, this, slot);
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

    template <typename Func>
    inline QAction *newAction(const QIcon &icon, const QString &title,
                              Func &&slot,
                              const QKeySequence &shortcut = QKeySequence()) {
        QAction *a = newAction(title, slot, shortcut);
        a->setIcon(icon);
        return a;
    }

    template <typename Func>
    inline QAction *newAction(const QString &iconName, const QString &title,
                              Func &&slot,
                              const QKeySequence &shortcut = QKeySequence()) {
        return newAction(ICONRES(iconName), title, slot, shortcut);
    }

    template <typename T>
    inline T qToBigEndian(T source) {
        return ::qToBigEndian(source);
    }

    template <typename T>
    inline T qToLittleEndian(T source) {
        return ::qToLittleEndian(source);
    }

    inline QByteArray qToBigEndian(QByteArray source) {
        QByteArray result;
        QBuffer buffer(&result);
        buffer.open(QIODevice::WriteOnly);

        QDataStream stream(&buffer);
        stream.setByteOrder(QDataStream::BigEndian);
        stream.writeRawData(source.constData(), source.size());
        return result;
    }

    inline QByteArray qToLittleEndian(QByteArray source) {
        QByteArray result;
        QBuffer buffer(&result);
        buffer.open(QIODevice::WriteOnly);

        QDataStream stream(&buffer);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.writeRawData(source.constData(), source.size());
        return result;
    }

    template <typename T>
    inline T processEndian(T source) {
        if (Utilities::checkIsLittleEndian()) {
            if (!m_islittle) {
                return qToBigEndian(source);
            }
        } else {
            if (m_islittle) {
                return qToLittleEndian(source);
            }
        }
        return source;
    }

    /* =============== some templates for async execution ===============*/
    template <typename ReturnType, typename ExecFunc, typename FinishFunc>
    void ExecAsync(ExecFunc &&execFunc, FinishFunc &&finishFunc,
                   const QString &tip = QString()) {
        QFutureWatcher<ReturnType> *watcher = new QFutureWatcher<ReturnType>();

        QObject::connect(watcher, &QFutureWatcher<ReturnType>::finished, this,
                         [=]() mutable {
                             ReturnType paramName = watcher->result();
                             finishFunc(paramName);
                             watcher->deleteLater();
                         });

        auto fu = QtConcurrent::run([=]() -> ReturnType { return execFunc(); });
        watcher->setFuture(fu);

        if (!tip.isEmpty()) {
            auto pdialog = new WingProgressDialog(tip, QString(), 0, 0);
            pdialog->setModal(true);
            pdialog->setValue(0);

            QObject::connect(watcher, &QFutureWatcher<ReturnType>::finished,
                             this, [pdialog]() mutable {
                                 pdialog->cancel();
                                 pdialog->deleteLater();
                             });

            pdialog->exec();
        }
    }

    template <typename ExecFunc, typename FinishFunc>
    void ExecAsync_VOID(ExecFunc &&execFunc, FinishFunc &&finishFunc,
                        const QString &tip = QString()) {
        QFutureWatcher<void> *watcher = new QFutureWatcher<void>();

        QObject::connect(watcher, &QFutureWatcher<void>::finished, this,
                         [=]() mutable {
                             finishFunc();
                             watcher->deleteLater();
                         });

        auto fu = QtConcurrent::run([=]() -> void { return execFunc(); });
        watcher->setFuture(fu);

        if (!tip.isEmpty()) {
            auto pdialog = new WingProgressDialog(tip, QString(), 0, 0);
            pdialog->setModal(true);
            pdialog->setValue(0);

            QObject::connect(watcher, &QFutureWatcher<void>::finished, this,
                             [pdialog]() mutable {
                                 pdialog->cancel();
                                 pdialog->deleteLater();
                             });

            pdialog->exec();
        }
    }

private:
    Ribbon *m_ribbon = nullptr;
    ads::CDockManager *m_dock = nullptr;
    QLabel *_status = nullptr;

    QString m_encoding;

    ScriptingDialog *m_scriptDialog = nullptr;
    ScriptingConsole *m_scriptConsole = nullptr;
    QTableViewExt *m_varshowtable = nullptr;

    bool m_isfinding = false;
    ads::CDockWidget *m_find = nullptr;
    QMenu *m_menuFind = nullptr;
    QHash<QString, QAction *> m_findEncoding;
    QTableViewExt *m_findresult = nullptr;
    FindResultModel *_findEmptyResult = nullptr;

    QTableViewExt *m_numshowtable = nullptr;
    NumShowModel *_numsitem = nullptr;

    QTableViewExt *m_hashtable = nullptr;
    CheckSumModel *_hashModel = nullptr;

    QTextBrowser *m_logbrowser = nullptr;
    QTextBrowser *m_txtDecode = nullptr;

    QTableViewExt *m_bookmarks = nullptr;
    BookMarksModel *_bookMarkEmpty = nullptr;

    QTableViewExt *m_metadatas = nullptr;
    MetaDataModel *_metadataEmpty = nullptr;

    // data visualization widgets
    QListView *m_infolist = nullptr;
    ClickedCallBack m_infoclickfn;
    DblClickedCallBack m_infodblclickfn;

    QTreeView *m_infotree = nullptr;
    ClickedCallBack m_infotreeclickfn;
    DblClickedCallBack m_infotreedblclickfn;

    QTableView *m_infotable = nullptr;
    QTextBrowser *m_infotxt = nullptr;
    ClickedCallBack m_infotableclickfn;
    DblClickedCallBack m_infotabledblclickfn;

    QMap<ToolButtonIndex, QToolButton *> m_toolBtneditors;

    QAction *m_aDelBookMark = nullptr;
    QAction *m_aDelMetaData = nullptr;

    //===================================================

    // QMap<EditorView* , Workspace>
    QMap<EditorView *, QString> m_views;

    QList<QWidget *> m_editStateWidgets;
    QList<QWidget *> m_driverStateWidgets;

    qsizetype _decstrlim = 10;

    QByteArray _defaultLayout;
    size_t m_newIndex = 1;

    QReadWriteLock _editorLock;
    EditorView *m_curEditor = nullptr;

    SettingDialog *m_setdialog = nullptr;
    SettingDialog *m_plgsetdlg = nullptr;
    RecentFileManager *m_recentmanager = nullptr;
    QMenu *m_recentMenu = nullptr;

    RibbonButtonGroup *m_scriptDBGroup = nullptr;
    RibbonButtonGroup *m_pluginSettingsGroup = nullptr;
    ScriptManager::ScriptActionMaps _scriptMaps;

    //===================================================

    QLabel *m_lblloc = nullptr;
    QLabel *m_lblsellen = nullptr;
    QStatusBar *m_status = nullptr;

    QAction *m_littleEndian = nullptr;
    QAction *m_bigEndian = nullptr;

    QAction *m_aShowMetafg = nullptr;
    QAction *m_aShowMetabg = nullptr;
    QAction *m_aShowMetaComment = nullptr;

    QToolButton *m_iSaved = nullptr;
    QToolButton *m_iReadWrite = nullptr;
    QToolButton *m_iLocked = nullptr;
    QToolButton *m_iCanOver = nullptr;

    QToolButton *m_sSaved = nullptr;
    QToolButton *m_sReadWrite = nullptr;
    QToolButton *m_sLocked = nullptr;
    QToolButton *m_sCanOver = nullptr;

    // cache icon
    QIcon _infoSaved, _infoUnsaved;
    QIcon _infoWriteable, _infoReadonly;
    QIcon _infoLock, _infoUnLock;
    QIcon _infoCanOver, _infoCannotOver;

    QPixmap _pixLock;
    QPixmap _pixCanOver, _pixCannotOver;

    //================================

    // for plugin system use
    QHash<QString, RibbonTabContent *> m_ribbonMaps;
    QList<QMenu *> m_hexContextMenu;
    QHash<IWingPlugin *, QList<QSharedPointer<WingEditorViewWidget::Creator>>>
        m_editorViewWidgets;
    QHash<SettingPage *, bool> m_settingPages;
    QList<PluginPage *> m_plgPages;

    // these variables will be invalid after restoring state
    ads::CDockAreaWidget *m_leftViewArea = nullptr;
    ads::CDockAreaWidget *m_rightViewArea = nullptr; // 该值使用时必不为空
    ads::CDockAreaWidget *m_topViewArea = nullptr;
    ads::CDockAreaWidget *m_bottomViewArea = nullptr; // 该值使用时必不为空

    //================================

    QString m_lastusedpath;
    bool m_islittle = true;
    bool m_enablePlugin = true;

    ShowTextDialog *_showtxt = nullptr;

    // Guard against recursion
    bool _isHandlingEvent = false;
};
#endif // MAINWINDOW_H
