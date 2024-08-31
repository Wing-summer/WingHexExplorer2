#include "chunks_win.h"

#ifdef Q_OS_WIN

#define READ_CHUNK_MASK Q_INT64_C(0xfffffffffffffd00)

/*this file is modified by wingsummer in order to fit the QHexView*/

// ***************************************** Constructors and file settings

Chunks_win::Chunks_win(QObject *parent) : QObject(parent) {}

Chunks_win::Chunks_win(const QStorageInfo &ioDevice, QObject *parent)
    : QObject(parent) {
    setIODevice(ioDevice);
}

Chunks_win::~Chunks_win() {
    if (_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(_handle);
    }
}

bool Chunks_win::setIODevice(const QStorageInfo &ioDevice) {
    if (ioDevice.isValid()) {
        auto device = ioDevice.device();
        auto devicePrefix = QStringLiteral("\\\\.\\");
        QString dd = devicePrefix +
                     device.mid(devicePrefix.length(),
                                device.length() - devicePrefix.length() - 1);

        _handle = CreateFileA(dd.toLatin1(), GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                              OPEN_EXISTING, 0, NULL);
        if (_handle == INVALID_HANDLE_VALUE) {
            return false;
        }

        DISK_GEOMETRY diskGeometry;
        DWORD bytesReturned;
        if (!DeviceIoControl(_handle, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0,
                             &diskGeometry, sizeof(diskGeometry),
                             &bytesReturned, NULL)) {

            CloseHandle(_handle);
            return false;
        }

        // dont use ioDevice.bytesTotal(),
        // because it's use GetDiskFreeSpaceEx API to get
        this->CHUNK_SIZE = diskGeometry.BytesPerSector;
        _size = diskGeometry.Cylinders.QuadPart *
                diskGeometry.TracksPerCylinder * diskGeometry.SectorsPerTrack *
                diskGeometry.BytesPerSector;

    } else {
        return false;
    }
    _chunks.clear();
    _pos = 0;
    return true;
}

// ***************************************** Getting data out of Chunks_win

QByteArray Chunks_win::data(qsizetype pos, qsizetype maxSize) {
    qsizetype ioDelta = 0;
    qsizetype chunkIdx = 0;

    Chunk_win chunk;
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
                qsizetype count;
                qsizetype chunkOfs = pos - chunk.absPos;
                if (qsizetype(maxSize) > chunk.data.size() - chunkOfs) {
                    count = chunk.data.size() - chunkOfs;
                    ioDelta += CHUNK_SIZE - chunk.data.size();
                } else
                    count = maxSize;
                if (count > 0) {
                    buffer += chunk.data.mid(chunkOfs, count);
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
            if (chunk.absPos - pos > qsizetype(maxSize))
                byteCount = maxSize;
            else
                byteCount = chunk.absPos - pos;

            maxSize -= byteCount;

            // Set the pointer to the start of the sector you want to read
            LARGE_INTEGER sectorOffset;
            auto sorquot = (pos + ioDelta) / CHUNK_SIZE;
            auto sorrem = (pos + ioDelta) % CHUNK_SIZE;

            sectorOffset.QuadPart = sorquot * CHUNK_SIZE;
            Q_ASSERT(SetFilePointerEx(_handle, sectorOffset, NULL, FILE_BEGIN));

            readBuffer.fill(0, CHUNK_SIZE);
            DWORD bytesRead;
            Q_ASSERT(ReadFile(_handle, readBuffer.data(), CHUNK_SIZE,
                              &bytesRead, NULL));
            readBuffer = readBuffer.mid(sorrem, byteCount);
            buffer += readBuffer;
            pos += readBuffer.size();
        }
    }

    return buffer;
}

bool Chunks_win::write(QIODevice *iODevice, qsizetype pos, qsizetype count) {
    if (count == -1)
        count = _size;

    // fix the bug
    bool ok = (iODevice->isOpen() && iODevice->isWritable()) ||
              iODevice->open(QIODevice::WriteOnly);
    if (ok) {
        for (auto idx = pos; idx < qsizetype(count); idx += CHUNK_SIZE) {
            QByteArray ba = data(idx, CHUNK_SIZE);
            iODevice->write(ba);
        }
        iODevice->close();
    }
    return ok;
}

