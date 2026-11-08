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
#include <QFile>
#include <QFileInfo>
#include <QReadLocker>
#include <QWriteLocker>

#define BUFFER_SIZE 0x10000
#define CHUNK_SIZE 0x1000
#define READ_CHUNK_MASK Q_INT64_C(0xfffffffffffff000)

/*this file is modified by wingsummer in order to fit the QHexView*/

// ***************************************** Constructors and file settings

Chunks::Chunks(QIODevice *ioDevice) { setIODevice(ioDevice); }

Chunks::~Chunks() {
    if (m_ioDevice) {
        resetIODevice();
    }
}

bool Chunks::setIODevice(QIODevice *ioDevice) {
    if (ioDevice == nullptr) {
        return false;
    }
    QWriteLocker locker(&m_mutex);
    m_ioDevice = ioDevice;
    m_size = m_ioDevice->size();
    m_chunks.clear();
    return true;
}

void Chunks::resetIODevice() {
    QWriteLocker locker(&m_mutex);
    m_ioDevice = nullptr;
    m_size = 0;
    m_chunks.clear();
}

// ***************************************** Getting data out of Chunks

QByteArray Chunks::data(qint64 pos, qint64 maxSize) const {
    QWriteLocker locker(&m_mutex);
    return __data(pos, maxSize);
}

bool Chunks::write(QIODevice *iODevice, qint64 pos, qint64 count) {
    QWriteLocker locker(&m_mutex);
    if (count == -1) {
        count = m_size;
    }
    if (iODevice && iODevice->isOpen() && iODevice->isWritable()) {
        for (qint64 idx = pos; idx < count; idx += BUFFER_SIZE) {
            QByteArray ba = __data(idx, BUFFER_SIZE);
            iODevice->write(ba);
        }
        return true;
    }
    return false;
}

// ***************************************** Search API

qint64 Chunks::indexOf(const QByteArray &ba, qint64 from) const {
    qint64 result = -1;
    QByteArray buffer;
    QWriteLocker locker(&m_mutex);
    for (qint64 pos = from; (pos < m_size) && (result < 0);
         pos += BUFFER_SIZE) {
        buffer = __data(pos, BUFFER_SIZE + ba.size() - 1);
        auto findPos = buffer.indexOf(ba);
        if (findPos >= 0) {
            result = pos + findPos;
        }
    }
    return result;
}

