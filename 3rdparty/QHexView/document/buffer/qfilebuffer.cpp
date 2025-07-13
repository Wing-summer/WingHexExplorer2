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
    _chunks = new Chunks(this);
}

QFileBuffer::~QFileBuffer() {}

uchar QFileBuffer::at(qsizetype idx) {
    auto data = _chunks->at(idx);
    return uchar(data);
}

qsizetype QFileBuffer::length() const { return _chunks->size(); }

void QFileBuffer::insert(qsizetype offset, const QByteArray &data) {
    _chunks->insert(offset, data);
}

void QFileBuffer::remove(qsizetype offset, qsizetype length) {
    _chunks->remove(offset, length);
}

QByteArray QFileBuffer::read(qsizetype offset, qsizetype length) {
    return _chunks->data(offset, length);
}

bool QFileBuffer::read(QIODevice *device) {
    auto d = _chunks->ioDevice();
    auto ret = _chunks->setIODevice(device);
    d->setParent(nullptr);
    d->deleteLater();
    return ret;
}

void QFileBuffer::write(QIODevice *device) { _chunks->write(device); }

qsizetype QFileBuffer::indexOf(const QByteArray &ba, qsizetype from) {
    return _chunks->indexOf(ba, from);
}

qsizetype QFileBuffer::lastIndexOf(const QByteArray &ba, qsizetype from) {
    return _chunks->lastIndexOf(ba, from);
}
