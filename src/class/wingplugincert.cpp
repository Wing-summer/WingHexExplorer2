/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include "wingplugincert.h"
#include "class/fileaccesscheck.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSslKey>
#include <QString>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

namespace {
constexpr qint64 MaxCertSize = 64 * 1024; // 64 KB
constexpr qint64 MaxSigSize = 16 * 1024;  // 16 KB (Base64 text)
constexpr qint64 ChunkSize = 64 * 1024;   // 64 KB
} // namespace

WingPluginCert &WingPluginCert::instance() {
    static WingPluginCert ins;
    return ins;
}

bool WingPluginCert::verify(const QFileInfo &plg, const QString &sigFileName,
                            const QByteArray &fprint) const {
    if (!_cs.contains(fprint)) {
        return false;
    }
    QFileInfo sigInfo(sigFileName);
    if (!sigInfo.exists() || !sigInfo.isFile() || !sigInfo.isReadable()) {
        return false;
    }
    if (sigInfo.size() <= 0 || sigInfo.size() > MaxSigSize) {
        return false;
    }

    QFile fsig(sigFileName);
    if (!fsig.open(QIODevice::ReadOnly)) {
        return false;
    }

    auto sig = fsig.readAll();
    auto c = _cs.value(fprint);
    return verify(plg.absoluteFilePath(), sig, c);
}

QFileInfo WingPluginCert::certLocation(const QByteArray &fprint) {
    auto c = cert(fprint);
    if (c.isNull()) {
        return {};
    }
    return certLocation(c);
}

QFileInfo WingPluginCert::certLocation(const QSslCertificate &cert) {
    return _certs.value(cert);
}

QSslCertificate WingPluginCert::cert(const QByteArray &fprint) {
    return _cs.value(fprint);
}

bool WingPluginCert::isSysCert(const QSslCertificate &cert) const {
    if (_sysCert.isNull()) {
        return false;
    }
    return _sysCert == cert;
}

QHash<QSslCertificate, QFileInfo> WingPluginCert::certificates() const {
    return _certs;
}

bool WingPluginCert::isValidFingerPrint(const QSslCertificate &cert) const {
    return _cs.contains(fingerPrint(cert));
}

QByteArray WingPluginCert::fingerPrint(const QSslCertificate &cert) {
    return cert.digest(QCryptographicHash::Sha256);
}

bool WingPluginCert::isValidCert(const QSslCertificate &cert) {
    // valid crt file but it's invalid
    if (cert.isNull() || cert.isBlacklisted()) {
        return false;
    }

    auto date = QDateTime::currentDateTime();
    if (date < cert.effectiveDate() || date > cert.expiryDate()) {
        return false;
    }
    return true;
}

bool WingPluginCert::verify(const QString &fileName, const QByteArray &sig,
                            const QSslCertificate &cert) const {
    if (!isValidCert(cert)) {
        return false;
    }

    auto key = cert.publicKey();
    const auto pem = key.toPem();
    if (pem.isEmpty()) {
        return false;
    }
    BIO *bio = BIO_new_mem_buf(pem.constData(), pem.size());
    if (!bio) {
        return false;
    }
    EVP_PKEY *pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    if (!pkey) {
        return false;
    }

    if (sig.isEmpty()) {
        EVP_PKEY_free(pkey);
        return false;
    }
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly)) {
        EVP_PKEY_free(pkey);
        return false;
    }
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) {
        EVP_PKEY_free(pkey);
        return false;
    }
    bool ok = false;
    char buffer[ChunkSize];
    do {
        if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) !=
            1) {
            EVP_MD_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            return false;
        }

        EVP_PKEY_CTX *pctx = EVP_MD_CTX_get_pkey_ctx(ctx);
        if (!pctx) {
            EVP_MD_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            return false;
        }

        if (EVP_PKEY_CTX_set_rsa_padding(pctx, RSA_PKCS1_PSS_PADDING) != 1) {
            EVP_MD_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            return false;
        }

        if (EVP_PKEY_CTX_set_rsa_pss_saltlen(pctx, RSA_PSS_SALTLEN_AUTO) != 1) {
            EVP_MD_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            return false;
        }

        while (true) {
            const qint64 len = f.read(buffer, ChunkSize);
            if (len < 0) {
                ok = false;
                goto done_verify;
            }
            if (len == 0)
                break;
            if (EVP_DigestVerifyUpdate(ctx, buffer, static_cast<size_t>(len)) !=
                1) {
                ok = false;
                goto done_verify;
            }
        }
        ok = (EVP_DigestVerifyFinal(
                  ctx, reinterpret_cast<const unsigned char *>(sig.constData()),
                  static_cast<size_t>(sig.size())) == 1);
    done_verify:;
    } while (false);
    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return ok;
}

WingPluginCert::WingPluginCert() {
    // load internal cert
    QFile f(QStringLiteral(":/com.wingsummer.winghex/src/WingCloudStudio.crt"));
    if (f.open(QFile::ReadOnly)) {
        QSslCertificate cert(&f, QSsl::Pem);
        if (!cert.isNull()) {
            if (isValidCert(cert)) {
                _certs.insert(cert, {});
                _sysCert = cert;
                _cs.insert(fingerPrint(cert), cert);
            }
        }
    }

    // load user cert
    QDir data(QCoreApplication::applicationDirPath());
    auto certdir = QStringLiteral("certs");
    if (!data.exists(certdir)) {
        return;
    }

    if (!data.cd(certdir)) {
        return;
    }

    const auto certs =
        data.entryInfoList({"*.crt"}, QDir::Files | QDir::NoSymLinks);
    for (const auto &c : certs) {
        if (c.isHidden()) {
            continue;
        }

        QFile f(c.absoluteFilePath());
        if (f.size() > MaxCertSize) {
            continue;
        }
        if (!f.open(QIODevice::ReadOnly)) {
            continue;
        }
        QSslCertificate cert(&f, QSsl::Pem);
        if (cert.isNull() || _certs.contains(cert)) {
            continue;
        }

        if (FileAccessCheck::canStandardUserWriteFile(c)) {
            // untrusted cert, maybe modified
            _certs.insert(cert, c);
            continue;
        }

        if (!isValidCert(cert)) {
            _certs.insert(cert, c);
            continue;
        }

        _certs.insert(cert, c);
        _cs.insert(fingerPrint(cert), cert);
    }
}
