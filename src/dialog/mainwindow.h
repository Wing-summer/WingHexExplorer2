#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "framelessmainwindow.h"

#include <QListWidget>
#include <QMainWindow>
#include <QMap>
#include <QPixmap>
#include <QShortcut>
#include <QTableWidget>
#include <QTextBrowser>
#include <QToolButton>
#include <QTreeWidget>
#include <QtConcurrent/QtConcurrent>

#include "../../QWingRibbon/ribbon.h"
#include "../../QWingRibbon/ribbonbuttongroup.h"
#include "../../QWingRibbon/ribbontabcontent.h"
#include "../../Qt-Advanced-Docking-System/src/DockManager.h"
#include "../../Qt-Advanced-Docking-System/src/DockWidget.h"
#include "../class/recentfilemanager.h"
#include "../class/wingprogressdialog.h"
#include "../control/editorview.h"
#include "../control/scriptingconsole.h"
#include "../plugin/iwingplugin.h"
#include "../utilities.h"

#include "scriptingdialog.h"
#include "settingdialog.h"

class PluginSystem;

class MainWindow : public FramelessMainWindow {
    Q_OBJECT

    friend class PluginSystem;

public:
    MainWindow(QWidget *parent = nullptr);
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
        UNDO_ACTION,
        REDO_ACTION
    };

    enum NumTableIndex {
        Byte,
        Char,
        Ushort,
        Short,
        Uint32,
        Int32,
        Uint64,
        Int64,
        NumTableIndexCount
    };

