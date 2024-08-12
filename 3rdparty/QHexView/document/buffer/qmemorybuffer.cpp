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
