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

#include "diffsource.h"

#include <algorithm>
#include <cstring>

#include <QCoreApplication>
#include <QFile>
#include <QMutex>
#include <QWaitCondition>

#include "binarydiff.h"

namespace WingDiff {
namespace {
struct ReadRequest {
    QMutex mutex;
    QWaitCondition ready;
    QByteArray bytes;
    bool done = false;
};

class OwnerDevice final : public QIODevice {
public:
    OwnerDevice(QPointer<QHexDocument> document, qint64 length,
                const QString &description,
                std::shared_ptr<std::atomic_bool> cancel)
        : m_document(document), m_length(length), m_cancel(std::move(cancel)) {
        setObjectName(description);
        QIODevice::open(QIODevice::ReadOnly | QIODevice::Unbuffered);
    }

    qint64 size() const override { return m_length; }

    bool isSequential() const override { return false; }

    bool seek(qint64 offset) override {
        return offset >= 0 && offset <= m_length && QIODevice::seek(offset);
    }

protected:
    qint64 writeData(const char *, qint64) override { return -1; }

    qint64 readData(char *out, qint64 maximum) override {
        const auto count =
            std::min({maximum, m_length - pos(), qint64(256 * 1024)});
        if (count <= 0) {
            return 0;
        }
        if (m_cancel->load()) {
            return -1;
        }
        auto request = std::make_shared<ReadRequest>();
        const auto document = m_document;
        const auto cancel = m_cancel;
        const auto offset = pos();
        // The application outlives its windows. A queued lambda never
        // dereferences the source on a worker, and cancelled pending requests
        // do no I/O.
        QMetaObject::invokeMethod(
            QCoreApplication::instance(),
            [document, cancel, request, offset, count] {
                QByteArray bytes;
                if (!cancel->load() && document) {
                    bytes = document->read(offset, count);
                }
                QMutexLocker lock(&request->mutex);
                request->bytes = std::move(bytes);
                request->done = true;
                request->ready.wakeAll();
            },
            Qt::QueuedConnection);
        QMutexLocker lock(&request->mutex);
        while (!request->done && !m_cancel->load()) {
            request->ready.wait(&request->mutex, 20);
        }
        if (m_cancel->load() || request->bytes.size() != count) {
            setErrorString(
                QStringLiteral("The source changed or could not be read."));
            return -1;
        }
        std::memcpy(out, request->bytes.constData(), size_t(count));
        return count;
    }

private:
    QPointer<QHexDocument> m_document;
    qint64 m_length;
    std::shared_ptr<std::atomic_bool> m_cancel;
};
} // namespace

SourceResult Source::capture(QHexDocument *document, quint64 budget) {
    Source source;
    if (!document) {
        qCWarning(lcBinaryDiff) << "Cannot capture a null document";
        return {{}, QHexReadError::InvalidDevice};
    }
    source.length = document->length();
    auto device = document->buffer()->ioDevice();
    if (!device || device->isSequential() || !device->isReadable()) {
        qCWarning(lcBinaryDiff)
            << "Document source is not readable and seekable" << device;
        return {{}, QHexReadError::InvalidDevice};
    }
    const auto file = qobject_cast<QFile *>(device);
    source.m_description = file ? file->fileName() : device->objectName();
    auto captured = document->buffer()->readSession(budget);
    if (captured.session) {
        source.m_session = std::move(captured.session);
        return {source, QHexReadError::None};
    }
    if (captured.error != QHexReadError::BudgetExceeded &&
        captured.error != QHexReadError::OwnerThreadRequired) {
        return {{}, captured.error};
    }
    source.m_document = document;
    return {source, QHexReadError::None};
}

QHexReadDeviceResult
Source::open(const std::shared_ptr<std::atomic_bool> &cancel) const {
    if (cancel->load()) {
        return {{}, QHexReadError::None};
    }
    if (m_session) {
        return m_session->createDevice();
    }
    if (!m_document) {
        if (!cancel->load()) {
            qCWarning(lcBinaryDiff)
                << "Comparison source is no longer available";
        }
        return {{}, QHexReadError::InvalidDevice};
    }
    return {std::make_unique<OwnerDevice>(m_document, length, m_description,
                                          cancel),
            QHexReadError::None};
}

bool Source::sourceUnchanged() const {
    return !m_session || m_session->sourceUnchanged();
}

bool Source::valid() const { return bool(m_session) || !m_document.isNull(); }

quint64 Source::mappingBytes() const {
    return m_session ? m_session->mappingBytes() : 0;
}
} // namespace WingDiff
