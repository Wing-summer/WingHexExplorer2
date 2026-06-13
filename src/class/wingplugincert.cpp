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
constexpr qint64 MaxPluginSize = 500LL * 1024 * 1024; // 500 MB
constexpr qint64 MaxCertSize = 64 * 1024;             // 64 KB
constexpr qint64 ExpectedSigSize = 64;                // Ed25519 fixed size
constexpr char Magic[8]{'W', 'I', 'N', 'G', 'S', 'I', 'G', '1'};
constexpr qint64 MagicSize = sizeof(Magic);
constexpr qint64 InternalSigSize = MagicSize + ExpectedSigSize;

EVP_PKEY *loadPkeyFromCertificate(const QSslCertificate &cert) {
    const QByteArray pem = cert.toPem();
    if (pem.isEmpty())
        return nullptr;

    BIO *bio = BIO_new_mem_buf(pem.constData(), static_cast<int>(pem.size()));
    if (!bio)
        return nullptr;

    X509 *x509 = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (!x509)
        return nullptr;

    EVP_PKEY *pkey = X509_get_pubkey(x509);
    X509_free(x509);

    if (!pkey)
        return nullptr;

    if (EVP_PKEY_id(pkey) != EVP_PKEY_ED25519) {
        EVP_PKEY_free(pkey);
        return nullptr;
    }

    return pkey;
}

} // namespace

WingPluginCert &WingPluginCert::instance() {
    static WingPluginCert ins;
    return ins;
}

bool WingPluginCert::verify(const QFileInfo &plg,
                            const QByteArray &fprint) const {
    if (!_cs.contains(fprint)) {
        return false;
    }
    if (plg.size() <= 0 || plg.size() > MaxPluginSize) {
        return false;
    }

    QFile fplg(plg.absoluteFilePath());
    if (!fplg.open(QIODevice::ReadOnly)) {
        return false;
    }
    auto data = fplg.readAll();

    QByteArray sig;
    if (data.size() > InternalSigSize) {
        auto maySig = data.last(InternalSigSize);
        if (QByteArrayView(maySig).first(MagicSize) ==
            QByteArrayView::fromArray(Magic)) {
            data.chop(InternalSigSize);
            sig = maySig.sliced(MagicSize);
        }
    }

    if (sig.isEmpty()) {
        const auto sigFileName = plg.absoluteDir().absoluteFilePath(
            plg.baseName() + QStringLiteral(".sig"));
        QFileInfo sigInfo(sigFileName);
        if (!sigInfo.exists() || !sigInfo.isFile() || !sigInfo.isReadable()) {
            return false;
        }
        if (sigInfo.size() != ExpectedSigSize) {
            return false;
        }
        QFile fsig(sigFileName);
        if (fsig.open(QIODevice::ReadOnly)) {
            sig = fsig.readAll();
            fsig.close();
        }
    }

    auto c = _cs.value(fprint);
    return verify(data, sig, c);
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

bool WingPluginCert::verify(const QByteArray &file, const QByteArray &sig,
                            const QSslCertificate &cert) const {
    if (!isValidCert(cert)) {
        return false;
    }

    EVP_PKEY *pkey = loadPkeyFromCertificate(cert);
    if (!pkey) {
        return false;
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) {
        EVP_PKEY_free(pkey);
        return false;
    }

    bool ok = false;
    do {
        if (EVP_DigestVerifyInit(ctx, nullptr, nullptr, nullptr, pkey) != 1)
            break;

        ok = (EVP_DigestVerify(
                  ctx, reinterpret_cast<const unsigned char *>(sig.constData()),
                  static_cast<size_t>(sig.size()),
                  reinterpret_cast<const unsigned char *>(file.constData()),
                  static_cast<size_t>(file.size())) == 1);
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
