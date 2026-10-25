// Copyright (c) 2026, wingsummer
// SPDX-License-Identifier: BSD-3-Clause
// See 3rdparty/WingLuauQt/LICENSE for license information.

#include "WingPlugin/binding/builder.h"
#include "WingPlugin/iwingplugin.h"
#include "class/settings/settingmanager.h"
#include "control/console/scriptingconsole.h"
#include "control/objtreewidget.h"
#include "luau/runtime/scriptmachine.h"
#include "luau/runtime/scriptwindowfactory.h"
#include "qtluau/execution.hpp"
#include <QApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QKeyEvent>
#include <QMenuBar>
#include <QPluginLoader>
#include <QStatusBar>
#include <QTemporaryDir>
#include <QTextDocument>
#include <QTimer>
#include <QToolBar>
#include <QTranslator>
#include <QTreeWidgetItemIterator>
#include <iostream>
#include <stdexcept>

static void run(lua_State *state, const QString &source, bool succeeds = true) {
    QString error;
    if (qtluau::executeLuau(state, source, &error) != succeeds) {
        throw std::runtime_error(error.toStdString() +
                                 "\nSource: " + source.toStdString());
    }
}

static void checkPluginDefinitions(const QString &path, bool testPlugin) {
    auto &server = LuauLanguageServer::instance();
    const auto uri = QUrl::fromLocalFile(path);
    QVector<lsp::Diagnostic> diagnostics;
    QTextDocument document(QStringLiteral(
        "--!strict\nlocal value: boolean = NativeProbe.enabled\nlocal "
        "resource: boolean = NativeProbeResource.enabled\nprint(value, "
        "resource)\n"));
    QObject::connect(&server, &LuauLanguageServer::onPublishDiagnostic,
                     &document,
                     [&](const lsp::DocumentUri &target,
                         const QVector<lsp::Diagnostic> &items) {
                         if (target == uri) {
                             diagnostics = items;
                         }
                     });
    server.onOpenDocument(uri, &document);
    if (!diagnostics.isEmpty()) {
        throw std::runtime_error(
            "Plugin file/resource declarations were not loaded: " +
            diagnostics.front().message.toStdString());
    }
    const auto definition = server.gotoDefinition(
        {uri,
         {2,
          static_cast<uint>(
              QStringLiteral("local resource: boolean = NativeProbeResource.")
                  .size() +
              1)}},
        nullptr);
    if (definition.empty() ||
        definition.front().uri.toLocalFile() !=
            QStringLiteral(":/wingluau_test/native_probe_resource.d.luau")) {
        throw std::runtime_error(
            "Plugin resource definition navigation failed");
    }
    document.setPlainText(QStringLiteral("--!strict\nNativeProbe."));
    server.onUpdateDocument(uri, &document);
    const auto completion = server.completion(
        {uri,
         {1, static_cast<uint>(QStringLiteral("NativeProbe.").size())},
         std::nullopt},
        nullptr);
    bool found = false;
    for (const auto &item : completion) {
        found |= item.label == QStringLiteral("enabled");
    }
    if (!found) {
        throw std::runtime_error("Plugin definition completion failed");
    }
    if (testPlugin) {
        document.setPlainText(QStringLiteral(
            "--!strict\nlocal result: number = TestPlugin.testAdd(2, 3)\n"
            "local text: string = TestPlugin.definition_echo('text')\n"
            "local object = TestPlugin.TestObj(7)\nobject.value = 9\n"
            "local value: number = object:getV()\nprint(result, text, "
            "value)\n"));
        server.onUpdateDocument(uri, &document);
        if (!diagnostics.isEmpty()) {
            throw std::runtime_error("TestPlugin declarations failed: " +
                                     diagnostics.front().message.toStdString());
        }
        const auto target = server.gotoDefinition(
            {uri,
             {1,
              static_cast<uint>(
                  QStringLiteral("local result: number = TestPlugin.").size() +
                  1)}},
            nullptr);
        if (target.empty() ||
            target.front().uri.toLocalFile() !=
                QStringLiteral(
                    ":/resources/TestPlugin/script/testplugin.d.luau")) {
            throw std::runtime_error("TestPlugin definition navigation failed");
        }
        document.setPlainText(QStringLiteral("--!strict\nTestPlugin."));
        server.onUpdateDocument(uri, &document);
        const auto pluginCompletion = server.completion(
            {uri,
             {1, static_cast<uint>(QStringLiteral("TestPlugin.").size())},
             std::nullopt},
            nullptr);
        bool foundEcho = false;
        for (const auto &item : pluginCompletion) {
            foundEcho |= item.label == QStringLiteral("definition_echo");
        }
        if (!foundEcho) {
            throw std::runtime_error(
                "TestPlugin definitions did not provide completion");
        }
    }
    server.onCloseDocument(uri);
}

