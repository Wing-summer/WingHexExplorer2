// Copyright (c) 2026, wingsummer
// SPDX-License-Identifier: BSD-3-Clause
// See 3rdparty/WingLuauQt/LICENSE for license information.
#ifndef LUAU_FILESYSTEM_CHECKS_H
#define LUAU_FILESYSTEM_CHECKS_H
#include "qtluau/execution.hpp"
#include "qtluau/value_conversion.hpp"
#include "src/luau/runtime/scriptfilesystem.h"
#include <QDataStream>
#include <QTemporaryDir>
#include <functional>
#include <iostream>
#include <stdexcept>
namespace FilesystemChecks {
inline void run(lua_State *state, const QString &source) {
    QString error;
    if (!qtluau::executeLuau(state, source, &error)) {
        throw std::runtime_error(error.toStdString() +
                                 "\nSource: " + source.toStdString());
    }
}
inline void check(lua_State *state,
                  const std::function<void(bool, bool)> &setPolicy) {
    const auto readMode =
        qtluau::qtFromLua<QIODeviceBase::OpenMode>(luabridge::LuaRef(state, 1));
    if (readMode != QIODevice::ReadOnly) {
        throw std::runtime_error("Filesystem flag conversion failed");
    }
    for (bool overwrite : {false, true}) {
        for (bool remove : {false, true}) {
            setPolicy(overwrite, remove);
            QTemporaryDir directory;
            if (!directory.isValid()) {
                throw std::runtime_error("Cannot create filesystem fixture");
            }
            QFile existing(directory.filePath(QStringLiteral("existing.bin")));
            if (!existing.open(QIODevice::WriteOnly) ||
                existing.write("original") != 8) {
                throw std::runtime_error("Cannot create existing-file fixture");
            }
            existing.close();
            auto path = directory.path();
            path.replace('\\', QStringLiteral("\\\\"));
            path.replace('"', QStringLiteral("\\\""));
            const auto script = QStringLiteral(R"LUA(
local root = "%1"
local fs = filesystem
local mode = fs.file.OpenModeFlag
assert(mode.ReadOnly ~= nil, "missing ReadOnly flag")
assert(type(mode.ReadOnly) == "number", "unexpected flag type: " .. type(mode.ReadOnly))
local directory = fs.dir(root)
assert(directory:exists())
assert(directory:mkdir("created_dir"))
local info = fs.fileinfo(root .. "/existing.bin")
assert(info:exists(), "existing file was not found")
assert(info:is_file(), "existing file was not a regular file")
assert(tostring(info:size()) == "8", "unexpected size: " .. tostring(info:size()))
local entries = directory:entry_info_list({"*.bin"}, fs.dir.Filter.Files, fs.dir.SortFlag.Name)
assert(#entries == 1 and entries[1]:file_name() == "existing.bin")
local input = fs.file(root .. "/existing.bin")
assert(input:open(mode.ReadOnly))
assert(input:read_all() == "original")
input:close()
local output = fs.file(root .. "/new.bin")
assert(output:open(mode.WriteOnly))
assert(tostring(output:write(bytearray("new"))) == "3")
output:close()
local binary = "A" .. string.char(0) .. "B"
local binary_file = fs.file(ustring(root .. "/中文😀.bin"))
assert(binary_file:open(mode.WriteOnly))
assert(tostring(binary_file:write(binary)) == "3")
assert(tostring(binary_file:write(bytearray(binary))) == "3")
binary_file:close()
assert(binary_file:open(mode.ReadOnly))
assert(binary_file:read_all() == binary .. binary)
binary_file:close()
local allowed = pcall(function()
    assert(input:open(bit32.bor(mode.WriteOnly, mode.Truncate)))
    assert(tostring(input:write(bytearray("changed"))) == "7")
    input:close()
end)
assert(allowed == %2)
for _, requested in {mode.Append, mode.ReadWrite} do
    local opened = pcall(function()
        assert(input:open(requested))
        input:close()
    end)
    assert(opened == %2)
end
local permissions = fs.file.Permission
local created = fs.file(root .. "/permissions_new.bin")
assert(created:open(mode.WriteOnly, bit32.bor(permissions.ReadOwner, permissions.WriteOwner)))
assert(tostring(created:write("created")) == "7")
created:close()
local permissions_open = pcall(function()
    assert(input:open(mode.WriteOnly, bit32.bor(permissions.ReadOwner, permissions.WriteOwner)))
    assert(tostring(input:write(bytearray("changed"))) == "7")
    input:close()
end)
assert(permissions_open == %2)
local resized = pcall(function()
    assert(fs.file.resize(root .. "/existing.bin", 7))
end)
assert(resized == %2)
local copied = fs.file.copy(root .. "/existing.bin", root .. "/copy.bin")
assert(copied)
assert(input:copy(root .. "/instance_copy.bin"))
assert(fs.file.exists(root .. "/copy.bin"))
assert(not fs.file.copy(root .. "/existing.bin", root .. "/existing.bin"))
assert(fs.dir.is_absolute_path(root))
if %3 then
    assert(not directory:rename("new.bin", "existing.bin"))
end
local removed = pcall(function()
    assert(fs.file.remove(root .. "/copy.bin"))
end)
assert(removed == %3)
local removed_dir = pcall(function()
    assert(directory:rmdir("created_dir"))
end)
assert(removed_dir == %3)
local renamed = pcall(function()
    assert(directory:rename("new.bin", "renamed.bin"))
end)
assert(renamed == %3)
)LUA")
                                    .arg(path,
                                         overwrite ? QStringLiteral("true")
                                                   : QStringLiteral("false"),
                                         remove ? QStringLiteral("true")
                                                : QStringLiteral("false"));
            run(state, script);
            if (!existing.open(QIODevice::ReadOnly) ||
                existing.readAll() != (overwrite ? QByteArray("changed")
                                                 : QByteArray("original"))) {
                throw std::runtime_error(
                    "Filesystem overwrite policy changed protected contents");
            }
            existing.close();
        }
    }
    // Permission changes must also reach an already-open QIODevice sink.
    QTemporaryDir directory;
    const auto path = directory.filePath(QStringLiteral("indirect.bin"));
    QFile initial(path);
    if (!initial.open(QIODevice::WriteOnly) || initial.write("safe") != 4) {
        throw std::runtime_error("Cannot create indirect-write fixture");
    }
    initial.close();
    setPolicy(true, false);
    ScriptFileSystem::ScriptFile indirect(path);
    if (!indirect.open(QIODevice::ReadWrite | QIODevice::Unbuffered)) {
        throw std::runtime_error("Cannot open indirect-write fixture");
    }
    setPolicy(false, false);
    QDataStream stream(&indirect);
    stream << quint32(0x12345678);
    if (stream.status() != QDataStream::WriteFailed || indirect.resize(0)) {
        throw std::runtime_error(
            "Indirect file write bypassed current settings");
    }
    indirect.close();
    if (!initial.open(QIODevice::ReadOnly) || initial.readAll() != "safe") {
        throw std::runtime_error("Indirect write damaged protected contents");
    }
    run(state, QStringLiteral("local qt = require('@qt'); "
                              "assert(qt.QFile == filesystem.file); "
                              "assert(qt.QFileInfo == filesystem.fileinfo); "
                              "assert(qt.QDir == filesystem.dir)"));
    auto aliasPath = path;
    aliasPath.replace('\\', QStringLiteral("\\\\"));
    aliasPath.replace('"', QStringLiteral("\\\""));
    run(state, QStringLiteral(R"LUA(
local qt = require("@qt")
local file = qt.QFile("%1")
assert(not pcall(function() file:open(qt.QFile.OpenModeFlag.WriteOnly) end))
assert(not pcall(qt.QFile.resize, "%1", 0))
assert(not pcall(qt.QFile.remove, "%1"))
local dir = qt.QDir("%1_missing")
assert(not pcall(function() dir:remove_recursively() end))
)LUA")
                   .arg(aliasPath));
    std::cout << "Filesystem permissions and indirect writes passed\n";
}

} // namespace FilesystemChecks
#endif
