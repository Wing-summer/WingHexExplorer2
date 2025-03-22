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
#include <QBuffer>

QHexBuffer::QHexBuffer(QObject *parent) : QObject(parent) {}

uchar QHexBuffer::at(qsizetype idx) {
    auto data = this->read(idx, 1);
    return uchar(data[0]);
}

bool QHexBuffer::isEmpty() const { return this->length() <= 0; }

void QHexBuffer::replace(qsizetype offset, const QByteArray &data) {
    this->remove(offset, data.length());
    this->insert(offset, data);
}

void QHexBuffer::read(char *data, qsizetype size) {
    QBuffer *buffer = new QBuffer(this);
    buffer->setData(data, size);

    if (!buffer->isOpen())
        buffer->open(QBuffer::ReadWrite);

    this->read(buffer);
}

void QHexBuffer::read(const QByteArray &ba) {
    QBuffer *buffer = new QBuffer(this);

    buffer->setData(ba);
    if (!buffer->isOpen())
        buffer->open(QBuffer::ReadWrite);

    this->read(buffer);
}
