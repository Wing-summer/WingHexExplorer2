/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#ifndef SCRIPTWINDOWFACTORY_H
#define SCRIPTWINDOWFACTORY_H

#include "dialog/framelessdialogbase.h"
#include "dialog/framelessmainwindow.h"
#include "qtluau/bindings.hpp"
#include "qtluau/ui_notifications.hpp"
#include "qtluau/window_menu.hpp"
#ifdef WINGHEX_USE_FRAMELESS
#include "class/settings/settingmanager.h"
#endif
#include <QLayout>
#include <QMenuBar>
#include <QPointer>
#include <QVBoxLayout>

class ScriptWindowFactory {
    class Dialog : public FramelessDialogBase {
    public:
        using FramelessDialogBase::FramelessDialogBase;
        QPointer<QWidget> payload;
        QList<QPointer<QWidget>> uiChildren;
        bool ready = false;
    };
    class Window : public FramelessMainWindow {
    public:
        explicit Window(QWidget *parent) : FramelessMainWindow(parent) {
#ifdef WINGHEX_USE_FRAMELESS
            nativeTitleBar = SettingManager::instance().useNativeTitleBar();
#endif
        }
        QPointer<QWidget> payload;
        QPointer<QWidget> container;
        QPointer<QWidget> menu;
        bool ready = false;
        bool nativeTitleBar = true;

        void ensureContainer() {
            if (container) {
                return;
            }
            auto *content = takeCentralWidget();
            container = new QWidget(this);
            auto *layout = new QVBoxLayout(container);
            layout->setContentsMargins(0, 0, 0, 0);
            if (content) {
                layout->addWidget(content);
            }
            setCentralWidget(container);
        }
        void replaceContent(QWidget *next) {
            if (payload == next) {
                return;
            }
            if (!container) {
                setCentralWidget(next);
            } else {
                auto *layout = container->layout();
                if (payload) {
                    layout->removeWidget(payload);
                    payload->hide();
                    payload->deleteLater();
                }
                if (next) {
                    layout->addWidget(next);
                }
            }
            payload = next;
        }
        QWidget *takeContent() {
            QWidget *previous = payload;
            if (!container) {
                takeCentralWidget();
            } else if (previous) {
                container->layout()->removeWidget(previous);
                previous->setParent(nullptr);
            }
            payload.clear();
            return previous;
        }
        void replaceMenu(QWidget *next) {
            if (nativeTitleBar) {
                setMenuWidget(next);
                return;
            }
            if (menu == next) {
                return;
            }
            ensureContainer();
            auto *layout = qobject_cast<QVBoxLayout *>(container->layout());
            if (menu) {
                layout->removeWidget(menu);
                menu->hide();
                menu->deleteLater();
            }
            if (next) {
                layout->insertWidget(0, next);
            }
            menu = next;
        }
        QWidget *getMenu(bool create) {
            if (nativeTitleBar) {
                return create ? menuBar() : menuWidget();
            }
            if (create && !qobject_cast<QMenuBar *>(menu.data())) {
                replaceMenu(new QMenuBar);
            }
            return menu;
        }
    };
    static void finish(QWidget *widget) {
        if (auto *dialog = dynamic_cast<Dialog *>(widget)) {
            if (dialog->ready) {
                return;
            }
            const QRect geometry = dialog->geometry();
            dialog->payload = new QWidget;
            dialog->payload->setWindowTitle(dialog->windowTitle());
            dialog->payload->setWindowIcon(dialog->windowIcon());
            if (auto *layout = dialog->layout()) {
                dialog->payload->setLayout(layout);
            }
            for (const auto &child : dialog->uiChildren) {
                if (child && child->parentWidget() == dialog) {
                    const bool hidden = child->isHidden();
                    child->setParent(dialog->payload, child->windowFlags());
                    child->setVisible(!hidden);
                }
            }
            dialog->uiChildren.clear();
            dialog->buildUpContent(dialog->payload);
            dialog->setGeometry(geometry);
            dialog->ready = true;
        } else if (auto *window = dynamic_cast<Window *>(widget)) {
            if (window->ready) {
                return;
            }
            const QRect geometry = window->geometry();
            window->payload = window->takeCentralWidget();
            if (!window->payload) {
                window->payload = new QWidget;
            }
            window->menu = window->menuWidget();
            QWidget *content = window->payload;
            if (!window->nativeTitleBar && window->menu) {
                window->container = new QWidget(window);
                auto *layout = new QVBoxLayout(window->container);
                layout->setContentsMargins(0, 0, 0, 0);
                window->menu->setParent(window->container);
                layout->addWidget(window->menu);
                layout->addWidget(content);
                content = window->container;
            }
            window->buildUpContent(content);
            window->setGeometry(geometry);
            window->ready = true;
        }
    }

public:
    static void created(QWidget *widget,
                        const qtluau::UiWidgetRequest &request) {
        if (request.fromUi) {
            if (auto *dialog = dynamic_cast<Dialog *>(request.parent)) {
                dialog->uiChildren.append(widget);
            }
        }
    }
    static qtluau::UiWidgetResult
    create(const qtluau::UiWidgetRequest &request) {
        QWidget *widget = nullptr;
        if (request.className == QStringLiteral("QDialog")) {
            widget = new Dialog(request.parent);
        } else if (request.className == QStringLiteral("QMainWindow")) {
            widget = new Window(request.parent);
        } else {
            return {};
        }
        if (!request.fromUi) {
            finish(widget);
        }
        return {qtluau::UiWidgetAction::Use, widget};
    }
    static void finalize(QWidget *root) {
        const auto widgets = root->findChildren<QWidget *>();
        for (auto *widget : widgets) {
            finish(widget);
        }
        finish(root);
    }
    static void setContent(QWidget *widget, QWidget *content) {
        if (auto *window = dynamic_cast<Window *>(widget)) {
            window->replaceContent(content);
        } else if (auto *window = qobject_cast<QMainWindow *>(widget)) {
            window->setCentralWidget(content);
        } else {
            throw std::runtime_error("Unsupported host content target");
        }
    }
    static QWidget *takeContent(QWidget *widget) {
        if (auto *window = dynamic_cast<Window *>(widget)) {
            return window->takeContent();
        }
        if (auto *window = qobject_cast<QMainWindow *>(widget)) {
            return window->takeCentralWidget();
        }
        throw std::runtime_error("Unsupported host content target");
    }
    static QWidget *menu(QWidget *widget, bool create) {
        if (auto *window = dynamic_cast<Window *>(widget)) {
            return window->getMenu(create);
        }
        if (auto *window = qobject_cast<QMainWindow *>(widget)) {
            return create ? window->menuBar() : window->menuWidget();
        }
        throw std::runtime_error("Unsupported host menu target");
    }
    static void setMenu(QWidget *widget, QWidget *menu) {
        if (auto *window = dynamic_cast<Window *>(widget)) {
            window->replaceMenu(menu);
        } else if (auto *window = qobject_cast<QMainWindow *>(widget)) {
            window->setMenuWidget(menu);
        } else {
            throw std::runtime_error("Unsupported host menu target");
        }
    }
    static QWidget *content(QWidget *widget) {
        if (auto *dialog = dynamic_cast<Dialog *>(widget)) {
            return dialog->ready ? dialog->payload.data() : widget;
        }
        if (auto *window = dynamic_cast<Window *>(widget)) {
            return window->ready ? window->payload.data() : widget;
        }
        return widget;
    }
};

#endif // SCRIPTWINDOWFACTORY_H
