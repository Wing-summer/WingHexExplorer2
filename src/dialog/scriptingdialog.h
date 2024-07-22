#ifndef SCRIPTINGDIALOG_H
#define SCRIPTINGDIALOG_H

#include "framelessmainwindow.h"

#include "QWingRibbon/ribbon.h"
#include "QWingRibbon/ribbonbuttongroup.h"
#include "Qsci/qsciscintilla.h"
#include "Qt-Advanced-Docking-System/src/DockManager.h"
#include "Qt-Advanced-Docking-System/src/DockWidget.h"
#include "src/class/recentfilemanager.h"
#include "src/class/scriptmachine.h"
#include "src/utilities.h"

#include <QListWidget>
#include <QShortcut>
#include <QTableWidget>
#include <QTextBrowser>

class ScriptingDialog : public FramelessMainWindow {
    Q_OBJECT
private:
    enum ToolButtonIndex : uint { UNDO_ACTION, REDO_ACTION };

public:
    explicit ScriptingDialog(QWidget *parent = nullptr);

private:
    void buildUpRibbonBar();
    RibbonTabContent *buildFilePage(RibbonTabContent *tab);
    RibbonTabContent *buildEditPage(RibbonTabContent *tab);
    RibbonTabContent *buildScriptPage(RibbonTabContent *tab);
    RibbonTabContent *buildDebugPage(RibbonTabContent *tab);
    RibbonTabContent *buildAboutPage(RibbonTabContent *tab);

    void buildUpDockSystem(QWidget *container);

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
            auto shortCut = new QShortcut(shortcut, a);
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

private slots:
    void on_newfile();
    void on_savesel();
    void on_openfile();
    void on_reload();

    void on_save();
    void on_saveas();
    void on_exportfile();

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

    void on_about();
    void on_sponsor();
    void on_wiki();

    void on_runscript();
    void on_rundbgscript();
    void on_pausescript();
    void on_continuescript();
    void on_stopscript();
    void on_restartscript();
    void on_stepinscript();
    void on_stepoutscript();
    void on_stepoverscript();

private:
    ads::CDockManager *m_dock = nullptr;
    ads::CDockAreaWidget *m_editorViewArea = nullptr;

    QMap<QsciScintilla *, QString> m_files;
    QList<QWidget *> m_editStateWidgets;
    QMap<ToolButtonIndex, QToolButton *> m_Tbtneditors;

    QMenu *m_recentMenu = nullptr;
    RecentFileManager *m_recentmanager = nullptr;
    Ribbon *m_ribbon = nullptr;

    ScriptMachine *m_runner = nullptr;
    // widgets for debugging
    QTextBrowser *m_consoleout = nullptr;
    QTableWidget *m_varshowtable = nullptr;
    QTableWidget *m_breakpointstable = nullptr;
    QTableWidget *m_watchtable = nullptr;
    QListWidget *m_callstack = nullptr;

    QStatusBar *m_status = nullptr;
};

#endif // SCRIPTINGDIALOG_H
