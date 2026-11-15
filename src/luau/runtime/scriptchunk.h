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

#ifndef SCRIPTCHUNK_H
#define SCRIPTCHUNK_H

#include <optional>

#include <QByteArray>
#include <QString>
#include <QStringList>

#include "lua.h"

#include "Luau/Compiler.h"

inline constexpr qsizetype DefaultScriptFileSizeLimit = 1024 * 1024;

enum class ScriptChunkFormat { Source, Bytecode };
enum class ScriptChunkError {
    None,
    Size,
    Compile,
    EmptyBytecode,
    Version,
    Load,
    Read,
    Format
};

struct ScriptChunkResult {
    ScriptChunkError error = ScriptChunkError::None;
    QString message;

    inline explicit operator bool() const {
        return error == ScriptChunkError::None;
    }
};

struct ScriptDataResult {
    ScriptChunkError error = ScriptChunkError::None;
    QString message;
    QByteArray bytes;

    inline explicit operator bool() const {
        return error == ScriptChunkError::None;
    }
};

struct ScriptFileResult : ScriptDataResult {
    ScriptChunkFormat format = ScriptChunkFormat::Source;
};

// Candidate order is shared by module resolution, package entries and filters.
const QStringList &scriptFileSuffixes();
QStringList scriptFileCandidates(const QString &basePath);
QString stripScriptFileSuffix(QString path);
ScriptDataResult readScriptData(const QString &path, qsizetype sizeLimit);
ScriptFileResult readScriptFile(const QString &path);

std::optional<ScriptChunkFormat> scriptChunkFormat(const QString &path);
// Success leaves one function; failure restores the original stack.
ScriptChunkResult loadScriptChunk(lua_State *state, const QByteArray &bytes,
                                  const QString &name, ScriptChunkFormat format,
                                  const Luau::CompileOptions &options = {});
#endif // SCRIPTCHUNK_H
