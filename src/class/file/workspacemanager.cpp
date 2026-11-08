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

#include "workspacemanager.h"

#include "workspaceformat.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

#include <limits>

namespace {

bool fail(QString *error, const QString &message) {
    if (error) {
        *error = message;
    }
    return false;
}

bool validPosition(quint64 pos) {
    return pos < quint64(std::numeric_limits<qsizetype>::max());
}

void configure(QDataStream &stream) {
    stream.setVersion(QDataStream::Qt_6_0);
    stream.setByteOrder(QDataStream::BigEndian);
}

// Qt reserves the declared list size before reading elements. Check the count
// first, while retaining Qt's native QList wire format and stream operators.
template <typename T>
bool readList(QDataStream &stream, QList<T> &items, quint32 limit) {
    stream.startTransaction();
    quint32 count = 0;
    stream >> count;
    const auto status = stream.status();
    stream.rollbackTransaction();
    stream.resetStatus();
    if (status != QDataStream::Ok || count > limit) {
        stream.setStatus(QDataStream::ReadCorruptData);
        return false;
    }
    stream >> items;
    return stream.status() == QDataStream::Ok;
}

} // namespace

namespace WorkspaceFormat {

QDataStream &operator<<(QDataStream &stream, const BookMark &item) {
    return stream << item.pos << item.comment;
}

QDataStream &operator>>(QDataStream &stream, BookMark &item) {
    BookMark value;
    stream >> value.pos >> value.comment;
    if (stream.status() == QDataStream::Ok) {
        item = std::move(value);
    }
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const MetaData &item) {
    return stream << item.begin << item.end << item.foreground
                  << item.background << item.comment;
}

QDataStream &operator>>(QDataStream &stream, MetaData &item) {
    MetaData value;
    stream >> value.begin >> value.end >> value.foreground >>
        value.background >> value.comment;
    if (stream.status() == QDataStream::Ok) {
        item = std::move(value);
    }
    return stream;
}

} // namespace WorkspaceFormat

QDataStream &operator<<(QDataStream &stream, const WorkSpaceInfo &info) {
    return stream << info.base << info.checkSum;
}

QDataStream &operator>>(QDataStream &stream, WorkSpaceInfo &info) {
    WorkSpaceInfo value;
    stream >> value.base >> value.checkSum;
    if (stream.status() == QDataStream::Ok) {
        info = std::move(value);
    }
    return stream;
}

bool WorkSpaceManager::validateWorkSpace(const WorkSpaceData &data,
                                         QString *error) {
    if (error) {
        error->clear();
    }
    if (!data.file.isValid() || data.file.isEmpty() || data.file.isRelative() ||
        (data.file.isLocalFile() &&
         !QDir::isAbsolutePath(data.file.toLocalFile()))) {
        return fail(error, QStringLiteral("Invalid source file URL"));
    }
    if (!data.file.isLocalFile() &&
        (data.file.scheme().compare(QStringLiteral("wdrv"),
                                    Qt::CaseInsensitive) != 0 ||
         data.file.authority().isEmpty() ||
         !data.file.path().startsWith(QLatin1Char('/')) ||
         data.file.path().size() <= 1)) {
        return fail(error, QStringLiteral("Source must be a local file or wdrv "
                                          "device, not a workspace URL"));
    }
    if (data.bookmarks.size() > QHEXVIEW_BOOKMARK_LIMIT ||
        data.metas.size() > QHEXVIEW_METADATA_LIMIT) {
        return fail(error, QStringLiteral("Too many bookmarks or metadata"));
    }
    for (auto it = data.bookmarks.cbegin(); it != data.bookmarks.cend(); ++it) {
        if (!validPosition(quint64(it.key())) ||
            it.value().size() > QHEXVIEW_COMMENT_LIMIT) {
            return fail(error,
                        QStringLiteral("Invalid bookmark at %1").arg(it.key()));
        }
    }
    for (qsizetype i = 0; i < data.metas.size(); ++i) {
        const auto &item = data.metas.at(i);
        if (!validPosition(quint64(item.begin)) ||
            !validPosition(quint64(item.end)) || item.begin > item.end ||
            item.comment.size() > QHEXVIEW_COMMENT_LIMIT) {
            return fail(error, QStringLiteral("Invalid metadata at %1").arg(i));
        }
    }
    for (auto it = data.plugindata.cbegin(); it != data.plugindata.cend();
         ++it) {
        if (it.key().isEmpty() ||
            it->metaType() != QMetaType::fromType<QByteArray>()) {
            return fail(
                error,
                QStringLiteral("Invalid plugindata entry: %1").arg(it.key()));
        }
    }
    return true;
}

QMap<QString, QByteArray>
WorkSpaceManager::pluginData(const WorkSpaceData &data) {
    QMap<QString, QByteArray> result;
    const auto &values = data.plugindata;
    for (auto it = values.cbegin(); it != values.cend(); ++it) {
        result.insert(it.key(), it->toByteArray());
    }
    return result;
}

