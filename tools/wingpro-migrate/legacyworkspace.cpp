/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
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

#include "legacyworkspace.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include <limits>

namespace {

bool fail(QString *error, const QString &field) {
    if (error) {
        *error =
            QStringLiteral("Invalid legacy workspace field: %1").arg(field);
    }
    return false;
}

bool readUnsigned(const QJsonValue &value, quint64 &result) {
    if (!value.isString()) {
        return false;
    }
    const auto text = value.toString();
    static const QRegularExpression pattern(
        QStringLiteral("^(?:[0-9]+|0[xX][0-9a-fA-F]+)$"));
    if (!pattern.match(text).hasMatch()) {
        return false;
    }
    bool ok = false;
    const bool hex = text.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive);
    result = text.toULongLong(&ok, hex ? 16 : 10);
    return ok;
}

bool readPosition(const QJsonValue &value, qsizetype &result) {
    quint64 number = 0;
    if (!readUnsigned(value, number) ||
        number >= quint64(std::numeric_limits<qsizetype>::max())) {
        return false;
    }
    result = qsizetype(number);
    return true;
}

bool readColor(const QJsonValue &value, QColor &result) {
    if (value.isUndefined()) {
        return true;
    }
    if (!value.isString()) {
        return false;
    }
    const auto text = value.toString();
    if (text.isEmpty()) {
        return true;
    }
    result = QColor(text);
    return result.isValid();
}

bool readMetadata(const QJsonValue &value, WorkSpaceData &data,
                  QString *error) {
    if (value.isUndefined()) {
        return true;
    }
    if (!value.isArray() || value.toArray().size() > QHEXVIEW_METADATA_LIMIT) {
        return fail(error, QStringLiteral("metas"));
    }
    const auto items = value.toArray();
    for (qsizetype i = 0; i < items.size(); ++i) {
        const auto field = QStringLiteral("metas[%1]").arg(i);
        if (!items.at(i).isObject()) {
            return fail(error, field);
        }
        const auto object = items.at(i).toObject();
        QHexMetadataItem item;
        if (!readPosition(object.value(QStringLiteral("begin")), item.begin) ||
            !readPosition(object.value(QStringLiteral("end")), item.end) ||
            item.begin > item.end ||
            !readColor(object.value(QStringLiteral("fgcolor")),
                       item.foreground) ||
            !readColor(object.value(QStringLiteral("bgcolor")),
                       item.background)) {
            return fail(error, field);
        }
        const auto comment = object.value(QStringLiteral("comment"));
        if (!comment.isUndefined() && !comment.isString()) {
            return fail(error, field + QStringLiteral(".comment"));
        }
        item.comment = comment.toString();
        if (item.comment.size() > QHEXVIEW_COMMENT_LIMIT) {
            return fail(error, field + QStringLiteral(".comment"));
        }
        data.metas.append(item);
    }
    return true;
}

bool readBookmarks(const QJsonValue &value, WorkSpaceData &data,
                   QString *error) {
    if (value.isUndefined()) {
        return true;
    }
    if (!value.isArray() || value.toArray().size() > QHEXVIEW_BOOKMARK_LIMIT) {
        return fail(error, QStringLiteral("bookmarks"));
    }
    const auto items = value.toArray();
    for (qsizetype i = 0; i < items.size(); ++i) {
        const auto field = QStringLiteral("bookmarks[%1]").arg(i);
        if (!items.at(i).isObject()) {
            return fail(error, field);
        }
        const auto object = items.at(i).toObject();
        qsizetype pos = 0;
        const auto comment = object.value(QStringLiteral("comment"));
        if (!readPosition(object.value(QStringLiteral("pos")), pos) ||
            data.bookmarks.contains(pos) || !comment.isString() ||
            comment.toString().size() > QHEXVIEW_COMMENT_LIMIT) {
            return fail(error, field);
        }
        data.bookmarks.insert(pos, comment.toString());
    }
    return true;
}

bool readPlugins(const QJsonValue &value, WorkSpaceData &data, QString *error) {
    if (value.isUndefined()) {
        return true;
    }
    if (!value.isArray()) {
        return fail(error, QStringLiteral("plugindata"));
    }
    QMap<QString, QByteArray> plugins;
    const auto items = value.toArray();
    for (qsizetype i = 0; i < items.size(); ++i) {
        const auto field = QStringLiteral("plugindata[%1]").arg(i);
        if (!items.at(i).isObject()) {
            return fail(error, field);
        }
        const auto object = items.at(i).toObject();
        const auto key = object.value(QStringLiteral("key"));
        const auto encoded = object.value(QStringLiteral("value"));
        if (!key.isString() || key.toString().isEmpty() ||
            plugins.contains(key.toString()) || !encoded.isString()) {
            return fail(error, field);
        }
        const auto text = encoded.toString();
        const auto decoded = QByteArray::fromBase64Encoding(
            text.toLatin1(), QByteArray::AbortOnBase64DecodingErrors);
        if (!decoded ||
            QString::fromLatin1(decoded.decoded.toBase64()) != text) {
            return fail(error, field + QStringLiteral(".value"));
        }
        plugins.insert(key.toString(), decoded.decoded);
    }
    WorkSpaceManager::setPluginData(data, plugins);
    return true;
}

} // namespace

bool LegacyWorkspace::load(const QString &filename, WorkSpaceData &data,
                           QString *error) {
    if (error) {
        error->clear();
    }
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }
    QJsonParseError parseError;
    const auto doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return fail(
            error,
            QStringLiteral("JSON document (%1)").arg(parseError.errorString()));
    }
    auto object = doc.object();
    const auto type = object.take(QStringLiteral("type"));
    if (!type.isString() || type.toString().compare(QStringLiteral("workspace"),
                                                    Qt::CaseInsensitive)) {
        return fail(error, QStringLiteral("type"));
    }
    WorkSpaceData value;
    const auto source = object.take(QStringLiteral("file"));
    if (!source.isString() || source.toString().isEmpty()) {
        return fail(error, QStringLiteral("file"));
    }
    value.file = QUrl::fromUserInput(source.toString(),
                                     QFileInfo(filename).absolutePath(),
                                     QUrl::AssumeLocalFile);
    if (value.file.isLocalFile()) {
        value.file =
            QUrl::fromLocalFile(QDir::cleanPath(value.file.toLocalFile()));
    }
    const auto base = object.take(QStringLiteral("base"));
    if (!base.isUndefined() && !readUnsigned(base, value.infos.base)) {
        return fail(error, QStringLiteral("base"));
    }
    const auto checksum = object.take(QStringLiteral("checksum"));
    if (!checksum.isUndefined()) {
        static const QRegularExpression hex(
            QStringLiteral("^(?:[a-fA-F0-9]{2})*$"));
        if (!checksum.isString() ||
            !hex.match(checksum.toString()).hasMatch()) {
            return fail(error, QStringLiteral("checksum"));
        }
        value.infos.checkSum =
            QByteArray::fromHex(checksum.toString().toLatin1());
    }
    if (!readMetadata(object.take(QStringLiteral("metas")), value, error) ||
        !readBookmarks(object.take(QStringLiteral("bookmarks")), value,
                       error) ||
        !readPlugins(object.take(QStringLiteral("plugindata")), value, error)) {
        return false;
    }
    if (!WorkSpaceManager::validateWorkSpace(value, error)) {
        return false;
    }
    data = std::move(value);
    return true;
}
