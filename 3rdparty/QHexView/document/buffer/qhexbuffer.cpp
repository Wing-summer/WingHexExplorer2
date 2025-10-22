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

#include "qhexbuffer.h"

QHexBuffer::QHexBuffer(QObject *parent) : QObject(parent) {}

QHexBuffer::~QHexBuffer() { QHexBuffer::close(); }

bool QHexBuffer::open(bool readonly) {
    if (_ioDevice) {
        return false;
    }
    m_buffer = new QBuffer(this);
    return open(m_buffer, readonly);
}

bool QHexBuffer::open(QIODevice *iodevice, bool readonly) {
    if (_ioDevice || !iodevice) {
        return false;
    }
    if (iodevice->isOpen()) {
        return false;
    }
    if (iodevice->open(readonly ? QIODevice::ReadOnly : QIODevice::ReadWrite)) {
        _ioDevice = iodevice;
        return true;
    }
    return false;
}

bool QHexBuffer::close() {
    if (_ioDevice == nullptr) {
        return true;
    }

    if (_ioDevice->isOpen()) {
        _ioDevice->close();
        delete _ioDevice;
        _ioDevice = nullptr;

        // intenal buffer?
        if (m_buffer) {
            m_buffer = nullptr;
        }
        return true;
    }
    return false;
}

uchar QHexBuffer::at(qsizetype idx) const {
    auto data = this->read(idx, 1);
    return uchar(data[0]);
}

uchar QHexBuffer::operator[](qsizetype pos) const { return at(pos); }

bool QHexBuffer::isEmpty() const { return this->length() == 0; }

bool QHexBuffer::isReadyRead(qsizetype offset) const {
    if (_ioDevice && _ioDevice->isOpen() && _ioDevice->isReadable()) {
        if (offset >= 0 && offset < this->length()) {
            return true;
        }
    }
    return false;
}

bool QHexBuffer::isReadyReplaceWrite(qsizetype offset, qsizetype length) const {
    if (_ioDevice && _ioDevice->isOpen() && _ioDevice->isWritable()) {
        if (offset >= 0 && offset + length <= this->length()) {
            return true;
        }
    }
    return false;
}

bool QHexBuffer::isReadyInsert(qsizetype offset) const {
    if (_ioDevice && _ioDevice->isOpen() && _ioDevice->isWritable()) {
        if (offset >= 0 && offset <= this->length()) {
            return true;
        }
    }
    return false;
}

QBuffer *QHexBuffer::internalBuffer() const { return m_buffer; }

QIODevice *QHexBuffer::ioDevice() const { return _ioDevice; }

bool QHexBuffer::isOpened() const { return _ioDevice && _ioDevice->isOpen(); }

bool QHexBuffer::isReadable() const { return openMode() & QIODevice::ReadOnly; }

bool QHexBuffer::isWritable() const {
    return openMode() & QIODevice::WriteOnly;
}

QIODevice::OpenMode QHexBuffer::openMode() const {
    if (_ioDevice == nullptr) {
        return QIODevice::NotOpen;
    }
    return _ioDevice->openMode();
}