static void checkSymbolTree(lua_State *state) {
    QElapsedTimer timer;
    timer.start();
    ObjTreeWidget tree;
    const auto creationTime = timer.nsecsElapsed() / 1000;
    bool nativeSymbols = false;
    for (QTreeWidgetItemIterator it(&tree); *it; ++it) {
        if ((*it)->text(0) == QStringLiteral("NativeProbe")) {
            (*it)->setExpanded(true);
            nativeSymbols =
                (*it)->childCount() == 1 &&
                (*it)->child(0)->text(0) == QStringLiteral("enabled");
            break;
        }
    }
    if (!nativeSymbols) {
        throw std::runtime_error("Typed plugin symbol index lost its child");
    }
    auto find = [&tree](const QString &id) -> QTreeWidgetItem * {
        for (QTreeWidgetItemIterator it(&tree); *it; ++it) {
            if ((*it)->data(0, Qt::UserRole).toString() == id) {
                return *it;
            }
        }
        return nullptr;
    };
    auto *qt = find(QStringLiteral("module:qt"));
    if (!qt || qt->childCount() != 0) {
        throw std::runtime_error("Qt browser initialized eagerly");
    }
    timer.restart();
    qt->setExpanded(true);
    auto *dialog = find(QStringLiteral("module:qt/QDialog"));
    if (!dialog || dialog->childCount() != 0) {
        throw std::runtime_error("Qt branch expanded recursively");
    }
    dialog->setExpanded(true);
    if (dialog->childCount() == 0) {
        throw std::runtime_error("Dialog methods missing from symbol index");
    }
    tree.setCurrentItem(dialog);
    const auto expansionTime = timer.nsecsElapsed() / 1000;
    tree.refresh();
    if (!tree.currentItem() ||
        tree.currentItem()->data(0, Qt::UserRole).toString() !=
            QStringLiteral("module:qt/QDialog") ||
        !find(QStringLiteral("module:qt"))->isExpanded()) {
        throw std::runtime_error(
            "Browser refresh lost stable selection/expansion");
    }
    auto *messageBox = find(QStringLiteral("module:qt/QMessageBox"));
    if (!messageBox) {
        throw std::runtime_error("Message box symbol missing");
    }
    messageBox->setExpanded(true);
    int warnings = 0;
    QString selectedOverload;
    for (int i = 0; i < messageBox->childCount(); ++i) {
        auto *child = messageBox->child(i);
        if (child->text(0) == QStringLiteral("warning")) {
            ++warnings;
            selectedOverload = child->data(0, Qt::UserRole).toString();
            tree.setCurrentItem(child);
        }
    }
    if (warnings < 2) {
        throw std::runtime_error("Overloads were not displayed separately");
    }
    tree.refresh();
    if (!tree.currentItem() ||
        tree.currentItem()->data(0, Qt::UserRole).toString() !=
            selectedOverload) {
        throw std::runtime_error("Overload selection was lost on refresh");
    }
    for (const auto &language :
         {QStringLiteral("zh_CN"), QStringLiteral("zh_TW")}) {
        QTranslator translator;
        const auto path =
            QCoreApplication::applicationDirPath() +
            QStringLiteral("/.tmp/lang/%1/winghex_%1.qm").arg(language);
        if (!translator.load(path)) {
            throw std::runtime_error("Cannot load browser translation");
        }
        QCoreApplication::installTranslator(&translator);
        tree.refresh();
        const auto expected = language == QStringLiteral("zh_CN")
                                  ? QStringLiteral("标准库")
                                  : QStringLiteral("標準函式庫");
        if (!find(QStringLiteral("category:standard")) ||
            find(QStringLiteral("category:standard"))->text(0) != expected) {
            throw std::runtime_error("Browser category was not translated");
        }
        QCoreApplication::removeTranslator(&translator);
    }
    lua_getfield(state, LUA_REGISTRYINDEX, "qtluau.registration.states");
    lua_getfield(state, -1, "base.widgets");
    const bool untouched = lua_isnil(state, -1);
    lua_pop(state, 2);
    if (!untouched) {
        throw std::runtime_error("Browsing registered the Qt runtime");
    }
    std::cout << "Object tree creation: " << creationTime
              << "us; Qt and dialog expansion: " << expansionTime << "us\n";
}

