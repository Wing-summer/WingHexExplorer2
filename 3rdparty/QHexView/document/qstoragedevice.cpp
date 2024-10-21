#include "qstoragedevice.h"

#ifdef Q_OS_WIN

#include <QDebug>

QStorageDevice::QStorageDevice(QObject *parent)
    : QIODevice(parent), hDevice(INVALID_HANDLE_VALUE), CHUNK_SIZE(0),
      _size(0) {}

void QStorageDevice::setStorage(const QStorageInfo &storage) {
    _storage = storage;
}

QStorageInfo QStorageDevice::storage() const { return _storage; }

DWORD QStorageDevice::cacheSize() const { return 20 * 1024 * CHUNK_SIZE; }

bool QStorageDevice::isSequential() const { return false; }

bool QStorageDevice::open(OpenMode mode) {
    if (hDevice != INVALID_HANDLE_VALUE) {
        setErrorString(QStringLiteral("A Storage file is still opened"));
        return false;
    }

    if (mode == OpenModeFlag::ReadOnly || mode == OpenModeFlag::WriteOnly ||
        mode == OpenModeFlag::ReadWrite) {
        auto device = _storage.device();
        auto devicePrefix = QStringLiteral("\\\\.\\");
        QString dd = devicePrefix +
                     device.mid(devicePrefix.length(),
                                device.length() - devicePrefix.length() - 1);

        DWORD flag =
            (mode.testFlag(OpenModeFlag::ReadOnly) ? GENERIC_READ : 0) |
            (mode.testFlag(OpenModeFlag::WriteOnly) ? GENERIC_WRITE : 0);

        // Open the physical drive using WinAPI
        hDevice = CreateFileW(
            reinterpret_cast<LPCWSTR>(dd.utf16()), flag,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
            FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, nullptr);

        if (hDevice == INVALID_HANDLE_VALUE) {
            qWarning() << "Failed to open device:" << device;
            return false;
        }

        DISK_GEOMETRY diskGeometry;
        DWORD bytesReturned;
        if (!DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr, 0,
                             &diskGeometry, sizeof(diskGeometry),
                             &bytesReturned, NULL)) {
            CloseHandle(hDevice);
            return false;
        }

        this->CHUNK_SIZE = diskGeometry.BytesPerSector;

        _cache.buffer = std::make_unique<char[]>(cacheSize());
        _cache.length = 0;

        // dont use ioDevice.bytesTotal(),
        // because it's use GetDiskFreeSpaceEx API to get.
        // QFile::size() is zero
        _size = diskGeometry.Cylinders.QuadPart *
                diskGeometry.TracksPerCylinder * diskGeometry.SectorsPerTrack *
                diskGeometry.BytesPerSector;

        return QIODevice::open(mode);
    } else {
        qWarning() << "Only OpenModeFlag::ReadOnly and OpenModeFlag::WriteOnly "
                      "are supported";
        return false;
    }
}

void QStorageDevice::close() {
    if (hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(hDevice);
        hDevice = INVALID_HANDLE_VALUE;
        _cache.clear();
    }
    QIODevice::close();
}

qint64 QStorageDevice::size() const { return _size; }

