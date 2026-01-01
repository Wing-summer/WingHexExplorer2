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

#include "CRaC/include/crac.h"

#include <QFile>
#include <QMetaEnum>
#include <QtEndian>

template <typename T, typename = std::enable_if<std::is_arithmetic_v<T>>>
static inline QByteArray getDisplayArray(const T &data) {
    auto r = qToBigEndian(data);
    return QByteArray(reinterpret_cast<const char *>(&r), sizeof(T));
}

QByteArray CryptographicHash::hash(const QByteArray &data, Algorithm method) {
    switch (method) {
    case Algorithm::CRC8: {
        auto input = reinterpret_cast<const uint8_t *>(data.constData());
        auto ret = crac::CRC8::calc(input, data.size());
        return getDisplayArray(ret);
    }
    case Algorithm::CRC8_MAXIM: {
        auto input = reinterpret_cast<const uint8_t *>(data.constData());
        auto ret = crac::CRC8_MAXIM::calc(input, data.size());
        return getDisplayArray(ret);
    }
    case Algorithm::CRC16: {
        auto input = reinterpret_cast<const uint8_t *>(data.constData());
        auto ret = crac::CRC16::calc(input, data.size());
        return getDisplayArray(ret);
    }
    case Algorithm::CRC16_CCITT_FALSE: {
        auto input = reinterpret_cast<const uint8_t *>(data.constData());
        auto ret = crac::CRC16_CCITT_FALSE::calc(input, data.size());
        return getDisplayArray(ret);
    }
    case Algorithm::CRC16_MODBUS: {
        auto input = reinterpret_cast<const uint8_t *>(data.constData());
        auto ret = crac::CRC16_MODBUS::calc(input, data.size());
        return getDisplayArray(ret);
    }
    case Algorithm::CRC32: {
        auto input = reinterpret_cast<const uint8_t *>(data.constData());
        auto ret = crac::CRC32::calc(input, data.size());
        return getDisplayArray(ret);
    }
    case Algorithm::CRC32_C: {
        auto input = reinterpret_cast<const uint8_t *>(data.constData());
        auto ret = crac::CRC32_C::calc(input, data.size());
        return getDisplayArray(ret);
    }
    case Algorithm::CRC64: {
        auto input = reinterpret_cast<const uint8_t *>(data.constData());
        auto ret = crac::CRC64::calc(input, data.size());
        return getDisplayArray(ret);
    }
    case Algorithm::NumAlgorithms:
        return {};
    default:
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
            switch (e) {
            case Algorithm::CRC8:
            case Algorithm::CRC8_MAXIM:
            case Algorithm::CRC16:
            case Algorithm::CRC16_CCITT_FALSE:
            case Algorithm::CRC16_MODBUS:
            case Algorithm::CRC32:
            case Algorithm::CRC32_C:
            case Algorithm::CRC64:
                algorithms << e;
                break;
            default:
                QCryptographicHash hash(QCryptographicHash::Md5);
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

template <typename T>
QByteArray block_cacl_ba(QIODevice *device) {
    if (device->reset()) {
        auto block = T::get_block_eng();
        char buffer[1024];
        int length;

        while ((length = device->read(buffer, sizeof(buffer))) > 0) {
            block.update(reinterpret_cast<const uint8_t *>(buffer), length);
        }

        return getDisplayArray(block.final());
    }
    return {};
}

QByteArray CryptographicHash::hash(QIODevice *device, Algorithm method) {
    if (device == nullptr) {
        return {};
    }

    switch (method) {
    case Algorithm::CRC8:
        return block_cacl_ba<crac::CRC8>(device);
    case Algorithm::CRC8_MAXIM:
        return block_cacl_ba<crac::CRC8_MAXIM>(device);
    case Algorithm::CRC16:
        return block_cacl_ba<crac::CRC16>(device);
    case Algorithm::CRC16_CCITT_FALSE:
        return block_cacl_ba<crac::CRC16_CCITT_FALSE>(device);
    case Algorithm::CRC16_MODBUS:
        return block_cacl_ba<crac::CRC16_MODBUS>(device);
    case Algorithm::CRC32:
        return block_cacl_ba<crac::CRC32>(device);
    case Algorithm::CRC32_C:
        return block_cacl_ba<crac::CRC32_C>(device);
    case Algorithm::CRC64:
        return block_cacl_ba<crac::CRC64>(device);
    default: {
        auto r = toQAlgorithm(method);
        if (r == QCryptographicHash::NumAlgorithms) {
            return {};
        }

        QCryptographicHash hash(r);
        if (device->reset() && hash.addData(device)) {
            return hash.result();
        }
    }
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
