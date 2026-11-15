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

#ifndef SCRIPTPLUGINPACKAGE_H
#define SCRIPTPLUGINPACKAGE_H

#include <optional>

#include <QByteArrayView>
#include <QJsonObject>

#include "class/file/memoryfilesystem.h"

enum class ScriptPluginError { None, Package, Initialization, Resource };

struct ScriptPluginResult {
    ScriptPluginError error = ScriptPluginError::None;
    QString message;
    QString path;

    inline operator bool() const { return error == ScriptPluginError::None; }
};

class ScriptPluginPackage final {
public:
    enum class Kind { Plugin, Device };
    static bool hasZipSignature(QByteArrayView header);

public:
    ScriptPluginResult load(const QString &path);

    inline Kind kind() const { return m_kind; }

    inline const QJsonObject &manifest() const { return m_manifest; }

    inline const QByteArray &archiveData() const { return m_archive; }

    QString entryPath() const;
    ScriptPluginResult resourcePath(const QString &relative) const;

    inline QString rootPath() const {
        return m_files ? m_files->rootPath() : QString();
    }

    std::optional<QByteArray> readResource(const QString &relative) const;

private:
    Kind m_kind = Kind::Plugin;
    std::unique_ptr<MemoryFileSystem> m_files;
    QJsonObject m_manifest;
    QString m_entry;
    QByteArray m_archive;
};
#endif // SCRIPTPLUGINPACKAGE_H
