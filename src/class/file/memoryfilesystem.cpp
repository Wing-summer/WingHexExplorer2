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

#include "memoryfilesystem.h"

#include <cstring>
#include <stdexcept>
#include <utility>

#include <QDir>
#include <QGlobalStatic>
#include <QReadWriteLock>
#include <QSet>
#include <QUuid>
#include <private/qabstractfileengine_p.h>

namespace {
constexpr auto Prefix = ":/winghex-memory/";

bool validRelativePath(const QString &path) {
    return !path.isEmpty() && path != QStringLiteral(".") &&
           path != QStringLiteral("..") &&
           !path.startsWith(QStringLiteral("../")) && !path.contains(':') &&
           !path.contains('\\') && !path.contains(QChar(0)) &&
           !QDir::isAbsolutePath(path) && path == QDir::cleanPath(path);
}

struct MemoryTree {
    QString root;
    QHash<QString, QByteArray> files;
    QSet<QString> directories{QString()};
    QHash<QString, QStringList> children;
};

class MemoryIterator final : public QAbstractFileEngineIterator {
public:
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    MemoryIterator(const QString &path, QDirListing::IteratorFlags filters,
                   const QStringList &names, std::weak_ptr<MemoryTree> tree,
                   QStringList children)
        : QAbstractFileEngineIterator(path, filters, names),
          m_tree(std::move(tree)), m_children(std::move(children)) {}

    bool advance() override {
        if (m_tree.expired() || m_index + 1 >= m_children.size()) {
            return false;
        }
        ++m_index;
        m_fileInfo = QFileInfo();
        return true;
    }
#else
    MemoryIterator(QDir::Filters filters, const QStringList &names,
                   std::weak_ptr<MemoryTree> tree, QStringList children)
        : QAbstractFileEngineIterator(filters, names), m_tree(std::move(tree)),
          m_children(std::move(children)) {}

    bool hasNext() const override {
        return !m_tree.expired() && m_index + 1 < m_children.size();
    }

    QString next() override {
        if (!hasNext()) {
            return {};
        }
        ++m_index;
        return currentFilePath();
    }
#endif
    QString currentFileName() const override {
        return m_tree.expired() || m_index < 0 ? QString()
                                               : m_children.value(m_index);
    }

private:
    std::weak_ptr<MemoryTree> m_tree;
    QStringList m_children;
    qsizetype m_index = -1;
};

class MemoryEngine final : public QAbstractFileEngine {
public:
    MemoryEngine(QString path, std::weak_ptr<MemoryTree> tree, QString relative)
        : m_path(std::move(path)), m_tree(std::move(tree)),
          m_relative(std::move(relative)) {}

    bool open(QIODevice::OpenMode mode,
              std::optional<QFile::Permissions> = std::nullopt) override {
        const auto tree = m_tree.lock();
        if (!(mode & QIODevice::ReadOnly) ||
            (mode & (QIODevice::WriteOnly | QIODevice::Append |
                     QIODevice::Truncate | QIODevice::NewOnly)) ||
            !tree || !tree->files.contains(m_relative)) {
            setError(
                QFile::OpenError,
                QStringLiteral(
                    "Virtual file is missing or is not openable for reading"));
            return false;
        }
        m_open = true;
        m_position = 0;
        return true;
    }

    bool close() override {
        m_open = false;
        return true;
    }

    qint64 size() const override {
        const auto tree = m_tree.lock();
        if (!tree) {
            return 0;
        }
        const auto file = tree->files.constFind(m_relative);
        return file == tree->files.cend() ? 0 : file->size();
    }

    qint64 pos() const override { return m_position; }

    bool seek(qint64 position) override {
        if (!m_open || m_tree.expired() || position < 0) {
            return false;
        }
        m_position = position;
        return true;
    }

    bool isSequential() const override { return false; }

    bool isRelativePath() const override { return false; }

    bool caseSensitive() const override { return true; }

    FileFlags fileFlags(FileFlags requested = FileInfoAll) const override {
        const auto tree = m_tree.lock();
        if (!tree) {
            return {};
        }
        FileFlags flags;
        if (tree->files.contains(m_relative)) {
            flags = FileType;
        } else if (tree->directories.contains(m_relative)) {
            flags = DirectoryType;
        } else {
            return {};
        }
        flags |= ExistsFlag | ReadOwnerPerm | ReadUserPerm | ReadGroupPerm |
                 ReadOtherPerm;
        if (m_relative.isEmpty()) {
            flags |= RootFlag;
        }
        if (m_relative.section('/', -1).startsWith('.')) {
            flags |= HiddenFlag;
        }
        return flags & requested;
    }

    QString fileName(FileName kind = DefaultName) const override {
        switch (kind) {
        case BaseName:
            return m_path.section('/', -1);
        case PathName:
        case AbsolutePathName:
            return m_path.left(m_path.lastIndexOf('/'));
        case CanonicalName:
            return fileFlags(ExistsFlag) ? m_path : QString();
        case CanonicalPathName:
            return fileFlags(ExistsFlag) ? m_path.left(m_path.lastIndexOf('/'))
                                         : QString();
        case DefaultName:
        case AbsoluteName:
            return m_path;
        default:
            return {};
        }
    }

