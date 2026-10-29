// Copyright (c) 2026, wingsummer
// SPDX-License-Identifier: BSD-3-Clause
// See 3rdparty/WingLuauQt/LICENSE for license information.

#include "filesystem_checks.h"
#include "generated_bindings.hpp"
#include "luau/runtime/scriptwindowfactory.h"
#include "luau/wingluaurequire.h"
#include "qtluau/execution.hpp"
#include "qtluau/module_values.hpp"
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
extern "C" int luaopen_cffi(lua_State *L);
#include <QAbstractButton>
#include <QApplication>
#include <QColorDialog>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QScopeGuard>
#include <QTimer>
#include <array>
#include <cctype>
#include <iostream>
#include <optional>
#include <set>
#include <string>

LUAU_FASTFLAG(LuauIntegerType2)

namespace {
void inspectMethods(lua_State *L, int index, std::set<const void *> &visited,
                    std::set<std::string> &invalid) {
    const int table = lua_absindex(L, index);
    if (!visited.insert(lua_topointer(L, table)).second) {
        return;
    }
    lua_pushnil(L);
    while (lua_next(L, table)) {
        const bool named = lua_type(L, -2) == LUA_TSTRING;
        const std::string name = named ? lua_tostring(L, -2) : "";
        if (name.rfind("__", 0) != 0) {
            if (table != LUA_REGISTRYINDEX && named && lua_isfunction(L, -1)) {
                // Qt type factories keep native class names, unlike methods.
                const bool typeFactory =
                    name.size() > 1 && name.front() == 'Q' &&
                    std::isupper(static_cast<unsigned char>(name[1]));
                for (const unsigned char character : name) {
                    if (!typeFactory && std::isupper(character)) {
                        invalid.insert(name);
                    }
                }
            } else if (lua_istable(L, -1) &&
                       (named || table == LUA_REGISTRYINDEX)) {
                inspectMethods(L, -1, visited, invalid);
            }
        }
        lua_pop(L, 1);
    }
    if (lua_getmetatable(L, table)) {
        inspectMethods(L, -1, visited, invalid);
        lua_pop(L, 1);
    }
}

bool checkMethodNames(lua_State *L) {
    std::set<const void *> visited;
    std::set<std::string> invalid;
    inspectMethods(L, LUA_REGISTRYINDEX, visited, invalid);
    inspectMethods(L, LUA_GLOBALSINDEX, visited, invalid);
    for (const auto &name : invalid) {
        std::cerr << "Method does not use snake_case: " << name << '\n';
    }
    return invalid.empty();
}
bool checkWarning(lua_State *L, const QString &script,
                  QMessageBox::StandardButtons expected,
                  QMessageBox::StandardButton clicked) {
    QTimer dismiss;
    dismiss.setSingleShot(true);
    bool seen = false;
    bool correct = false;
    QObject::connect(&dismiss, &QTimer::timeout, &dismiss, [&]() {
        for (QWidget *widget : QApplication::topLevelWidgets()) {
            auto *box = qobject_cast<QMessageBox *>(widget);
            if (box && box->isVisible()) {
                seen = true;
                correct = box->parentWidget() == nullptr &&
                          box->windowTitle() == QStringLiteral("title") &&
                          box->text() == QStringLiteral("text") &&
                          box->standardButtons() == expected;
                if (auto *button = box->button(clicked)) {
                    button->click();
                } else {
                    box->reject();
                }
                return;
            }
        }
    });
    dismiss.start(0);
    QString error;
    const bool success = qtluau::executeLuau(L, script, &error);
    dismiss.stop();
    if (!success || !seen || !correct) {
        std::cerr << "Message box overload failed: " << error.toStdString()
                  << "; shown=" << seen << "; correct=" << correct << '\n';
        return false;
    }
    return true;
}

bool checkColorDialog(lua_State *L) {
    QTimer dismiss;
    dismiss.setSingleShot(true);
    bool correct = false;
    QObject::connect(&dismiss, &QTimer::timeout, &dismiss, [&]() {
        for (QWidget *widget : QApplication::topLevelWidgets()) {
            auto *dialog = qobject_cast<QColorDialog *>(widget);
            if (dialog && dialog->isVisible()) {
                correct = dialog->parentWidget() == nullptr &&
                          dialog->windowTitle() == QStringLiteral("title") &&
                          dialog->currentColor() == QColor(4, 5, 6) &&
                          dialog->testOption(QColorDialog::DontUseNativeDialog);
                dialog->reject();
                return;
            }
        }
    });
    dismiss.start(0);
    QString error;
    const bool success = qtluau::executeLuau(L, QStringLiteral(R"LUA(
local qt = require("@qt")
local result = qt.QColorDialog.get_color(color(4, 5, 6), "title", qt.QColorDialog.ColorDialogOption.DontUseNativeDialog)
assert(not result:is_valid())
)LUA"),
                                             &error);
    dismiss.stop();
    if (!success || !correct) {
        std::cerr << "Color dialog overload failed: " << error.toStdString()
                  << '\n';
        return false;
    }
    return true;
}

template <typename Dialog, typename Complete>
bool checkDialog(lua_State *L, const QString &script, Complete complete) {
    QTimer dismiss;
    dismiss.setSingleShot(true);
    bool seen = false;
    QObject::connect(&dismiss, &QTimer::timeout, &dismiss, [&]() {
        for (auto *widget : QApplication::topLevelWidgets()) {
            if (auto *dialog = qobject_cast<Dialog *>(widget);
                dialog && dialog->isVisible()) {
                seen = dialog->parentWidget() == nullptr &&
                       dialog->windowTitle() == QStringLiteral("title");
                complete(dialog);
                return;
            }
        }
    });
    dismiss.start(0);
    QString error;
    const bool success = qtluau::executeLuau(L, script, &error);
    if (!success || !seen) {
        std::cerr << "Dialog result failed: " << error.toStdString() << '\n';
    }
    return success && seen;
}

bool checkCffiExports(lua_State *L, int factoryIndex) {
    QFile file(QString::fromUtf8(WING_CFFI_REPORT));
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    const auto report = QJsonDocument::fromJson(file.readAll())
                            .object()
                            .value("cffi")
                            .toObject();
    const auto exports = report.value("exports").toObject();
    lua_pushvalue(L, factoryIndex);
    auto check = [&](const QString &group, bool fields) {
        QSet<QString> expected;
        for (const auto &name : exports.value(group).toArray()) {
            expected.insert(name.toString());
        }
        if (fields) {
            for (const auto &name : report.value("fields").toArray()) {
                expected.insert(name.toString());
            }
        }
        QSet<QString> actual;
        lua_pushnil(L);
        while (lua_next(L, -2)) {
            actual.insert(QString::fromUtf8(lua_tostring(L, -2)));
            lua_pop(L, 1);
        }
        if (expected != actual) {
            std::cerr << "cffi export mismatch for " << group.toStdString()
                      << '\n';
            return false;
        }
        return true;
    };
    bool success = check(QStringLiteral("factory"), false);
    lua_getfield(L, -1, "new");
    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        lua_pop(L, 2);
        return false;
    }
    success = check(QStringLiteral("instance"), true) && success;
    lua_pop(L, 2);
    return success;
}

