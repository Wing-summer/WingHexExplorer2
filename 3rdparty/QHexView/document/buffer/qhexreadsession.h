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

#ifndef QHEXREADSESSION_H
#define QHEXREADSESSION_H

#include <QByteArray>
#include <QDateTime>
#include <QIODevice>
#include <QLoggingCategory>
#include <QString>

#include <memory>
#include <vector>

Q_DECLARE_LOGGING_CATEGORY(lcQHexRead)

enum class QHexReadError {
    None,
    InvalidDevice,
    OwnerThreadRequired,
    BudgetExceeded,
    SourceChanged,
    OpenFailed,
    InvalidMapping
};

class QHexReadSession;

struct QHexReadSessionResult {
    std::shared_ptr<const QHexReadSession> session;
    QHexReadError error = QHexReadError::None;
};

struct QHexReadDeviceResult {
    std::unique_ptr<QIODevice> device;
    QHexReadError error = QHexReadError::None;
};

// Immutable recipe. Construct/open each device on its consuming thread.
// No QObject or seek position is shared with the editor.
class QHexReadSession : public std::enable_shared_from_this<QHexReadSession> {
public:
    struct Piece {
        qint64 offset = 0;
        qint64 length = 0;
        qint64 sourceOffset = 0;
        QByteArray bytes;
        bool edited = false;
    };
    qint64 length = 0;
    QString fileName;
    qint64 sourceSize = 0;
    QDateTime modified;
    QByteArray memory;
    std::vector<Piece> pieces;

public:
    quint64 mappingBytes() const;
    bool sourceUnchanged() const;
    QHexReadDeviceResult createDevice() const;
};

#endif
