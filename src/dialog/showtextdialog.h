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

#ifndef SHOWTEXTDIALOG_H
#define SHOWTEXTDIALOG_H

#include "QHexView/document/buffer/qhexbuffer.h"

#include "QWingRibbon/ribbon.h"

#include "QWingRibbon/ribbonbuttongroup.h"
#include "QWingRibbon/ribbontabcontent.h"

#include "control/codeedit.h"
#include "dialog/framelessmainwindow.h"
#include "utilities.h"

#include <QPushButton>
#include <QShortcut>
#include <QStatusBar>
#include <QToolButton>

class ShowTextDialog : public FramelessMainWindow {
    Q_OBJECT
public:
    explicit ShowTextDialog(QWidget *parent = nullptr);
    virtual ~ShowTextDialog();

public:
    void setSyntax(const KSyntaxHighlighting::Definition &syntax);

public:
    void load(QHexBuffer *buffer, const QString &mime,
              const QString &encoding = {});

private:
    void buildUpRibbonBar();

    RibbonTabContent *buildEditPage(RibbonTabContent *tab);

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
        setPannelActionToolTip(a, shortcut);

        if (!shortcut.isEmpty()) {
            auto shortCut = new QShortcut(shortcut, this);
            shortCut->setContext(Qt::WindowShortcut);
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
    inline QAction *newAction(const QString &title, Func &&slot,
                              const QKeySequence &shortcut = QKeySequence()) {
        auto a = new QAction;
        a->setText(title);
        a->setShortcutVisibleInContextMenu(true);
        a->setShortcut(shortcut);
        connect(a, &QAction::triggered, this, slot);
        return a;
    }

    inline void
    setPannelActionToolTip(QToolButton *action,
                           const QKeySequence &shortcut = QKeySequence()) {
        Q_ASSERT(action);
        auto title = action->text();
        action->setToolTip(
            shortcut.isEmpty()
                ? QStringLiteral("<p align=\"center\">%1</p>").arg(title)
                : QStringLiteral(
                      "<p align=\"center\">%1</p><p align=\"center\">%2</p>")
                      .arg(title, shortcut.toString()));
    }

private slots:
    void on_copyfile();
    void on_findfile();
    void on_gotoline();
    void on_encoding();
    void on_updateDefs();

private:
    void setEncoding(const QString &enc);

    // QWidget interface
protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    Ribbon *m_ribbon = nullptr;
    CodeEdit *m_edit = nullptr;
    QStatusBar *m_status = nullptr;
    QToolButton *m_syntaxButton = nullptr;

    QPushButton *m_cancelButton = nullptr;
    bool m_continue = true;

    QByteArray buffer;
};

#endif // SHOWTEXTDIALOG_H
