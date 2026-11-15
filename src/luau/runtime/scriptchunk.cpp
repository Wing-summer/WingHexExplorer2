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

#include "scriptchunk.h"

#include <utility>

#include <QFile>

#include "Luau/Bytecode.h"

const QStringList &scriptFileSuffixes() {
    static const QStringList suffixes = {
        QStringLiteral(".luau"), QStringLiteral(".lua"),
        QStringLiteral(".luauc"), QStringLiteral(".luac")};
    return suffixes;
}

QStringList scriptFileCandidates(const QString &basePath) {
    QStringList candidates;
    for (const auto &suffix : scriptFileSuffixes()) {
        candidates.append(basePath + suffix);
    }
    return candidates;
}

QString stripScriptFileSuffix(QString path) {
    if (scriptChunkFormat(path)) {
        path.truncate(path.lastIndexOf('.'));
    }
    return path;
}

std::optional<ScriptChunkFormat> scriptChunkFormat(const QString &path) {
    if (path.endsWith(QStringLiteral(".d.luau"), Qt::CaseInsensitive)) {
        return std::nullopt;
    }
    const auto suffix = path.sliced(path.lastIndexOf('.') + 1);
    for (const auto &candidate : scriptFileSuffixes()) {
        if (suffix.compare(candidate.sliced(1), Qt::CaseInsensitive) == 0) {
            return candidate.endsWith('c') ? ScriptChunkFormat::Bytecode
                                           : ScriptChunkFormat::Source;
        }
    }
    return std::nullopt;
}

ScriptDataResult readScriptData(const QString &path, qsizetype sizeLimit) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {ScriptChunkError::Read,
                QStringLiteral("Cannot read script file: %1")
                    .arg(file.errorString()),
                {}};
    }
    if (sizeLimit >= 0 && file.size() > sizeLimit) {
        return {ScriptChunkError::Size,
                QStringLiteral("Script file exceeds size limit"),
                {}};
    }
    auto bytes = sizeLimit >= 0 ? file.read(sizeLimit + 1) : file.readAll();
    if (file.error() != QFile::NoError) {
        return {ScriptChunkError::Read,
                QStringLiteral("Cannot read script file: %1")
                    .arg(file.errorString()),
                {}};
    }
    if (sizeLimit >= 0 && bytes.size() > sizeLimit) {
        return {ScriptChunkError::Size,
                QStringLiteral("Script file exceeds size limit"),
                {}};
    }
    return {ScriptChunkError::None, {}, std::move(bytes)};
}

ScriptFileResult readScriptFile(const QString &path) {
    const auto format = scriptChunkFormat(path);
    if (!format) {
        return {{ScriptChunkError::Format,
                 QStringLiteral("Unsupported script extension"),
                 {}},
                {}};
    }
    return {readScriptData(path, DefaultScriptFileSizeLimit), *format};
}

ScriptChunkResult loadScriptChunk(lua_State *state, const QByteArray &bytes,
                                  const QString &name, ScriptChunkFormat format,
                                  const Luau::CompileOptions &options) {
    if (!state) {
        return {ScriptChunkError::Load, QStringLiteral("Missing script state")};
    }
    if (bytes.size() > DefaultScriptFileSizeLimit) {
        return {ScriptChunkError::Size,
                QStringLiteral("Script chunk exceeds size limit")};
    }
    std::string compiled;
    const char *data = bytes.constData();
    size_t size = size_t(bytes.size());
    if (format == ScriptChunkFormat::Source) {
        compiled = Luau::compile(std::string(data, size), options);
        data = compiled.data();
        size = compiled.size();
        if (!size || data[0] == 0) {
            return {ScriptChunkError::Compile,
                    size ? QString::fromUtf8(data + 1, qsizetype(size - 1))
                         : QStringLiteral("Compilation failed")};
        }
    } else {
        if (!size) {
            return {ScriptChunkError::EmptyBytecode,
                    QStringLiteral("Empty Luau bytecode")};
        }
        const auto version = static_cast<unsigned char>(data[0]);
        if (version < LBC_VERSION_MIN || version > LBC_VERSION_MAX) {
            return {ScriptChunkError::Version,
                    QStringLiteral("Unsupported Luau bytecode version: %1")
                        .arg(version)};
        }
    }
    const int top = lua_gettop(state);
    const auto chunk = name.toUtf8();
    if (luau_load(state, chunk.constData(), data, size, 0) != LUA_OK) {
        size_t length = 0;
        const auto *error = lua_tolstring(state, -1, &length);
        const auto message = error ? QString::fromUtf8(error, qsizetype(length))
                                   : QStringLiteral("Chunk loading failed");
        lua_settop(state, top);
        return {ScriptChunkError::Load, message};
    }
    return {};
}
