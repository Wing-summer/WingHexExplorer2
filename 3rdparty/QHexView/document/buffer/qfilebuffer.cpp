/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include <QFile>
#include <QSaveFile>

#include "QHexEdit2/chunks.h"

QFileBuffer::QFileBuffer() {}

QFileBuffer::~QFileBuffer() {}

bool QFileBuffer::open(bool readonly) {
    if (m_chunks) {
        return false;
    }
    m_buffer = new QBuffer;
    return open(m_buffer, readonly);
}

bool QFileBuffer::open(QIODevice *iodevice, bool readonly) {
    if (m_chunks || !iodevice) {
        return false;
    }
    if (iodevice->isOpen()) {
        return false;
    }
    if (iodevice->open(readonly ? QIODevice::ReadOnly : QIODevice::ReadWrite)) {
        m_chunks = new Chunks(iodevice);
        return true;
    }

    return false;
}

bool QFileBuffer::close() {
    // intenal buffer?
    delete m_chunks;
    m_chunks = nullptr;

    if (m_buffer) {
        delete m_buffer;
        m_buffer = nullptr;
    }
    return true;
}

bool QFileBuffer::save(QIODevice *iodevice) {
    if (iodevice) {
        if (iodevice->isOpen()) {
            iodevice->close();
        }

        if (iodevice->isSequential()) {
            if (!iodevice->open(QIODevice::WriteOnly)) {
                return false;
            }

            if (m_chunks) {
                auto r = m_chunks->write(iodevice);
                iodevice->close();
                return r;
            }

            iodevice->close();
        } else {
            if (auto file = qobject_cast<QFile *>(iodevice)) {
                QSaveFile sf(file->fileName());
                sf.setDirectWriteFallback(true);
                if (!sf.open(QIODevice::WriteOnly)) {
                    return false;
                }
                auto r = m_chunks->write(&sf);
                if (r) {
                    sf.commit();
                } else {
                    sf.cancelWriting();
                }
                return r;
            } else {
                // not a QFile?
                if (!iodevice->open(QIODevice::WriteOnly |
                                    QIODevice::Truncate)) {
                    return false;
                }

                if (m_chunks) {
                    auto r = m_chunks->write(iodevice);
                    iodevice->close();
                    return r;
                }
            }
        }
    } else {
        if (!m_buffer) {
            auto iodevice = this->ioDevice();
            if (auto file = qobject_cast<QFile *>(iodevice)) {
                QSaveFile sf(file->fileName());
                auto r = sf.open(QFile::WriteOnly);
                if (r) {
                    r = m_chunks->write(&sf);
                    if (r) {
                        auto isWritable = file->isWritable();
                        file->close(); // close temporarily
                        auto r = sf.commit();
                        // try to reopen
                        return file->open(isWritable ? QFile::ReadWrite
                                                     : QFile::ReadOnly) &&
                               r;
                    } else {
                        sf.cancelWriting();
                        return false;
                    }
                }
                return true;
            } else {
                // not a QFile?
                if (!iodevice->isWritable()) {
                    // there is no meaning for unwritable device
                    return false;
                }

                iodevice->close(); // close first then reopen
                if (!iodevice->open(QIODevice::ReadWrite |
                                    QIODevice::Truncate)) {
                    return false;
                }

                if (m_chunks) {
                    return m_chunks->write(iodevice);
                }
            }
        }
    }
    return false;
}

uchar QFileBuffer::at(qsizetype idx) const {
    if (m_chunks) {
        auto data = m_chunks->at(idx);
        return uchar(data);
    }
    return 0;
}

uchar QFileBuffer::operator[](qsizetype pos) const { return at(pos); }

QIODevice *QFileBuffer::ioDevice() const {
    if (m_chunks) {
        return m_chunks->ioDevice();
    }
    return nullptr;
}

qsizetype QFileBuffer::length() const {
    if (m_chunks) {
        return m_chunks->size();
    } else {
        return -1;
    }
}

QByteArray QFileBuffer::read(qsizetype offset, qsizetype length) const {
    if (isReadyRead(offset)) {
        return m_chunks->data(offset, length);
    }
    return {};
}

bool QFileBuffer::insert(qsizetype offset, const QByteArray &data) {
    if (isReadyInsert(offset)) {
        return m_chunks->insert(offset, data);
    }
    return false;
}

bool QFileBuffer::remove(qsizetype offset, qsizetype length) {
    if (isReadyReplaceWrite(offset, length)) {
        return m_chunks->remove(offset, length);
    }
    return false;
}

bool QFileBuffer::replace(qsizetype offset, const QByteArray &data) {
    if (isReadyReplaceWrite(offset, data.length())) {
        return m_chunks->overwrite(offset, data);
    }
    return false;
}

qsizetype QFileBuffer::indexOf(const QByteArray &ba, qsizetype from) const {
    if (m_chunks) {
        return m_chunks->indexOf(ba, from);
    } else {
        return -1;
    }
}

qsizetype QFileBuffer::lastIndexOf(const QByteArray &ba, qsizetype from) const {
    if (m_chunks) {
        return m_chunks->lastIndexOf(ba, from);
    } else {
        return -1;
    }
}

bool QFileBuffer::isOpened() const {
    auto d = ioDevice();
    return d && d->isOpen();
}

QIODeviceBase::OpenMode QFileBuffer::openMode() const {
    auto d = ioDevice();
    if (d == nullptr) {
        return QIODevice::NotOpen;
    }
    return d->openMode();
}

bool QFileBuffer::isReadable() const {
    return openMode() & QIODevice::ReadOnly;
}

bool QFileBuffer::isWritable() const {
    return openMode() & QIODevice::WriteOnly;
}

bool QFileBuffer::isEmpty() const { return length() == 0; }

bool QFileBuffer::isReadyRead(qsizetype offset) const {
    if (isReadable()) {
        if (offset >= 0 && offset < this->length()) {
            return true;
        }
    }
    return false;
}

bool QFileBuffer::isReadyReplaceWrite(qsizetype offset,
                                      qsizetype length) const {
    if (isWritable()) {
        if (offset >= 0 && offset + length <= this->length()) {
            return true;
        }
    }
    return false;
}

bool QFileBuffer::isReadyInsert(qsizetype offset) const {
    if (isWritable()) {
        if (offset >= 0 && offset <= this->length()) {
            return true;
        }
    }
    return false;
}

QHexReadSessionResult QFileBuffer::readSession(quint64 budget) const {
    if (!m_chunks) {
        qCWarning(lcQHexRead) << "Cannot capture a closed buffer";
        return {{}, QHexReadError::InvalidDevice};
    }
    return m_chunks->readSession(budget);
}
