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

int SharedMemoryDriver::sdkVersion() const { return WingHex::SDKVERSION; }

const QString SharedMemoryDriver::signature() const {
    return WingHex::WINGSUMMER;
}

bool SharedMemoryDriver::init(const std::unique_ptr<QSettings> &set) {
    Q_UNUSED(set);
    return true;
}

void SharedMemoryDriver::unload(std::unique_ptr<QSettings> &set) {
    Q_UNUSED(set);
}

const QString SharedMemoryDriver::pluginName() const {
    return tr("SharedMemoryDriver");
}

const QString SharedMemoryDriver::pluginComment() const {
    return tr("An extension for opening sshared memory in WingHexExplorer2.");
}

QList<WingHex::PluginPage *> SharedMemoryDriver::registeredPages() const {
    return _plgps;
}

WingHex::WingIODevice *SharedMemoryDriver::onOpenFile(const QString &path) {
    return new SharedMemory(path, SharedMemory::ReadWrite, this);
}

bool SharedMemoryDriver::onCloseFile(WingHex::WingIODevice *dev) {
    if (dev->isOpen()) {
        dev->close();
    }
    dev->deleteLater();
    return true;
}

QString SharedMemoryDriver::supportedFileExtDisplayName() const {
    return tr("ShareMemory");
}

QIcon SharedMemoryDriver::supportedFileIcon() const {
    return QIcon(QStringLiteral(":/images/ShareMemDrv/images/shmem.png"));
}

QString SharedMemoryDriver::onOpenFileBegin() {
    bool ok;
    auto id =
        emit inputbox.getText(nullptr, tr("SharedMemory"), tr("PleaseInputID:"),
                              QLineEdit::Normal, {}, &ok);
    if (!ok) {
        return {};
    }

    return id;
}
