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

uchar QMemoryBuffer::at(qsizetype idx) { return uchar(m_buffer.at(int(idx))); }

qsizetype QMemoryBuffer::length() const { return qsizetype(m_buffer.length()); }

void QMemoryBuffer::insert(qsizetype offset, const QByteArray &data) {
    m_buffer.insert(int(offset), data);
}

void QMemoryBuffer::remove(qsizetype offset, qsizetype length) {
    m_buffer.remove(int(offset), length);
}

QByteArray QMemoryBuffer::read(qsizetype offset, qsizetype length) {
    return m_buffer.mid(int(offset), length);
}

bool QMemoryBuffer::read(QIODevice *device) {
    m_buffer = device->readAll();
    return true;
}

void QMemoryBuffer::write(QIODevice *device) { device->write(m_buffer); }

qsizetype QMemoryBuffer::indexOf(const QByteArray &ba, qsizetype from) {
    return m_buffer.indexOf(ba, int(from));
}

qsizetype QMemoryBuffer::lastIndexOf(const QByteArray &ba, qsizetype from) {
    return m_buffer.lastIndexOf(ba, int(from));
}
