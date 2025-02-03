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

#include <QIODevice>
#include <QObject>

class QHexBuffer : public QObject {
    Q_OBJECT

public:
    explicit QHexBuffer(QObject *parent = nullptr);
    bool isEmpty() const;

public:
    virtual uchar at(qsizetype idx);
    virtual void replace(qsizetype offset, const QByteArray &data);
    virtual void read(char *data, qsizetype size);
    virtual void read(const QByteArray &ba);

public:
    virtual qsizetype length() const = 0;
    virtual void insert(qsizetype offset, const QByteArray &data) = 0;
    virtual void remove(qsizetype offset, qsizetype length) = 0;
    virtual QByteArray read(qsizetype offset, qsizetype length) = 0;
    virtual bool read(QIODevice *iodevice) = 0;
    virtual void write(QIODevice *iodevice) = 0;

    virtual qsizetype indexOf(const QByteArray &ba, qsizetype from) = 0;
    virtual qsizetype lastIndexOf(const QByteArray &ba, qsizetype from) = 0;
};

#endif // QHEXBUFFER_H
