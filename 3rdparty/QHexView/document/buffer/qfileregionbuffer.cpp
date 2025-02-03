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
#include "qfileregionbuffer.h"
#include <QFile>
#include <QFileInfo>

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

QFileRegionBuffer::QFileRegionBuffer(QObject *parent) : QHexBuffer(parent) {}

qsizetype QFileRegionBuffer::length() const { return buffer.length(); }

void QFileRegionBuffer::insert(qsizetype offset, const QByteArray &data) {
    buffer.insert(offset, data);
    return;
}

void QFileRegionBuffer::remove(qsizetype offset, qsizetype length) {
    buffer.remove(offset, length);
}

QByteArray QFileRegionBuffer::read(qsizetype offset, qsizetype length) {
    return buffer.mid(offset, length);
}

bool QFileRegionBuffer::read(QIODevice *iodevice) {
    auto file = qobject_cast<QFile *>(iodevice);
    if (file && file->open(QFile::ReadOnly)) {
#ifdef Q_OS_LINUX
        lseek(file->handle(), offset,
              SEEK_SET); // 有些特殊的文件 Qt 是不支持 seek 的，用原生函数

#else
        file->seek(offset);
#endif
        buffer = file->read(maxlength);
        file->close();
        return true;
    }
    return false;
}

#include <QDebug>

void QFileRegionBuffer::write(QIODevice *iodevice) {
    auto file = qobject_cast<QFile *>(iodevice);
#ifdef Q_OS_LINUX
    lseek(file->handle(), offset,
          SEEK_SET); // 有些特殊的文件 Qt 是不支持 seek 的，用原生函数
#else
    file->seek(offset);
#endif
    file->write(buffer);
}

qsizetype QFileRegionBuffer::indexOf(const QByteArray &ba, qsizetype from) {
    return buffer.indexOf(ba, int(from));
}

qsizetype QFileRegionBuffer::lastIndexOf(const QByteArray &ba, qsizetype from) {
    return buffer.lastIndexOf(ba, int(from));
}

void QFileRegionBuffer::setReadOffset(qsizetype offset) {
    this->offset = offset;
}

qsizetype QFileRegionBuffer::readOffset() { return offset; }

qsizetype QFileRegionBuffer::readMaxBytes() { return maxlength; }

void QFileRegionBuffer::setReadMaxBytes(qsizetype maxlength) {
    if (maxlength)
        this->maxlength = maxlength;
}
