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

#ifndef QHEXBUFFER_H
#define QHEXBUFFER_H

#include <QBuffer>
#include <QIODevice>
#include <QObject>

class QHexBuffer : public QObject {
    Q_OBJECT

public:
    explicit QHexBuffer(QObject *parent = nullptr);
    virtual ~QHexBuffer();

public:
    // note: QHexBuffer will take the ownership of iodevice
    virtual bool open(QIODevice *iodevice, bool readonly);
    // use internal buffer as iodevice
    bool open(bool readonly);

    virtual bool close();

    // save inplace if iodevice == nullptr
    // if iodevice != nullptr, you should keep iodevice un-opened
    // or it will be closed
    virtual bool save(QIODevice *iodevice) = 0;

public:
    virtual uchar at(qsizetype idx) const;
    uchar operator[](qsizetype pos) const;

public:
    QIODevice *ioDevice() const;
    bool isOpened() const;
    bool isReadable() const;
    bool isWritable() const;
    QIODevice::OpenMode openMode() const;

    bool isEmpty() const;

public:
    virtual qsizetype length() const = 0;
    virtual QByteArray read(qsizetype offset, qsizetype length) const = 0;
    virtual bool insert(qsizetype offset, const QByteArray &data) = 0;
    virtual bool remove(qsizetype offset, qsizetype length) = 0;
    virtual bool replace(qsizetype offset, const QByteArray &data) = 0;

public:
    virtual qsizetype indexOf(const QByteArray &ba, qsizetype from) const = 0;
    virtual qsizetype lastIndexOf(const QByteArray &ba,
                                  qsizetype from) const = 0;

    QBuffer *internalBuffer() const;

protected:
    bool isReadyRead(qsizetype offset) const;
    bool isReadyReplaceWrite(qsizetype offset, qsizetype length) const;
    bool isReadyInsert(qsizetype offset) const;

private:
    QIODevice *_ioDevice = nullptr;
    QBuffer *m_buffer = nullptr;
};

#endif // QHEXBUFFER_H