int registerEagerBaseline(lua_State *L) {
    bindings::registerAllModules(L, false);
    return 0;
}

int registerBindings(lua_State *L) {
    bindings::registerEagerModules(L);
    return 0;
}
} // namespace

int runRegistrationTest() {
    const std::array<const char *, 4> environmentNames = {
        "WING_LUAU_ENV_EMPTY", "WING_LUAU_ENV_INTEGER", "WING_LUAU_ENV_INVALID",
        "WING_LUAU_ENV_UNICODE"};
    std::array<std::optional<QByteArray>, 4> environmentValues;
    for (size_t index = 0; index < environmentNames.size(); ++index) {
        if (qEnvironmentVariableIsSet(environmentNames[index])) {
            environmentValues[index] = qgetenv(environmentNames[index]);
        }
    }
    auto restoreEnvironment = qScopeGuard([&] {
        for (size_t index = 0; index < environmentNames.size(); ++index) {
            if (environmentValues[index]) {
                qputenv(environmentNames[index], *environmentValues[index]);
            } else {
                qunsetenv(environmentNames[index]);
            }
        }
    });
    qputenv(environmentNames[0], QByteArray());
    qputenv(environmentNames[1], "2147483647");
    qputenv(environmentNames[2], "999999999999999999999999");
    qputenv(environmentNames[3], QString::fromUtf8("中文 😀").toUtf8());
    lua_State *L = luaL_newstate();
    FFlag::LuauIntegerType2.value = true;
    luaL_openlibs(L);
    WingLuauRequire require;
    if (!require.initRequire(L, WingLuauRequire::Options{})) {
        std::cerr << "Cannot initialize require\n";
        lua_close(L);
        return 1;
    }
    require.installRequire();
    require.installProxyRequire();
    QElapsedTimer registrationTimer;
    registrationTimer.start();
    lua_pushcfunction(L, registerBindings, "registerBindings");
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        std::cerr << lua_tostring(L, -1) << '\n';
        lua_close(L);
        return 1;
    }
    std::cout << "Public registration: "
              << registrationTimer.nsecsElapsed() / 1000 << "us\n";
    lua_pushcfunction(L, registerBindings, "registerBindingsAgain");
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        std::cerr << lua_tostring(L, -1) << '\n';
        lua_close(L);
        return 1;
    }
    if (lua_gettop(L) != 0) {
        std::cerr << "Module registration leaked Lua stack values\n";
        lua_close(L);
        return 1;
    }
    const auto modules = bindings::moduleFiles();
    if (modules.size() != 3 || !modules.contains(QStringLiteral("qt")) ||
        !modules.contains(QStringLiteral("cffi")) ||
        !modules.contains(QStringLiteral("network"))) {
        std::cerr << "Required module catalog is incorrect\n";
        lua_close(L);
        return 1;
    }
    qtluau::setUiWidgetResolver(ScriptWindowFactory::create);
    qtluau::setUiWidgetCreatedCallback(ScriptWindowFactory::created);
    qtluau::setWidgetContentResolver(ScriptWindowFactory::content);
    qtluau::setWidgetContentMutators(ScriptWindowFactory::setContent,
                                     ScriptWindowFactory::takeContent);
    qtluau::setUiWidgetFinalizer(ScriptWindowFactory::finalize);
    qtluau::setWindowMenuAccessors(ScriptWindowFactory::menu,
                                   ScriptWindowFactory::setMenu);
    int qtLoads = 0;
    if (!require.registerBuiltinLoader("qt", [&](lua_State *state) {
            ++qtLoads;
            QElapsedTimer timer;
            timer.start();
            const int result = qtluau::loadRegisteredModule(
                state, "qt", bindings::registerModule);
            std::cout << "First Qt registration: "
                      << timer.nsecsElapsed() / 1000 << "us\n";
            return result;
        })) {
        return 1;
    }
    require.registerBuiltinLoader("network", [](lua_State *state) {
        return qtluau::loadRegisteredModule(state, "network", bindings::registerModule);
    });
    luaopen_cffi(L);
    if (!require.registerBuiltinValue("cffi", -1)) {
        return 1;
    }
    if (!checkCffiExports(L, -1)) {
        return 1;
    }
    lua_pop(L, 1);
    int failureLoads = 0;
    require.registerBuiltinLoader("failed_probe", [&](lua_State *) -> int {
        ++failureLoads;
        throw std::runtime_error("expected loader failure");
    });
    lua_getfield(L, LUA_REGISTRYINDEX, "qtluau.registration.states");
    lua_getfield(L, -1, "base.widgets");
    const bool eagerlyRegisteredQt = !lua_isnil(L, -1);
    lua_pop(L, 2);
    if (eagerlyRegisteredQt || qtLoads != 0) {
        std::cerr << "Qt widgets were eagerly registered\n";
        return 1;
    }
    bindings::bind_public_reader(L, qtluau::NativeCallbacks_0_reader{});
    bindings::bind_public_ctl(L, qtluau::NativeCallbacks_0_ctl{});
    bindings::bind_public_log(L, qtluau::NativeCallbacks_0_log{});
    bindings::bind_toast_toast(L, qtluau::NativeCallbacks_0_toast{});
    bindings::bind_service_service(L, qtluau::NativeCallbacks_1_service{});
    qtluau::NativeCallbacks_0_service callbacks;
    callbacks.existsServiceHost = [](const QString &puid) {
        return puid == QStringLiteral("example");
    };
    bindings::bind_service_service(L, callbacks);
    if (!checkMethodNames(L)) {
        lua_close(L);
        return 1;
    }
    {
        const QByteArray xml =
            R"XML(<ui version="4.0"><class>Window</class><widget class="QMainWindow" name="Window"><widget class="QWidget" name="centralwidget"><layout class="QVBoxLayout" name="layout"><item><widget class="QPushButton" name="button"/></item></layout></widget><widget class="QMenuBar" name="menubar"/><widget class="QStatusBar" name="statusbar"/><widget class="QToolBar" name="toolbar"/></widget><resources/><connections/></ui>)XML";
        auto *root = qtluau::loadUiWidgetXmlNative(xml);
        auto *window = qobject_cast<FramelessMainWindow *>(root);
        if (!window || !window->centralWidget() || !window->menuBar() ||
            !window->findChild<QToolBar *>(QStringLiteral("toolbar")) ||
            !window->findChild<QStatusBar *>(QStringLiteral("statusbar"))) {
            std::cerr << "UI host window assembly failed\n";
            delete root;
            return 1;
        }
        delete root;
        const QByteArray dialogXml =
            R"XML(<ui version="4.0"><class>Dialog</class><widget class="QDialog" name="Dialog"><layout class="QVBoxLayout" name="layout"><item><widget class="QLabel" name="label"/></item></layout></widget><resources/><connections/></ui>)XML";
        root = qtluau::loadUiWidgetXmlNative(dialogXml);
        auto *dialog = qobject_cast<FramelessDialogBase *>(root);
        if (!dialog || !ScriptWindowFactory::content(dialog) ||
            !ScriptWindowFactory::content(dialog)->layout() ||
            !dialog->findChild<QWidget *>(QStringLiteral("label"))) {
            std::cerr << "UI dialog content assembly failed\n";
            delete root;
            return 1;
        }
        delete root;
    }
    std::cerr << "Window assembly passed\n";
    lua_pushcfunction(
        L,
        +[](lua_State *state) {
            lua_gc(state, LUA_GCCOLLECT, 0);
            return 0;
        },
        "_test_gc");
    lua_setglobal(L, "_test_gc");
    lua_pushstring(L, WING_CFFI_TESTLIB);
    lua_setglobal(L, "_test_library_path");
    luaL_sandbox(L);
    luaL_sandboxthread(L);
    QString error;
    const bool success = qtluau::executeLuau(L, QStringLiteral(R"LUA(
assert(_G.qt == nil)
for attempt = 1, 2 do
    local ok, message = pcall(require, "@b/failed_probe")
    assert(not ok and string.find(message, "expected loader failure", 1, true))
end
local qt = require("@b/qt")
assert(qt == require("@b/qt") and qt == require("@qt") and _G.qt == nil)
local network = require("@network")
assert(network == require("@b/network") and _G.network == nil)
assert(network.tcp_socket == qt.QTcpSocket and network.ssl_socket == qt.QSslSocket)
assert(qt.QMdiArea == nil and qt.QMdiSubWindow == nil and qt.QWizardPage == nil)
assert(qt.QProcessEnvironment == nil and qt.QProcess == nil and qt.QDebug == nil)
assert(exec == nil and printf == nil and warnprintf == nil and infoprintf == nil and errprintf == nil)
assert(env.read_environment_var("WING_LUAU_MISSING_62527", "默认"):to_std_string() == "默认")
assert(not env.is_environment_var_set("WING_LUAU_MISSING_62527"))
local number, ok = env.read_environment_var_int("WING_LUAU_MISSING_62527")
assert(number == 0 and not ok)
assert(env.is_environment_var_set("WING_LUAU_ENV_EMPTY"))
assert(env.is_environment_var_empty("WING_LUAU_ENV_EMPTY"))
assert(env.read_environment_var("WING_LUAU_ENV_EMPTY", "fallback"):to_std_string() == "")
local integer, valid = env.read_environment_var_int("WING_LUAU_ENV_INTEGER")
assert(integer == 2147483647 and valid)
local overflow, valid_overflow = env.read_environment_var_int("WING_LUAU_ENV_INVALID")
assert(overflow == 0 and not valid_overflow)
assert(env.read_environment_var("WING_LUAU_ENV_UNICODE"):to_std_string() == "中文 😀")
assert(env.insert == nil and env.remove == nil and env.clear == nil)
local factory = require("@cffi")
assert(factory == require("@b/cffi"))
local ffi = factory.new()
ffi.cdef("size_t test_strlen(const char *);")
local library = ffi.load(_test_library_path)
assert(ffi.tonumber(library.test_strlen("hello")) == 5)
local other = factory.new()
ffi.cdef("typedef int wing_test_int;")
assert(ffi.sizeof("wing_test_int") == 4)
assert(not pcall(function() other.typeof("wing_test_int") end))
local data = ffi.new("int[1]", {42})
assert(ffi.tonumber(data[0]) == 42)
local bytes = ffi.new("char[4]")
ffi.copy(bytes, "a\0b", 3)
assert(ffi.string(bytes, 3) == "a\0b")
ffi.fill(bytes, 3, 65)
assert(ffi.string(bytes, 3) == "AAA")
local callback = ffi.cast("int (*)(int)", function(value) return value + 1 end)
assert(ffi.tonumber(callback(4)) == 5)
callback:set(function(value) return value + 2 end)
assert(ffi.tonumber(callback(4)) == 6)
callback:free()
assert(type(json) == "table" and type(cbor) == "table")
assert(service.exists_service_host("example"))
assert(not service.exists_service_host("missing"))
assert(service.existsServiceHost == nil)
assert(qt.QString == ustring and qt.QByteArray == bytearray)
local dialog = qt.QDialog()
dialog:set_window_title("dialog")
local window = qt.QMainWindow()
local menu = window:menu_bar()
menu:set_object_name("menu")
assert(window:menu_widget():object_name() == "menu")
local central = qt.QWidget()
central:set_object_name("central")
window:set_central_widget(central)
assert(window:central_widget():object_name() == "central")
local taken = window:take_central_widget()
assert(taken:object_name() == "central")
central = nil
_test_gc()
assert(taken:object_name() == "central")
window:set_central_widget(taken)
assert(window:central_widget():object_name() == "central")
local mime = qt.QMimeData()
mime:set_data("application/x-wing-test", "a\0b")
qt.QClipboard.set_mime_data(mime)
local mime_view = qt.QClipboard.mime_data()
assert(mime_view:data("application/x-wing-test") == "a\0b")
assert(not pcall(function() mime_view:set_data("application/x-wing-test", "changed") end))
assert(ustring("text"):to_std_string() == "text")
qt.QClipboard.set_text("text")
local subtype = ustring("plain")
assert(qt.QClipboard.text(subtype) == "text")
local image = qt.QImage(2, 3, qt.QImage.Format.Format_ARGB32)
image:fill(color(4, 5, 6))
qt.QClipboard.set_image(image)
local copied = qt.QClipboard.image()
assert(copied:width() == 2 and copied:height() == 3)
assert(copied:pixel_color(1, 1):red() == 4)
local systems = qt.QCalendar.System
assert(type(systems) == "table")
assert(systems == qt.QCalendar.System)
assert(systems.Gregorian == 0)
local calendar = qt.QCalendar(systems.Gregorian)
assert(calendar:is_valid() and calendar:is_gregorian())
assert(qt.QClipboard.Mode.Clipboard == 0)
assert(qt.QImage.Format.Format_ARGB32 ~= nil)
assert(qt.QJsonDocument.JsonFormat.Compact ~= nil)
assert(not pcall(function() qt.QCalendar.System = {} end))
assert(not pcall(function() qt.QCalendar.System.Gregorian = 99 end))
assert(not pcall(function() rawset(qt.QCalendar, "unsafe_field", true) end))
)LUA"),
                                             &error);
    if (!success) {
        std::cerr << error.toStdString() << '\n';
    }
    std::cerr << "Lazy Qt/FFI execution: " << success << "\n";
    bool dialogs = success;
    if (dialogs) {
        bool overwrite = false;
        bool remove = false;
        ScriptFileSystem::setFileSystemAccessPolicy(
            {[&] { return overwrite; }, [&] { return remove; }});
        const auto restorePolicy = qScopeGuard(
            [] { ScriptFileSystem::setFileSystemAccessPolicy({}); });
        try {
            FilesystemChecks::check(L,
                                    [&](bool allowOverwrite, bool allowRemove) {
                                        overwrite = allowOverwrite;
                                        remove = allowRemove;
                                    });
        } catch (const std::exception &error) {
            std::cerr << error.what() << '\n';
            dialogs = false;
        }
    }
    if (dialogs) {
        dialogs = checkWarning(L, QStringLiteral(R"LUA(
local qt = require("@qt")
assert(qt.QMessageBox.warning("title", "text", qt.QMessageBox.StandardButton.No) == qt.QMessageBox.StandardButton.No)
)LUA"),
                               QMessageBox::No, QMessageBox::No);
    }
    if (dialogs) {
        dialogs = checkWarning(L, QStringLiteral(R"LUA(
local qt = require("@qt")
assert(qt.QMessageBox.warning(nil, "title", "text", qt.QMessageBox.StandardButton.No) == qt.QMessageBox.StandardButton.No)
)LUA"),
                               QMessageBox::No, QMessageBox::No);
    }
    if (dialogs) {
        dialogs =
            checkWarning(L, QStringLiteral(R"LUA(
local qt = require("@qt")
local buttons = bit32.bor(qt.QMessageBox.StandardButton.Yes, qt.QMessageBox.StandardButton.No)
assert(qt.QMessageBox.warning("title", "text", buttons) == qt.QMessageBox.StandardButton.No)
)LUA"),
                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    }
    if (dialogs) {
        dialogs = checkWarning(L, QStringLiteral(R"LUA(
local qt = require("@qt")
assert(qt.QMessageBox.warning("title", "text") == qt.QMessageBox.StandardButton.Ok)
)LUA"),
                               QMessageBox::Ok, QMessageBox::Ok);
    }
    if (dialogs) {
        dialogs = checkColorDialog(L);
    }
    if (dialogs) {
        dialogs = checkDialog<QInputDialog>(
            L, QStringLiteral(R"LUA(
local qt = require("@qt")
local text, ok = qt.QInputDialog.get_text("title", "label")
assert(ok and text == "  中文 😀  ")
)LUA"),
            [](QInputDialog *dialog) {
                dialog->setTextValue(QString::fromUtf8("  中文 😀  "));
                dialog->accept();
            });
    }
    if (dialogs) {
        dialogs = checkDialog<QInputDialog>(
            L, QStringLiteral(R"LUA(
local qt = require("@qt")
local value, ok = qt.QInputDialog.get_int("title", "label", 42)
assert(not ok and value == 42)
)LUA"),
            [](QInputDialog *dialog) { dialog->reject(); });
    }
    if (dialogs) {
        dialogs = checkDialog<QFileDialog>(
            L, QStringLiteral(R"LUA(
local qt = require("@qt")
local path, selected_filter = qt.QFileDialog.get_open_file_name("title", "", "Text (*.txt)", qt.QFileDialog.Option.DontUseNativeDialog)
assert(path == "" and typeof(selected_filter) == "string")
)LUA"),
            [](QFileDialog *dialog) { dialog->reject(); });
    }
    lua_pushcfunction(L, registerBindings, "registerEagerAgainAfterSandbox");
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        std::cerr << "Repeated eager registration failed after sandbox\n";
        dialogs = false;
    }
    if (failureLoads != 1) {
        dialogs = false;
    }
    if (qtLoads != 1) {
        std::cerr << "Qt loader ran " << qtLoads << " times\n";
        dialogs = false;
    }
    lua_getfield(L, LUA_REGISTRYINDEX, "qtluau.registration.counts");
    lua_pushnil(L);
    while (lua_next(L, -2)) {
        if (lua_tointeger(L, -1) != 1) {
            std::cerr << "Duplicate registration: " << lua_tostring(L, -2)
                      << '\n';
            dialogs = false;
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    qtluau::clearLuaCallbacks(L);
    lua_close(L);
    return dialogs ? 0 : 1;
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    if (app.arguments().contains(QStringLiteral("--eager-baseline"))) {
        FFlag::LuauIntegerType2.value = true;
        auto *state = luaL_newstate();
        luaL_openlibs(state);
        QElapsedTimer timer;
        timer.start();
        lua_pushcfunction(state, registerEagerBaseline,
                          "registerEagerBaseline");
        const int status = lua_pcall(state, 0, 0, 0);
        std::cout << "All modules eager registration: "
                  << timer.nsecsElapsed() / 1000 << "us\n";
        if (status != LUA_OK) {
            const char *message = lua_tostring(state, -1);
            std::cerr << (message ? message : "Non-string registration error")
                      << '\n';
        }
        lua_close(state);
        return status == LUA_OK ? 0 : 1;
    }
    for (int attempt = 0; attempt < 3; ++attempt) {
        if (runRegistrationTest() != 0) {
            return 1;
        }
    }
    return 0;
}
