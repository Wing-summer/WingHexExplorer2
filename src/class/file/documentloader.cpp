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

#include "documentloader.h"

#include <QFileInfo>

#include "QHexView/document/qhexdocument.h"

DocumentLoadResult loadFileDocument(const QString &path, bool readOnly) {
    DocumentLoadResult result;
    const QFileInfo info(path);
    result.path = info.absoluteFilePath();
    if (!info.exists()) {
        result.error = WingHex::ErrFile::NotExist;
        return result;
    }
    if (!info.isReadable()) {
        result.error = WingHex::ErrFile::Permission;
        return result;
    }
    if (!info.isFile()) {
        result.error = WingHex::ErrFile::InvalidFormat;
        return result;
    }
    result.readOnly = readOnly || !info.isWritable();
    auto document = QHexDocument::fromFile(result.path, result.readOnly);
    if (!document && !result.readOnly) {
        result.readOnly = true;
        document = QHexDocument::fromFile(result.path, true);
    }
    if (!document) {
        result.error = WingHex::ErrFile::Permission;
        return result;
    }
    result.document.reset(document);
    document->setDocSaved();
    result.error = WingHex::ErrFile::Success;
    return result;
}
