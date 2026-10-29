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

#ifndef SCRIPTFILESYSTEM_H
#define SCRIPTFILESYSTEM_H

#include <QDir>
#include <QFile>
#include <functional>
#include <utility>

namespace ScriptFileSystem {

enum class Access { Read, Write, Remove, Unknown };

struct FileSystemAccessPolicy {
    std::function<bool()> allowOverwrite;
    std::function<bool()> allowRemove;
};

void setFileSystemAccessPolicy(FileSystemAccessPolicy policy);

// The virtual sink also protects writes performed by other Qt APIs.
class ScriptFile final : public QFile {
public:
    using QFile::QFile;
    bool open(OpenMode mode) override;
    bool openWithPermissions(OpenMode mode, Permissions permissions);
    bool resize(qint64 size) override;
    bool permitsWrite() const;

protected:
    qint64 writeData(const char *data, qint64 size) override;

private:
    bool prepareOpen(OpenMode &mode);
    bool openedExisting_ = true;
};

template <class... Args>
QFile *createScriptFile(Args &&...args) {
    return new ScriptFile(std::forward<Args>(args)...);
}

void checkFileAccess(Access access, QFile *file);
void checkFileDeviceAccess(Access access, QFileDevice *file);
void checkIODeviceAccess(Access access, QIODevice *device);
void checkDirectoryAccess(Access access, QDir *directory);
template <class T>
void checkResourceAccess(Access access, T *) {
    checkDirectoryAccess(access, nullptr);
}

bool openScriptFile(QFile *file, QIODeviceBase::OpenMode mode);
bool openScriptFile(QFile *file, QIODeviceBase::OpenMode mode,
                    QFileDevice::Permissions permissions);

} // namespace ScriptFileSystem
#endif