qint64 Chunks::lastIndexOf(const QByteArray &ba, qint64 from) const {
    qint64 result = -1;
    QByteArray buffer;
    QWriteLocker locker(&m_mutex);
    for (qint64 pos = from; (pos > 0) && (result < 0); pos -= BUFFER_SIZE) {
        qint64 sPos = pos - BUFFER_SIZE - ba.size() + 1;
        if (sPos < 0) {
            sPos = 0;
        }
        buffer = __data(sPos, pos - sPos);
        auto findPos = buffer.lastIndexOf(ba);
        if (findPos >= 0) {
            result = sPos + findPos;
        }
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
    if ((pos < 0) || (pos > m_size)) {
        return false;
    }

    if (ba.isEmpty()) {
        return true;
    }

    QWriteLocker locker(&m_mutex);
    auto length = ba.length();
    qsizetype chunkIdx;
    if (m_size && pos == m_size) {
        chunkIdx = getChunkIndex(pos - 1);
    } else {
        chunkIdx = getChunkIndex(pos);
    }

    if (chunkIdx < 0) {
        return false;
    }

    qint64 posInBa = pos - m_chunks[chunkIdx].absPos;
    m_chunks[chunkIdx].data.insert(posInBa, ba);
    for (auto idx = chunkIdx + 1; idx < m_chunks.size(); idx++) {
        m_chunks[idx].absPos += length;
    }
    m_size += length;
    return true;
}

bool Chunks::overwrite(qint64 pos, const QByteArray &ba) {
    QWriteLocker locker(&m_mutex);
    return __overwrite(pos, ba);
}

bool Chunks::remove(qint64 pos, qint64 length) {
    QWriteLocker locker(&m_mutex);
    return __remove(pos, length);
}

// ***************************************** Utility functions

char Chunks::at(qint64 pos) const { return data(pos, 1).at(0); }

char Chunks::operator[](qint64 pos) const { return this->at(pos); }

qint64 Chunks::size() const { return m_size; }

qsizetype Chunks::getChunkIndex(qint64 absPos) const {
    // This routine checks, if there is already a copied chunk available. If os,
    // it returns a reference to it. If there is no copied chunk available,
    // original data will be copied into a new chunk.

    qsizetype foundIdx = -1;
    qsizetype insertIdx = 0;
    qint64 ioDelta = 0;

    for (qsizetype idx = 0; idx < m_chunks.size(); idx++) {
        Chunk chunk = m_chunks[idx];
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

    if (foundIdx == -1 && m_ioDevice) {
        auto oldPos = m_ioDevice->pos();
        Chunk newChunk;
        qint64 readAbsPos = absPos - ioDelta;
        qint64 readPos = (readAbsPos & READ_CHUNK_MASK);
        if (!m_ioDevice->seek(readPos)) {
            return -1;
        }
        newChunk.data = m_ioDevice->read(CHUNK_SIZE);
        newChunk.absPos = absPos - (readAbsPos - readPos);
        m_chunks.insert(insertIdx, newChunk);
        foundIdx = insertIdx;
        m_ioDevice->seek(oldPos);
    }
    return foundIdx;
}

bool Chunks::__overwrite(qint64 pos, const QByteArray &ba) {
    if ((pos < 0) || (pos >= m_size)) {
        return false;
    }

    qsizetype chunkIdx = getChunkIndex(pos);
    if (chunkIdx < 0) {
        return false;
    }

    auto &chunk = m_chunks[chunkIdx];
    qint64 posInBa = pos - chunk.absPos;

    auto length = ba.length();
    auto clen = chunk.data.size();
    auto dist = length + posInBa - clen;

    if (dist <= 0) {
        chunk.data.replace(posInBa, length, ba);
    } else {
        auto len = clen - posInBa;
        chunk.data.replace(posInBa, len, ba.first(len));
        if (!__overwrite(pos + clen, ba.right(dist))) {
            return false;
        }
    }

    return true;
}

bool Chunks::__remove(qint64 pos, qint64 length) {
    if ((pos < 0) || (pos >= m_size)) {
        return false;
    }

    qsizetype chunkIdx = getChunkIndex(pos);
    if (chunkIdx < 0) {
        return false;
    }

    auto &chunk = m_chunks[chunkIdx];
    qint64 posInBa = pos - chunk.absPos;

    auto clen = chunk.data.size();
    auto dist = length + posInBa - clen;

    if (dist <= 0) {
        chunk.data.remove(posInBa, length);
        for (auto idx = chunkIdx + 1; idx < m_chunks.size(); idx++) {
            m_chunks[idx].absPos += length;
        }
        m_size -= length;
    } else {
        auto len = clen - posInBa;
        chunk.data.remove(posInBa, len);
        for (auto idx = chunkIdx + 1; idx < m_chunks.size(); idx++) {
            m_chunks[idx].absPos -= len;
        }
        m_size -= len;

        if (!__remove(pos + clen, dist)) {
            return false;
        }
    }

    return true;
}

QByteArray Chunks::__data(qint64 pos, qint64 maxSize) const {
    qint64 ioDelta = 0;
    qsizetype chunkIdx = 0;

    Chunk chunk;
    QByteArray buffer;

    // Do some checks and some arrangements
    if (pos >= m_size) {
        return buffer;
    }

    if (maxSize < 0) {
        maxSize = m_size;
    } else if ((pos + maxSize) > m_size) {
        maxSize = m_size - pos;
    }

    if (!m_ioDevice || !m_ioDevice->isReadable()) {
        return {};
    }

    auto oldPos = m_ioDevice->pos(); // save the pos state
    while (maxSize > 0) {
        chunk.absPos = LLONG_MAX;
        bool chunksLoopOngoing = true;
        while ((chunkIdx < m_chunks.count()) && chunksLoopOngoing) {
            // In this section, we track changes before our required data and
            // we take the editdet data, if availible. ioDelta is a difference
            // counter to justify the read pointer to the original data, if
            // data in between was deleted or inserted.

            chunk = m_chunks[chunkIdx];
            if (chunk.absPos > pos) {
                chunksLoopOngoing = false;
            } else {
                chunkIdx += 1;
                qint64 count;
                qint64 chunkOfs = pos - chunk.absPos;
                if (maxSize > (chunk.data.size() - chunkOfs)) {
                    count = chunk.data.size() - chunkOfs;
                    ioDelta += CHUNK_SIZE - chunk.data.size();
                } else {
                    count = maxSize;
                }
                if (count > 0) {
                    buffer += chunk.data.sliced(chunkOfs, count);
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
            if ((chunk.absPos - pos) > maxSize) {
                byteCount = maxSize;
            } else {
                byteCount = chunk.absPos - pos;
            }

            maxSize -= byteCount;
            if (!m_ioDevice->seek(pos + ioDelta)) {
                return {};
            }
            readBuffer = m_ioDevice->read(byteCount);
            buffer += readBuffer;
            pos += readBuffer.size();
        }
    }
    m_ioDevice->seek(oldPos); // restore it
    return buffer;
}

QIODevice *Chunks::ioDevice() const { return m_ioDevice; }

QHexReadSessionResult Chunks::readSession(quint64 budget) const {
    QReadLocker locker(&m_mutex);
    if (!m_ioDevice || !m_ioDevice->isReadable() ||
        m_ioDevice->isSequential()) {
        return readSessionError(budget, QHexReadError::InvalidDevice);
    }
    auto result = std::make_shared<QHexReadSession>();
    result->length = m_size;
    if (auto file = qobject_cast<QFile *>(m_ioDevice)) {
        const QFileInfo info(file->fileName());
        if (!info.isFile()) {
            return readSessionError(budget, QHexReadError::OwnerThreadRequired);
        }
        result->fileName = info.absoluteFilePath();
        result->sourceSize = info.size();
        result->modified = info.lastModified();
    } else if (auto memory = qobject_cast<QBuffer *>(m_ioDevice)) {
        result->memory = memory->data();
        result->sourceSize = result->memory.size();
    } else {
        return readSessionError(budget, QHexReadError::OwnerThreadRequired);
    }
    // Sharing is initially cheap, but a concurrent editor mutation can detach
    // the retained allocation. Reserve for that cost instead of pinning an
    // arbitrarily large edited QByteArray throughout a comparison.
    quint64 retained = quint64(result->memory.capacity());
    if (retained > budget) {
        return readSessionError(budget, QHexReadError::BudgetExceeded);
    }
    for (const auto &chunk : std::as_const(m_chunks)) {
        const auto bytes = quint64(chunk.data.capacity());
        if (bytes > budget - retained) {
            return readSessionError(budget, QHexReadError::BudgetExceeded);
        }
        retained += bytes;
    }
    const quint64 capacity = quint64(m_chunks.size()) * 2 + 1;
    if (capacity > (budget - retained) / sizeof(QHexReadSession::Piece)) {
        return readSessionError(budget, QHexReadError::BudgetExceeded);
    }
    result->pieces.reserve(size_t(capacity));
    qint64 logical = 0;
    qint64 source = 0;
    for (const auto &chunk : std::as_const(m_chunks)) {
        const auto gap = chunk.absPos - logical;
        if (gap < 0) {
            return readSessionError(budget, QHexReadError::InvalidMapping);
        }
        if (gap) {
            result->pieces.push_back({logical, gap, source, {}, false});
            logical += gap;
            source += gap;
        }
        if (!chunk.data.isEmpty()) {
            result->pieces.push_back(
                {logical, chunk.data.size(), 0, chunk.data, true});
            logical += chunk.data.size();
        }
        source += CHUNK_SIZE;
    }
    if (logical < m_size) {
        result->pieces.push_back(
            {logical, m_size - logical, source, {}, false});
    }
    return {result, QHexReadError::None};
}

QHexReadSessionResult Chunks::readSessionError(quint64 budget,
                                               QHexReadError error) const {
    const auto file = qobject_cast<QFile *>(m_ioDevice);
    const auto name = file ? file->fileName() : QString();
    switch (error) {
    case QHexReadError::BudgetExceeded:
        qCDebug(lcQHexRead) << "The read session exceeds the memory budget."
                            << name << "budget" << budget;
        break;
    case QHexReadError::OwnerThreadRequired:
        qCDebug(lcQHexRead) << "The device needs reads on its owning thread."
                            << name << "budget" << budget;
        break;
    case QHexReadError::InvalidDevice:
        qCWarning(lcQHexRead)
            << "The device is not readable and seekable." << name;
        break;
    case QHexReadError::InvalidMapping:
        qCWarning(lcQHexRead) << "Invalid document mapping." << name;
        break;
    default:
        qCWarning(lcQHexRead)
            << "Cannot create read session" << name << "error" << int(error);
        break;
    }
    return {{}, error};
}
