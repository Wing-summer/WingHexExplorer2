/*
 * QHexEdit is a Hex Editor Widget for the Qt Framework
 * Copyright (C) 2010-2025 Winfried Simon
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * https://www.gnu.org/licenses/
 */

#include "chunks.h"
#include <limits.h>

#include <QBuffer>

#define NORMAL 0
#define HIGHLIGHTED 1

#define BUFFER_SIZE 0x10000
#define CHUNK_SIZE 0x1000
#define READ_CHUNK_MASK Q_INT64_C(0xfffffffffffff000)

/*this file is modified by wingsummer in order to fit the QHexView*/

// ***************************************** Constructors and file settings

Chunks::Chunks(QObject *parent) : QObject(parent) {
    QBuffer *buf = new QBuffer(this);
    setIODevice(buf);
}

Chunks::Chunks(QIODevice *ioDevice, QObject *parent) : QObject(parent) {
    setIODevice(ioDevice);
}

bool Chunks::setIODevice(QIODevice *ioDevice) {
    _ioDevice = ioDevice;
    bool ok = _ioDevice->open(QIODevice::ReadOnly);
    if (ok) // Try to open IODevice
    {
        ioDevice->setParent(this);
        _size = _ioDevice->size();
        _ioDevice->close();
    } else // Fallback is an empty buffer
    {
        QBuffer *buf = new QBuffer(this);
        _ioDevice = buf;
        _size = 0;
    }
    _chunks.clear();
    return ok;
}

// ***************************************** Getting data out of Chunks

QByteArray Chunks::data(qint64 pos, qint64 maxSize) const {
    qint64 ioDelta = 0;
    int chunkIdx = 0;

    Chunk chunk;
    QByteArray buffer;

    // Do some checks and some arrangements
    if (pos >= _size)
        return buffer;

    if (maxSize < 0)
        maxSize = _size;
    else if ((pos + maxSize) > _size)
        maxSize = _size - pos;

    _ioDevice->open(QIODevice::ReadOnly);

    while (maxSize > 0) {
        chunk.absPos = LLONG_MAX;
        bool chunksLoopOngoing = true;
        while ((chunkIdx < _chunks.count()) && chunksLoopOngoing) {
            // In this section, we track changes before our required data and
            // we take the editdet data, if availible. ioDelta is a difference
            // counter to justify the read pointer to the original data, if
            // data in between was deleted or inserted.

            chunk = _chunks[chunkIdx];
            if (chunk.absPos > pos)
                chunksLoopOngoing = false;
            else {
                chunkIdx += 1;
                qint64 count;
                qint64 chunkOfs = pos - chunk.absPos;
                if (maxSize > ((qint64)chunk.data.size() - chunkOfs)) {
                    count = (qint64)chunk.data.size() - chunkOfs;
                    ioDelta += CHUNK_SIZE - chunk.data.size();
                } else
                    count = maxSize;
                if (count > 0) {
                    buffer += chunk.data.mid(chunkOfs, (int)count);
                    maxSize -= count;
                    pos += count;
                }
            }
        }

        if ((maxSize > 0) && (pos < chunk.absPos)) {
            // In this section, we read data from the original source. This only
            // will happen, whe no copied data is available

            qint64 byteCount;
            QByteArray readBuffer;
            if ((chunk.absPos - pos) > maxSize)
                byteCount = maxSize;
            else
                byteCount = chunk.absPos - pos;

            maxSize -= byteCount;
            _ioDevice->seek(pos + ioDelta);
            readBuffer = _ioDevice->read(byteCount);
            buffer += readBuffer;
            pos += readBuffer.size();
        }
    }
    _ioDevice->close();
    return buffer;
}

bool Chunks::write(QIODevice *iODevice, qint64 pos, qint64 count) {
    if (count == -1)
        count = _size;
    if (iODevice->isOpen()) {
        if (iODevice->isWritable()) {
            for (qint64 idx = pos; idx < count; idx += BUFFER_SIZE) {
                QByteArray ba = data(idx, BUFFER_SIZE);
                iODevice->write(ba);
            }
            return true;
        }
        return false;
    } else {
        bool ok = iODevice->open(QIODevice::WriteOnly);
        if (ok) {
            for (qint64 idx = pos; idx < count; idx += BUFFER_SIZE) {
                QByteArray ba = data(idx, BUFFER_SIZE);
                iODevice->write(ba);
            }
            iODevice->close();
        }
        return ok;
    }
}

// ***************************************** Search API

qint64 Chunks::indexOf(const QByteArray &ba, qint64 from) const {
    qint64 result = -1;
    QByteArray buffer;

    for (qint64 pos = from; (pos < _size) && (result < 0); pos += BUFFER_SIZE) {
        buffer = data(pos, BUFFER_SIZE + ba.size() - 1);
        int findPos = buffer.indexOf(ba);
        if (findPos >= 0)
            result = pos + (qint64)findPos;
    }
    return result;
}