    qint64 read(char *data, qint64 maximum) override {
        const auto tree = m_tree.lock();
        if (!m_open || !tree || maximum < 0) {
            setError(
                QFile::ReadError,
                QStringLiteral(
                    "Virtual file is closed or its owner has been released"));
            return -1;
        }
        const auto file = tree->files.constFind(m_relative);
        if (file == tree->files.cend()) {
            return -1;
        }
        const qint64 count =
            qMin(maximum, qMax(qint64(0), qint64(file->size()) - m_position));
        if (count > 0) {
            std::memcpy(data, file->constData() + m_position, size_t(count));
        }
        m_position += count;
        return count;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    IteratorUniquePtr beginEntryList(const QString &path,
                                     QDirListing::IteratorFlags filters,
                                     const QStringList &names) override {
        return std::make_unique<MemoryIterator>(path, filters, names, m_tree,
                                                children());
    }
#else
    Iterator *beginEntryList(QDir::Filters filters,
                             const QStringList &names) override {
        return new MemoryIterator(filters, names, m_tree, children());
    }
#endif
private:
    QStringList children() const {
        const auto tree = m_tree.lock();
        if (!tree || !tree->directories.contains(m_relative)) {
            return {};
        }
        return QStringList{QStringLiteral("."), QStringLiteral("..")} +
               tree->children.value(m_relative);
    }

    QString m_path;
    std::weak_ptr<MemoryTree> m_tree;
    QString m_relative;
    qint64 m_position = 0;
    bool m_open = false;
};

// Keep the Qt handler installed while individual mounts come and go. Qt file
// lookups on other threads must not race destruction of a per-mount handler.
class MemoryRegistry final : public QAbstractFileEngineHandler {
public:
    void mount(const std::shared_ptr<MemoryTree> &tree) {
        QWriteLocker lock(&m_lock);
        m_trees.insert(tree->root, tree);
    }

    void unmount(const QString &root) {
        QWriteLocker lock(&m_lock);
        m_trees.remove(root);
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    std::unique_ptr<QAbstractFileEngine>
    create(const QString &name) const override {
#else
    QAbstractFileEngine *create(const QString &name) const override {
#endif
        auto prefix = QString::fromUtf8(Prefix);
        if (!name.startsWith(prefix)) {
            return {};
        }
        const auto path = QDir::cleanPath(name);
        const auto slash = path.indexOf('/', prefix.size());
        const auto root = slash < 0 ? path : path.first(slash);
        const auto relative = slash < 0 ? QString() : path.sliced(slash + 1);
        QReadLocker lock(&m_lock);
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        return std::make_unique<MemoryEngine>(path, m_trees.value(root),
                                              relative);
#else
        return new MemoryEngine(path, m_trees.value(root), relative);
#endif
    }

private:
    mutable QReadWriteLock m_lock;
    QHash<QString, std::weak_ptr<MemoryTree>> m_trees;
};
Q_GLOBAL_STATIC(MemoryRegistry, memoryRegistry)
} // namespace

class MemoryFileSystem::Private {
public:
    std::shared_ptr<MemoryTree> tree;
};

MemoryFileSystem::MemoryFileSystem(QHash<QString, QByteArray> files)
    : m_data(std::make_unique<Private>()) {
    auto tree = std::make_shared<MemoryTree>();
    tree->root = Prefix + QUuid::createUuid().toString(QUuid::WithoutBraces);
    tree->files = std::move(files);
    for (auto it = tree->files.cbegin(); it != tree->files.cend(); ++it) {
        const auto &path = it.key();
        if (!validRelativePath(path)) {
            throw std::invalid_argument("Invalid virtual file path");
        }
        QString child = path;
        while (!child.isEmpty()) {
            const auto slash = child.lastIndexOf('/');
            const auto parent = slash < 0 ? QString() : child.left(slash);
            const auto name = slash < 0 ? child : child.mid(slash + 1);
            if (!tree->children[parent].contains(name)) {
                tree->children[parent].append(name);
            }
            tree->directories.insert(parent);
            child = parent;
        }
    }
    for (const auto &directory : std::as_const(tree->directories)) {
        if (tree->files.contains(directory)) {
            throw std::invalid_argument(
                "Virtual path is both a file and a directory");
        }
    }
    for (auto &children : tree->children) {
        children.sort(Qt::CaseSensitive);
    }
    m_data->tree = tree;
    memoryRegistry->mount(tree);
}

MemoryFileSystem::~MemoryFileSystem() {
    if (!memoryRegistry.isDestroyed()) {
        memoryRegistry->unmount(m_data->tree->root);
    }
}

QString MemoryFileSystem::rootPath() const { return m_data->tree->root; }

QString MemoryFileSystem::filePath(const QString &relative) const {
    return validRelativePath(relative) && m_data->tree->files.contains(relative)
               ? rootPath() + '/' + relative
               : QString();
}

std::optional<QByteArray>
MemoryFileSystem::read(const QString &relative) const {
    if (!validRelativePath(relative)) {
        return std::nullopt;
    }
    const auto it = m_data->tree->files.constFind(relative);
    return it == m_data->tree->files.cend() ? std::nullopt
                                            : std::optional<QByteArray>(*it);
}
