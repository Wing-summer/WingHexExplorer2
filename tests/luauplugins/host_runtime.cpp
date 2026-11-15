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

#include <iostream>

#include <QApplication>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibraryInfo>
#include <QProcess>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTextDocument>
#include <QTimer>
#include <QtCore/private/qzipwriter_p.h>

#include "qtluau/value_conversion.hpp"

#include "class/settings/languagemanager.h"
#include "generated_bindings.hpp"
#include "luau/lsp/luaulanguageserver.h"
#include "luau/plugin/scriptplugin.h"
#include "luau/runtime/scriptmachine.h"
#include "luau/runtime/scriptserviceapi.h"

static std::shared_ptr<ScriptPluginPackage>
package(const QString &path, const QByteArray &source, bool binary,
        const QHash<QString, QByteArray> &resources = {}) {
    const auto compiled =
        binary ? Luau::compile(
                     std::string(source.constData(), size_t(source.size())))
               : std::string();
    {
        QZipWriter zip(path);
        const QString identifier =
            QStringLiteral("LuauVmProbe_") + QFileInfo(path).baseName();
        zip.addFile(
            QStringLiteral("manifest.json"),
            QJsonDocument(QJsonObject{{QStringLiteral("FormatVersion"), 1},
                                      {QStringLiteral("Id"), identifier},
                                      {QStringLiteral("SDK"), 22},
                                      {QStringLiteral("Version"),
                                       QStringLiteral("1.0.0")}})
                .toJson());
        zip.addFile(
            binary ? QStringLiteral("main.luauc") : QStringLiteral("main.luau"),
            binary ? QByteArray(compiled.data(), qsizetype(compiled.size()))
                   : source);
        for (auto it = resources.cbegin(); it != resources.cend(); ++it) {
            zip.addFile(it.key(), it.value());
        }
        zip.close();
    }
    auto result = std::make_shared<ScriptPluginPackage>();
    const auto loaded = result->load(path);
    if (!loaded) {
        throw std::runtime_error(loaded.message.toStdString());
    }
    return result;
}

static void require(bool value, const char *message) {
    if (!value) {
        throw std::runtime_error(message);
    }
}

static QByteArray translationFixture(const QString &directory) {
    QFile source(QDir(directory).filePath(QStringLiteral("translation.ts")));
    require(source.open(QIODevice::WriteOnly),
            "Cannot create translation fixture");
    source.write(
        R"(<TS version="2.1" language="en"><context><name>LuauVmProbe_definition_ordinary</name><message><source>Original</source><translation>Translated</translation></message></context></TS>)");
    source.close();
    QString tool = QDir(QLibraryInfo::path(QLibraryInfo::BinariesPath))
                       .filePath(QStringLiteral("lrelease"));
#ifdef Q_OS_WIN
    tool += QStringLiteral(".exe");
#endif
    const auto target =
        QDir(directory).filePath(QStringLiteral("translation.qm"));
    require(QProcess::execute(
                tool, {source.fileName(), QStringLiteral("-qm"), target}) == 0,
            "Cannot compile translation fixture");
    QFile output(target);
    require(output.open(QIODevice::ReadOnly),
            "Cannot read translation fixture");
    return output.readAll();
}

