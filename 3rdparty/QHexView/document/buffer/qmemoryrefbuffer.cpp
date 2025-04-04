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
**
** The original License is MIT from Dax89/QHexView. I have modified a lot so I
** decide to change the Open Source License. You can use the original library
** under MIT. Thanks for Dax89's efforts.
** =============================================================================
*/

#include "qmemoryrefbuffer.h"

#include <QObject>
#include <climits>

QMemoryRefBuffer::QMemoryRefBuffer(QObject *parent) : QHexBuffer(parent) {}

qsizetype QMemoryRefBuffer::length() const { return m_buffer->size(); }

void QMemoryRefBuffer::insert(qsizetype offset, const QByteArray &data) {
    Q_UNUSED(offset)
    Q_UNUSED(data)
    /* Insertion unsupported */
}

void QMemoryRefBuffer::remove(qsizetype offset, qsizetype length) {
    Q_UNUSED(offset)
    Q_UNUSED(length)
    /* Deletion unsupported */
}

QByteArray QMemoryRefBuffer::read(qsizetype offset, qsizetype length) {
    m_buffer->seek(offset);
    return m_buffer->read(length);
}

bool QMemoryRefBuffer::read(QIODevice *device) {
    m_buffer = qobject_cast<QBuffer *>(device);
    if (m_buffer) {
        m_buffer->setParent(this);
        return true;
    }
    return false;
}

void QMemoryRefBuffer::write(QIODevice *device) {
    m_buffer->seek(0);
    if (m_buffer->size() < INT_MAX) {
        device->write(m_buffer->readAll());
    } else {
        while (m_buffer->pos() < m_buffer->size()) {
            char tmpBuf[4096];
            qsizetype chunkLen = m_buffer->read(tmpBuf, 4096);
            if (chunkLen == -1)
                break;
            if (chunkLen > 0) {
                device->write(tmpBuf, chunkLen);
                m_buffer->seek(m_buffer->pos() + chunkLen);
            }
        }
    }
}

qsizetype QMemoryRefBuffer::indexOf(const QByteArray &ba, qsizetype from) {
    qsizetype findPos = -1;
    if (from < m_buffer->size()) {
        findPos = from;
        m_buffer->seek(from);

        while (findPos < m_buffer->size()) {
            QByteArray data = m_buffer->read(INT_MAX);
            int idx = data.indexOf(ba);
            if (idx >= 0) {
                findPos += idx;
                break;
            }
            if (findPos + data.size() >= m_buffer->size())
                return -1;
            m_buffer->seek(m_buffer->pos() + data.size() - ba.size());
        }
    }
    return findPos;
}

qsizetype QMemoryRefBuffer::lastIndexOf(const QByteArray &ba, qsizetype from) {
    qsizetype findPos = -1;
    if (from >= 0 && ba.size() < INT_MAX) {
        qsizetype currPos = from;
        while (currPos >= 0) {
            qsizetype readPos = (currPos < INT_MAX) ? 0 : currPos - INT_MAX;
            m_buffer->seek(readPos);
            QByteArray data = m_buffer->read(currPos - readPos);
            int idx = data.lastIndexOf(ba, from);
            if (idx >= 0) {
                findPos = readPos + idx;
                break;
            }
            if (readPos <= 0)
                break;
            currPos = readPos + ba.size();
        }
    }
    return findPos;
}
