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

#ifndef MEMORYFILESYSTEM_H
#define MEMORYFILESYSTEM_H

#include <memory>
#include <optional>

#include <QByteArray>
#include <QHash>
#include <QString>

// An immutable, lifetime-scoped Qt file tree. No disk files are created.
class MemoryFileSystem final {
public:
    explicit MemoryFileSystem(QHash<QString, QByteArray> files);
    ~MemoryFileSystem();

public:
    QString rootPath() const;
    QString filePath(const QString &relative) const;
    std::optional<QByteArray> read(const QString &relative) const;

private:
    class Private;
    std::unique_ptr<Private> m_data;
    Q_DISABLE_COPY_MOVE(MemoryFileSystem)
};

#endif // MEMORYFILESYSTEM_H
