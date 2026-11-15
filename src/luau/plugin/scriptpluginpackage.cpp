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

#include "scriptpluginpackage.h"

#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSet>

#include "luau/runtime/scriptchunk.h"
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 2)
#include <QtCore/private/qzipreader_p.h>
#else
#include <QtGui/private/qzipreader_p.h>
#endif

bool ScriptPluginPackage::hasZipSignature(QByteArrayView header) {
    // Routing only: load() still validates the complete ZIP and package
    // content.
    return header.size() >= 2 && header[0] == 'P' && header[1] == 'K';
}

ScriptPluginResult ScriptPluginPackage::load(const QString &path) {
    QString error;
    if (m_files) {
        error = QStringLiteral("Package is already loaded");
        return {ScriptPluginError::Package, error, {}};
    }
    Kind kind = Kind::Plugin;
    if (path.endsWith(QStringLiteral(".wingdrv"), Qt::CaseInsensitive)) {
        kind = Kind::Device;
    } else if (!path.endsWith(QStringLiteral(".wingplg"),
                              Qt::CaseInsensitive)) {
        error = QStringLiteral("Unsupported script plugin package extension");
        return {ScriptPluginError::Package, error, {}};
    }
    constexpr qint64 maxFile = 16 * 1024 * 1024;
    constexpr qint64 maxTotal = 64 * 1024 * 1024;
    const QFileInfo package(path);
    if (!package.isFile() || package.size() <= 0 || package.size() > maxTotal) {
        error = QStringLiteral("Invalid plugin package size");
        return {ScriptPluginError::Package, error, {}};
    }
    QFile archive(path);
    if (!archive.open(QIODevice::ReadOnly)) {
        error = archive.errorString();
        return {ScriptPluginError::Package, error, {}};
    }
    auto archiveBytes = archive.read(maxTotal + 1);
    if (archiveBytes.size() > maxTotal || archive.error() != QFile::NoError) {
        error = QStringLiteral("Cannot read plugin archive");
        return {ScriptPluginError::Package, error, {}};
    }
    if (!hasZipSignature(archiveBytes)) {
        return {ScriptPluginError::Package,
                QStringLiteral("Missing ZIP signature in plugin package"),
                path};
    }
    QBuffer buffer(&archiveBytes);
    buffer.open(QIODevice::ReadOnly);
    QZipReader zip(&buffer);
    const auto entries = zip.fileInfoList();
    if (entries.size() > 4096 || zip.status() != QZipReader::NoError) {
        error = QStringLiteral("Invalid plugin archive");
        return {ScriptPluginError::Package, error, {}};
    }
    QHash<QString, QByteArray> files;
    qint64 total = 0;
    QSet<QString> paths;
    for (const auto &entry : entries) {
        auto name = entry.filePath;
        if (entry.isDir && name.endsWith('/')) {
            name.chop(1);
        }
        if (!entry.isValid() || entry.isSymLink || name.isEmpty() ||
            name.contains('\\') || name.contains(':') ||
            name.contains(QChar(0)) || QDir::isAbsolutePath(name) ||
            name != QDir::cleanPath(name) || name == QStringLiteral("..") ||
            name.startsWith(QStringLiteral("../")) ||
            paths.contains(name.toCaseFolded()) || entry.size < 0 ||
            entry.size > maxFile) {
            error = QStringLiteral("Invalid or duplicate archive path: %1")
                        .arg(name);
            return {ScriptPluginError::Package, error, {}};
        }
        paths.insert(name.toCaseFolded());
        total += entry.size;
        if (total > maxTotal) {
            error = QStringLiteral("Plugin archive exceeds size limit");
            return {ScriptPluginError::Package, error, {}};
        }
        if (!entry.isFile) {
            continue;
        }
        const auto data = zip.fileData(entry.filePath);
        if (data.size() != entry.size || zip.status() != QZipReader::NoError) {
            error = QStringLiteral("Cannot read archive entry: %1").arg(name);
            return {ScriptPluginError::Package, error, {}};
        }
        if (scriptChunkFormat(name) &&
            !name.endsWith(QStringLiteral(".d.luau")) &&
            data.size() > DefaultScriptFileSizeLimit) {
            error = QStringLiteral(
                        "Executable package entry exceeds size limit: %1")
                        .arg(name);
            return {ScriptPluginError::Package, error, {}};
        }
        files.insert(name, data);
    }
    QJsonParseError jsonError;
    const auto document = QJsonDocument::fromJson(
        files.value(QStringLiteral("manifest.json")), &jsonError);
    if (jsonError.error != QJsonParseError::NoError || !document.isObject()) {
        error = QStringLiteral("Invalid plugin manifest");
        return {ScriptPluginError::Package, error, {}};
    }
    const auto manifest = document.object();
    if (manifest.value(QStringLiteral("FormatVersion")).toInt() != 1) {
        error = QStringLiteral("Unsupported plugin package format");
        return {ScriptPluginError::Package, error, {}};
    }
    QString entry;
    for (const auto &name : scriptFileCandidates(QStringLiteral("main"))) {
        if (files.contains(name)) {
            if (!entry.isEmpty()) {
                error = QStringLiteral("Ambiguous plugin entry point");
                return {ScriptPluginError::Package, error, {}};
            }
            entry = name;
        }
    }
    if (entry.isEmpty()) {
        error = QStringLiteral("Missing plugin entry point");
        return {ScriptPluginError::Package, error, {}};
    }
    try {
        m_files = std::make_unique<MemoryFileSystem>(std::move(files));
    } catch (const std::exception &exception) {
        return {ScriptPluginError::Package, QString::fromUtf8(exception.what()),
                path};
    }
    m_kind = kind;
    m_manifest = manifest;
    m_entry = entry;
    m_archive = std::move(archiveBytes);
    return {};
}

QString ScriptPluginPackage::entryPath() const {
    return QDir(rootPath()).filePath(m_entry);
}

std::optional<QByteArray>
ScriptPluginPackage::readResource(const QString &relative) const {
    return m_files ? m_files->read(relative) : std::nullopt;
}

ScriptPluginResult
ScriptPluginPackage::resourcePath(const QString &relative) const {
    const auto path = m_files ? m_files->filePath(relative) : QString();
    if (path.isEmpty()) {
        return {ScriptPluginError::Resource,
                QStringLiteral("Package resource not found"),
                {}};
    }
    return {ScriptPluginError::None, {}, path};
}
