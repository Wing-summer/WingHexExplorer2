#include "qwindriverbuffer.h"

#ifdef Q_OS_WIN

#include "document/qstoragedevice.h"

/*
 * this file is implemented by wingsummer,
 * 使用 QHexEdit2 的代码来实现轻松访问上 GB 的文件，
 * 该类作者并未实现
 */

QWinDriverBuffer::QWinDriverBuffer(QObject *parent) : QHexBuffer(parent) {
    _chunks = new Chunks_win(parent);
}

QWinDriverBuffer::~QWinDriverBuffer() {}

uchar QWinDriverBuffer::at(qsizetype idx) {
    return uchar(_chunks->data(idx, 1)[0]);
}

qsizetype QWinDriverBuffer::length() const { return _chunks->size(); }

void QWinDriverBuffer::insert(qsizetype offset, const QByteArray &data) {
    for (int i = 0; i < data.length(); i++) {
        _chunks->insert(offset + i, data.at(i));
    }
}

void QWinDriverBuffer::remove(qsizetype offset, qsizetype length) {
    for (uint i = 0; i < uint(length); i++) {
        _chunks->removeAt(offset + i);
    }
}

QByteArray QWinDriverBuffer::read(qsizetype offset, qsizetype length) {
    return _chunks->data(offset, length);
}

bool QWinDriverBuffer::read(QIODevice *device) {
    if (device == nullptr) {
        return false;
    }

    auto driver = qobject_cast<QStorageDevice *>(device);
    if (driver == nullptr) {
        delete device;
        return false;
    }

    auto storage = driver->storage(); // only use name
    delete device;

    _chunks->setIODevice(storage);
    return true;
}

void QWinDriverBuffer::write(QIODevice *device) { _chunks->write(device); }

qsizetype QWinDriverBuffer::indexOf(const QByteArray &ba, qsizetype from) {
    return _chunks->indexOf(ba, from);
}

qsizetype QWinDriverBuffer::lastIndexOf(const QByteArray &ba, qsizetype from) {
    return _chunks->lastIndexOf(ba, from);
}

#endif