static void checkExamples(const QString &root, const QString &fixturePath) {
    if (root.isEmpty()) {
        return;
    }
    QFile fixture(fixturePath);
    require(fixture.open(QIODevice::WriteOnly),
            "Cannot create example fixture");
    const QByteArray contents = QByteArray(1200, 'H') + QByteArray(4096, 'T');
    require(fixture.write(contents) == contents.size(),
            "Cannot write example fixture");
    fixture.close();
    for (const auto &variant : {QString(), QStringLiteral("-bytecode")}) {
        auto ordinaryPackage = std::make_shared<ScriptPluginPackage>();
        require(ordinaryPackage->load(
                    QDir(root).filePath(QStringLiteral("LuauByteInspector") +
                                        variant + QStringLiteral(".wingplg"))),
                "Cannot load statistics example");
        auto devicePackage = std::make_shared<ScriptPluginPackage>();
        require(devicePackage->load(
                    QDir(root).filePath(QStringLiteral("LuauTailWindow") +
                                        variant + QStringLiteral(".wingdrv"))),
                "Cannot load tail example");
        ScriptPlugin ordinary(ordinaryPackage);
        ScriptDevicePlugin device(devicePackage);
        require(ordinary.prepare(), "Statistics example protocol failed");
        require(device.prepare(), "Tail example protocol failed");
        std::unique_ptr<QSettings> settings;
        require(ordinary.init(settings),
                "Statistics example initialization failed");
        require(device.init(settings), "Tail example initialization failed");
        QVariant summary;
        require(ordinary.runtime()->invokeExport(
                    "byte_summary",
                    {QString(QString(QChar(0)) + QStringLiteral("Az"))},
                    summary),
                "Statistics service failed");
        const auto values = summary.toMap();
        require(values.value(QStringLiteral("length")).toInt() == 3 &&
                    values.value(QStringLiteral("zeros")).toInt() == 1 &&
                    values.value(QStringLiteral("printable")).toInt() == 2,
                "Statistics service lost byte content");
        auto *tail = device.onOpenFile(fixturePath);
        require(tail, qPrintable(device.runtime()->error()));
        require(!tail->isWritable() && tail->size() == 4096 &&
                    tail->readAll() == contents.right(4096) && tail->atEnd(),
                "Tail example read or EOF failed");
        require(tail->seek(4090) && tail->read(32) == QByteArray(6, 'T'),
                "Tail example offset mapping failed");
        require(device.onCloseFile(tail), "Tail example close failed");
        delete tail;
        require(fixture.open(QIODevice::ReadOnly),
                "Cannot reopen example fixture");
        require(fixture.readAll() == contents,
                "Read-only example modified source");
        fixture.close();
        require(fixture.open(QIODevice::WriteOnly | QIODevice::Truncate),
                "Cannot empty fixture");
        fixture.close();
        tail = device.onOpenFile(fixturePath);
        require(tail && tail->size() == 0 && tail->readAll().isEmpty(),
                "Tail example empty file failed");
        require(device.onCloseFile(tail), "Empty tail close failed");
        delete tail;
        device.unload(settings);
        ordinary.unload(settings);
        require(fixture.open(QIODevice::WriteOnly),
                "Cannot reset example fixture");
        fixture.write(contents);
        fixture.close();
    }
}

