/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
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

#include "sharememorydrv.h"

#include "sharedmemory.h"

SharedMemoryDriver::SharedMemoryDriver() : WingHex::IWingDevice() {}

SharedMemoryDriver::~SharedMemoryDriver() {}

bool SharedMemoryDriver::init(const std::unique_ptr<QSettings> &set) {
    Q_UNUSED(set);
    return true;
}

void SharedMemoryDriver::unload(std::unique_ptr<QSettings> &set) {
    Q_UNUSED(set);
}

const QString SharedMemoryDriver::pluginName() const {
    return tr("Shared memory driver");
}

const QString SharedMemoryDriver::pluginComment() const {
    return tr("Open shared memory in WingHexExplorer2.");
}

WingHex::WingIODevice *SharedMemoryDriver::onOpenFile(const QString &path) {
    m_count++;
    auto mem = new SharedMemory(path, SharedMemory::ReadWrite, this);
    connect(mem, &SharedMemory::destroyed, this, [this]() { m_count--; });
    return mem;
}

bool SharedMemoryDriver::onCloseFile(WingHex::WingIODevice *dev) {
    dev->close();
    return true;
}

bool SharedMemoryDriver::isShareMemUsed(const WingHex::SenderInfo &) {
    return m_count;
}

QString SharedMemoryDriver::supportedFileExtDisplayName() const {
    return tr("Shared memory");
}

QIcon SharedMemoryDriver::supportedFileIcon() const {
    return QIcon(
        QStringLiteral(":/images/ShareMemDrv/images/shared-memory.svg"));
}

QString SharedMemoryDriver::onOpenFileBegin() {
    bool ok;
    auto id = dlgGetText(nullptr, tr("Shared memory"),
                         tr("Enter the shared memory identifier:"),
                         QLineEdit::Normal, {}, &ok);
    if (!ok) {
        return {};
    }

    return id;
}
