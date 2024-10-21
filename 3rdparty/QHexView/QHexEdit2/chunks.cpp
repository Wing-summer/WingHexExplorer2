#include "chunks.h"

#define BUFFER_SIZE 0x10000
#define CHUNK_SIZE 0x1000
#define READ_CHUNK_MASK Q_INT64_C(0xfffffffffffff000)

/*this file is modified by wingsummer in order to fit the QHexView*/

// ***************************************** Constructors and file settings

Chunks::Chunks(QObject *parent) : QObject(parent) {}

Chunks::Chunks(QIODevice *ioDevice, QObject *parent) : QObject(parent) {
    setIODevice(ioDevice);
}

Chunks::~Chunks() {}

bool Chunks::setIODevice(QIODevice *ioDevice) {
    if (ioDevice && ioDevice->isOpen()) {
        ioDevice->setParent(this);
        _size = ioDevice->size();
        _ioDevice = ioDevice;
    } else {
        return false;
    }
    _chunks.clear();
    _pos = 0;
    return true;
}

// ***************************************** Getting data out of Chunks

QByteArray Chunks::data(qsizetype pos, qsizetype maxSize) {
    qsizetype ioDelta = 0;
    qsizetype chunkIdx = 0;

    Chunk chunk;
    QByteArray buffer;

    // Do some checks and some arrangements

    if (pos >= _size)
        return buffer;

    if (maxSize < 0)
        maxSize = _size;
    else if ((pos + maxSize) > _size)
        maxSize = _size - pos;

    while (maxSize > 0) {
        chunk.absPos = std::numeric_limits<qsizetype>::max();
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
                qint64 chunkOfs = qint64(pos - chunk.absPos);
                if (maxSize > (chunk.data.size() - chunkOfs)) {
                    count = qint64(chunk.data.size()) - chunkOfs;
                    ioDelta += CHUNK_SIZE - quint64(chunk.data.size());
                } else
                    count = maxSize;
                if (count > 0) {
                    buffer += chunk.data.mid(int(chunkOfs), int(count));
                    maxSize -= count;
                    pos += quint64(count);
                }
            }
        }

        if ((maxSize > 0) && (pos < chunk.absPos)) {
            // In this section, we read data from the original source. This only
            // will happen, whe no copied data is available

            qint64 byteCount;
            QByteArray readBuffer;
            if (chunk.absPos - pos > qsizetype(maxSize))
                byteCount = maxSize;
            else
                byteCount = chunk.absPos - pos;

            maxSize -= byteCount;
            _ioDevice->seek(pos + ioDelta);
            readBuffer = _ioDevice->read(byteCount);
            buffer += readBuffer;
            pos += quint64(readBuffer.size());
        }
    }

    return buffer;
}

bool Chunks::write(QIODevice *iODevice, qsizetype pos, qsizetype count) {
    if (count == -1)
        count = _size;

    bool ok = iODevice->isOpen() && iODevice->isWritable();
    if (ok) {
        for (auto idx = pos; idx < qsizetype(count); idx += BUFFER_SIZE) {
            QByteArray ba = data(idx, BUFFER_SIZE);
            iODevice->write(ba);
        }
    }
    return ok;
}

// ***************************************** Search API

qsizetype Chunks::indexOf(const QByteArray &ba, qsizetype from) {
    qsizetype result = -1;
    QByteArray buffer;

    for (auto pos = from; (pos < _size) && (result < 0); pos += BUFFER_SIZE) {
        buffer = data(pos, BUFFER_SIZE + ba.size() - 1);
        int findPos = buffer.indexOf(ba);
        if (findPos >= 0)
            result = pos + findPos;
    }
    return result;
}

qsizetype Chunks::lastIndexOf(const QByteArray &ba, qsizetype from) {
    qint64 result = -1;
    QByteArray buffer;

    for (auto pos = from; (pos > 0) && (result < 0); pos -= BUFFER_SIZE) {
        auto sPos = pos - BUFFER_SIZE - ba.size() + 1;
        /*if (sPos < 0)
          sPos = 0;*/
        buffer = data(sPos, pos - sPos);
        auto findPos = buffer.lastIndexOf(ba);
        if (findPos >= 0)
            result = sPos + findPos;
    }
    return result;
}

// ***************************************** Char manipulations

bool Chunks::insert(qsizetype pos, char b) {
    if (pos > _size)
        return false;
    qsizetype chunkIdx;
    if (pos == _size) {
        chunkIdx = getChunkIndex(pos - 1);
    } else
        chunkIdx = getChunkIndex(pos);
    auto posInBa = pos - _chunks[chunkIdx].absPos;
    _chunks[chunkIdx].data.insert(int(posInBa), b);
    _chunks[chunkIdx].dataChanged.insert(int(posInBa), char(1));
    for (auto idx = chunkIdx + 1; idx < _chunks.size(); idx++)
        _chunks[idx].absPos += 1;
    _size += 1;
    _pos = pos;
    return true;
}

bool Chunks::overwrite(qsizetype pos, char b) {
    if (pos >= _size)
        return false;
    auto chunkIdx = getChunkIndex(pos);
    auto posInBa = pos - _chunks[chunkIdx].absPos;
    _chunks[chunkIdx].data[int(posInBa)] = b;
    _chunks[chunkIdx].dataChanged[int(posInBa)] = char(1);
    _pos = pos;
    return true;
}

bool Chunks::removeAt(qsizetype pos) {
    if (pos >= _size)
        return false;
    auto chunkIdx = getChunkIndex(pos);
    auto posInBa = pos - _chunks[chunkIdx].absPos;
    _chunks[chunkIdx].data.remove(int(posInBa), 1);
    _chunks[chunkIdx].dataChanged.remove(int(posInBa), 1);
    for (int idx = chunkIdx + 1; idx < _chunks.size(); idx++)
        _chunks[idx].absPos -= 1;
    _size -= 1;
    _pos = pos;
    return true;
}

// ***************************************** Utility functions

char Chunks::operator[](qsizetype pos) {
    auto d = data(pos, 1);
    if (d.isEmpty())
        return '0';
    return d.at(0);
}

qsizetype Chunks::pos() { return _pos; }

qsizetype Chunks::size() { return _size; }

qsizetype Chunks::getChunkIndex(qsizetype absPos) {
    // This routine checks, if there is already a copied chunk available. If so,
    // it returns a reference to it. If there is no copied chunk available,
    // original data will be copied into a new chunk.

    qsizetype foundIdx = -1;
    qsizetype insertIdx = 0;
    qsizetype ioDelta = 0;

    // fix the bug by wingsummer
    if (absPos < 0) {
        Chunk newChunk;
        newChunk.data = QByteArray(CHUNK_SIZE, 0);
        newChunk.absPos = 0;
        newChunk.dataChanged = nullptr;
        _chunks.insert(insertIdx, newChunk);
        return insertIdx;
    }

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
        qsizetype readAbsPos = absPos - ioDelta;
        qsizetype readPos = (readAbsPos & READ_CHUNK_MASK);
        _ioDevice->seek(qint64(readPos));
        newChunk.data = _ioDevice->read(CHUNK_SIZE);
        newChunk.absPos = absPos - (readAbsPos - readPos);
        newChunk.dataChanged = QByteArray(newChunk.data.size(), char(0));
        _chunks.insert(insertIdx, newChunk);
        foundIdx = insertIdx;
    }
    return foundIdx;
}
