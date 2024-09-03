#include "qfilebuffer.h"
#include "QHexEdit2/chunks.h"

/*
 * this file is implemented by wingsummer,
 * 使用 QHexEdit2 的代码来实现轻松访问上 GB 的文件，
 * 该类作者并未实现
 */

QFileBuffer::QFileBuffer(QObject *parent) : QHexBuffer(parent) {
    _chunks = new Chunks(parent);
}

QFileBuffer::~QFileBuffer() {}

uchar QFileBuffer::at(qsizetype idx) { return uchar(_chunks->data(idx, 1)[0]); }

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
    _chunks->setIODevice(device);
    return true;
}

void QFileBuffer::write(QIODevice *device) { _chunks->write(device); }

qsizetype QFileBuffer::indexOf(const QByteArray &ba, qsizetype from) {
    return _chunks->indexOf(ba, from);
}

qsizetype QFileBuffer::lastIndexOf(const QByteArray &ba, qsizetype from) {
    return _chunks->lastIndexOf(ba, from);
}
