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

#ifndef CRCALGORITHM_H
#define CRCALGORITHM_H

#include <QByteArray>
#include <QIODevice>

class CrcAlgorithm {
    // crc16
public:
    static QByteArray hash16(const QByteArray &data);
    static QByteArray hash16(QIODevice *device);

    static quint16 __hash16(const QByteArray &data);
    static quint16 __hash16(QIODevice *device);

private:
    static quint16 __hash16(QDataStream &datastream);
    static QByteArray hash16(QDataStream &datastream);

    // crc32
public:
    static QByteArray hash(const QByteArray &data);
    static QByteArray hash(QIODevice *device);

    static quint32 __hash(const QByteArray &data);
    static quint32 __hash(QIODevice *device);

private:
    static quint32 __hash(QDataStream &datastream);
    static QByteArray hash(QDataStream &datastream);

    // crc64
public:
    static QByteArray hash64(const QByteArray &data);
    static QByteArray hash64(QIODevice *device);

    static quint64 __hash64(const QByteArray &data);
    static quint64 __hash64(QIODevice *device);

private:
    static quint64 __hash64(QDataStream &datastream);
    static QByteArray hash64(QDataStream &datastream);

private:
    static quint16 crc16table[16];
    static quint32 crc32table[256];
    static quint64 crc64table[256];
};

#endif // CRCALGORITHM_H
