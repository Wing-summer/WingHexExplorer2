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

#include "qhexreadsession.h"

#include <algorithm>
#include <cstring>

#include <QFile>
#include <QFileInfo>

Q_LOGGING_CATEGORY(lcQHexRead, "wing.qhexview.read")

namespace {
class ReadDevice final : public QIODevice {
public:
    explicit ReadDevice(std::shared_ptr<const QHexReadSession> session)
        : m_session(std::move(session)) {}

public:
    QHexReadError start() {
        if (!m_session->sourceUnchanged()) {
            setErrorString(QStringLiteral("The source file changed."));
            return QHexReadError::SourceChanged;
        }
        bool isMem = m_session->fileName.isEmpty();
        if (!isMem) {
            m_file.setFileName(m_session->fileName);
            if (!m_file.open(QIODevice::ReadOnly)) {
                setErrorString(m_file.errorString());
                return QHexReadError::OpenFailed;
            }
        }
        setObjectName(isMem ? QStringLiteral("memory") : m_session->fileName);
        return QIODevice::open(QIODevice::ReadOnly | QIODevice::Unbuffered)
                   ? QHexReadError::None
                   : QHexReadError::OpenFailed;
    }

    qint64 size() const override { return m_session->length; }

    bool isSequential() const override { return false; }

    bool seek(qint64 offset) override {
        if (offset < 0 || offset > size()) {
            return false;
        }
        return QIODevice::seek(offset);
    }

protected:
    virtual qint64 writeData(const char *, qint64) override { return -1; }

    virtual qint64 readData(char *out, qint64 maximum) override {
        auto offset = pos();
        const auto wanted = std::min(maximum, size() - offset);
        qint64 done = 0;
        auto it = std::upper_bound(
            m_session->pieces.begin(), m_session->pieces.end(), offset,
            [](qint64 p, const QHexReadSession::Piece &s) {
                return p < s.offset;
            });
        if (it != m_session->pieces.begin()) {
            --it;
        }
        while (done < wanted && it != m_session->pieces.end()) {
            const auto within = offset - it->offset;
            const auto count = std::min(wanted - done, it->length - within);
            if (within < 0 || count <= 0) {
                setErrorString(QStringLiteral("Invalid document mapping."));
                return -1;
            }
            if (it->edited) {
                std::memcpy(out + done, it->bytes.constData() + within, count);
            } else if (m_file.isOpen()) {
                if (!m_file.seek(it->sourceOffset + within)) {
                    setErrorString(m_file.errorString());
                    return -1;
                }
                qint64 read = 0;
                while (read < count) {
                    const auto n = m_file.read(out + done + read, count - read);
                    if (n <= 0) {
                        setErrorString(QStringLiteral(
                            "Could not read the complete source range."));
                        return -1;
                    }
                    read += n;
                }
            } else {
                std::memcpy(out + done,
                            m_session->memory.constData() + it->sourceOffset +
                                within,
                            count);
            }
            done += count;
            offset += count;
            ++it;
        }
        if (done != wanted) {
            setErrorString(QStringLiteral("Incomplete document mapping."));
            return -1;
        }
        return done;
    }

private:
    std::shared_ptr<const QHexReadSession> m_session;
    QFile m_file;
};
} // namespace

quint64 QHexReadSession::mappingBytes() const {
    return pieces.capacity() * sizeof(Piece);
}

bool QHexReadSession::sourceUnchanged() const {
    if (fileName.isEmpty()) {
        return true;
    }
    const QFileInfo info(fileName);
    return info.exists() && info.size() == sourceSize &&
           info.lastModified() == modified;
}

QHexReadDeviceResult QHexReadSession::createDevice() const {
    auto device = std::make_unique<ReadDevice>(shared_from_this());
    const auto error = device->start();
    if (error != QHexReadError::None) {
        qCWarning(lcQHexRead)
            << "Cannot open read session" << fileName << device->errorString();
        return {{}, error};
    }
    return {std::move(device), QHexReadError::None};
}
