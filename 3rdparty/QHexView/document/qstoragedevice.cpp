#include "qstoragedevice.h"

#ifdef Q_OS_WIN

#include <QDebug>
#include <memory>

QStorageDevice::QStorageDevice(QObject *parent)
    : QIODevice(parent), CHUNK_SIZE(0), _size(0) {
    _buffer.setTextModeEnabled(false);
}

void QStorageDevice::setStorage(const QStorageInfo &storage) {
    _storage = storage;
}

QStorageInfo QStorageDevice::storage() const { return _storage; }

bool QStorageDevice::isSequential() const { return false; }

bool QStorageDevice::open(OpenMode mode) {
    if (_buffer.isOpen()) {
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

        // Open the physical drive using WinAPI
        auto hDevice =
            CreateFileW(reinterpret_cast<LPCWSTR>(dd.utf16()), GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                        OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, nullptr);

        if (hDevice == INVALID_HANDLE_VALUE) {
            qWarning() << "Failed to open device:" << device;
            return false;
        }

        DISK_GEOMETRY diskGeometry;
        DWORD bytesReturned;
        if (!DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0,
                             &diskGeometry, sizeof(diskGeometry),
                             &bytesReturned, NULL)) {
            CloseHandle(hDevice);
            return false;
        }

        CloseHandle(hDevice);

        this->CHUNK_SIZE = diskGeometry.BytesPerSector;

        // dont use ioDevice.bytesTotal(),
        // because it's use GetDiskFreeSpaceEx API to get.
        // QFile::size() is zero
        _size = diskGeometry.Cylinders.QuadPart *
                diskGeometry.TracksPerCylinder * diskGeometry.SectorsPerTrack *
                diskGeometry.BytesPerSector;

        if (!_buffer.open(mode)) {
            // _buffer.setFileName({});
            return false;
        }

        return QIODevice::open(QIODevice::ReadOnly);
    } else {
        qWarning() << "Only OpenModeFlag::ReadOnly and OpenModeFlag::WriteOnly "
                      "are supported";
        return false;
    }
}

void QStorageDevice::close() {
    if (_buffer.isOpen()) {
        _buffer.close();
    }
    QIODevice::close();
}

qint64 QStorageDevice::size() const { return _size; }

bool QStorageDevice::seek(qint64 pos) {
    if (!_buffer.isOpen()) {
        return false;
    }

    return QIODevice::seek(pos);
}

bool QStorageDevice::canReadLine() const { return false; }

qint64 QStorageDevice::readData(char *data, qint64 maxlen) {
    if (!isOpen() || !maxlen) {
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
    _buffer.seek(rp.quot * CHUNK_SIZE);

    DWORD bytesRead = 0;

    if (rp.rem) {
        auto buf = std::make_unique<char[]>(maxlen);
        _buffer.read(buf.get(), maxlen);
        std::memcpy(data, buf.get() + rp.rem, maxlen);
    } else {
        _buffer.read(data, maxlen);
    }

    return maxlen;
}

qint64 QStorageDevice::writeData(const char *data, qint64 len) {
    // qt will check writeable attr
    if (!isOpen()) {
        return false;
    }

    // Ensure maxSize is a multiple of the sector size
    auto r = std::div(len, CHUNK_SIZE);
    auto alignLen = r.quot * CHUNK_SIZE;

    auto rp = std::div(this->pos(), CHUNK_SIZE);
    _buffer.seek(rp.quot * CHUNK_SIZE);
    if (rp.rem) {
    }

    _buffer.write(data, alignLen);

    if (r.rem) {
        _buffer.seek(this->pos() + alignLen);
        auto buf = std::make_unique<char[]>(CHUNK_SIZE);
        _buffer.read(buf.get(), CHUNK_SIZE);
        DWORD byteWritten_A = 0;
        std::memcpy(buf.get(), data + alignLen, r.rem);

        if (_buffer.write(buf.get(), CHUNK_SIZE)) {
            qWarning() << "Failed to write data to device";
            return -1;
        }
    }

    return len;
}

#endif
