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

#ifndef WINGPLUGINCERT_H
#define WINGPLUGINCERT_H

#include <QFileInfo>

#include <QHash>
#include <QSslCertificate>

class WingPluginCert {
public:
    static WingPluginCert &instance();

    QHash<QSslCertificate, QFileInfo> certificates() const;

    bool isValidFingerPrint(const QSslCertificate &cert) const;

    bool verify(const QFileInfo &plg, const QString &sigFileName,
                const QByteArray &fprint);

    QFileInfo certLocation(const QByteArray &fprint);
    QFileInfo certLocation(const QSslCertificate &cert);
    QSslCertificate cert(const QByteArray &fprint);
    static QByteArray fingerPrint(const QSslCertificate &cert);

private:
    WingPluginCert();
    Q_DISABLE_COPY_MOVE(WingPluginCert)

    QHash<QSslCertificate, QFileInfo> _certs;
    QHash<QByteArray, QSslCertificate> _cs;
};

#endif // WINGPLUGINCERT_H
