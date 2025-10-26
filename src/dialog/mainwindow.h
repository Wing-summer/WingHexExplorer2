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

#include "dialog/showtextdialog.h"
#include "dialog/splashdialog.h"
#include "framelessmainwindow.h"

#include <QFutureWatcher>
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
#include <QUndoView>
#include <QtConcurrent/QtConcurrentRun>

#include "QWingRibbon/ribbon.h"
#include "QWingRibbon/ribbonbuttongroup.h"
#include "QWingRibbon/ribbontabcontent.h"
#include "Qt-Advanced-Docking-System/src/DockManager.h"
#include "Qt-Advanced-Docking-System/src/DockWidget.h"
#include "WingPlugin/iwingplugin.h"
#include "class/consolehighlighanim.h"
#include "class/recentfilemanager.h"
#include "control/asidbtreeview.h"
#include "control/editorview.h"
#include "control/qtableviewext.h"
#include "control/scriptingconsole.h"
#include "model/bookmarksmodel.h"
#include "model/checksummodel.h"
#include "model/metadatamodel.h"
#include "model/numshowmodel.h"
#include "scriptingdialog.h"
#include "settingdialog.h"
#include "utilities.h"

class PluginSystem;
class EventFilter;

class MainWindow : public FramelessMainWindow {
    Q_OBJECT

    friend class PluginSystem;

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
    buildUpScriptBgOutputDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                              ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpScriptObjDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                         ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpUndoStackDock(ads::CDockManager *dock, ads::DockWidgetArea area,
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
    void on_inspectQt();

    void on_scriptwindow();
    void on_settingGeneral();
    void on_settingScript();
    void on_settingPlugin();

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

    void openScriptFile(const QString &filename, SplashDialog *splash);

    EditorView *currentEditor();
    void adjustEditorFocus(EditorView *closedEditor);

    QString getWorkSpaceFileName(const QString &curFile);

    void saveTableContent(QAbstractItemModel *model);

    void openFiles(const QStringList &files);

    RecentFileManager *recentManager() const;

private:
    void updateNumberTable(bool force);
    void updateStringDec(const QByteArrayList &content);
    void updateUI();

    void createScriptDialog(SplashDialog *d);

private:
    IWingPlugin::FileType getEditorViewFileType(EditorView *view);

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
    void updateWindowTitle();

    void loadFindResult(EditorView *view);

    void updateEditModeEnabled();

    void setCurrentHexEditorScale(qreal rate);

    QHexView *currentHexView();

    void loadCacheIcon();
    QMessageBox::StandardButton saveRequest();

    bool reportErrFileError(ErrFile err, const QPixmap &toastIcon,
                            const QString &okMsg, const QString &errMsg);

    inline ads::CDockAreaWidget *editorViewArea() const;

    void onOutputBgScriptOutput(const ScriptMachine::MessageInfo &message);

    void restoreLayout(const QByteArray &layout);

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
            a->setText(title + QChar(' ') + QChar(char16_t(0x25BC)));
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
        auto a = new QAction(this);
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
    inline T processEndian(T source) {
        return Utilities::processEndian(source, m_islittle);
    }

    /* =============== some templates for async execution ===============*/
    template <typename ReturnType, typename ExecFunc, typename FinishFunc>
    void ExecAsync(ExecFunc &&execFunc, FinishFunc &&finishFunc) {
        QFutureWatcher<ReturnType> *watcher = new QFutureWatcher<ReturnType>();

        QObject::connect(watcher, &QFutureWatcher<ReturnType>::finished, this,
                         [=]() mutable {
                             ReturnType paramName = watcher->result();
                             finishFunc(paramName);
                             watcher->deleteLater();
                         });

        auto fu = QtConcurrent::run([=]() -> ReturnType { return execFunc(); });
        watcher->setFuture(fu);
    }

    template <typename ExecFunc, typename FinishFunc>
    void ExecAsync_VOID(ExecFunc &&execFunc, FinishFunc &&finishFunc) {
        QFutureWatcher<void> *watcher = new QFutureWatcher<void>();

        QObject::connect(watcher, &QFutureWatcher<void>::finished, this,
                         [=]() mutable {
                             finishFunc();
                             watcher->deleteLater();
                         });

        auto fu = QtConcurrent::run([=]() -> void { return execFunc(); });
        watcher->setFuture(fu);
    }

signals:
    void closed();

private:
    Ribbon *m_ribbon = nullptr;
    ads::CDockManager *m_dock = nullptr;
    QLabel *_status = nullptr;

    // for show text
    QString m_encoding;

    ScriptingDialog *m_scriptDialog = nullptr;
    ScriptingConsole *m_scriptConsole = nullptr;
    QPlainTextEdit *m_bgScriptOutput = nullptr;

    ConsoleHighlighAnim *_hlAnim = nullptr;

    bool m_isfinding = false;
    ads::CDockWidget *m_find = nullptr;
    ads::CDockWidget *m_hashtable = nullptr;
    QMenu *m_menuFind = nullptr;
    QHash<QString, QAction *> m_findEncoding;

    QTableViewExt *m_findresult = nullptr;
    FindResultModel *_findResultModel = nullptr;
    CheckSumModel *_hashModel = nullptr;
    BookMarksModel *_bookMarkModel = nullptr;
    MetaDataModel *_metadataModel = nullptr;

    EventFilter *m_lazyVisibleFilter = nullptr;

    QTableViewExt *m_numshowtable = nullptr;
    NumShowModel *_numsitem = nullptr;

    QTextBrowser *m_logbrowser = nullptr;
    QTextBrowser *m_txtDecode = nullptr;

    QUndoView *_undoView = nullptr;
    asIDBTreeView *_scriptObjView = nullptr;

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
    SettingDialog *m_scriptsetdlg = nullptr;
    RecentFileManager *m_recentmanager = nullptr;
    QMenu *m_recentMenu = nullptr;

    RibbonButtonGroup *m_scriptDBGroup = nullptr;
    RibbonButtonGroup *m_pluginSettingsGroup = nullptr;
    QList<QMenu *> _scriptContexts;

    //===================================================

    QLabel *m_lblloc = nullptr;
    QLabel *m_lblsellen = nullptr;
    QStatusBar *m_status = nullptr;

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

    ShowTextDialog *_showtxt = nullptr;

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
    QList<SettingPage *> m_settingPages;

    // these variables will be invalid after restoring state
    ads::CDockAreaWidget *m_leftViewArea = nullptr;
    ads::CDockAreaWidget *m_rightViewArea = nullptr;
    ads::CDockAreaWidget *m_topViewArea = nullptr;
    ads::CDockAreaWidget *m_bottomViewArea = nullptr;

    //================================

    QString m_lastusedpath;
    bool m_islittle = true;
    bool m_unsignedHex = false;
    ScriptMachine::MessageType _lastOutputType =
        ScriptMachine::MessageType::Unknown;
};
#endif // MAINWINDOW_H
