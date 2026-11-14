// Copyright (C) 2026 WingSummer
// SPDX-License-Identifier: AGPL-3.0-only
#ifndef MODEPROBE_H
#define MODEPROBE_H

#include <cstdio>

#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QMessageBox>
#include <QThread>
#include <QTimer>

#include "class/application/applicationmode.h"
#include "control/editor/scripteditor.h"
#include "control/editorview.h"
#include "control/toast.h"
#include "dialog/diffdialog.h"
#include "dialog/mainwindow.h"
#include "dialog/scriptingdialog.h"
#include "luau/runtime/scriptmachine.h"
#include "settings/diffsettingdialog.h"

namespace {
bool modeProbeFailed = false;

inline void inspectModeWindow() {
    const auto mode = qEnvironmentVariable("WINGHEX_MODE_TEST");
    if (mode.isEmpty()) {
        return;
    }
    // Production constructors may process events while building the UI. Wait
    // for an actual event loop before exercising or closing the window.
    if (QThread::currentThread()->loopLevel() == 0) {
        QTimer::singleShot(50, qApp, inspectModeWindow);
        return;
    }
    const auto fail = [](const char *message) {
        std::fprintf(stderr, "Mode probe: %s\n", message);
        modeProbeFailed = true;
        qApp->exit(97);
    };
    if (mode == QStringLiteral("disabled")) {
        if (auto modal =
                qobject_cast<QDialog *>(QApplication::activeModalWidget())) {
            if (!modal->findChild<QMessageBox *>() ||
                ScriptMachine::instance().isInited()) {
                fail("disabled script mode initialized the VM or showed no "
                     "error");
            }
            modal->accept();
            return;
        }
    }
    if (mode == QStringLiteral("script-errors")) {
        if (auto modal =
                qobject_cast<QDialog *>(QApplication::activeModalWidget())) {
            auto message = modal->findChild<QMessageBox *>();
            if (!message ||
                !message->text().contains(QStringLiteral("missing-one.luau")) ||
                !message->text().contains(QStringLiteral("missing-two.luau"))) {
                fail("script file errors were not collected into one message");
            }
            modal->accept();
            QTimer::singleShot(0, qApp, inspectModeWindow);
            return;
        }
    }
    for (auto widget : QApplication::topLevelWidgets()) {
        if (auto window = qobject_cast<MainWindow *>(widget)) {
            if (mode != QStringLiteral("full")) {
                fail("standalone mode constructed MainWindow");
                return;
            }
            if (!window->isVisible()) {
                continue;
            }
            if (EditorView::instances().size() != 1 ||
                !ScriptMachine::instance().isInited()) {
                fail("full mode did not open the initial file");
                return;
            }
            std::fprintf(stdout, "full-ready\n");
            std::fflush(stdout);
            auto timer = new QTimer(window);
            QObject::connect(timer, &QTimer::timeout, window, [window, timer] {
                if (EditorView::instances().size() == 2) {
                    timer->stop();
                    window->close();
                }
            });
            timer->start(50);
            return;
        }
        if (auto window = qobject_cast<DiffDialog *>(widget);
            window && window->isVisible()) {
            if (mode != QStringLiteral("diff") ||
                !EditorView::instances().isEmpty() ||
                ScriptMachine::instance().isInited()) {
                fail("diff mode initialized editors or scripting");
                return;
            }
            const auto left =
                window->findChild<QHexView *>(QStringLiteral("diff.left.view"));
            const auto right = window->findChild<QHexView *>(
                QStringLiteral("diff.right.view"));
            const auto expected =
                qEnvironmentVariableIntValue("WINGHEX_MODE_FILES");
            if (!left || !right ||
                (expected > 0 && !left->document()->isReadOnly()) ||
                (expected > 1 && !right->document()->isReadOnly()) ||
                (expected > 0 &&
                 left->document()->read(0, 4) != QByteArray("left")) ||
                (expected > 1 &&
                 right->document()->read(0, 5) != QByteArray("right"))) {
                fail("diff did not load the requested sources");
                return;
            }
            // Loading through the public API must reuse documents, not editors.
            const auto path = qEnvironmentVariable("WINGHEX_MODE_REOPEN");
            if (!path.isEmpty()) {
                const auto document = left->document();
                if (!window->openFile(path) || left->document() != document ||
                    !EditorView::instances().isEmpty()) {
                    fail("reopening a comparison file duplicated its source");
                    return;
                }
            }
            if (!path.isEmpty()) {
                const auto document = left->document();
                QTimer::singleShot(0, qApp, [] {
                    if (auto modal = qobject_cast<QDialog *>(
                            QApplication::activeModalWidget())) {
                        modal->accept();
                    }
                });
                if (window->openFile(path + QStringLiteral(".missing")) ||
                    left->document() != document) {
                    fail("failed replacement discarded the current comparison "
                         "source");
                    return;
                }
                auto external = std::make_unique<EditorView>();
                if (external->openFile(path) != WingHex::ErrFile::Success) {
                    fail("could not open the shared-document fixture");
                    return;
                }
                auto shared = external->hexEditor()->document();
                shared->replace(0, QByteArray("edit"));
                if (!window->openFile(path) || left->document() != shared ||
                    left->document()->read(0, 4) != QByteArray("edit")) {
                    fail("comparison did not share the editor's unsaved "
                         "content");
                    return;
                }
                external.reset();
                auto choice = window->findChild<QComboBox *>(
                    QStringLiteral("diff.left.source"));
                if (!choice || choice->currentIndex() != -1 ||
                    !EditorView::instances().isEmpty()) {
                    fail("closing a shared editor did not invalidate its "
                         "comparison source");
                    return;
                }
            }
            const auto capture = qEnvironmentVariable("WINGHEX_MODE_CAPTURE");
            if (!capture.isEmpty()) {
                window->grab().save(capture);
            }
            if (expected == 2) {
                const auto rightChoice = window->findChild<QComboBox *>(
                    QStringLiteral("diff.right.source"));
                const auto rightPath =
                    rightChoice->currentData(Qt::ToolTipRole).toString();
                const auto document = right->document().toWeakRef();
                const auto closeLeft = window->findChild<QToolButton *>(
                    QStringLiteral("diff.left.close"));
                const auto closeRight = window->findChild<QToolButton *>(
                    QStringLiteral("diff.right.close"));
                if (!closeLeft || !closeRight ||
                    !window->openFile(rightPath, DiffDialog::Side::Left)) {
                    fail("cannot prepare shared comparison source");
                    return;
                }
                closeLeft->click();
                if (document.isNull() || right->document() != document ||
                    left->document() == document) {
                    fail("closing one pane discarded the other pane's source");
                    return;
                }
                closeRight->click();
                if (!document.isNull()) {
                    fail("closing both panes retained the owned document");
                    return;
                }
            }
            for (const auto side :
                 {QStringLiteral("left"), QStringLiteral("right")}) {
                const auto prefix = QStringLiteral("diff.") + side;
                const auto open = window->findChild<QToolButton *>(
                    prefix + QStringLiteral(".open"));
                const auto close = window->findChild<QToolButton *>(
                    prefix + QStringLiteral(".close"));
                const auto choice = window->findChild<QComboBox *>(
                    prefix + QStringLiteral(".source"));
                const auto view = window->findChild<QHexView *>(
                    prefix + QStringLiteral(".view"));
                if (!open || !close || !choice || !view || !open->autoRaise() ||
                    !close->autoRaise() || open->width() != open->height() ||
                    close->width() != close->height() ||
                    close->isEnabled() != (choice->currentIndex() >= 0)) {
                    fail(
                        "standalone source controls are missing or misaligned");
                    return;
                }
                close->click();
                if (choice->currentIndex() != -1 || close->isEnabled() ||
                    view->document()->length() != 0) {
                    fail("closing a comparison source did not clear its pane");
                    return;
                }
            }
            bool settingsOpened = false;
            const auto settingsButton = window->findChild<QToolButton *>(
                QStringLiteral("diff.settings"));
            if (!settingsButton) {
                fail("comparison settings action is missing");
                return;
            }
            QTimer::singleShot(0, window, [&settingsOpened] {
                if (auto modal = qobject_cast<QDialog *>(
                        QApplication::activeModalWidget())) {
                    settingsOpened =
                        modal->findChildren<DiffSettingDialog *>().size() == 1;
                    modal->accept();
                }
            });
            settingsButton->click();
            if (!settingsOpened || !EditorView::instances().isEmpty()) {
                fail("standalone comparison settings were not available or "
                     "created editors");
                return;
            }
            window->close();
            return;
        }
        if (auto window = qobject_cast<ScriptingDialog *>(widget);
            window && window->isVisible()) {
            if ((mode != QStringLiteral("script") &&
                 mode != QStringLiteral("script-errors")) ||
                !EditorView::instances().isEmpty() ||
                !ScriptMachine::instance().isInited() ||
                ScriptEditor::instances().size() !=
                    qEnvironmentVariableIntValue("WINGHEX_MODE_FILES")) {
                fail("script mode did not initialize only the requested script "
                     "editors");
                return;
            }
            if (qEnvironmentVariableIsSet("WINGHEX_MODE_TOAST")) {
                const bool parentless =
                    qEnvironmentVariableIsSet("WINGHEX_MODE_TOAST_PARENTLESS");
                const QPointer<ScriptingDialog> target(window);
                if (parentless) {
                    qApp->setQuitOnLastWindowClosed(false);
                    window->close();
                    QCoreApplication::sendPostedEvents(nullptr,
                                                       QEvent::DeferredDelete);
                } else {
                    window->activateWindow();
                }
                QCoreApplication::processEvents();
                if (QApplication::activeWindow() !=
                    (parentless ? nullptr : target.data())) {
                    fail("toast test could not establish its active-window "
                         "state");
                    return;
                }
                ScriptMachine::RegCallBacks callbacks;
                callbacks.printMsgFn =
                    [](const ScriptMachine::MessageInfo &message) {
                        if (message.type == ScriptMachine::MessageType::Error) {
                            std::fprintf(stderr, "%s\n",
                                         qUtf8Printable(message.message));
                        }
                    };
                ScriptMachine::instance().registerCallBack(
                    ScriptMachine::Background, callbacks);
                ScriptMachine::instance().executeScriptWithResult(
                    ScriptMachine::Background,
                    qEnvironmentVariable("WINGHEX_MODE_TOAST"), false,
                    [target,
                     parentless](ScriptMachine::ExecutionResult result) {
                        const auto overlays =
                            target ? target->findChildren<Toast *>()
                                   : QList<Toast *>{};
                        const bool shown =
                            overlays.size() == 1 &&
                            overlays.first()->isVisible() &&
                            overlays.first()->parentWidget() == target;
                        if (result != ScriptMachine::ExecutionResult::Success ||
                            (parentless ? !overlays.isEmpty() : !shown)) {
                            std::fprintf(
                                stderr,
                                "Toast result=%d parentless=%d overlays=%lld "
                                "shown=%d\n",
                                int(result), parentless,
                                static_cast<long long>(overlays.size()), shown);
                            modeProbeFailed = true;
                        }
                        if (parentless) {
                            qApp->quit();
                        } else {
                            target->close();
                        }
                    });
                return;
            }
            if (qEnvironmentVariableIsSet("WINGHEX_MODE_PLUGIN")) {
                ScriptMachine::instance().executeScriptWithResult(
                    ScriptMachine::Background,
                    qEnvironmentVariable("WINGHEX_MODE_PLUGIN"), false,
                    [window](ScriptMachine::ExecutionResult result) {
                        if (result != ScriptMachine::ExecutionResult::Success) {
                            modeProbeFailed = true;
                        }
                        window->close();
                    });
                return;
            }
            if (qEnvironmentVariableIsSet("WINGHEX_MODE_UNSAVED") &&
                !ScriptEditor::instances().isEmpty()) {
                const QPointer<ScriptEditor> modifiedEditor =
                    ScriptEditor::instances().first();
                modifiedEditor->editor()->document()->setModified(true);
                QTimer::singleShot(0, qApp, [] {
                    if (auto modal = qobject_cast<QDialog *>(
                            QApplication::activeModalWidget())) {
                        modal->done(QMessageBox::Cancel);
                    }
                });
                if (window->close() || !window->isVisible() ||
                    !modifiedEditor ||
                    !modifiedEditor->editor()->document()->isModified()) {
                    fail("canceling script window close lost the window or "
                         "modified document");
                    return;
                }
                QTimer::singleShot(0, qApp, [] {
                    if (auto modal = qobject_cast<QDialog *>(
                            QApplication::activeModalWidget())) {
                        modal->done(QMessageBox::No);
                    }
                });
            }
            window->close();
            return;
        }
    }
    QTimer::singleShot(50, qApp, inspectModeWindow);
}

inline void inspectHeadlessEnvironment() {
    if (QThread::currentThread()->loopLevel() == 0) {
        QTimer::singleShot(0, qApp, inspectHeadlessEnvironment);
        return;
    }
    if (!QApplication::topLevelWidgets().isEmpty() ||
        !EditorView::instances().isEmpty()) {
        std::fputs("Headless startup created UI widgets or editors\n", stderr);
        modeProbeFailed = true;
        qApp->exit(97);
    }
}

inline void installModeProbe() {
    if (qEnvironmentVariableIsSet("WINGHEX_HEADLESS_TEST_NO_UI")) {
        QTimer::singleShot(0, QCoreApplication::instance(),
                           inspectHeadlessEnvironment);
    }
    if (qEnvironmentVariableIsSet("WINGHEX_MODE_TEST")) {
        QTimer::singleShot(0, QCoreApplication::instance(), inspectModeWindow);
    }
}
Q_COREAPP_STARTUP_FUNCTION(installModeProbe)
} // namespace

#endif // MODEPROBE_H
