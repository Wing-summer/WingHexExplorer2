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

#ifndef QFILEREGIONBUFFER_H
#define QFILEREGIONBUFFER_H

#include "qhexbuffer.h"

class QFileRegionBuffer : public QHexBuffer {
    Q_OBJECT
public:
    explicit QFileRegionBuffer(QObject *parent = nullptr);

public:
    qsizetype length() const override;
    void insert(qsizetype offset, const QByteArray &data) override;
    void remove(qsizetype offset, qsizetype length) override;
    QByteArray read(qsizetype offset, qsizetype length) override;
    bool read(QIODevice *iodevice) override;
    void write(QIODevice *iodevice) override;

    qsizetype indexOf(const QByteArray &ba, qsizetype from) override;
    qsizetype lastIndexOf(const QByteArray &ba, qsizetype from) override;

    void setReadOffset(qsizetype offset);
    qsizetype readOffset();

    qsizetype readMaxBytes();
    void setReadMaxBytes(qsizetype maxlength);

private:
    qsizetype offset = 0;
    qsizetype maxlength = 1024;
    QByteArray buffer;
};

#endif // QFILEREGIONBUFFER_H
