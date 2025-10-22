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
**
** The original License is MIT from Dax89/QHexView. I have modified a lot so I
** decide to change the Open Source License. You can use the original library
** under MIT. Thanks for Dax89's efforts.
** =============================================================================
*/

#include "qmemorybuffer.h"

QMemoryBuffer::QMemoryBuffer(QObject *parent) : QHexBuffer(parent) {}

uchar QMemoryBuffer::at(qsizetype idx) const { return uchar(m_buffer.at(idx)); }

qsizetype QMemoryBuffer::length() const { return m_buffer.length(); }

bool QMemoryBuffer::insert(qsizetype offset, const QByteArray &data) {
    if (isReadyInsert(offset)) {
        if (!data.isEmpty()) {
            m_buffer.insert(offset, data);
        }
        return true;
    }
    return false;
}

bool QMemoryBuffer::remove(qsizetype offset, qsizetype length) {
    if (isReadyReplaceWrite(offset, length)) {
        if (length) {
            m_buffer.remove(offset, length);
        }
        return true;
    }
    return false;
}

QByteArray QMemoryBuffer::read(qsizetype offset, qsizetype length) const {
    if (isReadyRead(offset)) {
        return m_buffer.mid(offset, length);
    } else {
        return {};
    }
}

bool QMemoryBuffer::save(QIODevice *device) {
    if (device) {
        if (device->isOpen()) {
            device->close();
        }
        if (device->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            device->write(m_buffer);
            device->close();
            return true;
        }
    } else {
        if (!internalBuffer()) {
            if (isWritable()) {
                auto d = ioDevice();
                d->close();
                // reopen and truncate
                if (d->open(QIODevice::ReadWrite | QIODevice::Truncate)) {
                    d->write(m_buffer);
                    return true;
                }
            }
        }
    }
    return false;
}

qsizetype QMemoryBuffer::indexOf(const QByteArray &ba, qsizetype from) const {
    if (!isReadable()) {
        return -1;
    }
    return m_buffer.indexOf(ba, from);
}

qsizetype QMemoryBuffer::lastIndexOf(const QByteArray &ba,
                                     qsizetype from) const {
    if (!isReadable()) {
        return -1;
    }
    return m_buffer.lastIndexOf(ba, from);
}

bool QMemoryBuffer::open(QIODevice *iodevice, bool readonly) {
    if (QHexBuffer::open(iodevice, readonly)) {
        m_buffer = iodevice->readAll();
        return true;
    }
    return false;
}

bool QMemoryBuffer::close() {
    if (QHexBuffer::close()) {
        m_buffer.clear();
        return true;
    }
    return false;
}

bool QMemoryBuffer::replace(qsizetype offset, const QByteArray &data) {
    if (isReadyReplaceWrite(offset, data.length())) {
        if (!data.isEmpty()) {
            m_buffer.replace(offset, data.length(), data);
        }
        return true;
    }
    return false;
}