static void checkHostWindows() {
    auto &settings = SettingManager::instance();
    for (const bool native : {true, false}) {
        settings.setUseNativeTitleBar(native);
        QString error;
        const QByteArray xml =
            R"XML(<ui version="4.0"><class>Window</class><widget class="QMainWindow" name="Window"><widget class="QWidget" name="centralwidget"><layout class="QVBoxLayout" name="layout"><item><widget class="QPushButton" name="button"/></item></layout></widget><widget class="QMenuBar" name="menubar"/><widget class="QStatusBar" name="statusbar"/><widget class="QToolBar" name="toolbar"/></widget><resources/><connections/></ui>)XML";
        auto *root = qtluau::loadUiWidgetXmlNative(xml, &error);
        auto *window = qobject_cast<FramelessMainWindow *>(root);
        if (!window || !ScriptWindowFactory::content(window) ||
            !window->findChild<QToolBar *>(QStringLiteral("toolbar")) ||
            !window->findChild<QStatusBar *>(QStringLiteral("statusbar"))) {
            delete root;
            throw std::runtime_error("Real Frameless UI assembly failed: " +
                                     error.toStdString());
        }
        QPointer<QMenuBar> menu =
            window->findChild<QMenuBar *>(QStringLiteral("menubar"));
        if (!menu) {
            delete root;
            throw std::runtime_error("UI menu was lost");
        }
        auto *content = ScriptWindowFactory::takeContent(window);
        ScriptWindowFactory::setContent(window, new QWidget);
        delete content;
        if (!menu || (!native && window->menuWidget() == menu)) {
            delete root;
            throw std::runtime_error(
                "Content replacement damaged the menu/title bar");
        }
        delete root;
        const auto direct = ScriptWindowFactory::create(
            {QStringLiteral("QMainWindow"), QString(), nullptr, true, true,
             false});
        auto *constructed = qobject_cast<FramelessMainWindow *>(direct.widget);
        QPointer<QWidget> titleBar = constructed->menuWidget();
        auto *createdMenu = ScriptWindowFactory::menu(constructed, true);
        ScriptWindowFactory::setMenu(constructed, new QMenuBar);
        if (!createdMenu ||
            (!native && (!titleBar || constructed->menuWidget() != titleBar))) {
            delete constructed;
            throw std::runtime_error(
                "Direct menu access replaced the Frameless title bar");
        }
        delete constructed;
        const QByteArray dialogXml =
            R"XML(<ui version="4.0"><class>Dialog</class><widget class="QDialog" name="Dialog"><layout class="QVBoxLayout" name="layout"><item><widget class="QLabel" name="label"/></item></layout></widget><resources/><connections/></ui>)XML";
        root = qtluau::loadUiWidgetXmlNative(dialogXml, &error);
        auto *dialog = qobject_cast<FramelessDialogBase *>(root);
        if (!dialog || !ScriptWindowFactory::content(dialog) ||
            !ScriptWindowFactory::content(dialog)->layout()) {
            delete root;
            throw std::runtime_error("Real Frameless dialog assembly failed");
        }
        delete root;
        const QByteArray absoluteXml =
            R"XML(<ui version="4.0"><class>Absolute</class><widget class="QDialog" name="Absolute"><property name="geometry"><rect><x>11</x><y>13</y><width>426</width><height>243</height></rect></property><property name="wingScriptWindowReady" stdset="0"><bool>true</bool></property><widget class="QLabel" name="absolute_label"><property name="geometry"><rect><x>12</x><y>23</y><width>100</width><height>30</height></rect></property></widget></widget><resources/><connections/></ui>)XML";
        root = qtluau::loadUiWidgetXmlNative(absoluteXml, &error);
        auto *label =
            root ? root->findChild<QWidget *>(QStringLiteral("absolute_label"))
                 : nullptr;
        if (!root || !label || root->size() != QSize(426, 243) ||
            label->parentWidget() != ScriptWindowFactory::content(root)) {
            delete root;
            throw std::runtime_error(
                "UI geometry/absolute child assembly was lost");
        }
        delete root;
        auto *ordinary = new FramelessDialogBase;
        ordinary->buildUpContent(new QWidget);
        auto *layout = ordinary->layout();
        ScriptWindowFactory::finalize(ordinary);
        if (ordinary->layout() != layout ||
            ScriptWindowFactory::content(ordinary) != ordinary) {
            delete ordinary;
            throw std::runtime_error(
                "Finalizer changed an ordinary host dialog");
        }
        delete ordinary;
    }
}

