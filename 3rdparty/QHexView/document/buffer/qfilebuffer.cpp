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

#include <QSaveFile>

QFileBuffer::QFileBuffer(QObject *parent) : QHexBuffer(parent) {}

QFileBuffer::~QFileBuffer() {}

bool QFileBuffer::open(QIODevice *iodevice, bool readonly) {
    if (QHexBuffer::open(iodevice, readonly)) {
        _chunks = new Chunks(iodevice, this);
        return true;
    }
    return false;
}

bool QFileBuffer::close() {
    if (QHexBuffer::close()) {
        _chunks->deleteLater();
        _chunks = nullptr;
        return true;
    }
    return false;
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

            if (_chunks) {
                auto r = _chunks->write(iodevice);
                iodevice->close();
                return r;
            }

            iodevice->close();
        } else {
            if (auto file = qobject_cast<QFile *>(iodevice)) {
                QSaveFile sf(file->fileName());
                sf.setDirectWriteFallback(true);
                auto r = _chunks->write(&sf);
                if (r) {
                    sf.commit();
                } else {
                    sf.cancelWriting();
                }
                return true;
            } else {
                // not a QFile?
                if (!iodevice->open(QIODevice::WriteOnly |
                                    QIODevice::Truncate)) {
                    return false;
                }

                if (_chunks) {
                    auto r = _chunks->write(iodevice);
                    iodevice->close();
                    return r;
                }
            }
        }
    } else {
        if (!internalBuffer()) {
            auto iodevice = this->ioDevice();
            if (auto file = qobject_cast<QFile *>(iodevice)) {
                QSaveFile sf(file->fileName());
                auto r = sf.open(QFile::WriteOnly);
                if (r) {
                    r = _chunks->write(&sf);
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
                if (!iodevice->open(QIODevice::WriteOnly |
                                    QIODevice::Truncate)) {
                    return false;
                }

                if (_chunks) {
                    auto r = _chunks->write(iodevice);
                    iodevice->close();
                    return r;
                }
            }
        }
    }
    return false;
}

uchar QFileBuffer::at(qsizetype idx) const {
    if (_chunks) {
        auto data = _chunks->at(idx);
        return uchar(data);
    }
    return 0;
}

qsizetype QFileBuffer::length() const {
    if (_chunks) {
        return _chunks->size();
    } else {
        return -1;
    }
}

QByteArray QFileBuffer::read(qsizetype offset, qsizetype length) const {
    if (isReadyRead(offset)) {
        return _chunks->data(offset, length);
    }
    return {};
}

bool QFileBuffer::insert(qsizetype offset, const QByteArray &data) {
    if (isReadyInsert(offset)) {
        return _chunks->insert(offset, data);
    }
    return false;
}

bool QFileBuffer::remove(qsizetype offset, qsizetype length) {
    if (isReadyReplaceWrite(offset, length)) {
        return _chunks->remove(offset, length);
    }
    return false;
}

bool QFileBuffer::replace(qsizetype offset, const QByteArray &data) {
    if (isReadyReplaceWrite(offset, data.length())) {
        return _chunks->overwrite(offset, data);
    }
    return false;
}

qsizetype QFileBuffer::indexOf(const QByteArray &ba, qsizetype from) const {
    if (_chunks) {
        return _chunks->indexOf(ba, from);
    } else {
        return -1;
    }
}

qsizetype QFileBuffer::lastIndexOf(const QByteArray &ba, qsizetype from) const {
    if (_chunks) {
        return _chunks->lastIndexOf(ba, from);
    } else {
        return -1;
    }
}