int main(int argc, char **argv) {
    QTemporaryDir directory;
    qputenv("XDG_CONFIG_HOME", directory.path().toUtf8());
    qputenv("XDG_DATA_HOME", directory.path().toUtf8());
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("WingPluginProbe"));
    QApplication::setApplicationName(QStringLiteral("Runtime"));
    try {
        for (bool binary : {false, true}) {
            const QByteArray source = R"(
local P = plugin.Plugin:extend()
function P:plugin_name() return "Probe" end
function P:plugin_comment() return "Probe VM" end
function P:event_ready() self.ready_count = (self.ready_count or 0) + 1 end
function P:init()
    assert(plugin.io == nil and plugin.DevicePlugin == nil)
    local io = require("@b/_plugin_device_io")
    local types = require("@b/_plugin_device_types")
    local paint = require("@b/_plugin_paint")
    assert(type(io.open_file) == "function" and types.IODevice and paint.PaintContext)
    local allowed, reason = pcall(io.open_file, "unavailable-device-file", 1)
    assert(not allowed and tostring(reason):find("context is unavailable", 1, true))
    allowed, reason = pcall(io.wrap, "unavailable-grant")
    assert(not allowed and tostring(reason):find("context is unavailable", 1, true))
    assert(not pcall(function() return types.IODevice() end))
    assert(not pcall(function() return paint.PaintContext() end))
    self.ran = 0
    plugin.task.spawn(function()
        local qt = require("@b/qt")
        assert(qt.QPushButton, "missing QPushButton: " .. tostring(qt) .. " QWidget=" .. tostring(qt.QWidget) .. " QString=" .. tostring(qt.QString))
        self.button = qt.QPushButton()
        self.signals = 0
        self.button:connect("clicked", function() self.signals += 1 end)
        self.ran += 1; coroutine.yield(); self.ran += 1
    end)
    return true
end
plugin.service.export({
    sum = function(self, a, b) return a + b end,
    ran = function(self) return self.ran end,
    ready = function(self) return self.ready_count or 0 end,
    object = function(self) return host_object end,
    echo = function(self, value) return value end,
    button = function(self) return self.button end,
    signals = function(self) return self.signals end,
})
return P:new()
)";
            ScriptPlugin first(
                package(directory.filePath(QStringLiteral("first.wingplg")),
                        source, binary));
            ScriptPlugin second(
                package(directory.filePath(QStringLiteral("second.wingplg")),
                        source, binary));
            const auto firstPrepared = first.prepare();
            if (!firstPrepared) {
                throw std::runtime_error(firstPrepared.message.toStdString());
            }
            const auto secondPrepared = second.prepare();
            if (!secondPrepared) {
                throw std::runtime_error(secondPrepared.message.toStdString());
            }
            require(first.runtime()->state() != second.runtime()->state(),
                    "Plugins share a VM");
            require(first.pluginName() == QStringLiteral("Probe"),
                    "Inherited plugin name failed");
            std::unique_ptr<QSettings> settings;
            if (!first.init(settings)) {
                throw std::runtime_error(
                    first.runtime()->error().toStdString());
            }
            require(second.init(settings),
                    qPrintable(second.runtime()->error()));
            first.eventReady();
            QVariant result;
            require(first.runtime()->invokeExport("sum", {2, 3}, result) &&
                        result.toInt() == 5,
                    "Cross-VM value service failed");
            int sum = 0;
            const int left = 7, right = 4;
            const void *parameters[] = {&sum, &left, &right};
            const char *names[] = {"int", "int", "int"};
            const QtPrivate::QMetaTypeInterface *types[] = {
                QMetaType::fromType<int>().iface(),
                QMetaType::fromType<int>().iface(),
                QMetaType::fromType<int>().iface()};
            const WingHex::MetaCallInfo callInfo = {
                "sum", Qt::DirectConnection, 3, parameters, names, types};
            require(invokeScriptPluginService(*first.runtime(), &second,
                                              callInfo) &&
                        sum == 11,
                    "SDK script service argument/result adaptation failed");
            for (int i = 0; i < 10; ++i) {
                QCoreApplication::processEvents();
            }
            if (!first.runtime()->invokeExport("ran", {}, result) ||
                result.toInt() != 2) {
                throw std::runtime_error(
                    "Managed task failed: " +
                    first.runtime()->error().toStdString() +
                    "; value=" + result.toString().toStdString());
            }
            require(first.runtime()->invokeExport("button", {}, result),
                    "Task-created widget export failed");
            auto buttonRef =
                qtluau::variantToLua(first.runtime()->state(), result)
                    .cast<qtluau::QtWidgetRef>();
            require(bool(buttonRef), "Task-created widget expired");
            auto *button = qobject_cast<QPushButton *>(buttonRef->get());
            require(button, "Task-created widget lost its type");
            button->click();
            for (int i = 0; i < 10; ++i) {
                QCoreApplication::processEvents();
            }
            require(first.runtime()->invokeExport("signals", {}, result) &&
                        result.toInt() == 1,
                    "Task-created signal callback failed");
            require(first.runtime()->invokeExport("ready", {}, result) &&
                        result.toInt() == 1,
                    "Queued ready event failed");
            QObject hostObject;
            qtluau::qtToLua(first.runtime()->state(), &hostObject).push();
            lua_setglobal(first.runtime()->state(), "host_object");
            QVariant borrowed;
            require(first.runtime()->invokeExport("object", {}, borrowed),
                    "QObject export failed");
            require(second.runtime()->invokeExport("echo", {borrowed}, result),
                    "QObject import failed");
            auto reference =
                qtluau::variantToLua(second.runtime()->state(), result)
                    .cast<qtluau::QObjectRef>();
            require(reference && reference->get() == &hostObject,
                    "QObject identity was not retained");
            first.unload(settings);
            bool expired = false;
            try {
                reference->get();
            } catch (const std::exception &) {
                expired = true;
            }
            require(expired,
                    "QObject borrow survived exporting plugin deactivation");
            require(second.runtime()->invokeExport("sum", {5, 6}, result) &&
                        result.toInt() == 11,
                    "Unloading a VM broke another plugin");
            second.unload(settings);
        }
        {
            ScriptPlugin plain(
                package(directory.filePath(QStringLiteral("plain.wingplg")),
                        "return {plugin_name=function(self) return 'Plain' "
                        "end, plugin_comment=function(self) return '' end}",
                        false));
            const auto prepared = plain.prepare();
            require(prepared, qPrintable(prepared.message));
            std::unique_ptr<QSettings> settings;
            require(plain.init(settings),
                    "Plain table default initialization failed");
            plain.unload(settings);
        }
        {
            ScriptPlugin invalid(
                package(directory.filePath(QStringLiteral("invalid.wingplg")),
                        "return {plugin_name=5, plugin_comment=function() "
                        "return '' end}",
                        false));
            require(!invalid.prepare(), "Invalid callback field was accepted");
            ScriptPlugin faulty(
                package(directory.filePath(QStringLiteral("faulty.wingplg")),
                        "local P=plugin.Plugin:extend(); function "
                        "P:plugin_name() return 'Faulty' end; function "
                        "P:plugin_comment() return '' end; function "
                        "P:event_closing() return 'yes' end; return P:new()",
                        false));
            const auto prepared = faulty.prepare();
            require(prepared, qPrintable(prepared.message));
            std::unique_ptr<QSettings> settings;
            require(faulty.init(settings), "Fault probe did not initialize");
            faulty.eventClosing();
            require(!faulty.runtime()->active(),
                    "Invalid boolean callback result was accepted");
        }
        const QByteArray deviceSource = R"(
