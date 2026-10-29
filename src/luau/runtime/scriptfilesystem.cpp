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

#include "scriptfilesystem.h"
#include <QFileInfo>
#include <QGlobalStatic>
#include <stdexcept>

namespace ScriptFileSystem {
namespace {
Q_GLOBAL_STATIC(FileSystemAccessPolicy, accessPolicy)

bool mayOverwrite() {
    return !accessPolicy.isDestroyed() && accessPolicy->allowOverwrite &&
           accessPolicy->allowOverwrite();
}

void requireRemoval() {
    if (accessPolicy.isDestroyed() || !accessPolicy->allowRemove ||
        !accessPolicy->allowRemove()) {
        throw std::runtime_error(
            "Filesystem mutation is prohibited by script settings");
    }
}

bool writesFile(QIODeviceBase::OpenMode mode) {
    return mode.testFlag(QIODeviceBase::WriteOnly) ||
           mode.testFlag(QIODeviceBase::Append) ||
           mode.testFlag(QIODeviceBase::Truncate);
}

void requireFileWrite(QFile *file) {
    const auto *scriptFile = dynamic_cast<ScriptFile *>(file);
    if (!(scriptFile ? scriptFile->permitsWrite() : mayOverwrite())) {
        throw std::runtime_error(
            "File overwrite is prohibited by script settings");
    }
}

void checkOpen(QFile *file, QIODeviceBase::OpenMode mode) {
    if (writesFile(mode) && !mayOverwrite() &&
        (file->exists() || QFileInfo(file->fileName()).isSymbolicLink())) {
        throw std::runtime_error(
            "File overwrite is prohibited by script settings");
    }
}
} // namespace

void setFileSystemAccessPolicy(FileSystemAccessPolicy policy) {
    *accessPolicy() = std::move(policy);
}

bool ScriptFile::prepareOpen(OpenMode &mode) {
    openedExisting_ = exists() || QFileInfo(fileName()).isSymbolicLink();
    if (writesFile(mode) && !mayOverwrite()) {
        if (openedExisting_) {
            setErrorString(QStringLiteral(
                "File overwrite is prohibited by script settings"));
            return false;
        }
        // Enforce no replacement atomically, including symlinks and races.
        mode |= QIODeviceBase::NewOnly;
    }
    return true;
}

bool ScriptFile::open(OpenMode mode) {
    return prepareOpen(mode) && QFile::open(mode);
}

bool ScriptFile::openWithPermissions(OpenMode mode, Permissions permissions) {
    return prepareOpen(mode) && QFile::open(mode, permissions);
}

bool ScriptFile::permitsWrite() const {
    return (isOpen() && !openedExisting_) || mayOverwrite();
}

qint64 ScriptFile::writeData(const char *data, qint64 size) {
    if (!permitsWrite()) {
        setErrorString(
            QStringLiteral("File overwrite is prohibited by script settings"));
        return -1;
    }
    return QFile::writeData(data, size);
}

bool ScriptFile::resize(qint64 size) {
    if (!permitsWrite()) {
        return false;
    }
    return QFile::resize(size);
}

bool openScriptFile(QFile *file, QIODeviceBase::OpenMode mode) {
    checkOpen(file, mode);
    return file->open(mode);
}

bool openScriptFile(QFile *file, QIODeviceBase::OpenMode mode,
                    QFileDevice::Permissions permissions) {
    checkOpen(file, mode);
    if (auto *scriptFile = dynamic_cast<ScriptFile *>(file)) {
        return scriptFile->openWithPermissions(mode, permissions);
    }
    if (writesFile(mode) && !mayOverwrite()) {
        mode |= QIODeviceBase::NewOnly;
    }
    return file->open(mode, permissions);
}

void checkFileAccess(Access access, QFile *file) {
    switch (access) {
    case Access::Read:
        return;
    case Access::Write:
        requireFileWrite(file);
        return;
    case Access::Remove:
        requireRemoval();
        return;
    case Access::Unknown:
        requireRemoval();
        requireFileWrite(file);
        return;
    }
}

void checkDirectoryAccess(Access access, QDir *) {
    if (access != Access::Read) {
        requireRemoval();
    }
}

void checkFileDeviceAccess(Access access, QFileDevice *file) {
    checkFileAccess(access, dynamic_cast<QFile *>(file));
}

void checkIODeviceAccess(Access access, QIODevice *device) {
    if (auto *file = dynamic_cast<QFile *>(device)) {
        checkFileAccess(access, file);
    }
}

} // namespace ScriptFileSystem
