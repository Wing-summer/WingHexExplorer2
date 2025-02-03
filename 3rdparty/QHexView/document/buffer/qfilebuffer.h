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

#include "qhexbuffer.h"
#include <QFile>

class Chunks;

class QFileBuffer : public QHexBuffer {
    Q_OBJECT

public:
    explicit QFileBuffer(QObject *parent = nullptr);
    ~QFileBuffer() override;
    uchar at(qsizetype idx) override;
    qsizetype length() const override;
    void insert(qsizetype offset, const QByteArray &data) override;
    void remove(qsizetype offset, qsizetype length) override;
    QByteArray read(qsizetype offset, qsizetype length) override;
    bool read(QIODevice *device) override;
    void write(QIODevice *device) override;

    qsizetype indexOf(const QByteArray &ba, qsizetype from) override;
    qsizetype lastIndexOf(const QByteArray &ba, qsizetype from) override;

private:
    Chunks *_chunks = nullptr;
    uchar *m_memory = nullptr;
};

#endif // QFILEBUFFER_H
