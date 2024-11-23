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
#include "framelessdialogbase.h"
#include "utilities.h"

#include <QShortcut>
#include <QToolButton>

class ShowTextDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    explicit ShowTextDialog(QWidget *parent = nullptr);
    virtual ~ShowTextDialog();

public:
    void load(QHexBuffer *buffer, const QString &enc, qsizetype offset = 0,
              qsizetype size = -1);

private:
    void buildUpRibbonBar();

    RibbonTabContent *buildEditPage(RibbonTabContent *tab);
    RibbonTabContent *buildViewPage(RibbonTabContent *tab);

    void setCurrentEditorScale(qreal rate);

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

    void on_cancel();

private:
    Ribbon *m_ribbon = nullptr;
    CodeEdit *m_edit = nullptr;
    bool m_canceled = false;

    struct {
        QHexBuffer *buffer = nullptr;
        QString enc;
        qsizetype offset = 0;
        qsizetype size = -1;
    } m_last;
};

#endif // SHOWTEXTDIALOG_H
