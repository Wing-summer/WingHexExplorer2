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

#include "pluginiodevice.h"

#include <stdexcept>

#include <QFile>
#include <QThread>
#include <QUuid>

namespace {
void checkContext(const std::shared_ptr<PluginIOContext> &context) {
    if (!context || !context->active || !context->devicePlugin) {
        throw std::runtime_error("Device-plugin IO context is unavailable");
    }
}
} // namespace

PluginIODevice::PluginIODevice(std::shared_ptr<PluginIOContext> context,
                               QIODevice *device, bool owned, bool allowClose,
                               QObject *parent)
    : QObject(parent), m_context(std::move(context)), m_device(device),
      m_owned(owned), m_allowClose(allowClose) {
    if (owned) {
        device->setParent(this);
    }
}

PluginIODevice::~PluginIODevice() {
    if (m_owned && m_device) {
        m_device->close();
    }
}

PluginIODevice *
PluginIODevice::openFile(const std::shared_ptr<PluginIOContext> &context,
                         const QString &path, int mode, QObject *parent) {
    checkContext(context);
    if (!parent || parent->thread() != QThread::currentThread()) {
        throw std::runtime_error("Invalid device-plugin IO owner");
    }
    constexpr int allowed = QIODevice::ReadOnly | QIODevice::WriteOnly |
                            QIODevice::Append | QIODevice::Truncate |
                            QIODevice::Text | QIODevice::Unbuffered |
                            QIODevice::NewOnly | QIODevice::ExistingOnly;
    if ((mode & ~allowed) || !(mode & QIODevice::ReadWrite)) {
        throw std::runtime_error("Invalid device open mode");
    }
    auto file = std::make_unique<QFile>(path);
    if (!file->open(QIODevice::OpenMode(mode))) {
        throw std::runtime_error(file->errorString().toStdString());
    }
    auto *result = new PluginIODevice(context, file.get(), true, true, parent);
    file.release();
    return result;
}

PluginIODevice *
PluginIODevice::wrap(const std::shared_ptr<PluginIOContext> &context,
                     QIODevice *device, bool allowClose, QObject *parent) {
    checkContext(context);
    if (!device || !parent || device->thread() != QThread::currentThread() ||
        parent->thread() != QThread::currentThread()) {
        throw std::runtime_error("Invalid device-plugin IO grant");
    }
    return new PluginIODevice(context, device, false, allowClose, parent);
}

QIODevice *PluginIODevice::checked() const {
    checkContext(m_context);
    if (thread() != QThread::currentThread() || !m_device ||
        m_device->thread() != QThread::currentThread()) {
        throw std::runtime_error("Device-plugin IO object has expired");
    }
    return m_device;
}

QByteArray PluginIODevice::read(qint64 maximum) {
    if (maximum < 0 || maximum > 64 * 1024 * 1024) {
        throw std::runtime_error("Invalid device read size");
    }
    return checked()->read(maximum);
}

QByteArray PluginIODevice::readAll() {
    auto *device = checked();
    constexpr qint64 limit = 64 * 1024 * 1024;
    if (device->bytesAvailable() > limit) {
        throw std::runtime_error("Device read exceeds size limit");
    }
    return device->read(limit);
}

qint64 PluginIODevice::write(const QByteArray &bytes) {
    return checked()->write(bytes);
}

bool PluginIODevice::seek(qint64 position) {
    return position >= 0 && checked()->seek(position);
}

qint64 PluginIODevice::pos() const { return checked()->pos(); }

qint64 PluginIODevice::size() const { return checked()->size(); }

bool PluginIODevice::atEnd() const { return checked()->atEnd(); }

bool PluginIODevice::isOpen() const { return checked()->isOpen(); }

bool PluginIODevice::isReadable() const { return checked()->isReadable(); }

bool PluginIODevice::isWritable() const { return checked()->isWritable(); }

bool PluginIODevice::isSequential() const { return checked()->isSequential(); }

int PluginIODevice::openMode() const { return int(checked()->openMode()); }

QString PluginIODevice::errorString() const { return checked()->errorString(); }

void PluginIODevice::close() {
    auto *device = checked();
    if (!m_allowClose) {
        throw std::runtime_error("Closing borrowed IO is not permitted");
    }
    device->close();
}

bool PluginIODevice::flush() {
    auto *file = qobject_cast<QFile *>(checked());
    if (!file) {
        throw std::runtime_error("Device does not support flush");
    }
    return file->flush();
}

bool PluginIODevice::resize(qint64 size) {
    auto *file = qobject_cast<QFile *>(checked());
    if (!file || size < 0) {
        throw std::runtime_error("Device does not support resize");
    }
    return file->resize(size);
}

PluginIOService::PluginIOService(std::shared_ptr<PluginIOContext> context,
                                 QObject *parent)
    : QObject(parent), m_context(std::move(context)) {}

PluginIODevice *PluginIOService::openFile(const QString &path, int mode) {
    return PluginIODevice::openFile(m_context, path, mode, this);
}

PluginIODevice *PluginIOService::wrap(const QString &handle) {
    checkContext(m_context);
    const auto found = m_grants.constFind(handle);
    if (found == m_grants.cend() || !found->device) {
        throw std::runtime_error("Unknown or revoked device handle");
    }
    return PluginIODevice::wrap(found->context, found->device,
                                found->allowClose, this);
}

QString PluginIOService::grant(QIODevice *device, bool allowClose) {
    checkContext(m_context);
    if (!device || device->thread() != thread()) {
        throw std::runtime_error("Invalid device grant");
    }
    const auto handle = QUuid::createUuid().toString(QUuid::WithoutBraces);
    auto context = std::make_shared<PluginIOContext>();
    context->devicePlugin = true;
    m_grants.insert(handle, {device, allowClose, context});
    return handle;
}

void PluginIOService::revoke(const QString &handle) {
    const auto found = m_grants.find(handle);
    if (found != m_grants.end()) {
        found->context->active = false;
        m_grants.erase(found);
    }
}

void PluginIOService::invalidate() {
    m_context->active = false;
    for (const auto &grant : std::as_const(m_grants)) {
        grant.context->active = false;
    }
    m_grants.clear();
}