int main(int argc, char **argv) {
    QTemporaryDir directory;
    qputenv("XDG_CONFIG_HOME", directory.path().toUtf8());
    qputenv("XDG_DATA_HOME", directory.path().toUtf8());
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    QApplication::setOrganizationName(QStringLiteral("WingLuauProbe"));
    QApplication::setApplicationName(QStringLiteral("HostInputProbe"));
    auto &machine = ScriptMachine::instance();
    QPluginLoader testPluginLoader;
    const bool testPlugin =
        app.arguments().contains(QStringLiteral("--test-plugin"));
    try {
        if (!machine.init()) {
            throw std::runtime_error("Cannot initialize real host bindings");
        }
        auto *interactive = machine.contextState(ScriptMachine::Interactive);
        // These tests call helpers directly, outside AppManager's scheduler.
        lua_callbacks(interactive)->interrupt = nullptr;
        QFile definitionFile(directory.filePath("native_probe.d.luau"));
        if (!definitionFile.open(QIODevice::WriteOnly)) {
            throw std::runtime_error("Cannot create plugin definition fixture");
        }
        definitionFile.write("declare NativeProbe: { enabled: boolean }\n");
        definitionFile.close();
        {
            auto scope = machine.scriptEngine()->global().beginNamespace(
                QStringLiteral("NativeProbe"));
            scope.addConstant("enabled", true)
                .addDefinitionFile(definitionFile.fileName())
                .addDefinitionFile(definitionFile.fileName());
            bool rejected = false;
            try {
                scope.addDefinitionFile(
                    QStringLiteral("relative_missing.d.luau"));
            } catch (const WingScriptBind::ScriptException &error) {
                rejected = error.getError() ==
                           WingScriptBind::Exception::InvalidArgument;
            }
            if (!rejected) {
                throw std::runtime_error(
                    "Invalid definition path was accepted");
            }
            auto resource = machine.scriptEngine()->global().beginNamespace(
                QStringLiteral("NativeProbeResource"));
            resource.addConstant("enabled", true)
                .addDefinitionFile(QStringLiteral(
                    ":/wingluau_test/native_probe_resource.d.luau"));
        }
        if (static_cast<WingScriptBind::ScriptBindingEngine *>(
                machine.scriptEngine())
                ->definitionFiles()
                .size() != 2) {
            throw std::runtime_error(
                "Definition registration was not deduplicated");
        }
        if (testPlugin) {
            testPluginLoader.setFileName(
                QCoreApplication::applicationDirPath() +
                QStringLiteral("/plugin/TestPlugin.wingplg"));
            auto *plugin = qobject_cast<WingHex::IWingPlugin *>(
                testPluginLoader.instance());
            if (!plugin) {
                throw std::runtime_error(
                    testPluginLoader.errorString().toStdString());
            }
            plugin->onRegisterScriptObj(
                machine.scriptEngine()->global().beginNamespace(
                    QStringLiteral("TestPlugin")));
            if (static_cast<WingScriptBind::ScriptBindingEngine *>(
                    machine.scriptEngine())
                    ->definitionFiles()
                    .size() != 3) {
                throw std::runtime_error("TestPlugin resource definition "
                                         "registration was not deduplicated");
            }
        }
        const auto snapshot =
            static_cast<WingScriptBind::ScriptBindingEngine *>(
                machine.scriptEngine())
                ->symbolCatalog();
        bool foundConstant = false;
        for (const auto &symbol : snapshot) {
            foundConstant |=
                symbol.label == QStringLiteral("enabled") &&
                symbol.kind == WingScriptBind::ScriptSymbol::Kind::Constant &&
                !symbol.parent.isEmpty();
        }
        if (!foundConstant) {
            throw std::runtime_error(
                "Typed native symbol snapshot lost a registration");
        }
        machine.finishScriptRegistration();
        checkPluginDefinitions(directory.filePath("plugin_usage.luau"),
                               testPlugin);
        if (testPlugin) {
            run(interactive, QStringLiteral(R"LUA(
assert(TestPlugin.testAdd(2, 3) == 5)
assert(TestPlugin.definition_echo(7) == 7)
assert(TestPlugin.definition_echo("中文 😀") == "中文 😀")
local object = TestPlugin.TestObj(7)
assert(object:getV() == 7)
object.value = 9
assert(object:getV() == 9)
local ok, err = pcall(TestPlugin.raiseScriptException)
assert(not ok and string.find(err, "raiseScriptException", 1, true))
)LUA"));
            std::cout
                << "Real TestPlugin runtime and resource definitions passed\n";
        }
        checkSymbolTree(interactive);
        ScriptingConsole console;
        console.init();
        console.show();
        for (auto mode : {ScriptMachine::Interactive, ScriptMachine::Scripting,
                          ScriptMachine::Background}) {
            auto *context = machine.context(mode);
            if (!context->state) {
                context->state = lua_newthread(interactive);
                context->refID = lua_ref(interactive, -1);
                lua_pop(interactive, 1);
                lua_setthreaddata(context->state, context->data);
            }
            ScriptMachine::RegCallBacks callbacks;
            QString output;
            callbacks.printMsgFn =
                [&output](const ScriptMachine::MessageInfo &message) {
                    output += message.message;
                };
            callbacks.getInputFn = [&console, mode] {
                return console.getInput(mode);
            };
            machine.registerCallBack(mode, callbacks);
            QTimer::singleShot(0, &console, [&console] {
                console.insertPlainText(QStringLiteral("hello world"));
                QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return,
                                Qt::NoModifier);
                QApplication::sendEvent(&console, &enter);
            });
            run(context->state, QStringLiteral("print(input())"));
            if (output != QStringLiteral("hello world ")) {
                throw std::runtime_error(
                    "print(input()) did not print its text");
            }
            output.clear();
            for (const auto &text : {QString::fromUtf8("  中文 😀  "),
                                     QStringLiteral("  "), QString()}) {
                QTimer::singleShot(0, &console, [&console, text] {
                    console.insertPlainText(text);
                    QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return,
                                    Qt::NoModifier);
                    QApplication::sendEvent(&console, &enter);
                });
                const QString quoted =
                    text.isEmpty() ? QStringLiteral("\"\"") : '"' + text + '"';
                run(context->state,
                    QStringLiteral("local value = input(); print(value); "
                                   "assert(tostring(value) == ") +
                        quoted +
                        "); assert(value:to_std_string() == " + quoted + ')');
                if (output != text + ' ') {
                    throw std::runtime_error(
                        "Input output lost Unicode text: " +
                        output.toStdString());
                }
                output.clear();
            }
            QTimer::singleShot(0, &console, [&console] {
                console.insertPlainText(QString::fromUtf8("协程 😀"));
                QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return,
                                Qt::NoModifier);
                QApplication::sendEvent(&console, &enter);
            });
            run(context->state,
                QStringLiteral("local wrapped = coroutine.wrap(function(value) "
                               "coroutine.yield(value); return "
                               "input():to_std_string() end)\n"
                               "assert(wrapped(7) == 7)\n"
                               "assert(wrapped() == '协程 😀')"));
            QTimer::singleShot(0, &console,
                               [&machine, mode] { machine.abortScript(mode); });
            run(context->state, QStringLiteral("input()"), false);
            context->requestStop = false;
            QTimer::singleShot(0, &console, [&console] { console.hide(); });
            run(context->state,
                QStringLiteral("assert(input():to_std_string() == '')"));
            console.show();
            QTimer::singleShot(0, &console, [&console] { console.close(); });
            run(context->state,
                QStringLiteral("assert(input():to_std_string() == '')"));
            console.show();
            if (console.mode() != QConsoleWidget::Output) {
                throw std::runtime_error(
                    "Console input state was not restored");
            }
            // No input provider is a valid empty ustring, including in a child
            // coroutine with inherited LuauThreadData.
            machine.registerCallBack(mode, {});
            run(context->state,
                QStringLiteral("local co = coroutine.create(function() "
                               "assert(input():to_std_string() == '') end)\n"
                               "assert(coroutine.resume(co))"));
        }
        run(interactive,
            QStringLiteral(
                "assert(type(stringify({x = 1}):to_std_string()) == "
                "'string')\n"
                "assert_x(true, ustring('中文'))\n"
                "local ok, err = pcall(assert_x, false, "
                "ustring('中文'))\n"
                "assert(not ok and string.find(err, '中文', 1, true))"));
        checkHostWindows();
        machine.destoryMachine();
        std::cout << "Host input and helper checks passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        machine.destoryMachine();
        return 1;
    }
}
