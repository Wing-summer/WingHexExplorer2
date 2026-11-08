// Copyright (C) 2026 WingSummer
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <cstring>
#include <iostream>
#include <thread>

#include <QApplication>
#include <QEventLoop>
#include <QFile>
#include <QTemporaryDir>
#include <QTimer>
#include <QTreeWidgetItemIterator>
#include <QUndoStack>

#include <Luau/Compiler.h>

#include "WingPlugin/iwingplugin.h"

#include "class/application/appmanager.h"
#include "class/file/workspacemanager.h"
#include "class/plugin/pluginsystem.h"
#include "class/settings/settingmanager.h"
#include "class/settings/thememanager.h"
#include "control/objtreewidget.h"
#include "dialog/mainwindow.h"
#include "luau/runtime/scriptdocumentregistry.h"
#include "luau/runtime/scriptmachine.h"
#include "luau/wingluaurequire.h"
using namespace WingHex;
#define REQUIRE(expression)                                                    \
    do {                                                                       \
        if (!(expression)) {                                                   \
            std::cerr << "FAIL line " << __LINE__ << ": " << #expression       \
                      << '\n';                                                 \
            return false;                                                      \
        }                                                                      \
    } while (false)

class QueryPlugin final : public IWingPlugin {
    Q_OBJECT
    Q_INTERFACES(WingHex::IWingPlugin)
public:
    bool init(const std::unique_ptr<QSettings> &) override { return true; }

    void unload(std::unique_ptr<QSettings> &) override {}

    const QString pluginName() const override {
        return QStringLiteral("query");
    }

    const QString pluginComment() const override { return {}; }
};

class ProbeDevice final : public WingIODevice {
public:
    QByteArray bytes = QByteArrayLiteral("device-data");

    qint64 size() const override { return bytes.size(); }

    bool isSequential() const override { return false; }

    bool seek(qint64 offset) override {
        return offset >= 0 && offset <= size() && QIODevice::seek(offset);
    }

    bool defaultIsLocked() const override { return false; }

protected:
    qint64 readData(char *data, qint64 length) override {
        const auto count = std::min(length, size() - pos());
        std::memcpy(data, bytes.constData() + pos(), size_t(count));
        return count;
    }

    qint64 writeData(const char *data, qint64 length) override {
        const auto count = std::min(length, size() - pos());
        std::memcpy(bytes.data() + pos(), data, size_t(count));
        return count;
    }
};

class ProbeDriver final : public IWingDevice {
    Q_OBJECT
    Q_INTERFACES(WingHex::IWingDevice)
public:
    bool allowClose = true;
    int opens = 0;
    int closes = 0;
    QString lastPath;

    bool init(const std::unique_ptr<QSettings> &) override { return true; }

    void unload(std::unique_ptr<QSettings> &) override {}

    const QString pluginName() const override {
        return QStringLiteral("driver");
    }

    const QString pluginComment() const override { return {}; }

    QString supportedFileExtDisplayName() const override {
        return QStringLiteral("driver");
    }

    WingIODevice *onOpenFile(const QString &path) override {
        ++opens;
        lastPath = path;
        return new ProbeDevice;
    }

    bool onCloseFile(WingIODevice *) override {
        ++closes;
        return allowClose;
    }
};

struct LuaOwner {
    lua_State *state = luaL_newstate();

    ~LuaOwner() {
        qtluau::clearLuaCallbacks(state);
        lua_close(state);
    }
};

static bool runLua(lua_State *L, const char *source) {
    const auto bytecode = Luau::compile(source);
    if (luau_load(L, "=hexdoc-host", bytecode.data(), bytecode.size(), 0) !=
            LUA_OK ||
        lua_pcall(L, 0, 0, 0) != LUA_OK) {
        std::cerr << lua_tostring(L, -1) << '\n';
        return false;
    }
    return true;
}