// ***************************************** Search API

qsizetype Chunks_win::indexOf(const QByteArray &ba, qsizetype from) {
    qsizetype result = -1;
    QByteArray buffer;

    for (auto pos = from; (pos < _size) && (result < 0); pos += CHUNK_SIZE) {
        buffer = data(pos, CHUNK_SIZE + ba.size() - 1);
        int findPos = buffer.indexOf(ba);
        if (findPos >= 0)
            result = pos + findPos;
    }
    return result;
}

qsizetype Chunks_win::lastIndexOf(const QByteArray &ba, qsizetype from) {
    qint64 result = -1;
    QByteArray buffer;

    for (auto pos = from; (pos > 0) && (result < 0); pos -= CHUNK_SIZE) {
        auto sPos = pos - CHUNK_SIZE - ba.size() + 1;
        /*if (sPos < 0)
          sPos = 0;*/
        buffer = data(sPos, pos - sPos);
        int findPos = buffer.lastIndexOf(ba);
        if (findPos >= 0)
            result = sPos + findPos;
    }
    return result;
}

// ***************************************** Char manipulations

bool Chunks_win::insert(qsizetype pos, char b) {
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
    for (int idx = chunkIdx + 1; idx < _chunks.size(); idx++)
        _chunks[idx].absPos += 1;
    _size += 1;
    _pos = pos;
    return true;
}

bool Chunks_win::overwrite(qsizetype pos, char b) {
    if (pos >= _size)
        return false;
    auto chunkIdx = getChunkIndex(pos);
    auto posInBa = pos - _chunks[chunkIdx].absPos;
    _chunks[chunkIdx].data[int(posInBa)] = b;
    _chunks[chunkIdx].dataChanged[int(posInBa)] = char(1);
    _pos = pos;
    return true;
}

bool Chunks_win::removeAt(qsizetype pos) {
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

char Chunks_win::operator[](qsizetype pos) {
    auto d = data(pos, 1);
    if (d.isEmpty())
        return '0';
    return d.at(0);
}

qsizetype Chunks_win::pos() { return _pos; }

qsizetype Chunks_win::size() { return _size; }

qsizetype Chunks_win::getChunkIndex(qsizetype absPos) {
    // This routine checks, if there is already a copied chunk available. If so,
    // it returns a reference to it. If there is no copied chunk available,
    // original data will be copied into a new chunk.

    qsizetype foundIdx = -1;
    qsizetype insertIdx = 0;
    qsizetype ioDelta = 0;

    // fix the bug by wingsummer
    if (absPos < 0) {
        Chunk_win newChunk;
        newChunk.data = QByteArray(CHUNK_SIZE, 0);
        newChunk.absPos = 0;
        newChunk.dataChanged = nullptr;
        _chunks.insert(insertIdx, newChunk);
        return insertIdx;
    }

    for (int idx = 0; idx < _chunks.size(); idx++) {
        Chunk_win chunk = _chunks[idx];
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
        Chunk_win newChunk;
        auto readAbsPos = absPos - ioDelta;
        auto readPos = (readAbsPos & READ_CHUNK_MASK);

        // Set the pointer to the start of the sector you want to read
        LARGE_INTEGER sectorOffset;
        sectorOffset.QuadPart = readPos;
        Q_ASSERT(SetFilePointerEx(_handle, sectorOffset, NULL, FILE_BEGIN));

        newChunk.data.fill(0, CHUNK_SIZE);
        DWORD bytesRead;
        Q_ASSERT(ReadFile(_handle, newChunk.data.data(), CHUNK_SIZE, &bytesRead,
                          NULL));
        newChunk.data.resize(bytesRead);

        newChunk.absPos = absPos - (readAbsPos - readPos);
        newChunk.dataChanged = QByteArray(newChunk.data.size(), char(0));
        _chunks.insert(insertIdx, newChunk);
        foundIdx = insertIdx;
    }
    return foundIdx;
}

#endif
