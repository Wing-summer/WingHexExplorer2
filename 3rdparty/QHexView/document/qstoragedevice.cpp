#include "qstoragedevice.h"
#include "qstorageinfo.h"

QStorageDevice::QStorageDevice() : QIODevice() {}

void QStorageDevice::setStorage(const QStorageInfo &storage) {
    _storage = storage;
}

QStorageInfo QStorageDevice::storage() const { return _storage; }

qint64 QStorageDevice::readData(char *data, qint64 maxlen) { return -1; }

qint64 QStorageDevice::writeData(const char *data, qint64 len) { return -1; }