local P = plugin.DevicePlugin:extend()
local deviceIo = require("@b/_plugin_device_io")
assert(require("@b/_plugin_device_types").IODevice)
assert(require("@b/_plugin_paint").PaintContext)
function P:plugin_name() return "Device" end
function P:plugin_comment() return "Device probe" end
function P:supported_file_ext_display_name() return "Probe" end
function P:on_open_file(path)
    local backend = deviceIo.open_file(path, 3)
    local D = {writable=true}
    function D:read(count) return backend:read(count) end
    function D:write(bytes) return backend:write(bytes) end
    function D:seek(position) return backend:seek(position) end
    function D:size() return backend:size() end
    function D:close() backend:close(); return true end
    return D
end
function P:on_close_file() return true end
return P:new()
)";
        ScriptDevicePlugin device(
            package(directory.filePath(QStringLiteral("device.wingdrv")),
                    deviceSource, true));
        const auto devicePrepared = device.prepare();
        if (!devicePrepared) {
            throw std::runtime_error(devicePrepared.message.toStdString());
        }
        std::unique_ptr<QSettings> settings;
        if (!device.init(settings)) {
            throw std::runtime_error(device.runtime()->error().toStdString());
        }
        const auto path = directory.filePath(QStringLiteral("backend.bin"));
        QFile file(path);
        require(file.open(QIODevice::WriteOnly), "Cannot create backend");
        file.write("old");
        file.close();
        auto *backend = device.onOpenFile(path);
        require(backend, qPrintable(device.runtime()->error()));
        require(backend->write("new") == 3 && backend->seek(0) &&
                    backend->readAll() == QByteArray("new"),
                "Device IO failed");
        require(device.onCloseFile(backend), "Device close failed");
        delete backend;
        device.unload(settings);
        checkExamples(qEnvironmentVariable("WING_PLUGIN_EXAMPLES_DIR"),
                      directory.filePath(QStringLiteral("tail-window.bin")));
        auto &machine = ScriptMachine::instance();
        require(machine.init(), "ScriptMachine initialization failed");
        QHash<QString, QByteArray> ordinaryDefinitions = {
            {QStringLiteral("definitions/api.d.luau"),
             "declare LuauVmProbe_definition_ordinary: {sum: (number, number) "
             "-> number}"},
            {QStringLiteral("definitions/unused.d.luau"),
             "this is deliberately invalid syntax"}};
        ordinaryDefinitions.insert(
            QStringLiteral("translations/LuauVmProbe_definition_ordinary_") +
                LanguageManager::instance().defaultLocale().name() +
                QStringLiteral(".qm"),
            translationFixture(directory.path()));
        ordinaryDefinitions.insert(QStringLiteral("modules/source.luau"),
                                   "return {value = 13}");
        const auto compiledModule = Luau::compile("return {value = 29}");
        ordinaryDefinitions.insert(
            QStringLiteral("modules/binary.luauc"),
            QByteArray(compiledModule.data(),
                       qsizetype(compiledModule.size())));
        ordinaryDefinitions.insert(QStringLiteral("resources/value.txt"),
                                   "virtual-resource");
        ScriptPlugin declaredOrdinary(package(
            directory.filePath(QStringLiteral("definition_ordinary.wingplg")),
            R"(local source = require("./modules/source")
local binary = require("./modules/binary")
assert(source.value == 13 and binary.value == 29)
assert(require("./modules/source") == source)
assert(tostring(plugin.i18n.tr("Original")) == "Translated")
local qt = require("@b/qt")
local resource = qt.QFile(plugin.resources.path("resources/value.txt"))
assert(resource:open(1))
assert(resource:read_all() == plugin.resources.read("resources/value.txt"))
resource:close()
plugin.script.add_definition_file("definitions/api.d.luau")
plugin.script.add_definition_file("definitions/api.d.luau")
local P = plugin.Plugin:extend()
function P:plugin_name() return "Definition registration probe" end
function P:plugin_comment() return "Explicit registration only" end
plugin.service.export({
    sum = function(self, a, b) return a + b end,
    late_definition = function(self)
        return pcall(plugin.script.add_definition_file, "definitions/api.d.luau")
    end,
})
return P:new())",
            false, ordinaryDefinitions));
        ScriptDevicePlugin declaredDevice(package(
            directory.filePath(QStringLiteral("definition_device.wingdrv")),
            QByteArray("plugin.script.add_definition_file(\"definitions/"
                       "device.d.luau\")\n") +
                deviceSource,
            true,
            {{QStringLiteral("definitions/device.d.luau"),
              "declare LuauVmProbe_definition_device: {read_count: () -> "
              "number}"}}));
        require(declaredOrdinary.prepare(),
                "Ordinary definition registration failed");
        require(declaredDevice.prepare(),
                "Device definition registration failed");
        require(declaredOrdinary.init(settings),
                "Ordinary definition initialization failed");
        require(declaredDevice.init(settings),
                "Device definition initialization failed");
        auto *engine = static_cast<WingScriptBind::ScriptBindingEngine *>(
            machine.scriptEngine());
        declaredOrdinary.onRegisterScriptObj(engine->global().beginNamespace(
            QStringLiteral("LuauVmProbe_definition_ordinary")));
        declaredDevice.onRegisterScriptObj(engine->global().beginNamespace(
            QStringLiteral("LuauVmProbe_definition_device")));
        QVariant lateDefinition;
        require(declaredOrdinary.runtime()->invokeExport("late_definition", {},
                                                         lateDefinition) &&
                    !lateDefinition.toBool(),
                "Late definition registration was accepted");
        require(engine->definitionFiles().size() == 2,
                "Definitions were scanned automatically or duplicates were "
                "retained");
        machine.finishScriptRegistration();
        QString executionError;
        machine.registerCallBack(
            ScriptMachine::Background,
            {[] { return QString(); }, [] {},
             [&executionError](const ScriptMachine::MessageInfo &message) {
                 if (message.type == ScriptMachine::MessageType::Error) {
                     executionError = message.message;
                 }
             }});
        machine.registerCallBack(
            ScriptMachine::Interactive,
            {[] { return QString(); }, [] {},
             [&executionError](const ScriptMachine::MessageInfo &message) {
                 if (message.type == ScriptMachine::MessageType::Error) {
                     executionError = message.message;
                 }
             }});
        const auto compiled = Luau::compile("return 17");
        bool completed = false;
        machine.executeChunk(
            ScriptMachine::Background,
            QByteArray(compiled.data(), qsizetype(compiled.size())),
            QStringLiteral("memory.luauc"), ScriptChunkFormat::Bytecode, false,
            [&completed](bool done) { completed = done; });
        for (int i = 0; i < 20 && !completed; ++i) {
            QCoreApplication::processEvents();
        }
        require(completed && !machine.isRunning(ScriptMachine::Background),
                "Memory bytecode execution failed");
        require(executionError.isEmpty(), qPrintable(executionError));
        for (const auto &suffix :
             {QStringLiteral(".lua"), QStringLiteral(".luau"),
              QStringLiteral(".luac"), QStringLiteral(".luauc")}) {
            QFile script(
                directory.filePath(QStringLiteral("execute") + suffix));
            require(script.open(QIODevice::WriteOnly),
                    "Cannot create execution fixture");
            script.write(
                scriptChunkFormat(script.fileName()) ==
                        ScriptChunkFormat::Source
                    ? QByteArray("return 17")
                    : QByteArray(compiled.data(), qsizetype(compiled.size())));
            script.close();
            completed = false;
            machine.executeScript(
                ScriptMachine::Background, script.fileName(), false,
                [&completed](bool done) { completed = done; });
            for (int i = 0; i < 20 && !completed; ++i) {
                QCoreApplication::processEvents();
            }
            require(completed && executionError.isEmpty() &&
                        !machine.isRunning(ScriptMachine::Background),
                    "File execution regression");
        }
        completed = false;
        machine.executeCode(ScriptMachine::Interactive, QStringLiteral(R"(
local io = require("@b/_plugin_device_io")
assert(require("@b/_plugin_device_types").IODevice)
assert(require("@b/_plugin_paint").PaintContext)
assert(type(io.open_file) == "function")
local allowed, reason = pcall(io.open_file, "unavailable-device-file", 1)
assert(not allowed and tostring(reason):find("context is unavailable", 1, true))
return 42
)"),
                            [&completed](bool done) { completed = done; });
        for (int i = 0; i < 20 && !completed; ++i) {
            QCoreApplication::processEvents();
        }
        require(completed && executionError.isEmpty(),
                "Source executeCode regression");
        bool needsMore = false;
        machine.executeCode(ScriptMachine::Interactive,
                            QStringLiteral("local function unfinished("),
                            [&needsMore](bool done) { needsMore = !done; });
        require(needsMore && executionError.isEmpty(),
                "Incomplete console source regression");
        completed = false;
        machine.executeChunk(ScriptMachine::Background, {},
                             QStringLiteral("empty.luauc"),
                             ScriptChunkFormat::Bytecode, false,
                             [&completed](bool done) { completed = done; });
        require(completed && !executionError.isEmpty() &&
                    !machine.isRunning(ScriptMachine::Background) &&
                    !machine.contextState(ScriptMachine::Background),
                "Failed chunk did not clean up its thread");
        {
            LuauLanguageServer server;
            LuauLanguageServer::InitializationOptions options;
            options.definitionsFiles = bindings::definitionFiles();
            options.builtinModuleFiles = bindings::moduleFiles();
            options.pluginDefinitionFiles = engine->definitionFiles();
            QVector<lsp::Diagnostic> diagnostics;
            QString definitionError;
            QObject::connect(
                &server, &LuauLanguageServer::sigOnPublishDiagnostic, &app,
                [&](const lsp::DocumentUri &uri,
                    const QVector<lsp::Diagnostic> &items) {
                    if (uri.toLocalFile().endsWith(QStringLiteral(".d.luau")) &&
                        !items.isEmpty()) {
                        definitionError = items.first().message;
                    }
                    if (uri.toLocalFile().endsWith(
                            QStringLiteral("check.luau"))) {
                        diagnostics = items;
                    }
                });
            server.initialize(options);
            require(definitionError.isEmpty(), qPrintable(definitionError));
            const auto uri = QUrl::fromLocalFile(
                directory.filePath(QStringLiteral("check.luau")));
            QTextDocument document(
                QStringLiteral("--!strict\nlocal sum: number = "
                               "LuauVmProbe_definition_ordinary.sum(2, 3)\n"
                               "local count: number = "
                               "LuauVmProbe_definition_device.read_count()"
                               "\nreturn sum, count\n"));
            server.onOpenDocument(uri, &document);
            require(diagnostics.isEmpty(),
                    diagnostics.isEmpty()
                        ? ""
                        : qPrintable(diagnostics.first().message));
            const QString expression =
                QStringLiteral("LuauVmProbe_definition_ordinary.");
            document.setPlainText(QStringLiteral("--!strict\n") + expression);
            server.onUpdateDocument(uri, &document);
            const auto completion = server.completion(
                {uri, {1, uint(expression.size())}, std::nullopt}, nullptr);
            bool foundSum = false;
            for (const auto &item : completion) {
                foundSum |= item.label == QStringLiteral("sum");
            }
            require(foundSum,
                    "Registered script definitions did not supply completion");
            document.setPlainText(
                QStringLiteral("--!strict\nlocal io = "
                               "require(\"@b/_plugin_device_io\")\nio."));
            server.onUpdateDocument(uri, &document);
            const auto ioCompletion =
                server.completion({uri, {2, 3}, std::nullopt}, nullptr);
            bool foundOpenFile = false;
            for (const auto &item : ioCompletion) {
                foundOpenFile |= item.label == QStringLiteral("open_file");
            }
            require(foundOpenFile, "Public plugin module completion failed");
            server.onCloseDocument(uri);
        }
        declaredDevice.unload(settings);
        declaredOrdinary.unload(settings);
        machine.destoryMachine();
        std::cout << "Source/binary plugin VMs, inheritance, tasks, events, "
                     "services, QObject lifetime, device IO and registered "
                     "definitions "
                     "passed\n";
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
