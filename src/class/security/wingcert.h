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

#ifndef WINGCERT_H
#define WINGCERT_H

#include <QFileInfo>
#include <QHash>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslError>

class WingCert {
public:
    enum class ConfigurationError {
        None,
        BackendUnavailable,
        FileError,
        InvalidCredentials
    };

    struct ServerConfigurationResult {
        ConfigurationError error = ConfigurationError::None;
        QSslConfiguration configuration;
        QString message;
    };

public:
    static WingCert &instance();

    QHash<QSslCertificate, QFileInfo> certificates() const;

    bool isValidFingerPrint(const QSslCertificate &cert) const;

    bool isTrustedCertificate(const QSslCertificate &certificate) const;
    QList<QSslCertificate> trustedCertificates() const;
    bool verifyPeer(const QSslCertificate &certificate,
                    const QList<QSslError> &errors) const;

    ServerConfigurationResult
    serverConfiguration(const QString &certificateFile,
                        const QString &keyFile) const;

    bool verify(const QFileInfo &plg, const QByteArray &fprint) const;

    QFileInfo certLocation(const QByteArray &fprint);
    QFileInfo certLocation(const QSslCertificate &cert);
    QSslCertificate cert(const QByteArray &fprint);

    bool isSysCert(const QSslCertificate &cert) const;

    static QByteArray fingerPrint(const QSslCertificate &cert);

private:
    static bool isValidCert(const QSslCertificate &cert);

private:
    bool verify(const QByteArray &file, const QByteArray &sig,
                const QSslCertificate &cert) const;

private:
    WingCert();
    Q_DISABLE_COPY_MOVE(WingCert)

    QSslCertificate m_sysCert;
    QHash<QSslCertificate, QFileInfo> m_certs;
    QHash<QByteArray, QSslCertificate> m_cs;
};

#endif // WINGCERT_H
