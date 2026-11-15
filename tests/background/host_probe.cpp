// Copyright (C) 2026 WingSummer
// SPDX-License-Identifier: AGPL-3.0-only
#include <cstdio>
#include <functional>

#include <QAction>
#include <QApplication>
#include <QDeadlineTimer>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPointer>
#include <QSettings>
#include <QStackedWidget>
#include <QTemporaryDir>
#include <QTimer>
#include <QTranslator>

#include "control/console/backgroundscriptoutput.h"
#include "luau/runtime/scriptmachine.h"

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition);  \
            return false;                                                      \
        }                                                                      \
    } while (false)

namespace {
QHash<quint64, QPointer<QObject>> trackedObjects;

int trackObject(lua_State *state) {
    auto *object = new QObject;
    trackedObjects.insert(ScriptMachine::contextData(state)->taskId, object);
    qtluau::trackContextObject(state, object);
    return 0;
}

bool waitFor(const std::function<bool()> &condition) {
    QDeadlineTimer deadline(10000);
    while (!condition() && !deadline.hasExpired()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }
    return condition();
}

QString writeScript(const QString &directory, const QString &name,
                    const QByteArray &source) {
    const auto path =
        directory + QLatin1Char('/') + name + QStringLiteral(".luau");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly) ||
        file.write(source) != source.size()) {
        qFatal("Could not create test script");
    }
    return path;
}

bool verifyRuntime(const QString &directory) {
    auto &machine = ScriptMachine::instance();
    writeScript(directory, QStringLiteral("shared"), "return {count=0}");
    const auto path = writeScript(
        directory, QStringLiteral("resident"),
        "assert(value == nil); value = 42; "
        "local shared=require('./shared'); assert(shared.count==0); "
        "shared.count=1; "
        "require('@b/background_probe').track(); println('isolated')");
    QList<ScriptExecution *> tasks;
    QList<quint64> ids;
    QHash<quint64, QString> output;
    ScriptMachine::RegCallBacks callbacks;
    callbacks.printMsgFn =
        [&output](const ScriptMachine::MessageInfo &message) {
            if (message.type == ScriptMachine::MessageType::Print) {
                output[message.taskId] += message.message;
            }
        };
    machine.registerCallBack(ScriptMachine::Background, callbacks);
    for (int index = 0; index < 16; ++index) {
        const auto created = machine.createBackgroundExecution(
            path, ScriptExecution::Lifetime::Resident);
        CHECK(created.result == ScriptMachine::ExecutionResult::Success);
        CHECK(created.execution);
        tasks.append(created.execution);
        CHECK(!ids.contains(created.execution->taskId()));
        ids.append(created.execution->taskId());
    }
    CHECK(machine.activeBackgroundTasks() == 16);
    CHECK(machine.isRunning(ScriptMachine::Background));
    CHECK(
        machine
            .createBackgroundExecution(path, ScriptExecution::Lifetime::OneShot)
            .result == ScriptMachine::ExecutionResult::Capacity);
    tasks.last()->stop();
    CHECK(machine.activeBackgroundTasks() == 16);
    CHECK(
        waitFor([&machine] { return machine.activeBackgroundTasks() == 15; }));
    delete tasks.takeLast();
    const auto replacement = machine.createBackgroundExecution(
        path, ScriptExecution::Lifetime::Resident);
    CHECK(replacement.execution);
    tasks.append(replacement.execution);
    for (auto *task : tasks) {
        task->start();
    }
    CHECK(waitFor([&tasks] {
        for (const auto *task : tasks) {
            if (task->state() != ScriptExecution::State::Resident) {
                return false;
            }
        }
        return true;
    }));
    CHECK(machine.activeBackgroundTasks() == 16);
    for (auto *task : tasks) {
        CHECK(output.value(task->taskId()).trimmed() ==
              QStringLiteral("isolated"));
        CHECK(trackedObjects.value(task->taskId()));
    }
    machine.abortBackgroundTask(tasks.first()->taskId());
    CHECK(waitFor([&tasks] { return !tasks.first()->active(); }));
    CHECK(tasks.last()->state() == ScriptExecution::State::Resident);
    CHECK(trackedObjects.value(tasks.first()->taskId()).isNull());
    CHECK(trackedObjects.value(tasks.last()->taskId()));
    const auto cachePath =
        writeScript(directory, QStringLiteral("cache-loop"),
                    "local m=require('./shared'); m.count=1; "
                    "while true do assert(require('./shared').count==1); "
                    "coroutine.yield() end");
    const auto cacheLoop = machine.createBackgroundExecution(
        cachePath, ScriptExecution::Lifetime::OneShot);
    CHECK(cacheLoop.execution);
    cacheLoop.execution->start();
    const auto consolePath =
        writeScript(directory, QStringLiteral("console"), "return");
    bool consoleDone = false;
    machine.executeScriptWithResult(
        ScriptMachine::Scripting, consolePath, false,
        [&consoleDone](ScriptMachine::ExecutionResult result) {
            consoleDone = result == ScriptMachine::ExecutionResult::Success;
        });
    CHECK(waitFor([&consoleDone] { return consoleDone; }));
    QEventLoop cacheSlice;
    QTimer::singleShot(50, &cacheSlice, &QEventLoop::quit);
    cacheSlice.exec();
    CHECK(cacheLoop.execution->active());
    cacheLoop.execution->stop();
    CHECK(waitFor([&cacheLoop] { return !cacheLoop.execution->active(); }));
    CHECK(cacheLoop.execution->state() == ScriptExecution::State::Aborted);
    delete cacheLoop.execution;
    machine.abortScript(ScriptMachine::Background);
    CHECK(waitFor([&machine] { return machine.activeBackgroundTasks() == 0; }));
    for (const auto *task : tasks) {
        CHECK(trackedObjects.value(task->taskId()).isNull());
    }
    qDeleteAll(tasks);
    for (const auto &entry :
         QList<QPair<QByteArray, ScriptMachine::ExecutionResult>>{
             {"println('once')", ScriptMachine::ExecutionResult::Success},
             {"local = invalid", ScriptMachine::ExecutionResult::Failed},
             {"error('failure')", ScriptMachine::ExecutionResult::Failed}}) {
        const auto script =
            writeScript(directory, QStringLiteral("once"), entry.first);
        int completions = 0;
        ScriptMachine::ExecutionResult result =
            ScriptMachine::ExecutionResult::Busy;
        machine.executeScriptWithResult(
            ScriptMachine::Background, script, false,
            [&completions, &result](ScriptMachine::ExecutionResult finished) {
                ++completions;
                result = finished;
            });
        CHECK(waitFor([&completions] { return completions == 1; }));
        CHECK(result == entry.second);
        CHECK(machine.activeBackgroundTasks() == 0);
    }
    const auto callback =
        writeScript(directory, QStringLiteral("callback"),
                    "local c=require('@b/core'); local t=c.timer(); "
                    "t:set_single_shot(true); t:connect('timeout', function() "
                    "error('callback failure') end); t:start(0); "
                    "while true do coroutine.yield() end");
    bool callbackFailed = false;
    machine.executeScriptWithResult(
        ScriptMachine::Background, callback, false,
        [&callbackFailed](ScriptMachine::ExecutionResult result) {
            callbackFailed = result == ScriptMachine::ExecutionResult::Failed;
        });
    CHECK(waitFor([&callbackFailed] { return callbackFailed; }));
    CHECK(machine.activeBackgroundTasks() == 0);
    int completions = 0;
    machine.executeScriptWithResult(
        ScriptMachine::Background, directory + QStringLiteral("/missing.luau"),
        false, [&completions](ScriptMachine::ExecutionResult result) {
            if (result == ScriptMachine::ExecutionResult::FileError) {
                ++completions;
            }
        });
    CHECK(completions == 1);
    CHECK(machine.activeBackgroundTasks() == 0);
    bool chunkDone = false;
    machine.executeChunk(
        ScriptMachine::Background, QByteArrayLiteral("println('chunk')"),
        QStringLiteral("chunk.luau"), ScriptChunkFormat::Source, false,
        [&chunkDone](bool finished) { chunkDone = finished; });
    CHECK(waitFor([&chunkDone] { return chunkDone; }));
    machine.registerCallBack(ScriptMachine::Background, {});
    return true;
}