class PluginDocumentHostProbe {
    static EditorView *findView(const QString &path) {
        for (auto *view : EditorView::instances()) {
            if (!view->isClosed() &&
                view->fileNameUrl() == WingHex::fileUrl(path)) {
                return view;
            }
        }
        return nullptr;
    }

    static bool queryDocuments(PluginSystem &host, MainWindow &window,
                               const QString &path, const QString &workspace,
                               const QString &deviceWorkspace) {
        QueryPlugin plugin, other;
        host.applyFunctionTables(&plugin);
        host.applyFunctionTables(&other);
        const auto fileUrl = WingHex::fileUrl(path);
        const auto workspaceUrl = WingHex::workspaceUrl(workspace);
        const auto deviceUrl = WingHex::deviceUrl(
            QStringLiteral("probedriver"), QStringLiteral("/query 中文 # %?"));
        REQUIRE(plugin.openedFiles().isEmpty());
        REQUIRE(!plugin.isFileOpened(-1));
        const auto a = plugin.openFile(fileUrl);
        const auto alias = plugin.openFile(fileUrl);
        const auto b = other.openFile(fileUrl);
        REQUIRE(a >= 0 && alias == a && b >= 0);
        REQUIRE(plugin.isFileOpened(a));
        REQUIRE(!other.isFileOpened(a) && !plugin.isFileOpened(b));
        REQUIRE(plugin.openedFiles() == QList<QUrl>{fileUrl});
        REQUIRE(other.openedFiles() == QList<QUrl>{fileUrl});
        REQUIRE(plugin.allOpenedFiles().count(fileUrl) == 1);
        bool workerQueries = false;
        std::thread reader([&] {
            workerQueries = plugin.isFileOpened(a) &&
                            plugin.openedFiles() == QList<QUrl>{fileUrl} &&
                            plugin.allOpenedFiles().count(fileUrl) == 1;
        });
        reader.join();
        REQUIRE(workerQueries);
        REQUIRE(plugin.closeFileHandle(a) == ErrFile::Success);
        REQUIRE(!plugin.isFileOpened(a) && !plugin.isFileOpened(alias));
        REQUIRE(plugin.closeFileHandle(alias) == ErrFile::NotExist);
        REQUIRE(plugin.openedFiles().isEmpty());
        REQUIRE(other.allOpenedFiles().contains(fileUrl));
        REQUIRE(other.closeFile(b, false) == ErrFile::Success);
        REQUIRE(!other.isFileOpened(b));
        REQUIRE(!plugin.allOpenedFiles().contains(fileUrl));

        auto ids = std::make_shared<ScriptDocumentIds>();
        auto current = [&window] { return window.currentEditor(); };
        auto open = [&host](const QUrl &url, EditorView **view) {
            return host.openDocument(url, view);
        };
        auto close = [&host](EditorView *view, bool force) {
            return host.closeDocument(view, force);
        };
        ScriptDocumentContext first(current, open, close, ids, 1);
        ScriptDocumentContext second(current, open, close, ids, 2);
        for (const auto &url : {fileUrl, deviceUrl, workspaceUrl,
                                WingHex::workspaceUrl(deviceWorkspace)}) {
            const auto handle = plugin.openFile(url);
            REQUIRE(handle >= 0);
            const auto one = first.openFile(url);
            const auto two = first.openFile(url);
            const auto foreign = second.openFile(url);
            REQUIRE(one >= 0 && two == one && foreign >= 0);
            REQUIRE(first.isFileOpened(one) && !first.isFileOpened(foreign));
            REQUIRE(!second.isFileOpened(one));
            REQUIRE(plugin.openedFiles() == QList<QUrl>{url});
            REQUIRE(first.openedFiles() == QList<QUrl>{url});
            REQUIRE(second.openedFiles() == QList<QUrl>{url});
            REQUIRE(plugin.allOpenedFiles().count(url) == 1);
            auto *view = host.handle2EditorView(&plugin, handle);
            REQUIRE(view);
            REQUIRE(view->openedFileUrl() == url);
            if (url == fileUrl) {
                auto *document = view->hexEditor()->document().data();
                auto *undo = document->undoStack();
                const auto original = document->read(0, 1);
                REQUIRE(first.switchDocument(one));
                REQUIRE(!first.endMacro());
                REQUIRE(first.beginMacro(QStringLiteral("outer")));
                REQUIRE(first.beginMacro(QStringLiteral("inner")));
                REQUIRE(document->Replace(view->hexEditor()->cursor(), 0,
                                          QByteArrayLiteral("Z")));
                REQUIRE(first.endMacro());
                REQUIRE(!undo->canUndo());
                REQUIRE(!first.switchDocument(two));
                REQUIRE(first.closeFileHandle(one) == ErrFile::Error);
                REQUIRE(first.endMacro());
                REQUIRE(!first.endMacro());
                REQUIRE(undo->canUndo());
                undo->undo();
                REQUIRE(document->read(0, 1) == original);

                REQUIRE(first.beginMacro(QStringLiteral("reset outer")));
                REQUIRE(first.beginMacro(QStringLiteral("reset inner")));
                REQUIRE(document->Replace(view->hexEditor()->cursor(), 0,
                                          QByteArrayLiteral("Z")));
                REQUIRE(first.resetMacro());
                REQUIRE(!first.endMacro());
                REQUIRE(document->read(0, 1) == original);
                REQUIRE(first.switchDocument(two));

                {
                    ScriptDocumentContext unfinished(current, open, close, ids,
                                                     3);
                    REQUIRE(unfinished.bind(view) >= 0);
                    REQUIRE(unfinished.beginMacro(QStringLiteral("cleanup")));
                    REQUIRE(unfinished.beginMacro(QStringLiteral("nested")));
                    REQUIRE(document->Replace(view->hexEditor()->cursor(), 0,
                                              QByteArrayLiteral("Z")));
                }
                REQUIRE(undo->canUndo());
                undo->undo();
                REQUIRE(document->read(0, 1) == original);
            }

            if (url.scheme() == QStringLiteral("wingpro")) {
                REQUIRE(view->fileNameUrl() != url);
            }
            {
                auto clone = std::unique_ptr<EditorView>(view->clone());
                REQUIRE(clone && clone->workSpaceName().isEmpty());
                const auto cloneHandle = host.assginHandleForOpenPluginView(
                    &plugin, clone.get(), false);
                const auto scriptClone = first.bind(clone.get(), false);
                REQUIRE(cloneHandle == handle && scriptClone == one);
                REQUIRE(host.handle2EditorView(&plugin, cloneHandle) == view);
                REQUIRE(first.switchDocument(scriptClone));
                REQUIRE(first.view() == view);
                REQUIRE(plugin.openedFiles().count(url) == 1);
                REQUIRE(first.openedFiles().count(url) == 1);
                REQUIRE(second.openedFiles().count(url) == 1);
                REQUIRE(plugin.allOpenedFiles().count(url) == 1);
                REQUIRE(clone->openedFileUrl() == url);
                {
                    QueryPlugin cloneOnlyPlugin;
                    host.applyFunctionTables(&cloneOnlyPlugin);
                    host.applyFunctionTables(&cloneOnlyPlugin);
                    const auto onlyClone = host.assginHandleForOpenPluginView(
                        &cloneOnlyPlugin, clone.get(), false);
                    REQUIRE(onlyClone >= 0);
                    REQUIRE(cloneOnlyPlugin.isFileOpened(onlyClone));
                    REQUIRE(cloneOnlyPlugin.openedFiles() == QList<QUrl>{url});
                    ScriptDocumentContext cloneOnlyContext(current, open, close,
                                                           ids, 4);
                    const auto onlyScriptClone =
                        cloneOnlyContext.bind(clone.get());
                    REQUIRE(onlyScriptClone >= 0);
                    REQUIRE(cloneOnlyContext.isFileOpened(onlyScriptClone));
                    REQUIRE(cloneOnlyContext.openedFiles() == QList<QUrl>{url});
                    REQUIRE(cloneOnlyContext.view() == view);
                }
            }
            REQUIRE(first.closeFileHandle(one) == ErrFile::Success);
            REQUIRE(!first.isFileOpened(two));
            REQUIRE(first.closeFileHandle(two) == ErrFile::NotExist);
            REQUIRE(first.openedFiles().isEmpty());
            REQUIRE(plugin.closeFile(handle, true) == ErrFile::Success);
            REQUIRE(!second.isFileOpened(foreign));
            REQUIRE(second.openedFiles().isEmpty());
            REQUIRE(!plugin.allOpenedFiles().contains(url));
            REQUIRE(second.closeFileHandle(foreign) == ErrFile::Success);
        }
        return true;
    }

public:
    static bool run() {
        auto &settings = SettingManager::instance();
        settings.setScriptEnabled(false);
        settings.setCheckUpdate(false);
        ThemeManager::instance();
        auto &window = *AppManager::instance()->mainWindow();
        auto &host = PluginSystem::instance();
        ProbeDriver driver;
        PluginInfo driverInfo;
        driverInfo.id = QStringLiteral("probedriver");
        host.m_loadeddevs.append(&driver);
        host.m_pinfos.insert(&driver, driverInfo);
        QTemporaryDir directory;
        REQUIRE(directory.isValid());
        const auto path = directory.filePath(QStringLiteral("中文 # %.bin"));
        {
            QFile file(path);
            REQUIRE(file.open(QIODevice::WriteOnly));
            REQUIRE(file.write("abcdefgh") == 8);
        }
        const auto extraPath = directory.filePath(QStringLiteral("extra.bin"));
        REQUIRE(QFile::copy(path, extraPath));
        const auto workspace =
            directory.filePath(QStringLiteral("document.wingpro"));
        WorkSpaceData data;
        data.file = WingHex::fileUrl(path);
        REQUIRE(WorkSpaceManager::saveWorkSpace(workspace, data));
        const auto deviceWorkspace =
            directory.filePath(QStringLiteral("device 中文.wingpro"));
        WorkSpaceData deviceData;
        deviceData.file = WingHex::deviceUrl(
            QStringLiteral("probedriver"), QStringLiteral("/query 中文 # %?"));
        REQUIRE(WorkSpaceManager::saveWorkSpace(deviceWorkspace, deviceData));
        REQUIRE(queryDocuments(host, window, path, workspace, deviceWorkspace));
        const auto priorCloses = driver.closes;
        LuaOwner owner;
        auto *L = owner.state;
        luaL_openlibs(L);
        luabridge::enableExceptions(L);
        luabridge::registerMainThread(L);
        int firstContext = 1, secondContext = 2;
        lua_setthreaddata(L, &firstContext);
        qtluau::setLuaContextResolver(L, [](lua_State *caller) -> const void * {
            auto *key = lua_getthreaddata(caller);
            return key ? key : caller;
        });
        lua_callbacks(L)->userthread = [](lua_State *parent, lua_State *child) {
            if (parent && child) {
                lua_setthreaddata(child, lua_getthreaddata(parent));
            }
        };
        WingLuauRequire require;
        WingLuauRequire::Options options;
        options.systemRoot =
            QCoreApplication::applicationDirPath() + QStringLiteral("/luaulib");
        options.onLuauFileLoading = [](lua_State *parent, lua_State *child,
                                       const QString &, const QByteArray &) {
            lua_setthreaddata(child, lua_getthreaddata(parent));
            return true;
        };
        REQUIRE(require.initRequire(L, options));
        require.installRequire();
        require.installProxyRequire();
        host.installAPI(L);
        REQUIRE(host.scriptHandles(L).isEmpty());
        host.cleanScriptHandles(L, {});
        auto setString = [L](const char *name, const QString &value) {
            const auto utf8 = value.toUtf8();
            lua_pushlstring(L, utf8.constData(), utf8.size());
            lua_setglobal(L, name);
        };
        setString("test_path", path);
        setString("extra_path", extraPath);
        setString("workspace_path", workspace);
        REQUIRE(runLua(L, R"(
            local hexdoc = require("@s/hexdoc")
            doc = hexdoc.new()
            assert(not doc:is_valid())
            assert(doc:open_file(test_path) == 0)
            assert(doc:is_valid())
            assert(#reader.opened_files() == 1)
            assert(#reader.all_opened_files() >= 1)
            local alias = hexdoc.new()
            assert(alias:open_file(test_path) == 0)
            doc:use(function(d)
                assert(d:read_bytes(0, 3) == "abc")
                assert(d:find_next(0, "bc") == 1)
                assert(d:select(0, 2))
                assert(d:selected_bytes(0) == "ab")
                assert(d:set_locked_file(false))
                assert(d:write_bytes(0, "A"))
                assert(d:read_bytes(0, 3) == "Abc")
                assert(d:add_book_mark(1, "mark"))
                assert(d:book_mark_comment(1) == "mark")
                local co = coroutine.create(function()
                    local ok = pcall(function() alias:use(function() end) end)
                    assert(not ok)
                end)
                assert(coroutine.resume(co))
            end)
            assert(alias:is_valid())
            assert(alias:close_handle() == 0)
            assert(not alias:is_valid())
            assert(not doc:is_valid())
            assert(doc:open_file(test_path) == 0)
            assert(doc:close(true) == 0)
            assert(not pcall(function() alias:use(function() end) end))
            assert(doc:open_ext("probedriver", "/中文 # %?x/a") == 0)
            doc:use(function(d) assert(d:read_bytes(0, 6) == "device") end)
            assert(doc:close() == 0)
            assert(doc:open_workspace(workspace_path) == 0)
            assert(doc:is_valid())
            local urls = reader.opened_files()
            assert(#urls == 1 and tostring(urls[1]:scheme()) == "wingpro")
            doc:use(function(d) assert(d:read_bytes(0, 3) == "abc") end)
            assert(doc:close() == 0)
            assert(doc:open_file(test_path) == 0)
            raw_handle = ctl.open_file(ctl._file_url(test_path))
            assert(raw_handle >= 0)
            assert(ctl.switch_document(raw_handle))
            assert(ctl.open_current() == raw_handle)
            assert(ctl.open_current() == raw_handle)
            assert(ctl.close_current() == 0)
            assert(not doc:is_valid())
            assert(doc:open_file(test_path) == 0)
        )"));
        const auto baseline = host.scriptHandles(L);
        REQUIRE(!baseline.isEmpty());
        REQUIRE(host.scriptHandles(nullptr).isEmpty());
        REQUIRE(runLua(
            L, "extra_handle = ctl.open_file(ctl._file_url(extra_path))"));
        REQUIRE(host.scriptHandles(L).size() == baseline.size() + 1);
        lua_setthreaddata(L, &secondContext);
        REQUIRE(host.scriptHandles(L).isEmpty());
        REQUIRE(runLua(
            L, "other_handle = ctl.open_file(ctl._file_url(test_path))"));
        const auto otherHandles = host.scriptHandles(L);
        REQUIRE(otherHandles.size() == 1);
        lua_setthreaddata(L, &firstContext);
        host.cleanScriptHandles(L, baseline);
        REQUIRE(host.scriptHandles(L) == baseline);
        REQUIRE(runLua(L, "assert(not reader.is_file_opened(extra_handle))"));
        REQUIRE(findView(path));
        host.cleanScriptHandles(L, baseline);
        host.cleanScriptHandles(nullptr, {});
        lua_setthreaddata(L, &secondContext);
        REQUIRE(host.scriptHandles(L) == otherHandles);
        REQUIRE(runLua(L, "assert(reader.is_file_opened(other_handle))"));
        host.cleanScriptHandles(L, {});
        REQUIRE(host.scriptHandles(L).isEmpty());
        REQUIRE(window.closeEditor(findView(extraPath), true) ==
                ErrFile::Success);
        lua_setthreaddata(L, &firstContext);
        auto *current = window.currentEditor();
        lua_pushboolean(L, current && !current->isClosed());
        lua_setglobal(L, "expected_current");
        REQUIRE(runLua(
            L, "assert(reader.is_current_doc_editing() == expected_current)"));
        REQUIRE(driver.lastPath == QStringLiteral("/中文 # %?x/a"));
        REQUIRE(driver.closes == priorCloses + 1);
        // The same key must create a fresh generation after completion/reset.
        auto registry = std::make_shared<ScriptDocumentRegistry>(
            [&window]() { return window.currentEditor(); },
            [&host](const QUrl &file, EditorView **view) {
                return host.openDocument(file, view);
            },
            [&host](EditorView *view, bool force) {
                return host.closeDocument(view, force);
            });
        QPointer<ScriptDocumentContext> first = registry->forCaller(L);
        const auto firstToken = first->token();
        REQUIRE(first->openFile(WingHex::fileUrl(path)) >= 0);
        lua_setthreaddata(L, &secondContext);
        QPointer<ScriptDocumentContext> second = registry->forCaller(L);
        REQUIRE(first != second);
        const auto otherHandle = second->openFile(WingHex::fileUrl(path));
        REQUIRE(otherHandle >= 0);
        lua_setthreaddata(L, &firstContext);
        qtluau::clearLuaContext(L);
        REQUIRE(first.isNull());
        REQUIRE(runLua(L, "assert(not reader.is_file_opened(raw_handle))"));
        REQUIRE(second && second->switchDocument(otherHandle));
        REQUIRE(findView(path));
        REQUIRE(registry->forCaller(L)->token() != firstToken);
        REQUIRE(runLua(L, R"(
            assert(not doc:is_valid())
            assert(#reader.opened_files() == 0)
            assert(not pcall(function() doc:document_info() end))
            local hexdoc = require("@s/hexdoc")
            doc = hexdoc.new()
            assert(doc:open_file(test_path) == 0)
            assert(ctl.close_file_handle(raw_handle) == hex.ErrFile.NotExist)
            doc:use(function(d) assert(d:read_bytes(0, 1) == "a") end)
            error("completion error")
        )") == false);
        qtluau::clearLuaContext(L);
        REQUIRE(findView(path));
        lua_setthreaddata(L, &secondContext);
        qtluau::clearLuaContext(L);
        REQUIRE(second.isNull());
        REQUIRE(findView(path));
        REQUIRE(window.closeEditor(findView(path), true) == ErrFile::Success);
        // Exercise completion, console reset and cancellation through
        // ScriptMachine.
        auto &machine = ScriptMachine::instance();
        REQUIRE(machine.init());
        machine.finishScriptRegistration();
        {
            ObjTreeWidget tree;
            QTreeWidgetItem *module = nullptr;
            for (QTreeWidgetItemIterator it(&tree); *it; ++it) {
                if ((*it)->text(0) == QStringLiteral("hexdoc")) {
                    module = *it;
                    break;
                }
            }
            REQUIRE(module);
            REQUIRE(module->text(1) ==
                    QStringLiteral("require(\"@s/hexdoc\")"));
            module->setExpanded(true);
            QTreeWidgetItem *document = nullptr;
            for (int i = 0; i < module->childCount(); ++i) {
                if (module->child(i)->text(0) == QStringLiteral("Document")) {
                    document = module->child(i);
                }
            }
            REQUIRE(document);
            document->setExpanded(true);
            bool foundRead = false;
            bool foundUse = false;
            for (int i = 0; i < document->childCount(); ++i) {
                foundRead |=
                    document->child(i)->text(0) == QStringLiteral("read_bytes");
                foundUse |=
                    document->child(i)->text(0) == QStringLiteral("use");
            }
            REQUIRE(foundRead && foundUse);
        }
        auto runConsole = [&](const QString &code) {
            QEventLoop loop;
            bool finished = false;
            machine.executeCode(ScriptMachine::Interactive, code,
                                [&](bool done) {
                                    if (done) {
                                        finished = true;
                                        loop.quit();
                                    }
                                });
            if (!finished) {
                loop.exec();
            }
            return finished;
        };
        QStringList errors;
        ScriptMachine::RegCallBacks callbacks;
        callbacks.printMsgFn = [&](const ScriptMachine::MessageInfo &info) {
            if (info.type == ScriptMachine::MessageType::Error) {
                errors.append(info.message);
            }
        };
        machine.registerCallBack(ScriptMachine::Interactive, callbacks);
        const auto openCode =
            QStringLiteral("local hexdoc = require(\"@s/hexdoc\"); saved_doc = "
                           "hexdoc.new(); "
                           "assert(saved_doc:open_file([[%1]]) == 0); "
                           "saved_doc:use(function(d) assert(d:read_bytes(0, "
                           "1) == \"a\") end)")
                .arg(path);
        REQUIRE(runConsole(openCode));
        REQUIRE(errors.isEmpty());
        REQUIRE(findView(path));
        REQUIRE(
            runConsole(QStringLiteral("assert(not pcall(function() "
                                      "saved_doc:use(function() end) end))")));
        REQUIRE(errors.isEmpty());
        REQUIRE(runConsole(
            openCode + QStringLiteral("; error(\"expected script failure\")")));
        REQUIRE(!errors.isEmpty());
        REQUIRE(findView(path));
        errors.clear();
        REQUIRE(
            runConsole(QStringLiteral("assert(not pcall(function() "
                                      "saved_doc:use(function() end) end))")));
        REQUIRE(errors.isEmpty());
        const auto scriptPath =
            directory.filePath(QStringLiteral("cancel.luau"));
        {
            QFile script(scriptPath);
            REQUIRE(script.open(QIODevice::WriteOnly));
            const auto source =
                (openCode +
                 QStringLiteral("; while true do coroutine.yield() end"))
                    .toUtf8();
            REQUIRE(script.write(source) == source.size());
        }
        QEventLoop cancelLoop;
        bool cancelled = false;
        bool scriptEntered = false;
        machine.executeScript(ScriptMachine::Scripting, scriptPath, false,
                              [&](bool done) {
                                  if (done) {
                                      cancelled = true;
                                      cancelLoop.quit();
                                  }
                              });
        QPointer<QObject> contextObject = new QObject;
        qtluau::trackContextObject(
            machine.contextState(ScriptMachine::Scripting), contextObject);
        QTimer::singleShot(30, &cancelLoop, [&] {
            auto *state = machine.contextState(ScriptMachine::Scripting);
            if (state && machine.isRunning(ScriptMachine::Scripting)) {
                lua_getglobal(state, "saved_doc");
                scriptEntered = lua_istable(state, -1);
                lua_pop(state, 1);
                machine.abortScript(ScriptMachine::Scripting);
            }
        });
        if (!cancelled) {
            cancelLoop.exec();
        }
        REQUIRE(cancelled && scriptEntered && contextObject.isNull());
        REQUIRE(findView(path));
        machine.destoryMachine();
        REQUIRE(window.closeEditor(findView(path), true) == ErrFile::Success);
        host.m_loadeddevs.removeAll(&driver);
        host.m_pinfos.remove(&driver);
        std::cout << "hexdoc host integration passed\n";
        return true;
    }
};

int main(int argc, char **argv) {
    qputenv("WINGHEX_DISABLE_PLUGIN_SYSTEM", "1");
    QCoreApplication::setApplicationName(QStringLiteral("HexdocScriptProbe"));
    QCoreApplication::setOrganizationName(QStringLiteral("WingHexTests"));
    AppManager application(argc, argv);
    return PluginDocumentHostProbe::run() ? 0 : 1;
}

#include "host_probe.moc"