qint64 Chunks::lastIndexOf(const QByteArray &ba, qint64 from) const {
    qint64 result = -1;
    QByteArray buffer;

    for (qint64 pos = from; (pos > 0) && (result < 0); pos -= BUFFER_SIZE) {
        qint64 sPos = pos - BUFFER_SIZE - (qint64)ba.size() + 1;
        if (sPos < 0)
            sPos = 0;
        buffer = data(sPos, pos - sPos);
        int findPos = buffer.lastIndexOf(ba);
        if (findPos >= 0)
            result = sPos + (qint64)findPos;
    }
    return result;
}

// ***************************************** Char manipulations

bool Chunks::insert(qint64 pos, char b) {
    return insert(pos, QByteArray(1, b));
}

bool Chunks::overwrite(qint64 pos, char b) {
    return overwrite(pos, QByteArray(1, b));
}

bool Chunks::removeAt(qint64 pos) { return remove(pos, 1); }

bool Chunks::insert(qint64 pos, const QByteArray &ba) {
    if ((pos < 0) || (pos > _size))
        return false;

    if (ba.isEmpty()) {
        return true;
    }

    auto length = ba.length();
    int chunkIdx;
    if (pos == _size)
        chunkIdx = getChunkIndex(pos - 1);
    else
        chunkIdx = getChunkIndex(pos);
    qint64 posInBa = pos - _chunks[chunkIdx].absPos;
    _chunks[chunkIdx].data.insert(posInBa, ba);
    for (int idx = chunkIdx + 1; idx < _chunks.size(); idx++)
        _chunks[idx].absPos += length;
    _size += length;
    return true;
}

bool Chunks::overwrite(qint64 pos, const QByteArray &ba) {
    if ((pos < 0) || (pos >= _size))
        return false;
    int chunkIdx = getChunkIndex(pos);

    auto &chunk = _chunks[chunkIdx];
    qint64 posInBa = pos - chunk.absPos;

    auto length = ba.length();
    auto clen = chunk.data.size();
    auto dist = length + posInBa - clen;

    if (dist <= 0) {
        chunk.data.replace(posInBa, length, ba);
    } else {
        auto len = clen - posInBa;
        chunk.data.replace(posInBa, len, ba.left(len));
        if (!overwrite(pos + clen, ba.right(dist))) {
            return false;
        }
    }

    return true;
}

bool Chunks::remove(qint64 pos, qint64 length) {
    if ((pos < 0) || (pos >= _size))
        return false;
    int chunkIdx = getChunkIndex(pos);

    auto &chunk = _chunks[chunkIdx];
    qint64 posInBa = pos - chunk.absPos;

    auto clen = chunk.data.size();
    auto dist = length + posInBa - clen;

    if (dist <= 0) {
        chunk.data.remove(posInBa, length);
        for (int idx = chunkIdx + 1; idx < _chunks.size(); idx++)
            _chunks[idx].absPos += length;
        _size -= length;
    } else {
        auto len = clen - posInBa;
        chunk.data.remove(posInBa, len);
        for (int idx = chunkIdx + 1; idx < _chunks.size(); idx++)
            _chunks[idx].absPos -= len;
        _size -= len;

        if (!remove(pos + clen, dist)) {
            return false;
        }
    }

    return true;
}

// ***************************************** Utility functions

char Chunks::at(qint64 pos) const { return data(pos, 1).at(0); }

char Chunks::operator[](qint64 pos) const { return this->at(pos); }

qint64 Chunks::size() const { return _size; }

int Chunks::getChunkIndex(qint64 absPos) {
    // This routine checks, if there is already a copied chunk available. If os,
    // it returns a reference to it. If there is no copied chunk available,
    // original data will be copied into a new chunk.

    int foundIdx = -1;
    int insertIdx = 0;
    qint64 ioDelta = 0;

    for (int idx = 0; idx < _chunks.size(); idx++) {
        Chunk chunk = _chunks[idx];
        if ((absPos >= chunk.absPos) &&
            (absPos < (chunk.absPos + chunk.data.size()))) {
            foundIdx = idx;
            break;
        }
        if (absPos < chunk.absPos) {
            insertIdx = idx;
            break;
        }
        ioDelta += chunk.data.size() - CHUNK_SIZE;
        insertIdx = idx + 1;
    }

    if (foundIdx == -1) {
        Chunk newChunk;
        qint64 readAbsPos = absPos - ioDelta;
        qint64 readPos = (readAbsPos & READ_CHUNK_MASK);
        _ioDevice->open(QIODevice::ReadOnly);
        _ioDevice->seek(readPos);
        newChunk.data = _ioDevice->read(CHUNK_SIZE);
        _ioDevice->close();
        newChunk.absPos = absPos - (readAbsPos - readPos);
        _chunks.insert(insertIdx, newChunk);
        foundIdx = insertIdx;
    }
    return foundIdx;
}

QIODevice *Chunks::ioDevice() const { return _ioDevice; }
