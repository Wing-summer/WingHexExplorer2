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

#include <cstring>
#include <memory>
#include <utility>

#include <QBuffer>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMimeDatabase>
#include <QTemporaryDir>
#include <QTest>

#include <openssl/evp.h>
#include <openssl/pem.h>

#include "class/file/fileaccesscheck.h"
#include "class/security/wingcert.h"
#include "utilities.h"

namespace {
QByteArray readFixture(const QString &name) {
    QFile file(QDir(QCoreApplication::applicationDirPath()).filePath(name));
    if (!file.open(QIODevice::ReadOnly)) {
        qFatal("Missing fixture");
    }
    return file.readAll();
}

QByteArray signEmpty() {
    const auto pem = readFixture(QStringLiteral("key.pem"));
    const std::unique_ptr<BIO, decltype(&BIO_free)> input(
        BIO_new_mem_buf(pem.constData(), int(pem.size())), BIO_free);
    if (!input) {
        return {};
    }
    const std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> key(
        PEM_read_bio_PrivateKey(input.get(), nullptr, nullptr, nullptr),
        EVP_PKEY_free);
    const std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> context(
        EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!key || !context ||
        EVP_DigestSignInit(context.get(), nullptr, nullptr, nullptr,
                           key.get()) != 1) {
        return {};
    }
    size_t size = 64;
    QByteArray signature(qsizetype(size), Qt::Uninitialized);
    if (EVP_DigestSign(
            context.get(), reinterpret_cast<unsigned char *>(signature.data()),
            &size, reinterpret_cast<const unsigned char *>(""), 0) != 1) {
        return {};
    }
    signature.resize(qsizetype(size));
    return signature;
}

class PartialDevice final : public QIODevice {
public:
    explicit PartialDevice(QByteArray data) : m_data(std::move(data)) {
        open(QIODevice::ReadOnly);
    }

    bool isSequential() const override { return true; }

    bool atEnd() const override { return m_offset == m_data.size(); }

protected:
    qint64 readData(char *target, qint64 maximum) override {
        const auto length =
            qMin(qMin(maximum, qint64(3)), qint64(m_data.size() - m_offset));
        std::memcpy(target, m_data.constData() + m_offset, size_t(length));
        m_offset += length;
        return length;
    }

    qint64 writeData(const char *, qint64) override { return -1; }

private:
    QByteArray m_data;
    qsizetype m_offset = 0;
};
} // namespace

class ScriptIntegrationTest : public QObject {
    Q_OBJECT
private slots:

    void emptyContent() {
        QMimeDatabase database;
        const auto emptyType = database.mimeTypeForData(QByteArray());
        QVERIFY2(Utilities::isTextFile(emptyType),
                 qPrintable(emptyType.name()));
        QTemporaryDir directory;
        for (const auto &name :
             {QStringLiteral("empty.luau"), QStringLiteral("empty.bin"),
              QStringLiteral("noextension")}) {
            const auto path = directory.filePath(name);
            QFile file(path);
            QVERIFY(file.open(QIODevice::WriteOnly));
            file.close();
            QVERIFY2(Utilities::isTextFile(QFileInfo(path)), qPrintable(name));
        }
        QVERIFY(!Utilities::isTextFile(
            QFileInfo(directory.filePath(QStringLiteral("missing.txt")))));
        QVERIFY(!Utilities::isTextFile(QFileInfo(directory.path())));
    }

    void textAndBinary() {
        QTemporaryDir directory;
        QFile file(directory.filePath(QStringLiteral("data.txt")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        const auto text = QStringLiteral("文本与 Unicode 🌍\n").toUtf8();
        QCOMPARE(file.write(text), text.size());
        file.close();
        QVERIFY(Utilities::isTextFile(QFileInfo(file)));
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QCOMPARE(file.write(QByteArray(128, '\0')), 128);
        file.close();
        QVERIFY(!Utilities::isTextFile(QFileInfo(file)));
    }

    void signaturePrimitive() {
        const QSslCertificate certificate(
            readFixture(QStringLiteral("certs/test.crt")));
        const auto payload = readFixture(QStringLiteral("payload.bin"));
        const auto signature = readFixture(QStringLiteral("payload.sig"));
        QVERIFY(WingCert::verifySignature(payload, signature, certificate));
        const auto emptySignature = signEmpty();
        QVERIFY(!emptySignature.isEmpty());
        QVERIFY(WingCert::verifySignature({}, emptySignature, certificate));
        QVERIFY(
            !WingCert::verifySignature(payload + 'x', signature, certificate));
        QVERIFY(!WingCert::verifySignature(payload, signature.left(12),
                                           certificate));
        QVERIFY(
            !WingCert::verifySignature(payload, signature, QSslCertificate{}));
    }

    void signedContent() {
        const QSslCertificate certificate(
            readFixture(QStringLiteral("certs/test.crt")));
        auto &certificates = WingCert::instance();
        if (!certificates.isTrustedCertificate(certificate)) {
            QSKIP("Certificate fixture permissions do not satisfy this "
                  "platform's trust policy");
        }
        const auto fingerprint = WingCert::fingerPrint(certificate);
        const auto payload = readFixture(QStringLiteral("payload.bin"));
        const auto signature = readFixture(QStringLiteral("payload.sig"));
        QTemporaryDir directory;
        QFile file(directory.filePath(QStringLiteral("content.bin")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(payload), payload.size());
        file.close();
        QFile detached(WingCert::signatureFilePath(QFileInfo(file)));
        QVERIFY(detached.open(QIODevice::WriteOnly));
        QCOMPARE(detached.write(signature), signature.size());
        detached.close();
        QVERIFY(certificates.verify(QFileInfo(file), fingerprint));
        QBuffer snapshot;
        snapshot.setData(payload);
        QVERIFY(snapshot.open(QIODevice::ReadOnly));
        QVERIFY(
            certificates.verify(snapshot, fingerprint, detached.fileName()));
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        file.write("modified after snapshot");
        file.close();
        QVERIFY(!certificates.verify(QFileInfo(file), fingerprint));
        QVERIFY(snapshot.seek(0));
        QVERIFY(
            certificates.verify(snapshot, fingerprint, detached.fileName()));
        const auto embedded = payload + QByteArray("WINGSIG1") + signature;
        PartialDevice partial(embedded);
        QVERIFY(certificates.verify(partial, fingerprint));
        PartialDevice empty(QByteArray("WINGSIG1") + signEmpty());
        QVERIFY(certificates.verify(empty, fingerprint));
        QBuffer closed;
        QVERIFY(!certificates.verify(closed, fingerprint));
        QVERIFY(snapshot.seek(0));
        QVERIFY(!certificates.verify(
            snapshot, QByteArray("unknown fingerprint"), detached.fileName()));
        snapshot.close();
        snapshot.setData(embedded.left(embedded.size() - 3));
        snapshot.open(QIODevice::ReadOnly);
        QVERIFY(!certificates.verify(snapshot, fingerprint));
    }
};
QTEST_GUILESS_MAIN(ScriptIntegrationTest)
#include "tst_scriptintegration.moc"