private:
    void buildUpRibbonBar();
    void buildUpDockSystem(QWidget *container);
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
    buildUpHexBookMarkDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                           ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpDecodingStrShowDock(ads::CDockManager *dock,
                               ads::DockWidgetArea area,
                               ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpScriptConsoleDock(ads::CDockManager *dock, ads::DockWidgetArea area,
                             ads::CDockAreaWidget *areaw = nullptr);
    ads::CDockAreaWidget *
    buildUpScriptVarShowDock(ads::CDockManager *dock, ads::DockWidgetArea area,
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

private:
    EditorView *newfileGUI();

private slots:
    void on_newfile();
    void on_openfile();
    void on_openregion();
    void on_openworkspace();
    void on_opendriver();
    void on_reload();

    void on_save();
    void on_saveas();
    void on_exportfile();
    void on_savesel();

    void on_undofile();
    void on_redofile();
    void on_cutfile();
    void on_copyfile();
    void on_pastefile();
    void on_delete();

    void on_findfile();
    void on_gotoline();
    void on_encoding();
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
    void on_bookmarkChanged(BookMarkModEnum flag, int index, qint64 pos,
                            QString comment);

    void on_metadatafg(bool checked);
    void on_metadatabg(bool checked);
    void on_metadatacomment(bool checked);
    void on_metashowall();
    void on_metahideall();

    void on_clearfindresult();
    void on_exportfindresult();
    void on_locChanged();

    void on_fullScreen();
    void on_restoreLayout();
    void on_exportlog();
    void on_clslog();

    void on_scriptwindow();
    void on_loadplg();
    void on_setting_general();
    void on_setting_plugin();

    void on_about();
    void on_sponsor();
    void on_wiki();

private:
    QString saveLog();

    ads::CDockWidget *buildDockWidget(ads::CDockManager *dock,
                                      const QString &widgetName,
                                      const QString &displayName,
                                      QWidget *content,
                                      ToolButtonIndex index = TOOL_VIEWS);
    EditorView *findEditorView(const QString &filename);

    bool newOpenFileSafeCheck();
    void registerEditorView(EditorView *editor);

    void connectEditorView(EditorView *editor);
    void swapEditorConnection(EditorView *old, EditorView *cur);

    void openFiles(const QStringList &files);
    ErrFile openFile(const QString &file, EditorView **editor);
    ErrFile openDriver(const QString &driver, EditorView **editor);
    ErrFile openWorkSpace(const QString &file, EditorView **editor);
    ErrFile openRegionFile(QString file, EditorView **editor, qsizetype start,
                           qsizetype length);

    void setEditModeEnabled(bool b, bool isdriver = false);
    void enableDirverLimit(bool isdriver);

    void loadCacheIcon();
    QMessageBox::StandardButton saveRequest();

protected:
    void closeEvent(QCloseEvent *event) override;

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
        a->setText(title);
        a->setIcon(icon);

        if (!shortcut.isEmpty()) {
            auto shortCut = new QShortcut(shortcut, this);
            shortCut->setContext(Qt::ApplicationShortcut);
            connect(shortCut, &QShortcut::activated, a, &QToolButton::click);
        }

        a->setMenu(menu);
        if (menu) {
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

        if (!shortcut.isEmpty()) {
            auto shortCut = new QShortcut(shortcut, this);
            connect(shortCut, &QShortcut::activated, a, &QAction::trigger);
        }

        a->setCheckable(true);
        connect(a, &QAction::triggered, this, slot);
        return a;
    }

    template <typename Func>
    inline QAction *newAction(const QString &iconName, const QString &title,
                              Func &&slot,
                              const QKeySequence &shortcut = QKeySequence()) {
        auto a = new QAction;
        a->setText(title);
        a->setIcon(ICONRES(iconName));
        a->setShortcutVisibleInContextMenu(true);

        if (!shortcut.isEmpty()) {
            auto shortCut = new QShortcut(shortcut, this);
            connect(shortCut, &QShortcut::activated, a, &QAction::trigger);
        }

        a->setCheckable(true);
        connect(a, &QAction::triggered, this, slot);
        return a;
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
            std::unique_ptr<WingProgressDialog> pdialog(
                new WingProgressDialog(tip, QString(), 0, 0));
            auto dialog = pdialog->dialog();
            dialog->setModal(true);
            dialog->setValue(0);

            QObject::connect(watcher, &QFutureWatcher<ReturnType>::finished,
                             this, [dialog]() mutable {
                                 dialog->cancel();
                                 dialog->deleteLater();
                             });

            pdialog->pdialog()->exec();
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
            std::unique_ptr<WingProgressDialog> pdialog(
                new WingProgressDialog(tip, QString(), 0, 0));
            auto dialog = pdialog->dialog();
            dialog->setModal(true);
            dialog->setValue(0);

            QObject::connect(watcher, &QFutureWatcher<void>::finished, this,
                             [dialog]() mutable {
                                 dialog->cancel();
                                 dialog->deleteLater();
                             });

            pdialog->pdialog()->exec();
        }
    }

private:
    Ribbon *m_ribbon = nullptr;
    ads::CDockManager *m_dock = nullptr;

    ScriptingDialog *m_scriptDialog = nullptr;
    ScriptingConsole *m_scriptConsole = nullptr;
    QTableWidget *m_varshowtable = nullptr;

    QMutex m_findMutex;

    QTableWidget *m_findresult = nullptr;
    QTableWidget *m_numshowtable = nullptr;
    QTableWidgetItem *_numsitem = nullptr;
    QTableWidgetItem (*_findresitem)[3] = {nullptr};
    QTextBrowser *m_logbrowser = nullptr;
    QTextBrowser *m_txtDecode = nullptr;
    QListWidget *m_bookmarks = nullptr;
    QListWidget *m_metadatas = nullptr;

    ads::CDockAreaWidget *m_editorViewArea = nullptr;

    // data visualization widgets
    QListWidget *m_infolist = nullptr;
    QTreeWidget *m_infotree = nullptr;
    QTableWidget *m_infotable = nullptr;
    QTextBrowser *m_infotxt = nullptr;

    QMap<ToolButtonIndex, QToolButton *> m_toolBtneditors;

    //===================================================

    // file manager cache
    QStringList m_openedFileNames;
    // QMap<EditorView* , Workspace>
    QMap<EditorView *, QString> m_views;

    QList<QWidget *> m_editStateWidgets;
    QList<QWidget *> m_driverStateWidgets;

    qsizetype _findmax = 100;
    int _findres = 0;
    qsizetype _decstrlim = 10;

    size_t m_newIndex = 1;
    QAtomicPointer<EditorView> m_curEditor = nullptr;
    SettingDialog *m_setdialog = nullptr;
    RecentFileManager *m_recentmanager = nullptr;
    QMenu *m_recentMenu = nullptr;

    RibbonButtonGroup *m_scriptDBGroup = nullptr;

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
    QToolButton *m_iWorkSpace = nullptr;
    QToolButton *m_iLocked = nullptr;
    QToolButton *m_iCanOver = nullptr;

    // cache icon
    QIcon _infoSaved, _infoUnsaved;
    QIcon _infoWriteable, _infoReadonly;
    QIcon _infow, _infouw;
    QIcon _infoLock, _infoUnLock;
    QIcon _infoCanOver, _infoCannotOver;

    QPixmap _pixLock;
    QPixmap _pixCanOver, _pixCannotOver;

    //================================

    // for plugin system use
    QHash<QString, RibbonTabContent *> m_ribbonMaps;
    QList<QMenu *> m_hexContextMenu;
    QList<WingEditorViewWidget *> m_editorViewWidgets;
    QList<SettingPage *> m_settingPages;

    ads::CDockAreaWidget *m_leftViewArea = nullptr;
    ads::CDockAreaWidget *m_rightViewArea = nullptr; // 该值使用时必不为空
    ads::CDockAreaWidget *m_topViewArea = nullptr;
    ads::CDockAreaWidget *m_bottomViewArea = nullptr; // 该值使用时必不为空

    //================================

    QString m_lastusedpath;
    bool m_islittle = true;
    bool m_enablePlugin = true;
    bool m_isOnClosing = false;
};
#endif // MAINWINDOW_H
