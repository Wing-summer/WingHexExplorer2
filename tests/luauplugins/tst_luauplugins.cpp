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

#include <QBuffer>
#include <QImage>
#include <QTemporaryDir>
#include <QTest>
#include <QtCore/private/qzipwriter_p.h>

#include "lualib.h"

#include "luau/plugin/pluginiodevice.h"
#include "luau/plugin/pluginpaintcontext.h"
#include "luau/plugin/scriptpluginpackage.h"
#include "luau/runtime/scriptchunk.h"
#include "luau/wingluaurequire.h"

class LuauPluginsTest : public QObject {
    Q_OBJECT
private slots:

    void paintScope() {
        QImage image(4, 4, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        QObject owner;
        PluginPaintContext context(&painter, &owner);
        context.fillRect(0, 0, 4, 4, QColor(Qt::red));
        QCOMPARE(image.pixelColor(1, 1), QColor(Qt::red));
        context.expire();
        QVERIFY_EXCEPTION_THROWN(context.drawLine(0, 0, 1, 1),
                                 std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(context.setPen(QColor(Qt::blue)),
                                 std::runtime_error);
    }

    void deviceIo() {
        auto context = std::make_shared<PluginIOContext>();
        QObject owner;
        QTemporaryDir directory;
        const auto path = directory.filePath("existing.bin");
        QFile seed(path);
        QVERIFY(seed.open(QIODevice::WriteOnly));
        QCOMPARE(seed.write("old"), 3);
        seed.close();
        QVERIFY_EXCEPTION_THROWN(PluginIODevice::openFile(context, path,
                                                          QIODevice::ReadWrite,
                                                          &owner),
                                 std::runtime_error);
        context->devicePlugin = true;
        auto *device = PluginIODevice::openFile(context, path,
                                                QIODevice::ReadWrite, &owner);
        QCOMPARE(device->write("new"), 3);
        QVERIFY(device->seek(0));
        QCOMPARE(device->read(2), QByteArray("ne"));
        QCOMPARE(device->read(2), QByteArray("w"));
        QVERIFY(device->atEnd());
        QCOMPARE(device->read(2), QByteArray());
        QVERIFY(device->resize(2));
        QVERIFY(device->flush());
        context->active = false;
        QVERIFY_EXCEPTION_THROWN(device->write("x"), std::runtime_error);
        context->active = true;
        QBuffer borrowed;
        QVERIFY(borrowed.open(QIODevice::ReadWrite));
        auto *wrapper = PluginIODevice::wrap(context, &borrowed, false, &owner);
        QVERIFY_EXCEPTION_THROWN(wrapper->close(), std::runtime_error);
        QCOMPARE(wrapper->write("data"), 4);
        QVERIFY_EXCEPTION_THROWN(wrapper->flush(), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(wrapper->resize(1), std::runtime_error);
        delete wrapper;
        QVERIFY(borrowed.isOpen());
    }

    void grants() {
        auto firstContext = std::make_shared<PluginIOContext>();
        firstContext->devicePlugin = true;
        auto otherContext = std::make_shared<PluginIOContext>();
        otherContext->devicePlugin = true;
        PluginIOService first(firstContext, nullptr),
            other(otherContext, nullptr);
        QBuffer device;
        QVERIFY(device.open(QIODevice::ReadWrite));
        const auto handle = first.grant(&device, false);
        auto *borrowed = first.wrap(handle);
        QVERIFY_EXCEPTION_THROWN(other.wrap(handle), std::runtime_error);
        QCOMPARE(borrowed->write("data"), 4);
        first.revoke(handle);
        QVERIFY_EXCEPTION_THROWN(borrowed->seek(0), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(first.wrap(handle), std::runtime_error);
        QVERIFY(device.isOpen());
        auto *ephemeral = new QBuffer;
        const auto temporaryHandle = first.grant(ephemeral, true);
        auto *expired = first.wrap(temporaryHandle);
        delete ephemeral;
        QVERIFY_EXCEPTION_THROWN(expired->isOpen(), std::runtime_error);
    }

    void packageDetection() {
        QVERIFY(ScriptPluginPackage::hasZipSignature(
            QByteArray::fromHex("504b0304")));
        QVERIFY(ScriptPluginPackage::hasZipSignature(
            QByteArray::fromHex("504b0506")));
        QVERIFY(!ScriptPluginPackage::hasZipSignature(
            QByteArray::fromHex("7f454c46")));
        QVERIFY(!ScriptPluginPackage::hasZipSignature(QByteArray("MZ")));
        QVERIFY(!ScriptPluginPackage::hasZipSignature(QByteArray("P")));
        QVERIFY(!ScriptPluginPackage::hasZipSignature(QByteArray()));
        QTemporaryDir directory;
        for (const auto &suffix :
             {QStringLiteral(".wingplg"), QStringLiteral(".WINGDRV")}) {
            const auto path =
                directory.filePath(QStringLiteral("kind") + suffix);
            {
                QZipWriter writer(path);
                writer.addFile(
                    "manifest.json",
                    QByteArray(R"({"FormatVersion":1,"Id":"KindProbe"})"));
                writer.addFile("main.luau", QByteArray("return {}"));
                writer.close();
            }
            QFile file(path);
            QVERIFY(file.open(QIODevice::ReadOnly));
            QVERIFY(ScriptPluginPackage::hasZipSignature(file.peek(2)));
            file.close();
            ScriptPluginPackage package;
            const auto loaded = package.load(path);
            QVERIFY2(loaded, qPrintable(loaded.message));
            QCOMPARE(package.kind(), suffix == QStringLiteral(".wingplg")
                                         ? ScriptPluginPackage::Kind::Plugin
                                         : ScriptPluginPackage::Kind::Device);
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
            file.write("PKbroken");
            file.close();
            ScriptPluginPackage damaged;
            QVERIFY(!damaged.load(path));
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
            file.write("MZnative");
            file.close();
            ScriptPluginPackage native;
            const auto rejected = native.load(path);
            QVERIFY(!rejected);
            QVERIFY(rejected.message.contains("ZIP signature"));
        }
    }

    void virtualFiles() {
        QByteArray png;
        QBuffer encoded(&png);
        QVERIFY(encoded.open(QIODevice::WriteOnly));
        QImage image(2, 2, QImage::Format_ARGB32);
        image.fill(Qt::green);
        QVERIFY(image.save(&encoded, "PNG"));
        auto files =
            std::make_unique<MemoryFileSystem>(QHash<QString, QByteArray>{
                {QStringLiteral("modules/value.luau"), QByteArray("return 17")},
                {QStringLiteral("modules/empty.lua"), QByteArray()},
                {QStringLiteral("resources/数据.bin"), QByteArray("a\0b", 3)},
                {QStringLiteral("resources/icon.png"), png}});
        const auto root = files->rootPath();
        QVERIFY(root.startsWith(QStringLiteral(":/")));
        QVERIFY(QDir::isAbsolutePath(root));
        QCOMPARE(QFileInfo(root).canonicalFilePath(), root);
        const auto path = files->filePath(QStringLiteral("resources/数据.bin"));
        QCOMPARE(QFileInfo(path).size(), 3);
        QVERIFY(!QFileInfo(path).isWritable());
        QFile data(path);
        QVERIFY(data.open(QIODevice::ReadOnly | QIODevice::Unbuffered));
        QCOMPARE(data.read(1), QByteArray("a"));
        QVERIFY(data.seek(1));
        QCOMPARE(data.readAll(), QByteArray("\0b", 2));
        QVERIFY(data.atEnd());
        QVERIFY(data.seek(10));
        QVERIFY(data.readAll().isEmpty());
        QFile output(path);
        QVERIFY(!output.open(QIODevice::WriteOnly));
        QVERIFY(!output.remove());
        QVERIFY(!output.resize(1));
        QVERIFY(!output.rename(root + QStringLiteral("/renamed.bin")));
        QVERIFY(!QDir(root).mkdir(QStringLiteral("new")));
        QCOMPARE(QDir(root).entryList(QDir::Dirs | QDir::NoDotAndDotDot),
                 QStringList(
                     {QStringLiteral("modules"), QStringLiteral("resources")}));
        QCOMPARE(QDir(root + QStringLiteral("/modules"))
                     .entryList({QStringLiteral("*.lua")}, QDir::Files),
                 QStringList({QStringLiteral("empty.lua")}));
        QCOMPARE(QImage(files->filePath(QStringLiteral("resources/icon.png"))),
                 image);
        QVERIFY(files->filePath(QStringLiteral("../escape")).isEmpty());
        QVERIFY(
            files->filePath(QStringLiteral("resources/../modules/value.luau"))
                .isEmpty());
        MemoryFileSystem other(
            {{QStringLiteral("resources/数据.bin"), QByteArray("other")}});
        QVERIFY(other.rootPath() != root);
        QCOMPARE(other.read(QStringLiteral("resources/数据.bin")).value(),
                 QByteArray("other"));
        files.reset();
        QVERIFY(!QFileInfo::exists(path));
        QFile expired(path);
        QVERIFY(!expired.open(QIODevice::ReadOnly));
        QVERIFY(!data.seek(0));
        QVERIFY_EXCEPTION_THROWN(
            MemoryFileSystem(QHash<QString, QByteArray>{
                {QStringLiteral("file"), QByteArray()},
                {QStringLiteral("file/child"), QByteArray()}}),
            std::invalid_argument);
    }

    void packages() {
        QTemporaryDir directory;
        const auto path = directory.filePath("sample.wingplg");
        {
            QZipWriter writer(path);
            writer.addFile("manifest.json",
                           QByteArray(R"({"FormatVersion":1,"Id":"Sample"})"));
            writer.addFile("main.luau", QByteArray("return {}"));
            writer.addFile("modules/value.luauc", QByteArray("binary"));
            writer.addFile("resources/data.bin", QByteArray("data"));
            writer.addFile("definitions/value.d.luau",
                           QByteArray("export type Value = number"));
            writer.close();
        }
        ScriptPluginPackage package;
        QString error;
        QVERIFY2(package.load(path), qPrintable(error));
        QCOMPARE(package.kind(), ScriptPluginPackage::Kind::Plugin);
        QCOMPARE(package.readResource(QStringLiteral("main.luau")).value(),
                 QByteArray("return {}"));
        QVERIFY(!package.readResource(QStringLiteral("../outside.luau")));
        const auto resource =
            QDir(package.rootPath()).filePath("resources/data.bin");
        QVERIFY(QFileInfo::exists(resource));
        QCOMPARE(package.resourcePath("resources/data.bin").path, resource);
        QFile extracted(resource);
        QVERIFY(extracted.open(QIODevice::ReadOnly));
        QCOMPARE(extracted.readAll(), QByteArray("data"));
        QVERIFY(package.resourcePath("../escape").path.isEmpty());
        const auto invalid = directory.filePath("invalid.wingdrv");
        {
            QZipWriter writer(invalid);
            writer.addFile("manifest.json",
                           QByteArray(R"({"FormatVersion":1})"));
            writer.addFile("main.luau", QByteArray("return {}"));
            writer.addFile("main.luauc", QByteArray("binary"));
            writer.close();
        }
        ScriptPluginPackage ambiguous;
        const auto rejected = ambiguous.load(invalid);
        QVERIFY(!rejected);
        QVERIFY(rejected.message.contains("Ambiguous"));
    }

    void virtualRequire() {
        const auto binary = Luau::compile("return {value = 23}");
        MemoryFileSystem files(
            {{QStringLiteral("main.luau"), QByteArray()},
             {QStringLiteral(".luaurc"),
              QByteArray(R"({"aliases":{"virtual":"./modules"}})")},
             {QStringLiteral("modules/value.luau"),
              QByteArray("return {value = 17}")},
             {QStringLiteral("modules/binary.luauc"),
              QByteArray(binary.data(), qsizetype(binary.size()))},
             {QStringLiteral("modules/directory/init.lua"),
              QByteArray("return 19")},
             {QStringLiteral("modules/ambiguous.lua"), QByteArray("return 1")},
             {QStringLiteral("modules/ambiguous.luauc"),
              QByteArray(binary.data(), qsizetype(binary.size()))},
             {QStringLiteral("modules/first.luau"),
              QByteArray("return require('./second')")},
             {QStringLiteral("modules/second.luau"),
              QByteArray("return require('./first')")}});
        std::unique_ptr<lua_State, decltype(&lua_close)> state(luaL_newstate(),
                                                               &lua_close);
        QVERIFY(state);
        luaL_openlibs(state.get());
        WingLuauRequire require;
        WingLuauRequire::Options options;
        options.readRoot = files.rootPath();
        QVERIFY(require.initRequire(state.get(), options));
        require.installRequire();
        const auto loaded =
            loadScriptChunk(state.get(), R"(
local source = require('./modules/value')
assert(require('./modules/value') == source)
assert(require('@virtual/value') == source)
assert(require('./modules/binary').value == 23)
assert(require('./modules/directory') == 19)
assert(not pcall(require, './modules/ambiguous'))
assert(not pcall(require, './modules/first'))
return source.value
)",
                            '@' + files.filePath(QStringLiteral("main.luau")),
                            ScriptChunkFormat::Source);
        QVERIFY2(bool(loaded), qPrintable(loaded.message));
        const auto status = lua_pcall(state.get(), 0, 1, 0);
        QVERIFY2(status == LUA_OK, lua_tostring(state.get(), -1));
        QCOMPARE(lua_tonumber(state.get(), -1), 17.0);
    }

    void requireReadRoot() {
        QTemporaryDir directory;
        const auto root = directory.filePath(QStringLiteral("allowed"));
        QVERIFY(QDir().mkpath(root));
        QVERIFY(QDir().mkpath(QDir(root).filePath(QStringLiteral("modules"))));
        QVERIFY(
            QDir().mkpath(directory.filePath(QStringLiteral("allowed-other"))));
        const QHash<QString, QByteArray> files = {
            {QStringLiteral(".luaurc"),
             QByteArray(R"({"aliases":{"parent":"./allowed-other"}})")},
            {QStringLiteral("outside.luau"), QByteArray("return 99")},
            {QStringLiteral("allowed-other/value.luau"),
             QByteArray("return 98")},
            {QStringLiteral("allowed/main.luau"), QByteArray()},
            {QStringLiteral("allowed/value.luau"), QByteArray("return 17")},
            {QStringLiteral("allowed/modules/value.luau"),
             QByteArray("return 18")},
            {QStringLiteral("allowed/.luaurc"),
             QByteArray(
                 R"({"aliases":{"local":"./modules","escape":"../allowed-other"}})")}};
        for (auto it = files.cbegin(); it != files.cend(); ++it) {
            QFile file(directory.filePath(it.key()));
            QVERIFY(file.open(QIODevice::WriteOnly));
            QCOMPARE(file.write(it.value()), it.value().size());
        }
        std::unique_ptr<lua_State, decltype(&lua_close)> state(luaL_newstate(),
                                                               &lua_close);
        QVERIFY(state);
        luaL_openlibs(state.get());
        WingLuauRequire require;
        WingLuauRequire::Options options;
        options.readRoot = root;
        QVERIFY(require.initRequire(state.get(), options));
        lua_pushinteger(state.get(), 37);
        QVERIFY(require.registerBuiltinValue("marker"));
        lua_pop(state.get(), 1);
        require.installRequire();
        const auto loaded = loadScriptChunk(
            state.get(), R"(
assert(require('@b/marker') == 37)
assert(require('./value') == 17)
assert(require('@local/value') == 18)
local ok, message = pcall(require, '../outside')
assert(not ok and tostring(message):find('outside the allowed root', 1, true))
assert(not pcall(require, '@escape/value'))
assert(not pcall(require, '@parent/value'))
assert(not pcall(require, '../allowed-other/value'))
return true
)",
            '@' + QDir(root).filePath(QStringLiteral("main.luau")),
            ScriptChunkFormat::Source);
        QVERIFY2(bool(loaded), qPrintable(loaded.message));
        const auto status = lua_pcall(state.get(), 0, 1, 0);
        QVERIFY2(status == LUA_OK, lua_tostring(state.get(), -1));
#ifdef Q_OS_UNIX
        QVERIFY(
            QFile::link(directory.filePath(QStringLiteral("outside.luau")),
                        QDir(root).filePath(QStringLiteral("linked.luau"))));
        const auto linkCheck = loadScriptChunk(
            state.get(), "assert(not pcall(require, './linked')); return true",
            '@' + QDir(root).filePath(QStringLiteral("main.luau")),
            ScriptChunkFormat::Source);
        QVERIFY2(bool(linkCheck), qPrintable(linkCheck.message));
        const auto linkStatus = lua_pcall(state.get(), 0, 1, 0);
        QVERIFY2(linkStatus == LUA_OK, lua_tostring(state.get(), -1));
#endif
    }

    void binaryRequire() {
        QTemporaryDir directory;
        const auto binary = Luau::compile("return {value = 17}");
        QFile file(directory.filePath("module.luauc"));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(binary.data(), qsizetype(binary.size()));
        file.close();
        auto *state = luaL_newstate();
        luaL_openlibs(state);
        WingLuauRequire require;
        QVERIFY(require.initRequire(state, {}));
        require.installRequire();
        QFile mainFile(directory.filePath("main.luau"));
        QVERIFY(mainFile.open(QIODevice::WriteOnly));
        mainFile.close();
        const auto chunk = "@" + directory.filePath("main.luau");
        auto loaded = loadScriptChunk(state, "return require('./module').value",
                                      chunk, ScriptChunkFormat::Source);
        QVERIFY2(bool(loaded), qPrintable(loaded.message));
        const auto status = lua_pcall(state, 0, 1, 0);
        QVERIFY2(status == LUA_OK, lua_tostring(state, -1));
        QCOMPARE(lua_tonumber(state, -1), 17.0);
        lua_close(state);
    }

    void scriptFiles() {
        QCOMPARE(scriptFileCandidates(QStringLiteral("main")),
                 QStringList({QStringLiteral("main.luau"),
                              QStringLiteral("main.lua"),
                              QStringLiteral("main.luauc"),
                              QStringLiteral("main.luac")}));
        for (const auto &suffix : scriptFileSuffixes()) {
            const auto path = QStringLiteral("dir/module") + suffix.toUpper();
            QVERIFY(scriptChunkFormat(path));
            QCOMPARE(stripScriptFileSuffix(path), QStringLiteral("dir/module"));
        }
        QVERIFY(!scriptChunkFormat(QStringLiteral("api.d.LUAU")));
        QCOMPARE(stripScriptFileSuffix(QStringLiteral("api.d.luau")),
                 QStringLiteral("api.d.luau"));
        const auto compiled = Luau::compile("return 'a\\0b'");
        const QByteArray binary(compiled.data(), qsizetype(compiled.size()));
        MemoryFileSystem files(
            {{QStringLiteral("empty.lua"), QByteArray()},
             {QStringLiteral("source.luau"), QByteArray("return 17")},
             {QStringLiteral("bytecode.luac"), binary},
             {QStringLiteral("empty.luauc"), QByteArray()},
             {QStringLiteral("api.d.luau"), QByteArray("declare Api: {}")},
             {QStringLiteral("large.lua"),
              QByteArray(DefaultScriptFileSizeLimit + 1, ' ')}});
        const auto bounded =
            readScriptData(files.filePath(QStringLiteral("source.luau")), 3);
        QCOMPARE(bounded.error, ScriptChunkError::Size);
        const auto unlimited =
            readScriptData(files.filePath(QStringLiteral("bytecode.luac")), -1);
        QVERIFY(unlimited);
        QCOMPARE(unlimited.bytes, binary);
        const auto empty =
            readScriptFile(files.filePath(QStringLiteral("empty.lua")));
        QVERIFY(empty);
        QVERIFY(empty.bytes.isEmpty());
        QCOMPARE(empty.format, ScriptChunkFormat::Source);
        const auto source =
            readScriptFile(files.filePath(QStringLiteral("source.luau")));
        QVERIFY(source);
        QCOMPARE(source.bytes, QByteArray("return 17"));
        const auto bytecode =
            readScriptFile(files.filePath(QStringLiteral("bytecode.luac")));
        QVERIFY(bytecode);
        QCOMPARE(bytecode.format, ScriptChunkFormat::Bytecode);
        QCOMPARE(bytecode.bytes, binary);
        const auto emptyBytecode =
            readScriptFile(files.filePath(QStringLiteral("empty.luauc")));
        QVERIFY(emptyBytecode);
        QCOMPARE(emptyBytecode.format, ScriptChunkFormat::Bytecode);
        QVERIFY(emptyBytecode.bytes.isEmpty());
        QCOMPARE(
            readScriptFile(files.rootPath() + QStringLiteral("/missing.lua"))
                .error,
            ScriptChunkError::Read);
        QCOMPARE(
            readScriptFile(files.filePath(QStringLiteral("api.d.luau"))).error,
            ScriptChunkError::Format);
        QCOMPARE(
            readScriptFile(files.filePath(QStringLiteral("large.lua"))).error,
            ScriptChunkError::Size);
    }

    void chunks() {
        auto *state = luaL_newstate();
        QVERIFY(state);
        auto result = loadScriptChunk(state, "return 42", "@main.luau",
                                      ScriptChunkFormat::Source);
        QVERIFY2(bool(result), qPrintable(result.message));
        QCOMPARE(lua_pcall(state, 0, 1, 0), LUA_OK);
        QCOMPARE(lua_tonumber(state, -1), 42.0);
        lua_settop(state, 0);
        const auto compiled = Luau::compile("return 'binary'");
        result = loadScriptChunk(
            state, QByteArray(compiled.data(), qsizetype(compiled.size())),
            "@main.luauc", ScriptChunkFormat::Bytecode);
        QVERIFY2(bool(result), qPrintable(result.message));
        QCOMPARE(lua_pcall(state, 0, 1, 0), LUA_OK);
        QCOMPARE(QByteArray(lua_tostring(state, -1)), QByteArray("binary"));
        const int top = lua_gettop(state);
        QVERIFY(
            !loadScriptChunk(state, {}, "empty", ScriptChunkFormat::Bytecode));
        QCOMPARE(lua_gettop(state), top);
        QVERIFY(!loadScriptChunk(state, QByteArray(1, char(255)), "version",
                                 ScriptChunkFormat::Bytecode));
        QCOMPARE(lua_gettop(state), top);
        QVERIFY(!loadScriptChunk(state, "local =", "syntax",
                                 ScriptChunkFormat::Source));
        QCOMPARE(lua_gettop(state), top);
        lua_close(state);
    }

    void sourceAndBinaryData() {
        auto *state = luaL_newstate();
        luaL_openlibs(state);
        for (auto format :
             {ScriptChunkFormat::Source, ScriptChunkFormat::Bytecode}) {
            const QByteArray source("return 'a\\0b'");
            const auto compiled = Luau::compile(
                std::string(source.constData(), size_t(source.size())));
            const auto bytes =
                format == ScriptChunkFormat::Source
                    ? source
                    : QByteArray(compiled.data(), qsizetype(compiled.size()));
            const auto loaded =
                loadScriptChunk(state, bytes, "@nul.luau", format);
            QVERIFY2(bool(loaded), qPrintable(loaded.message));
            QCOMPARE(lua_pcall(state, 0, 1, 0), LUA_OK);
            size_t length = 0;
            const auto *value = lua_tolstring(state, -1, &length);
            QCOMPARE(QByteArray(value, qsizetype(length)),
                     QByteArray("a\0b", 3));
            lua_settop(state, 0);
        }
        const auto embedded =
            loadScriptChunk(state, QByteArray("return 'a\0b'", 12),
                            "@nul-source.lua", ScriptChunkFormat::Source);
        QVERIFY(!embedded);
        QCOMPARE(lua_gettop(state), 0);
        QVERIFY(loadScriptChunk(state, {}, "@empty.lua",
                                ScriptChunkFormat::Source));
        QCOMPARE(lua_pcall(state, 0, 0, 0), LUA_OK);
        QVERIFY(loadScriptChunk(state, "error('failure')", "@error.lua",
                                ScriptChunkFormat::Source));
        QCOMPARE(lua_pcall(state, 0, 0, 0), LUA_ERRRUN);
        QVERIFY(QByteArray(lua_tostring(state, -1)).contains("failure"));
        lua_close(state);
    }

    void invalidPackagePaths() {
        for (const auto &entry :
             {QStringLiteral("../escape"), QStringLiteral("/absolute"),
              QStringLiteral("a/../b"), QStringLiteral("a\\b"),
              QStringLiteral("main.luau")}) {
            QTemporaryDir directory;
            const auto path = directory.filePath("invalid.wingplg");
            {
                QZipWriter zip(path);
                zip.addFile("manifest.json",
                            QByteArray(R"({"FormatVersion":1})"));
                zip.addFile("main.luau", QByteArray("return {}"));
                zip.addFile(entry, QByteArray("duplicate or invalid"));
                zip.close();
            }
            ScriptPluginPackage package;
            QString error;
            QVERIFY2(!package.load(path), qPrintable(entry));
        }
    }

    void extensions() {
        QCOMPARE(scriptChunkFormat("a.LUAUC"),
                 std::optional<ScriptChunkFormat>(ScriptChunkFormat::Bytecode));
        QCOMPARE(scriptChunkFormat("a.lua"),
                 std::optional<ScriptChunkFormat>(ScriptChunkFormat::Source));
        QVERIFY(!scriptChunkFormat("a.txt"));
        QVERIFY(!scriptChunkFormat("a.d.LUAU"));
    }

    void moduleResolution() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile file(dir.filePath("module.luauc"));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
        using Kind = WingLuauRequire::FileModuleResolution::Kind;
        QCOMPARE(WingLuauRequire::resolveFileModule(dir.path(), "module").kind,
                 Kind::File);
        QFile source(dir.filePath("module.luau"));
        QVERIFY(source.open(QIODevice::WriteOnly));
        source.close();
        QCOMPARE(WingLuauRequire::resolveFileModule(dir.path(), "module").kind,
                 Kind::Ambiguous);
        QCOMPARE(
            WingLuauRequire::resolveFileModule(dir.path(), "module.luauc").kind,
            Kind::File);
    }
};
QTEST_GUILESS_MAIN(LuauPluginsTest)
#include "tst_luauplugins.moc"
