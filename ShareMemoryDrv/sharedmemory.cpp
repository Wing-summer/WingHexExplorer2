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

#include "sharedmemory.h"

#include <QDebug>

SharedMemory::SharedMemory(const QString &key, OpenMode mode, QObject *parent)
    : QIODevice(parent), m_key(key), m_mode(mode), m_sharedMemory(key) {

    if (m_mode == WriteOnly) {
        if (!m_sharedMemory.create(1024)) {
            qWarning() << "Unable to create shared memory segment.";
        }
    }
}

SharedMemory::~SharedMemory() {
    if (m_sharedMemory.isAttached()) {
        m_sharedMemory.unlock();
        m_sharedMemory.detach();
    }
}

bool SharedMemory::open(OpenMode mode) {
    if ((m_mode == ReadOnly && !(mode & QIODevice::ReadOnly)) ||
        (m_mode == WriteOnly && !(mode & QIODevice::WriteOnly))) {
        return false;
    }

    if (m_mode == ReadOnly && !m_sharedMemory.attach()) {
        qWarning() << "Unable to attach to shared memory segment.";
        return false;
    }

    m_sharedMemory.lock(); // Lock memory for reading or writing safely

    return true;
}

void SharedMemory::close() {
    m_sharedMemory.unlock();
    if (m_mode == ReadOnly) {
        m_sharedMemory.detach();
    }
}

qint64 SharedMemory::readData(char *data, qint64 maxSize) {
    if (m_mode == WriteOnly) {
        return -1; // Writing is not allowed in this mode
    }

    if (maxSize <= 0) {
        return -1;
    }

    const char *sharedMemoryData =
        static_cast<const char *>(m_sharedMemory.constData());

    // Copy the data from shared memory
    qint64 dataSize = qMin(maxSize, m_sharedMemory.size());
    memcpy(data, sharedMemoryData, dataSize);

    return dataSize;
}

qint64 SharedMemory::writeData(const char *data, qint64 maxSize) {
    if (m_mode == ReadOnly) {
        return -1; // Reading is not allowed in this mode
    }

    if (maxSize <= 0) {
        return -1;
    }

    // Ensure the shared memory is large enough
    if (maxSize > m_sharedMemory.size()) {
        qWarning() << "Data exceeds shared memory size.";
        return -1;
    }

    // Copy data to the shared memory
    char *sharedMemoryData = static_cast<char *>(m_sharedMemory.data());
    memcpy(sharedMemoryData, data, maxSize);

    return maxSize;
}