bool verifyPanel(const QString &directory) {
    auto &machine = ScriptMachine::instance();
    BackgroundScriptOutput panel;
    panel.resize(850, 320);
    panel.show();
    ScriptMachine::RegCallBacks callbacks;
    QHash<QString, quint64> pageIds;
    callbacks.printMsgFn =
        [&panel, &pageIds](const ScriptMachine::MessageInfo &message) {
            if (!message.section.isEmpty()) {
                pageIds.insert(message.section, message.taskId);
            }
            panel.appendOutput(message);
        };
    machine.registerCallBack(ScriptMachine::Background, callbacks);
    machine.setBackgroundInputHandler(
        [&panel](quint64 taskId) { return panel.requestInput(taskId); });
    auto *list = panel.findChild<QListWidget *>();
    auto *stack = panel.findChild<QStackedWidget *>();
    CHECK(list && stack);
    const auto path = writeScript(directory, QStringLiteral("page"),
                                  "println('page output')");
    panel.runScript(path);
    panel.runScript(path);
    CHECK(list->count() == 1);
    CHECK(machine.activeBackgroundTasks() == 1);
    CHECK(waitFor([&machine] { return machine.activeBackgroundTasks() == 0; }));
    auto *editor = qobject_cast<QPlainTextEdit *>(stack->currentWidget());
    CHECK(editor &&
          editor->toPlainText().contains(QStringLiteral("page output")));
    editor->appendPlainText(QStringLiteral("old output must disappear"));
    panel.runScript(
        QFileInfo(path).dir().filePath(QStringLiteral("./page.luau")));
    CHECK(list->count() == 1);
    CHECK(editor->toPlainText().isEmpty());
    CHECK(waitFor([&machine] { return machine.activeBackgroundTasks() == 0; }));
    CHECK(QDir(directory).mkdir(QStringLiteral("alternate")));
    const auto duplicate =
        writeScript(directory + QStringLiteral("/alternate"),
                    QStringLiteral("page"), "println('other directory')");
    panel.runScript(duplicate);
    CHECK(waitFor([&machine] { return machine.activeBackgroundTasks() == 0; }));
    CHECK(list->count() == 2);
    ScriptMachine::MessageInfo unread;
    unread.taskId = pageIds.value(QFileInfo(path).canonicalFilePath());
    unread.type = ScriptMachine::MessageType::Print;
    unread.message = QStringLiteral("unread output");
    panel.appendOutput(unread);
    CHECK(list->currentRow() == 1);
    CHECK(list->item(0)->text().startsWith(QStringLiteral("• ")));
    list->setCurrentRow(0);
    CHECK(!list->item(0)->text().startsWith(QStringLiteral("• ")));
    for (int index = 0; index < 16; ++index) {
        const auto next =
            writeScript(directory, QStringLiteral("script-%1").arg(index),
                        QByteArrayLiteral("println('different output')"));
        panel.runScript(next);
        CHECK(waitFor(
            [&machine] { return machine.activeBackgroundTasks() == 0; }));
    }
    CHECK(list->count() == 16);
    CHECK(stack->count() == 17);
    for (int row = 0; row < list->count(); ++row) {
        CHECK(!list->item(row)->text().contains(QStringLiteral("page.luau")));
    }
    // Reusing a page must ignore notifications from an older run.
    ScriptMachine::MessageInfo stale;
    stale.taskId = 1;
    stale.message = QStringLiteral("stale message");
    panel.appendOutput(stale);
    editor = qobject_cast<QPlainTextEdit *>(stack->currentWidget());
    CHECK(!editor->toPlainText().contains(stale.message));
    auto actions = panel.findChildren<QAction *>();
    QAction *stop = nullptr;
    QAction *close = nullptr;
    for (auto *action : actions) {
        if (action->text() == BackgroundScriptOutput::tr("Terminate script")) {
            stop = action;
        }
        if (action->text() == BackgroundScriptOutput::tr("Close output")) {
            close = action;
        }
    }
    CHECK(stop && close);
    CHECK(close->isEnabled() && !stop->isEnabled());
    close->trigger();
    CHECK(list->count() == 15);
    const auto loop =
        writeScript(directory, QStringLiteral("loop"), "while true do end");
    panel.runScript(loop);
    panel.runScript(loop);
    CHECK(list->count() == 16);
    CHECK(!close->isEnabled() && stop->isEnabled());
    QEventLoop slice;
    QTimer::singleShot(50, &slice, &QEventLoop::quit);
    slice.exec();
    CHECK(machine.activeBackgroundTasks() == 1);
    stop->trigger();
    CHECK(!close->isEnabled() && !stop->isEnabled());
    CHECK(waitFor([&machine] { return machine.activeBackgroundTasks() == 0; }));
    CHECK(close->isEnabled());
    const auto input =
        writeScript(directory, QStringLiteral("input"), "input()");
    panel.runScript(input);
    bool cancelled = false;
    QTimer cancel;
    QObject::connect(&cancel, &QTimer::timeout, &panel, [&] {
        if (panel.findChild<QInputDialog *>()) {
            cancelled = true;
            stop->trigger();
            cancel.stop();
        }
    });
    cancel.start(10);
    CHECK(waitFor([&machine] { return machine.activeBackgroundTasks() == 0; }));
    CHECK(cancelled);
    panel.resize(360, 220);
    QCoreApplication::processEvents();
    CHECK(stack->width() >= 100);
    panel.resize(850, 320);
    QCoreApplication::processEvents();
    const auto screenshot =
        qEnvironmentVariable("WINGHEX_BACKGROUND_SCREENSHOT");
    if (!screenshot.isEmpty()) {
        panel.grab().save(screenshot);
    }
    machine.setBackgroundInputHandler({});
    machine.registerCallBack(ScriptMachine::Background, {});
    return true;
}
} // namespace

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    QTemporaryDir config;
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                       config.path());
    app.setOrganizationName(QStringLiteral("WingHexTests"));
    app.setApplicationName(QStringLiteral("BackgroundProbe"));
    QTranslator translator;
    const auto translation =
        qEnvironmentVariable("WINGHEX_BACKGROUND_TRANSLATION");
    if (!translation.isEmpty()) {
        if (!translator.load(translation)) {
            return 3;
        }
        app.installTranslator(&translator);
    }
    auto &machine = ScriptMachine::instance();
    if (!machine.init({}, {true, false, false, false})) {
        return 2;
    }
    machine.registerBuiltin(QStringLiteral("background_probe"),
                            [](lua_State *state) {
                                lua_newtable(state);
                                lua_pushcfunction(state, trackObject, "track");
                                lua_setfield(state, -2, "track");
                            });
    machine.finishScriptRegistration();
    const auto success =
        verifyRuntime(config.path()) && verifyPanel(config.path());
    machine.destoryMachine();
    std::puts(success ? "Background runtime and panel checks passed"
                      : "Background checks failed");
    return success ? 0 : 1;
}
