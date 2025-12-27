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

#include "cryptographichash.h"
#include "crcalgorithm.h"

#include <QFile>
#include <QMetaEnum>
#include <QtEndian>

QByteArray CryptographicHash::hash(const QByteArray &data, Algorithm method) {
    if (method == Algorithm::Crc16) {
        return CrcAlgorithm::hash16(data);
    } else if (method == Algorithm::Crc32) {
        return CrcAlgorithm::hash(data);
    } else if (method == Algorithm::Crc64) {
        return CrcAlgorithm::hash64(data);
    } else {
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(data);
        return hash.result();
    }
}

QList<CryptographicHash::Algorithm>
CryptographicHash::supportedHashAlgorithms() {
    static QList<CryptographicHash::Algorithm> algorithms;
    if (algorithms.isEmpty()) {
        auto hashe = QMetaEnum::fromType<CryptographicHash::Algorithm>();
        for (int i = 0; i < hashe.keyCount(); ++i) {
            auto e = CryptographicHash::Algorithm(hashe.value(i));
            if (e == CryptographicHash::Algorithm::Crc16 ||
                e == CryptographicHash::Algorithm::Crc32 ||
                e == CryptographicHash::Algorithm::Crc64) {
                algorithms << e;
            } else {
                auto r = toQAlgorithm(e);
                if (r != QCryptographicHash::NumAlgorithms) {
                    if (QCryptographicHash::hashLength(r)) {
                        algorithms << e;
                    }
                }
            }
        }
    }
    return algorithms;
}

QStringList CryptographicHash::supportedHashAlgorithmStringList() {
    static QStringList hashNames;
    if (hashNames.isEmpty()) {
        auto hashes = supportedHashAlgorithms();
        auto hashe = QMetaEnum::fromType<CryptographicHash::Algorithm>();
        for (auto &hash : hashes) {
            hashNames << QString::fromLatin1(hashe.valueToKey(int(hash)));
        }
    }
    return hashNames;
}

QByteArray CryptographicHash::hash(QIODevice *device, Algorithm method) {
    if (method == Algorithm::Crc16) {
        if (device->seek(0)) {
            return CrcAlgorithm::hash16(device);
        } else {
            return {};
        }
    } else if (method == Algorithm::Crc32) {
        if (device->seek(0)) {
            return CrcAlgorithm::hash(device);
        } else {
            return {};
        }
    } else if (method == Algorithm::Crc64) {
        if (device->seek(0)) {
            return CrcAlgorithm::hash64(device);
        } else {
            return {};
        }
    }

    auto r = toQAlgorithm(method);
    if (r == QCryptographicHash::NumAlgorithms) {
        return {};
    }

    QCryptographicHash hash(r);
    if (device->seek(0) && hash.addData(device)) {
        return hash.result();
    }

    return {};
}

QCryptographicHash::Algorithm
CryptographicHash::toQAlgorithm(Algorithm algotithm) {
    switch (algotithm) {
    case Algorithm::Md4:
        return QCryptographicHash::Algorithm::Md4;
    case Algorithm::Md5:
        return QCryptographicHash::Algorithm::Md5;
    case Algorithm::Sha1:
        return QCryptographicHash::Algorithm::Sha1;
    case Algorithm::Sha224:
        return QCryptographicHash::Algorithm::Sha224;
    case Algorithm::Sha256:
        return QCryptographicHash::Algorithm::Sha256;
    case Algorithm::Sha384:
        return QCryptographicHash::Algorithm::Sha384;
    case Algorithm::Sha512:
        return QCryptographicHash::Algorithm::Sha512;
    case Algorithm::Keccak_224:
        return QCryptographicHash::Algorithm::Keccak_224;
    case Algorithm::Keccak_256:
        return QCryptographicHash::Algorithm::Keccak_256;
    case Algorithm::Keccak_384:
        return QCryptographicHash::Algorithm::Keccak_384;
    case Algorithm::Keccak_512:
        return QCryptographicHash::Algorithm::Keccak_512;
    case Algorithm::RealSha3_224:
        return QCryptographicHash::Algorithm::RealSha3_224;
    case Algorithm::RealSha3_256:
        return QCryptographicHash::Algorithm::RealSha3_256;
    case Algorithm::RealSha3_384:
        return QCryptographicHash::Algorithm::RealSha3_384;
    case Algorithm::RealSha3_512:
        return QCryptographicHash::Algorithm::RealSha3_512;
    case Algorithm::Blake2b_160:
        return QCryptographicHash::Algorithm::Blake2b_160;
    case Algorithm::Blake2b_256:
        return QCryptographicHash::Algorithm::Blake2b_256;
    case Algorithm::Blake2b_384:
        return QCryptographicHash::Algorithm::Blake2b_384;
    case Algorithm::Blake2b_512:
        return QCryptographicHash::Algorithm::Blake2b_512;
    case Algorithm::Blake2s_128:
        return QCryptographicHash::Algorithm::Blake2s_128;
    case Algorithm::Blake2s_160:
        return QCryptographicHash::Algorithm::Blake2s_160;
    case Algorithm::Blake2s_224:
        return QCryptographicHash::Algorithm::Blake2s_224;
    case Algorithm::Blake2s_256:
        return QCryptographicHash::Algorithm::Blake2s_256;
    default:
        return QCryptographicHash::Algorithm::NumAlgorithms;
    }
}