bool QStorageDevice::seek(qint64 pos) {
    if (hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    return QIODevice::seek(pos);
}

bool QStorageDevice::canReadLine() const { return false; }

qint64 QStorageDevice::readData(char *data, qint64 maxlen) {
    if (hDevice == INVALID_HANDLE_VALUE || !maxlen) {
        return -1;
    }

    if (maxlen > std::numeric_limits<DWORD>::max() ||
        (maxlen < 0 && _size > 1024 * 1024 * 1024)) {
        qWarning() << "Read a lot mount of data once is not allowed";
        return -1;
    }

    if (maxlen < 0) {
        maxlen = _size;
    }

    auto rp = std::div(this->pos(), CHUNK_SIZE);
    auto off = rp.quot * CHUNK_SIZE;

    if (_cache.offset < 0 || this->pos() < _cache.offset ||
        this->pos() + maxlen >= _cache.offset + _cache.length) {
        OVERLAPPED overlapped{0};
        LARGE_INTEGER offset;
        offset.QuadPart = off;
        overlapped.Offset = offset.LowPart;
        overlapped.OffsetHigh = offset.HighPart;

        if (!ReadFile(hDevice, _cache.buffer.get(), cacheSize(), nullptr,
                      &overlapped)) {
            auto lastError = GetLastError();
            if (lastError == ERROR_IO_PENDING) {
                if (!GetOverlappedResult(hDevice, &overlapped, &_cache.length,
                                         TRUE)) {
                    return -1;
                }
            } else {
                return -1;
            }
        }

        _cache.offset = off;
    }

    std::memcpy(data, _cache.buffer.get() + this->pos() - _cache.offset,
                maxlen);

    return maxlen;
}

qint64 QStorageDevice::writeData(const char *data, qint64 len) {
    // qt will check writeable attr
    if (!isOpen()) {
        return false;
    }

    // Ensure maxSize is a multiple of the sector size
    auto rp = std::div(this->pos(), CHUNK_SIZE);
    auto header = CHUNK_SIZE - rp.rem;
    auto r = std::div(len - header, CHUNK_SIZE);
    auto alignLen = r.quot * CHUNK_SIZE;

    OVERLAPPED overlapped{0};
    LARGE_INTEGER offset;
    DWORD length = 0;

    // _buffer.seek(rp.quot * CHUNK_SIZE);
    if (rp.rem) {
        // read some and write back
        offset.QuadPart = rp.quot * CHUNK_SIZE;

        auto buffer = std::make_unique<char[]>(CHUNK_SIZE);

        if (!ReadFile(hDevice, buffer.get(), CHUNK_SIZE, nullptr,
                      &overlapped)) {
            auto lastError = GetLastError();
            if (lastError == ERROR_IO_PENDING) {
                if (!GetOverlappedResult(hDevice, &overlapped, &length, TRUE)) {
                    return -1;
                }
            } else {
                return -1;
            }
        }

        std::memcpy(buffer.get(), data, CHUNK_SIZE - rp.rem);

        if (!WriteFile(hDevice, buffer.get(), CHUNK_SIZE, nullptr,
                       &overlapped)) {
            auto lastError = GetLastError();
            if (lastError == ERROR_IO_PENDING) {
                if (!GetOverlappedResult(hDevice, &overlapped, &length, TRUE)) {
                    return -1;
                }
            } else {
                return -1;
            }
        }
    }

    offset.QuadPart += CHUNK_SIZE;
    overlapped.Offset = offset.LowPart;
    overlapped.OffsetHigh = offset.HighPart;

    // write aligned
    if (!WriteFile(hDevice, data - header, alignLen, nullptr, &overlapped)) {
        auto lastError = GetLastError();
        if (lastError == ERROR_IO_PENDING) {
            if (!GetOverlappedResult(hDevice, &overlapped, &length, TRUE)) {
                return header;
            }
        } else {
            return header;
        }
    }

    if (r.rem) {
        offset.QuadPart += alignLen;
        overlapped.Offset = offset.LowPart;
        overlapped.OffsetHigh = offset.HighPart;

        auto buffer = std::make_unique<char[]>(CHUNK_SIZE);
        if (!ReadFile(hDevice, buffer.get(), CHUNK_SIZE, nullptr,
                      &overlapped)) {
            auto lastError = GetLastError();
            if (lastError == ERROR_IO_PENDING) {
                if (!GetOverlappedResult(hDevice, &overlapped, &length, TRUE)) {
                    return header + alignLen;
                }
            } else {
                return header + alignLen;
            }
        }
        std::memcpy(buffer.get(), data + len - r.rem, r.rem);

        if (!WriteFile(hDevice, buffer.get(), CHUNK_SIZE, nullptr,
                       &overlapped)) {
            auto lastError = GetLastError();
            if (lastError == ERROR_IO_PENDING) {
                if (!GetOverlappedResult(hDevice, &overlapped, &length, TRUE)) {
                    return header + alignLen;
                }
            } else {
                return header + alignLen;
            }
        }
    }

    return len;
}

#endif
