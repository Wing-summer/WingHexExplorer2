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
