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

#ifndef DIFFSOURCE_H
#define DIFFSOURCE_H

#include <atomic>
#include <memory>

#include <QPointer>

#include "QHexView/document/qhexdocument.h"

namespace WingDiff {

// Captured on the GUI thread; devices are created and used on their worker.
struct SourceResult;

class Source {
public:
    static SourceResult capture(QHexDocument *document, quint64 budget);

public:
    QHexReadDeviceResult
    open(const std::shared_ptr<std::atomic_bool> &cancel) const;

    bool sourceUnchanged() const;
    bool valid() const;
    quint64 mappingBytes() const;

public:
    qint64 length = 0;

private:
    std::shared_ptr<const QHexReadSession> m_session;
    QPointer<QHexDocument> m_document;
    QString m_description;
};

struct SourceResult {
    Source source;
    QHexReadError error = QHexReadError::None;
};

} // namespace WingDiff
#endif
