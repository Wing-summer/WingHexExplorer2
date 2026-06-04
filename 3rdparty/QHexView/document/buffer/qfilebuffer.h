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
#ifndef QFILEBUFFER_H
#define QFILEBUFFER_H

#include <QBuffer>
#include <QIODevice>

class Chunks;

class QFileBuffer {
public:
    explicit QFileBuffer();
    virtual ~QFileBuffer();

public:
    // note: QHexBuffer will take the ownership of iodevice
    bool open(QIODevice *iodevice, bool readonly);
    // use internal buffer as iodevice
    bool open(bool readonly);

    bool close();

    // save inplace if iodevice == nullptr
    // if iodevice != nullptr, you should keep iodevice un-opened
    // or it will be closed
    bool save(QIODevice *iodevice);

public:
    uchar at(qsizetype idx) const;
    uchar operator[](qsizetype pos) const;

public:
    QIODevice *ioDevice() const;
    bool isOpened() const;
    bool isReadable() const;
    bool isWritable() const;

    QIODevice::OpenMode openMode() const;

    bool isEmpty() const;

public:
    qsizetype length() const;
    QByteArray read(qsizetype offset, qsizetype length) const;
    bool insert(qsizetype offset, const QByteArray &data);
    bool remove(qsizetype offset, qsizetype length);
    bool replace(qsizetype offset, const QByteArray &data);

public:
    qsizetype indexOf(const QByteArray &ba, qsizetype from) const;
    qsizetype lastIndexOf(const QByteArray &ba, qsizetype from) const;

protected:
    bool isReadyRead(qsizetype offset) const;
    bool isReadyReplaceWrite(qsizetype offset, qsizetype length) const;
    bool isReadyInsert(qsizetype offset) const;

private:
    Chunks *_chunks = nullptr;
    QBuffer *m_buffer = nullptr;
};

#endif // QFILEBUFFER_H