void WorkSpaceManager::setPluginData(WorkSpaceData &data,
                                     const QMap<QString, QByteArray> &values) {
    QVariantMap plugins;
    for (auto it = values.cbegin(); it != values.cend(); ++it) {
        plugins.insert(it.key(), it.value());
    }
    data.plugindata = std::move(plugins);
}

void WorkSpaceManager::setPluginData(WorkSpaceData &data, const QString &id,
                                     const QByteArray &value) {
    data.plugindata.insert(id, value);
}

bool WorkSpaceManager::saveWorkSpace(const QString &filename,
                                     const WorkSpaceData &data,
                                     QString *error) {
    if (!validateWorkSpace(data, error)) {
        return false;
    }
    QString file = data.file.toString(QUrl::FullyEncoded);
    if (data.file.isLocalFile()) {
        file = QFileInfo(filename).absoluteDir().relativeFilePath(
            data.file.toLocalFile());
    }
    QList<WorkspaceFormat::BookMark> bookmarks;
    bookmarks.reserve(data.bookmarks.size());
    for (auto it = data.bookmarks.cbegin(); it != data.bookmarks.cend(); ++it) {
        bookmarks.append({quint64(it.key()), it.value()});
    }
    QList<WorkspaceFormat::MetaData> metas;
    metas.reserve(data.metas.size());
    for (const auto &item : data.metas) {
        metas.append({quint64(item.begin), quint64(item.end), item.foreground,
                      item.background, item.comment});
    }
    QSaveFile output(filename);
    if (!output.open(QIODevice::WriteOnly)) {
        return fail(error, output.errorString());
    }
    QDataStream stream(&output);
    configure(stream);
    stream.writeRawData(WorkspaceFormat::Magic, 7);
    stream << WorkspaceFormat::Version;
    stream << file << data.infos << bookmarks << metas << data.plugindata;
    if (stream.status() != QDataStream::Ok) {
        output.cancelWriting();
        return fail(error, QStringLiteral("Failed to serialize workspace"));
    }
    if (!output.commit()) {
        return fail(error, output.errorString());
    }
    return true;
}

bool WorkSpaceManager::loadWorkSpace(const QString &filename,
                                     WorkSpaceData &data, QString *error) {
    if (error) {
        error->clear();
    }
    QFile input(filename);
    if (!input.open(QIODevice::ReadOnly)) {
        return fail(error, input.errorString());
    }
    QDataStream stream(&input);
    configure(stream);
    char magic[7] = {};
    quint16 version = 0;
    if (stream.readRawData(magic, 7) != 7 ||
        QByteArray(magic, 7) != QByteArray(WorkspaceFormat::Magic, 7)) {
        return fail(error, QStringLiteral("Invalid workspace magic; migrate "
                                          "legacy JSON workspaces first"));
    }
    stream >> version;
    if (stream.status() != QDataStream::Ok ||
        version != WorkspaceFormat::Version) {
        return fail(error, QStringLiteral("Unsupported workspace version"));
    }
    WorkSpaceData value;
    QString file;
    QList<WorkspaceFormat::BookMark> bookmarks;
    QList<WorkspaceFormat::MetaData> metas;
    stream >> file >> value.infos;
    if (stream.status() != QDataStream::Ok ||
        !readList(stream, bookmarks, QHEXVIEW_BOOKMARK_LIMIT) ||
        !readList(stream, metas, QHEXVIEW_METADATA_LIMIT)) {
        return fail(error,
                    QStringLiteral("Invalid workspace contents or list size"));
    }
    stream >> value.plugindata;
    if (stream.status() != QDataStream::Ok || !input.atEnd()) {
        return fail(error,
                    QStringLiteral("Truncated workspace or trailing data"));
    }
    if (file.isEmpty()) {
        return fail(error, QStringLiteral("Missing source file"));
    }
    value.file = QUrl::fromUserInput(file, QFileInfo(filename).absolutePath(),
                                     QUrl::AssumeLocalFile);
    if (value.file.isLocalFile()) {
        value.file =
            QUrl::fromLocalFile(QDir::cleanPath(value.file.toLocalFile()));
    }
    for (const auto &item : bookmarks) {
        if (!validPosition(item.pos) ||
            value.bookmarks.contains(qsizetype(item.pos))) {
            return fail(error, QStringLiteral(
                                   "Invalid or duplicate bookmark position"));
        }
        value.bookmarks.insert(qsizetype(item.pos), item.comment);
    }
    for (const auto &item : metas) {
        if (!validPosition(item.begin) || !validPosition(item.end)) {
            return fail(error, QStringLiteral("Invalid metadata position"));
        }
        value.metas.append(
            QHexMetadataItem(qsizetype(item.begin), qsizetype(item.end),
                             item.foreground, item.background, item.comment));
    }
    if (!validateWorkSpace(value, error)) {
        return false;
    }
    data = std::move(value);
    return true;
}
