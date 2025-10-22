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

#ifndef QMEMORYBUFFER_H
#define QMEMORYBUFFER_H

#include "qhexbuffer.h"

#include <QBuffer>

class QMemoryBuffer : public QHexBuffer {
    Q_OBJECT

public:
    explicit QMemoryBuffer(QObject *parent = nullptr);

private:
    QByteArray m_buffer;

    // QHexBuffer interface
public:
    // reimplement for faster operation
    virtual uchar at(qsizetype idx) const override;

    virtual bool open(QIODevice *iodevice, bool readonly) override;
    virtual bool close() override;
    virtual bool save(QIODevice *iodevice) override;
    virtual qsizetype length() const override;
    virtual QByteArray read(qsizetype offset, qsizetype length) const override;
    virtual bool insert(qsizetype offset, const QByteArray &data) override;
    virtual bool remove(qsizetype offset, qsizetype length) override;
    virtual bool replace(qsizetype offset, const QByteArray &data) override;
    virtual qsizetype indexOf(const QByteArray &ba,
                              qsizetype from) const override;
    virtual qsizetype lastIndexOf(const QByteArray &ba,
                                  qsizetype from) const override;
};

#endif // QMEMORYBUFFER_H
