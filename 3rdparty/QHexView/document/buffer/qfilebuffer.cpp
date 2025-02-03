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
#include "qfilebuffer.h"
#include "QHexEdit2/chunks.h"

QFileBuffer::QFileBuffer(QObject *parent) : QHexBuffer(parent) {
    _chunks = new Chunks(parent);
}

QFileBuffer::~QFileBuffer() {}

uchar QFileBuffer::at(qsizetype idx) {
    auto data = _chunks->data(idx, 1);
    return uchar(data[0]);
}

qsizetype QFileBuffer::length() const { return _chunks->size(); }

void QFileBuffer::insert(qsizetype offset, const QByteArray &data) {
    for (int i = 0; i < data.length(); i++) {
        _chunks->insert(offset + i, data.at(i));
    }
}

void QFileBuffer::remove(qsizetype offset, qsizetype length) {
    for (uint i = 0; i < uint(length); i++) {
        _chunks->removeAt(offset + i);
    }
}

QByteArray QFileBuffer::read(qsizetype offset, qsizetype length) {
    return _chunks->data(offset, length);
}

bool QFileBuffer::read(QIODevice *device) {
    return _chunks->setIODevice(device);
}

void QFileBuffer::write(QIODevice *device) { _chunks->write(device); }

qsizetype QFileBuffer::indexOf(const QByteArray &ba, qsizetype from) {
    return _chunks->indexOf(ba, from);
}

qsizetype QFileBuffer::lastIndexOf(const QByteArray &ba, qsizetype from) {
    return _chunks->lastIndexOf(ba, from);
}
