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

#ifndef CRYPTOGRAPHICHASH_H
#define CRYPTOGRAPHICHASH_H

#include <QCryptographicHash>

class CryptographicHash {
    Q_GADGET
public:
    enum class Algorithm {
        Md4,
        Md5,
        Crc16,
        Crc32,
        Crc64,

        Sha1,
        Sha224,
        Sha256,
        Sha384,
        Sha512,

        Keccak_224,
        Keccak_256,
        Keccak_384,
        Keccak_512,
        RealSha3_224,
        RealSha3_256,
        RealSha3_384,
        RealSha3_512,

        Blake2b_160,
        Blake2b_256,
        Blake2b_384,
        Blake2b_512,
        Blake2s_128,
        Blake2s_160,
        Blake2s_224,
        Blake2s_256,
        NumAlgorithms
    };
    Q_ENUM(Algorithm)

public:
    static QByteArray hash(const QByteArray &data,
                           CryptographicHash::Algorithm method);

    static QList<CryptographicHash::Algorithm> supportedHashAlgorithms();

    static QStringList supportedHashAlgorithmStringList();

    static QByteArray hash(QIODevice *device,
                           CryptographicHash::Algorithm method);

private:
    static QCryptographicHash::Algorithm
    toQAlgorithm(CryptographicHash::Algorithm algotithm);
};

#endif // CRYPTOGRAPHICHASH_H
