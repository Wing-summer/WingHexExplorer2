/*==============================================================================
 ** Copyright (C) 2026-2029 WingSummer
 **
 ** This program is free software: you can redistribute it and/or modify it
 ** under the terms of the GNU Affero General Public License as published by the
 ** Free Software Foundation, version 3.
 **
 ** This program is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 ** for more details.
 **
 ** You should have received a copy of the GNU Affero General Public License
 ** along with this program. If not, see <https://www.gnu.org/licenses/>.
 ** =============================================================================
 */

#include "wingluaurequire.h"

#include "lua.h"
#include "lualib.h"

#include "Luau/Compiler.h"

#include <QCryptographicHash>
#include <QDebug>
#include <qlatin1stringview.h>
#include <qnamespace.h>

WingLuauRequire::WingLuauRequire() {}

bool WingLuauRequire::initRequire(lua_State *L, Options options) {
    if (!L) {
        return false;
    }

    L_ = L;
    options_ = std::move(options);
    options_.systemRoot = normalizeRoot(options_.systemRoot);
    options_.userRoot = normalizeRoot(options_.userRoot);
    return true;
}

void WingLuauRequire::installRequire() {
    if (!L_)
        return;

    const int top = lua_gettop(L_);

    // Keep the native Luau require as the first upvalue of the adapter.
    luarequire_pushrequire(L_, &WingLuauRequire::configInit, this);
    lua_pushlightuserdata(L_, this);
    lua_pushboolean(L_, false);
    lua_pushcclosure(L_, &WingLuauRequire::cbRequireDispatch, "require", 3);
    lua_setglobal(L_, "require");

    LUAU_ASSERT(lua_gettop(L_) == top);
}

void WingLuauRequire::installProxyRequire(const char *globalName) {
    if (!L_ || !globalName || !*globalName)
        return;

    const int top = lua_gettop(L_);
    luarequire_pushproxyrequire(L_, &WingLuauRequire::configInit, this);
    lua_pushlightuserdata(L_, this);
    lua_pushboolean(L_, true);
    lua_pushcclosure(L_, &WingLuauRequire::cbRequireDispatch, "proxyrequire",
                     3);
    lua_setglobal(L_, globalName);
    LUAU_ASSERT(lua_gettop(L_) == top);
}

bool WingLuauRequire::registerBuiltinValue(const QString &moduleName,
                                           int valueIndex) {
    if (!L_) {
        qCritical("lua_State is null");
        return false;
    }

    const auto normalized = normalizeBuiltinName(moduleName);
    if (normalized.isEmpty()) {
        qCritical("invalid builtin module name");
        return false;
    }

    const int top = lua_gettop(L_);
    const int absoluteValueIndex = lua_absindex(L_, valueIndex);
    if (absoluteValueIndex <= 0 || absoluteValueIndex > top) {
        qCritical("invalid Lua value index");
        return false;
    }

    auto normalizedName = normalized.toUtf8();
    auto normalizedNameData = normalizedName.constData();

    lua_pushcfunction(L_, luarequire_registermodule,
                      "luarequire_registermodule");
    lua_pushfstring(L_, "@b/%s", normalizedNameData);
    lua_pushvalue(L_, absoluteValueIndex);

    const int status = lua_pcall(L_, 2, 0, 0);
    if (status != LUA_OK) {
        const char *message = lua_tostring(L_, -1);
        if (message) {
            qCritical("%s", message);
        } else {
            qCritical("register builtin failed");
        }
        lua_settop(L_, top);
        return false;
    }

    // Keep a private registry copy as well. Luau's registered-module table
    // is intentionally an internal implementation detail, while this table
    // lets the host loader serve the builtin when @<name> resolves through
    // the normal alias fallback without colliding with another package source.
    lua_getfield(L_, LUA_REGISTRYINDEX, builtinRegistryKey());
    if (lua_isnil(L_, -1)) {
        lua_pop(L_, 1);
        lua_newtable(L_);
        lua_pushvalue(L_, -1);
        lua_setfield(L_, LUA_REGISTRYINDEX, builtinRegistryKey());
    }

    lua_pushvalue(L_, absoluteValueIndex);
    lua_setfield(L_, -2, normalizedNameData);
    lua_pop(L_, 1);

    builtinModules_.insert(normalized);

    const auto parts = normalized.split('/', Qt::SkipEmptyParts);
    QString prefix;
    for (const auto &part : parts) {
        prefix = prefix.isEmpty() ? part : prefix + QStringLiteral("/") + part;
        builtinNamespaces_.insert(prefix);
    }

    lua_settop(L_, top);
    return true;
}

void WingLuauRequire::clearRequireCache() {
    if (!L_)
        return;

    lua_pushcfunction(L_, luarequire_clearcache, "luarequire_clearcache");
    lua_call(L_, 0, 0);
}

void WingLuauRequire::clearRequireCacheEntry(const char *cacheKey) {
    if (!L_ || qstrlen(cacheKey) == 0) {
        return;
    }

    lua_pushcfunction(L_, luarequire_clearcacheentry,
                      "luarequire_clearcacheentry");
    lua_pushstring(L_, cacheKey);
    lua_call(L_, 1, 0);
}

const WingLuauRequire::Options &WingLuauRequire::options() const {
    return options_;
}

QString WingLuauRequire::callerChunkname(lua_State *L) {
    if (!L)
        return {};

    lua_Debug ar;
    int level = 1;
    do {
        if (!lua_getinfo(L, level++, "s", &ar))
            return {};
    } while (ar.what[0] == 'C');

    return ar.source ? QString::fromUtf8(ar.source) : QString();
}

QString WingLuauRequire::normalizeRoot(const QString &input) {
    if (input.isEmpty()) {
        return {};
    }

    QFileInfo info(input);
    QString path = QDir::cleanPath(info.absoluteFilePath());
    path = QDir::fromNativeSeparators(path);

    // Prefer canonical paths when they already exist. This makes cache keys
    // stable across symlinked package roots.
    const QString canonical = info.canonicalFilePath();
    if (!canonical.isEmpty()) {
        path = QDir::fromNativeSeparators(canonical);
    }

#ifdef Q_OS_WIN
    path = path.toLower();
#endif
    return path;
}

QString WingLuauRequire::normalizeExistingPath(const QString &input) {
    if (input.isEmpty()) {
        return {};
    }

    QFileInfo info(input);
    QString path;
    if (info.exists()) {
        path = info.canonicalFilePath();
    }
    if (path.isEmpty()) {
        path = info.absoluteFilePath();
    }
    path = QDir::cleanPath(QDir::fromNativeSeparators(path));
#ifdef Q_OS_WIN
    path = path.toLower();
#endif
    return path;
}

QString WingLuauRequire::joinPath(const QString &base, const QString &name) {
    return QDir::cleanPath(QDir(base).filePath(name));
}

bool WingLuauRequire::pathEqualOrBelow(const QString &path,
                                       const QString &root) {
    if (root.isEmpty())
        return false;

    const auto p = normalizeExistingPath(path);
    const auto r = normalizeExistingPath(root);
    if (p.isEmpty() || r.isEmpty())
        return false;

    if (p == r)
        return true;

    auto prefix = r;
    if (!prefix.endsWith('/')) {
        prefix += '/';
    }

    return p.startsWith(prefix);
}

QString WingLuauRequire::relativePath(const QString &root,
                                      const QString &path) {
    QString relative = QDir(root).relativeFilePath(path);
    relative = QDir::cleanPath(QDir::fromNativeSeparators(relative));
    if (relative == QLatin1String(".")) {
        relative.clear();
    }
    return relative;
}

QString WingLuauRequire::stripModuleSuffix(QString path) {
    if (path.endsWith(QLatin1String(".luau"), Qt::CaseInsensitive)) {
        path.chop(5);
    } else if (path.endsWith(QLatin1String(".lua"), Qt::CaseInsensitive)) {
        path.chop(4);
    }
    return path;
}

bool WingLuauRequire::isFileModuleCandidate(const QFileInfo &info) {
    return info.exists() && info.isFile();
}

WingLuauRequire::FileModuleResolution
WingLuauRequire::resolveFileModule(const QString &basePath,
                                   const QString &requestedPath) {
    if (basePath.isEmpty() || requestedPath.isEmpty()) {
        return {};
    }

    const auto targetPath = joinPath(basePath, requestedPath);
    const QFileInfo targetInfo(targetPath);
    if (!targetInfo.suffix().isEmpty()) {
        if (isFileModuleCandidate(targetInfo))
            return {FileModuleResolution::Kind::File,
                    normalizeExistingPath(targetPath)};
        if (targetInfo.exists() && targetInfo.isDir()) {
            QString initPath;
            const auto initStatus = directoryInitStatus(targetPath, &initPath);
            if (initStatus == InitStatus::Ambiguous)
                return {FileModuleResolution::Kind::Ambiguous, {}};
            if (initStatus == InitStatus::Present)
                return {FileModuleResolution::Kind::ModuleDirectory, initPath};
            return {FileModuleResolution::Kind::NamespaceDirectory,
                    normalizeExistingPath(targetPath)};
        }
        return {};
    }

    const auto luauPath = targetPath + QStringLiteral(".luau");
    const auto luaPath = targetPath + QStringLiteral(".lua");
    const bool hasLuauFile = isFileModuleCandidate(QFileInfo(luauPath));
    const bool hasLuaFile = isFileModuleCandidate(QFileInfo(luaPath));
    const bool hasDirectory = targetInfo.exists() && targetInfo.isDir();
    if (int(hasLuauFile) + int(hasLuaFile) + int(hasDirectory) > 1) {
        return {FileModuleResolution::Kind::Ambiguous, {}};
    }

    if (hasLuauFile || hasLuaFile) {
        return {FileModuleResolution::Kind::File,
                normalizeExistingPath(hasLuauFile ? luauPath : luaPath)};
    }
    if (!hasDirectory) {
        return {};
    }

    QString initPath;
    const auto initStatus = directoryInitStatus(targetPath, &initPath);
    if (initStatus == InitStatus::Ambiguous) {
        return {FileModuleResolution::Kind::Ambiguous, {}};
    }
    if (initStatus == InitStatus::Present) {
        return {FileModuleResolution::Kind::ModuleDirectory, initPath};
    }

    return {FileModuleResolution::Kind::NamespaceDirectory,
            normalizeExistingPath(targetPath)};
}

bool WingLuauRequire::isSafeModulePath(const QString &input) {
    if (input.isEmpty()) {
        return false;
    }

    QString path = QDir::fromNativeSeparators(input);
    while (path.startsWith('/')) {
        path.remove(0, 1);
    }

    if (path.isEmpty() || path == QLatin1String(".")) {
        return false;
    }

    const auto parts = path.split('/', Qt::KeepEmptyParts);
    for (const auto &part : parts) {
        if (part.isEmpty() || part == QLatin1String(".") ||
            part == QLatin1String("..")) {
            return false;
        }
    }

    return true;
}

QString WingLuauRequire::normalizeBuiltinName(const QString &input) {
    QString normalized = QDir::fromNativeSeparators(input).trimmed();
    while (normalized.startsWith('/')) {
        normalized.remove(0, 1);
    }

    // Builtins have one canonical public namespace: @b/<module>. Do not
    // silently reinterpret another @-prefixed name as a builtin.
    if (normalized.startsWith('@')) {
        if (!normalized.startsWith(QLatin1String("@b/"), Qt::CaseInsensitive)) {
            return {};
        }
        normalized.remove(0, 3);
    }

    normalized = QDir::cleanPath(normalized);
    while (normalized.startsWith(QLatin1String("./"))) {
        normalized.remove(0, 2);
    }
    if (!isSafeModulePath(normalized)) {
        return {};
    }
    normalized = normalized.toLower();
    return normalized;
}

QString WingLuauRequire::builtinChunknameFor(const QString &moduleName) {
    return QStringLiteral("@b/") + moduleName;
}

QString WingLuauRequire::externalChunkname(const Node &node) {
    const QString physical = normalizeExistingPath(node.physicalPath);
    const QByteArray digest =
        QCryptographicHash::hash(physical.toUtf8(), QCryptographicHash::Sha256)
            .toHex();
    const QString name = QFileInfo(physical).completeBaseName();
    return QStringLiteral("@x/") + QString::fromLatin1(digest) +
           (name.isEmpty() ? QString() : QStringLiteral("/") + name);
}

WingLuauRequire::Node
WingLuauRequire::makeScopeRootNode(Scope scope, const QString &physicalRoot,
                                   bool explicitSource) {
    Node node;
    node.scope = scope;
    node.kind = NodeKind::ScopeRoot;
    node.physicalPath = physicalRoot;
    node.rootPath = physicalRoot;
    node.explicitSource = explicitSource;
    return node;
}
bool WingLuauRequire::isModuleNode(const Node &node) const {
    switch (node.kind) {
    case NodeKind::Builtin:
        return builtinModules_.contains(node.builtinPath);

    case NodeKind::ModuleFile:
    case NodeKind::ExternalFile:
        return QFileInfo(node.physicalPath).isFile();

    case NodeKind::ModuleDirectory:
    case NodeKind::ExternalDirectory:
        return directoryInitStatus(node.physicalPath) == InitStatus::Present;

    case NodeKind::ScopeRoot:
    case NodeKind::Directory:
        return false;
    }

    return false;
}

WingLuauRequire::AliasSources
WingLuauRequire::findAliasSources(const QString &alias) const {
    AliasSources result;
    if (alias.isEmpty()) {
        return result;
    }

    // A builtin namespace is considered an owner of the shorthand
    // namespace. The exact module is checked later by
    // isModuleNode()/builtinModules_.
    result.builtin = builtinNamespaces_.contains(alias);
    result.user = findPackageInScope(Scope::User, alias);
    result.system = findPackageInScope(Scope::System, alias);
    return result;
}

QString WingLuauRequire::sourcePrefixForNode(const Node &node) const {
    if (node.kind == NodeKind::Builtin)
        return QStringLiteral("@b/");

    if (node.scope == Scope::User)
        return QStringLiteral("@u/");

    if (node.scope == Scope::System)
        return QStringLiteral("@s/");

    return QStringLiteral("@x/");
}

WingLuauRequire::InitStatus
WingLuauRequire::directoryInitStatus(const QString &directory,
                                     QString *loadPath) {
    const QString luau = joinPath(directory, QStringLiteral("init.luau"));
    const QFileInfo luauInfo(luau);

    const QString lua = joinPath(directory, QStringLiteral("init.lua"));
    const QFileInfo luaInfo(lua);

    const bool hasLuau = isFileModuleCandidate(luauInfo);
    const bool hasLua = isFileModuleCandidate(luaInfo);

    if (hasLuau && hasLua) {
        return InitStatus::Ambiguous;
    }
    if (!hasLuau && !hasLua) {
        return InitStatus::None;
    }
    if (loadPath) {
        *loadPath = normalizeExistingPath(hasLuau ? luau : lua);
    }
    return InitStatus::Present;
}

WingLuauRequire::Candidate WingLuauRequire::makeCandidateFromBase(
    const QString &base, Scope scope, const QString &rootPath,
    const QString &component, const QString &packageName,
    bool explicitSource) const {
    Candidate result;

    if (component.isEmpty() || component == QLatin1String(".") ||
        component == QLatin1String("..") || component.contains('/'))
        return result;

    const QString basePath = normalizeExistingPath(base);
    if (basePath.isEmpty())
        return result;

    const auto resolution = resolveFileModule(basePath, component);
    using ResolutionKind = FileModuleResolution::Kind;
    if (resolution.kind == ResolutionKind::Ambiguous) {
        result.result = Candidate::Result::Ambiguous;
        return result;
    }

    if (resolution.kind == ResolutionKind::NotFound) {
        return result;
    }

    Node node;
    node.scope = scope;
    node.rootPath = rootPath;
    node.packageName = packageName;
    node.explicitSource = explicitSource;

    if (resolution.kind == ResolutionKind::File) {
        result.result = Candidate::Result::Success;
        node.kind = (scope == Scope::External) ? NodeKind::ExternalFile
                                               : NodeKind::ModuleFile;
        node.physicalPath = resolution.path;
        result.node = node;
        return result;
    }

    if (resolution.kind == ResolutionKind::ModuleDirectory ||
        resolution.kind == ResolutionKind::NamespaceDirectory) {
        result.result = Candidate::Result::Success;
        node.kind =
            resolution.kind == ResolutionKind::ModuleDirectory
                ? ((scope == Scope::External) ? NodeKind::ExternalDirectory
                                              : NodeKind::ModuleDirectory)
                : ((scope == Scope::External) ? NodeKind::ExternalDirectory
                                              : NodeKind::Directory);
        node.physicalPath = resolution.kind == ResolutionKind::ModuleDirectory
                                ? QFileInfo(resolution.path).absolutePath()
                                : resolution.path;
        result.node = node;
        return result;
    }

    return result;
}

WingLuauRequire::Candidate
WingLuauRequire::resolveChildFromSingleRoot(const QString &root, Scope scope,
                                            const QString &component,
                                            bool explicitSource) const {
    if (root.isEmpty()) {
        return {};
    }

    return makeCandidateFromBase(root, scope, root, component, component,
                                 explicitSource);
}

WingLuauRequire::Candidate
WingLuauRequire::resolveChild(const QString &component) const {
    // A module directory exposes its children, but its reserved init file
    // is not itself a child module. Keep this rule on the actual navigation
    // path, not in makeCandidateFromBase(), which also resolves package roots.
    if (current_.kind == NodeKind::ModuleDirectory &&
        component == QLatin1String("init")) {
        return {};
    }

    switch (current_.kind) {
    case NodeKind::ScopeRoot:
        if (current_.scope == Scope::System) {
            return resolveChildFromSingleRoot(options_.systemRoot,
                                              Scope::System, component,
                                              current_.explicitSource);
        }

        if (current_.scope == Scope::User) {
            return resolveChildFromSingleRoot(options_.userRoot, Scope::User,
                                              component,
                                              current_.explicitSource);
        }

        return {};

    case NodeKind::Builtin: {
        const QString child = current_.builtinPath.isEmpty()
                                  ? component.toLower()
                                  : current_.builtinPath + QStringLiteral("/") +
                                        component.toLower();

        // Builtins form a logical registry tree, not a filesystem root.
        // A registered module can also have registered descendants, so a
        // node may act as both a module and a namespace.
        if (!builtinNamespaces_.contains(child))
            return {};

        Candidate result;
        result.result = Candidate::Result::Success;
        result.node.scope = Scope::External;
        result.node.kind = NodeKind::Builtin;
        result.node.builtinPath = child;
        return result;
    }

    case NodeKind::Directory:
    case NodeKind::ModuleDirectory:
    case NodeKind::ExternalDirectory:
        return makeCandidateFromBase(
            current_.physicalPath, current_.scope, current_.rootPath, component,
            current_.packageName, current_.explicitSource);

    case NodeKind::ModuleFile:
    case NodeKind::ExternalFile:
        return {};
    }

    return {};
}

QString WingLuauRequire::rootForScope(Scope scope) const {
    switch (scope) {
    case Scope::System:
        return options_.systemRoot;
    case Scope::User:
        return options_.userRoot;
    case Scope::External:
        break;
    }

    return {};
}

WingLuauRequire::PackageLookup
WingLuauRequire::findPackageInScope(Scope scope,
                                    const QString &packageName) const {
    const QString root = rootForScope(scope);
    if (root.isEmpty())
        return {};

    const Candidate candidate =
        resolveChildFromSingleRoot(root, scope, packageName, false);

    return {candidate.result, candidate.node};
}

WingLuauRequire::PackageLookup
WingLuauRequire::findInstalledPackage(const QString &packageName) const {
    const PackageLookup user = findPackageInScope(Scope::User, packageName);
    const PackageLookup system = findPackageInScope(Scope::System, packageName);
    const bool hasUser = user.result == Candidate::Result::Success;
    const bool hasSystem = system.result == Candidate::Result::Success;

    if (user.result == Candidate::Result::Ambiguous ||
        system.result == Candidate::Result::Ambiguous) {
        return {Candidate::Result::Ambiguous, {}};
    }
    if (hasUser && hasSystem) {
        return {Candidate::Result::Ambiguous, {}};
    }
    if (hasUser) {
        return user;
    }
    if (hasSystem) {
        return system;
    }
    return {};
}

QString WingLuauRequire::chunknameForNode(const Node &node) const {
    if (node.kind == NodeKind::Builtin) {
        const QString logical = node.builtinPath;
        if (logical.isEmpty()) {
            return {};
        }
        return sourcePrefixForNode(node) + logical;
    }

    if (node.scope == Scope::External) {
        return externalChunkname(node);
    }
    if (node.kind == NodeKind::ScopeRoot || node.packageName.isEmpty()) {
        return {};
    }

    const QString relative = relativePath(node.rootPath, node.physicalPath);
    QStringList parts = relative.split('/', Qt::SkipEmptyParts);
    if (!parts.isEmpty()) {
        // Handle both:
        //   foo/
        // and:
        //   foo.luau
        // as package root "foo".
        const QString firstLogical = stripModuleSuffix(parts.front());

        if (firstLogical.compare(node.packageName, Qt::CaseInsensitive) == 0) {
            parts.removeFirst();
        }
    }

    // A file module contributes its filename without .luau/.lua.
    if (node.kind == NodeKind::ModuleFile && !parts.isEmpty()) {
        parts.last() = stripModuleSuffix(parts.last());
    }

    const QString rest = parts.join('/');
    const QString logical = rest.isEmpty()
                                ? node.packageName
                                : node.packageName + QStringLiteral("/") + rest;

    return sourcePrefixForNode(node) + logical;
}

QString WingLuauRequire::loadPathForNode(const Node &node) const {
    if (node.kind == NodeKind::Builtin) {
        return QStringLiteral("builtin:") + node.builtinPath;
    }

    if (node.kind == NodeKind::ModuleFile ||
        node.kind == NodeKind::ExternalFile) {
        return normalizeExistingPath(node.physicalPath);
    }

    if (node.kind == NodeKind::ModuleDirectory ||
        node.kind == NodeKind::ExternalDirectory) {
        QString loadPath;
        if (directoryInitStatus(node.physicalPath, &loadPath) ==
            InitStatus::Present) {
            return loadPath;
        }
    }

    return {};
}

QString WingLuauRequire::cacheKeyForNode(const Node &node) const {
    if (node.kind == NodeKind::Builtin)
        return QStringLiteral("builtin:") + node.builtinPath;

    const QString loadPath = loadPathForNode(node);
    if (loadPath.isEmpty())
        return {};

    QString key = QStringLiteral("file:") + normalizeExistingPath(loadPath);
#ifdef Q_OS_WIN
    key = key.toLower();
#endif
    return key;
}

QString WingLuauRequire::configDirectory() const {
    switch (current_.kind) {
    case NodeKind::Builtin:
        // Builtins have no filesystem location and therefore no project
        // configuration scope of their own.
        return {};
    case NodeKind::ScopeRoot:
    case NodeKind::Directory:
    case NodeKind::ModuleDirectory:
    case NodeKind::ExternalDirectory:
        return current_.physicalPath;
    case NodeKind::ModuleFile:
    case NodeKind::ExternalFile:
        return QFileInfo(current_.physicalPath).absolutePath();
    }

    return {};
}

luarequire_ConfigStatus WingLuauRequire::configStatus() const {
    const QString dir = configDirectory();
    if (dir.isEmpty()) {
        return CONFIG_ABSENT;
    }

    const bool hasJson =
        QFileInfo(joinPath(dir, QStringLiteral(".luaurc"))).isFile();
    const bool hasLuau =
        QFileInfo(joinPath(dir, QStringLiteral(".config.luau"))).isFile();

    if (hasJson && hasLuau) {
        return CONFIG_AMBIGUOUS;
    }
    if (hasJson) {
        return CONFIG_PRESENT_JSON;
    }
    if (hasLuau) {
        return CONFIG_PRESENT_LUAU;
    }
    return CONFIG_ABSENT;
}

QString WingLuauRequire::configPath() const {
    const QString dir = configDirectory();
    if (dir.isEmpty()) {
        return {};
    }

    const QString json = joinPath(dir, QStringLiteral(".luaurc"));
    const QString luau = joinPath(dir, QStringLiteral(".config.luau"));
    const bool hasJson = QFileInfo(json).isFile();
    const bool hasLuau = QFileInfo(luau).isFile();

    if (hasJson && !hasLuau) {
        return json;
    }
    if (hasLuau && !hasJson) {
        return luau;
    }
    return {};
}

std::optional<WingLuauRequire::Node>
WingLuauRequire::mapAbsolutePathToNode(Scope scope, const QString &root,
                                       const QString &physical,
                                       bool explicitSource) const {
    if (root.isEmpty() || !pathEqualOrBelow(physical, root)) {
        return std::nullopt;
    }

    const QFileInfo info(physical);
    Node node;
    node.scope = scope;
    node.rootPath = root;
    node.explicitSource = explicitSource;

    const QString relative = relativePath(root, physical);
    const QStringList parts = relative.split('/', Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return makeScopeRootNode(scope, root, explicitSource);
    }

    if (info.isFile() && parts.size() == 1) {
        node.packageName = stripModuleSuffix(parts.front());
    } else {
        node.packageName = parts.front();
    }

    if (info.isDir()) {
        const InitStatus initStatus = directoryInitStatus(physical);
        if (initStatus == InitStatus::Ambiguous) {
            return std::nullopt;
        }

        node.kind = initStatus == InitStatus::Present
                        ? NodeKind::ModuleDirectory
                        : NodeKind::Directory;
        node.physicalPath = physical;
        return node;
    }

    if (!info.isFile()) {
        return std::nullopt;
    }

    const QString fileName = info.fileName();
    const QString parent = normalizeExistingPath(info.absolutePath());
    const bool isInitLuau =
        fileName.compare(QLatin1String("init.luau"), Qt::CaseInsensitive) == 0;
    const bool isInitLua =
        fileName.compare(QLatin1String("init.lua"), Qt::CaseInsensitive) == 0;

    if (isInitLuau || isInitLua) {
        node.kind = NodeKind::ModuleDirectory;
        node.physicalPath = parent;
        return node;
    }

    if (fileName.endsWith(QLatin1String(".luau"), Qt::CaseInsensitive) ||
        fileName.endsWith(QLatin1String(".lua"), Qt::CaseInsensitive)) {
        node.kind = NodeKind::ModuleFile;
        node.physicalPath = physical;
        return node;
    }

    return std::nullopt;
}

luarequire_NavigateResult
WingLuauRequire::resetToChunkname(const char *chunkname) {
    if (!chunkname) {
        return NAVIGATE_NOT_FOUND;
    }

    const QString key = QString::fromUtf8(chunkname);

    // Treat the REPL as a script rooted at the process working directory.
    // This enables the same filesystem and package requires as file scripts.
    if (key.startsWith('=')) {
        current_ = {};
        current_.scope = Scope::External;
        current_.kind = NodeKind::ExternalFile;
        current_.physicalPath = normalizeExistingPath(
            joinPath(QDir::currentPath(), QStringLiteral(".__wing_repl__")));
        return current_.physicalPath.isEmpty() ? NAVIGATE_NOT_FOUND
                                               : NAVIGATE_SUCCESS;
    }

    // Exact known chunk names take priority.
    //
    // This is essential for hidden-source names:
    //
    //     @json
    //
    // may actually represent a User, System, or Builtin node. The exact
    // Node recorded when the chunkname was generated is authoritative.
    const auto known = knownChunks_.constFind(key);

    if (known != knownChunks_.constEnd()) {
        current_ = known.value();
        return isModuleNode(current_) ? NAVIGATE_SUCCESS : NAVIGATE_NOT_FOUND;
    }

    // ScriptMachine loads file scripts with an @-prefixed filesystem path
    // (for example "@/home/user/project/main.luau"). These are not aliases:
    // restore the source node so both relative paths and alias config lookup
    // start from the requiring file's real directory.
    const QString sourcePath = key.sliced(1);
    if (QFileInfo(sourcePath).isAbsolute()) {
        const QByteArray absolutePath = sourcePath.toUtf8();
        return jumpToAbsolutePath(absolutePath.constData());
    }

    if (!key.startsWith('@')) {
        return NAVIGATE_NOT_FOUND;
    }

    const QStringList parts = key.sliced(1).split('/', Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return NAVIGATE_NOT_FOUND;
    }

    const QString first = parts.front();

    // ------------------------------------------------------------
    // Explicit Builtin:
    //
    //     @b/json
    // ------------------------------------------------------------
    if (first.compare(QLatin1String("b"), Qt::CaseInsensitive) == 0) {
        if (parts.size() < 2)
            return NAVIGATE_NOT_FOUND;

        const QString builtinPath = parts.sliced(1).join('/').toLower();

        if (!builtinModules_.contains(builtinPath)) {
            return NAVIGATE_NOT_FOUND;
        }

        current_ = {};
        current_.scope = Scope::External;
        current_.kind = NodeKind::Builtin;
        current_.builtinPath = builtinPath;

        return NAVIGATE_SUCCESS;
    }

    // ------------------------------------------------------------
    // Explicit System/User:
    //
    //     @s/foo/bar
    //     @u/foo/bar
    // ------------------------------------------------------------
    const bool isSysPak =
        first.compare(QLatin1String("s"), Qt::CaseInsensitive) == 0;
    const bool isUsrPak =
        first.compare(QLatin1String("u"), Qt::CaseInsensitive) == 0;
    if (isSysPak || isUsrPak) {
        const Scope scope = isSysPak ? Scope::System : Scope::User;

        if (scope == Scope::System && options_.systemRoot.isEmpty()) {
            return NAVIGATE_NOT_FOUND;
        }

        if (scope == Scope::User && options_.userRoot.isEmpty()) {
            return NAVIGATE_NOT_FOUND;
        }

        if (parts.size() < 2)
            return NAVIGATE_NOT_FOUND;

        current_ = makeScopeRootNode(scope, {}, true);

        for (int i = 1; i < parts.size(); ++i) {
            const Candidate candidate = resolveChild(parts.at(i));

            if (candidate.result == Candidate::Result::Ambiguous) {
                return NAVIGATE_AMBIGUOUS;
            }

            if (candidate.result != Candidate::Result::Success) {
                return NAVIGATE_NOT_FOUND;
            }

            current_ = candidate.node;
        }

        return isModuleNode(current_) ? NAVIGATE_SUCCESS : NAVIGATE_NOT_FOUND;
    }

    // ------------------------------------------------------------
    // Hidden-source shorthand, retained for compatibility with previously
    // generated chunknames:
    //
    //     @json
    //     @json/encoder
    //
    // The source can only be reconstructed safely if exactly one of
    // Builtin/User/System owns the first alias component.
    // ------------------------------------------------------------

    const AliasSources sources = findAliasSources(first);
    if (sources.ambiguous()) {
        return NAVIGATE_AMBIGUOUS;
    }

    if (sources.count() != 1) {
        return NAVIGATE_NOT_FOUND;
    }

    // Builtin shorthand:
    //
    //     @json
    //     @json/encoder
    //
    // uses the full logical builtin path. Unlike filesystem modules,
    // the complete builtin path must be registered.
    if (sources.builtin) {
        const QString builtinPath = parts.join('/').toLower();
        if (!builtinModules_.contains(builtinPath)) {
            return NAVIGATE_NOT_FOUND;
        }

        current_ = {};
        current_.scope = Scope::External;
        current_.kind = NodeKind::Builtin;
        current_.builtinPath = builtinPath;

        return NAVIGATE_SUCCESS;
    }

    // User/System shorthand.
    current_ = sources.hasUser() ? sources.user.node : sources.system.node;

    for (int i = 1; i < parts.size(); ++i) {
        const Candidate candidate = resolveChild(parts.at(i));

        if (candidate.result == Candidate::Result::Ambiguous) {
            return NAVIGATE_AMBIGUOUS;
        }

        if (candidate.result != Candidate::Result::Success) {
            return NAVIGATE_NOT_FOUND;
        }

        current_ = candidate.node;
    }

    // A shorthand chunkname must resolve to an actual module, never merely
    // to a directory/namespace.
    return isModuleNode(current_) ? NAVIGATE_SUCCESS : NAVIGATE_NOT_FOUND;
}

luarequire_NavigateResult
WingLuauRequire::jumpToAbsolutePath(const char *path) {
    if (!path) {
        return NAVIGATE_NOT_FOUND;
    }

    const auto physical = normalizeExistingPath(QString::fromUtf8(path));
    if (physical.isEmpty()) {
        return NAVIGATE_NOT_FOUND;
    }

    std::optional<Node> systemNode;
    std::optional<Node> userNode;

    if (!options_.systemRoot.isEmpty()) {
        systemNode = mapAbsolutePathToNode(Scope::System, options_.systemRoot,
                                           physical, false);
    }

    if (!options_.userRoot.isEmpty()) {
        userNode = mapAbsolutePathToNode(Scope::User, options_.userRoot,
                                         physical, false);
    }

    // A deliberately overlapping systemRoot/userRoot is ambiguous for an
    // absolute alias target. Do not silently choose one scope.
    if (systemNode && userNode) {
        return NAVIGATE_AMBIGUOUS;
    }

    if (systemNode) {
        current_ = *systemNode;
        return NAVIGATE_SUCCESS;
    }

    if (userNode) {
        current_ = *userNode;
        return NAVIGATE_SUCCESS;
    }

    QFileInfo info(physical);
    if (info.isFile()) {
        auto filename = info.fileName();
        if (!filename.endsWith(QLatin1String(".luau"), Qt::CaseInsensitive) &&
            !filename.endsWith(QLatin1String(".lua"), Qt::CaseInsensitive)) {
            return NAVIGATE_NOT_FOUND;
        }

        current_ = {};
        current_.scope = Scope::External;
        current_.kind = NodeKind::ExternalFile;
        current_.physicalPath = physical;
        return NAVIGATE_SUCCESS;
    }

    if (info.isDir()) {
        const InitStatus initStatus = directoryInitStatus(physical);
        if (initStatus == InitStatus::Ambiguous) {
            return NAVIGATE_AMBIGUOUS;
        }

        current_ = {};
        current_.scope = Scope::External;
        current_.kind = NodeKind::ExternalDirectory;
        current_.physicalPath = physical;
        return NAVIGATE_SUCCESS;
    }

    return NAVIGATE_NOT_FOUND;
}

int WingLuauRequire::cbRequireDispatch(lua_State *L) {
    auto *self = static_cast<WingLuauRequire *>(
        lua_tolightuserdata(L, lua_upvalueindex(2)));
    const bool isProxyRequire = lua_toboolean(L, lua_upvalueindex(3));

    if (!self || !L) {
        luaL_error(L, "invalid WingLuauRequire context");
    }

    lua_settop(L, isProxyRequire ? 2 : 1);

    const char *rawPath = luaL_checkstring(L, 1);
    const QString requested =
        QDir::fromNativeSeparators(QString::fromUtf8(rawPath));
    const QString requirerChunkname =
        isProxyRequire ? QString::fromUtf8(luaL_checkstring(L, 2))
                       : callerChunkname(L);
    const bool relativePath = requested.startsWith(QLatin1String("./")) ||
                              requested.startsWith(QLatin1String("../"));
    if (relativePath && requirerChunkname.startsWith('=')) {
        luaL_error(L, "relative require paths are not supported from the REPL");
    }

    // Pass the path and proxy chunkname unchanged to Luau.Require. In
    // particular, @package paths go through configured aliases and the host
    // fallback; only @b/ paths use Luau's registered-module fast path.
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_insert(L, 1);
    lua_call(L, isProxyRequire ? 2 : 1, 1);
    return 1;
}

bool WingLuauRequire::cbIsRequireAllowed(lua_State *, void *ctx,
                                         const char *requirerChunkname) {
    auto *self = static_cast<WingLuauRequire *>(ctx);
    if (!self || !requirerChunkname) {
        return false;
    }

    const QString source = QString::fromUtf8(requirerChunkname);
    return source.startsWith('@') || source.startsWith('=');
}

luarequire_NavigateResult
WingLuauRequire::cbReset(lua_State *, void *ctx,
                         const char *requirerChunkname) {
    auto *self = static_cast<WingLuauRequire *>(ctx);
    return self ? self->resetToChunkname(requirerChunkname)
                : NAVIGATE_NOT_FOUND;
}

luarequire_NavigateResult WingLuauRequire::cbJumpToAlias(lua_State *, void *ctx,
                                                         const char *path) {
    auto *self = static_cast<WingLuauRequire *>(ctx);
    return self ? self->jumpToAbsolutePath(path) : NAVIGATE_NOT_FOUND;
}

luarequire_NavigateResult
WingLuauRequire::cbToAliasOverride(lua_State *, void *ctx,
                                   const char *aliasUnprefixed) {
    auto *self = static_cast<WingLuauRequire *>(ctx);
    if (!self || !aliasUnprefixed) {
        return NAVIGATE_NOT_FOUND;
    }

    const QString alias = QString::fromUtf8(aliasUnprefixed);

    // @s and @u are reserved host aliases and intentionally override
    // project configuration aliases.
    if (alias.compare(QLatin1String("s"), Qt::CaseInsensitive) == 0) {
        if (self->options_.systemRoot.isEmpty()) {
            return NAVIGATE_NOT_FOUND;
        }
        self->current_ = makeScopeRootNode(Scope::System, {}, true);
        return NAVIGATE_SUCCESS;
    }

    if (alias.compare(QLatin1String("u"), Qt::CaseInsensitive) == 0) {
        if (self->options_.userRoot.isEmpty()) {
            return NAVIGATE_NOT_FOUND;
        }
        self->current_ = makeScopeRootNode(Scope::User, {}, true);
        return NAVIGATE_SUCCESS;
    }

    // Registered @b/<module> values are returned by Luau's fast path. If a
    // builtin path is missing, navigate from the builtin namespace so Luau
    // reports the missing child component instead of an invalid @b alias.
    if (alias.compare(QLatin1String("b"), Qt::CaseInsensitive) == 0) {
        self->current_ = {};
        self->current_.scope = Scope::External;
        self->current_.kind = NodeKind::Builtin;
        return NAVIGATE_SUCCESS;
    }

    return NAVIGATE_NOT_FOUND;
}

luarequire_NavigateResult
WingLuauRequire::cbToAliasFallback(lua_State *, void *ctx,
                                   const char *aliasUnprefixed) {
    auto *self = static_cast<WingLuauRequire *>(ctx);

    if (!self || !aliasUnprefixed) {
        return NAVIGATE_NOT_FOUND;
    }

    const QString packageName = QString::fromUtf8(aliasUnprefixed);

    if (!isSafeModulePath(packageName) || packageName.contains('/')) {
        return NAVIGATE_NOT_FOUND;
    }

    // @b is reserved for explicitly registered builtins and must never
    // fall through to a package named "b". @s and @u are handled by
    // to_alias_override().
    if (packageName.compare(QLatin1String("b"), Qt::CaseInsensitive) == 0 ||
        packageName.compare(QLatin1String("s"), Qt::CaseInsensitive) == 0 ||
        packageName.compare(QLatin1String("u"), Qt::CaseInsensitive) == 0) {
        return NAVIGATE_NOT_FOUND;
    }

    // Installed-package shorthand is deliberately the final fallback so
    // .luaurc / .config.luau aliases keep precedence.
    //
    // At this point project configuration did not resolve the alias.
    // Compare every Host source using the same shared logic used by
    // chunknameForNode() and resetToChunkname().
    const AliasSources sources = self->findAliasSources(packageName);
    if (sources.ambiguous()) {
        return NAVIGATE_AMBIGUOUS;
    }

    switch (sources.count()) {
    case 0:
        return NAVIGATE_NOT_FOUND;
    case 1:
        break;
    default:
        return NAVIGATE_AMBIGUOUS;
    }

    if (sources.builtin) {
        // Builtin shorthand is a logical registry node, not a filesystem root.
        self->current_ = {};
        self->current_.scope = Scope::External;
        self->current_.kind = NodeKind::Builtin;
        self->current_.builtinPath = packageName;
        return NAVIGATE_SUCCESS;
    }

    self->current_ =
        sources.hasUser() ? sources.user.node : sources.system.node;

    return NAVIGATE_SUCCESS;
}

luarequire_NavigateResult WingLuauRequire::cbToParent(lua_State *, void *ctx) {
    auto *self = static_cast<WingLuauRequire *>(ctx);
    if (!self) {
        return NAVIGATE_NOT_FOUND;
    }

    if (self->current_.kind == NodeKind::ScopeRoot) {
        return NAVIGATE_NOT_FOUND;
    }

    if (self->current_.kind == NodeKind::Builtin) {
        const int slash = self->current_.builtinPath.lastIndexOf('/');
        if (slash < 0) {
            return NAVIGATE_NOT_FOUND;
        }

        self->current_.builtinPath = self->current_.builtinPath.left(slash);
        return NAVIGATE_SUCCESS;
    }

    if (self->current_.physicalPath.isEmpty()) {
        return NAVIGATE_NOT_FOUND;
    }

    const QString parent = normalizeExistingPath(
        QFileInfo(self->current_.physicalPath).absolutePath());
    if (parent.isEmpty() || parent == self->current_.physicalPath) {
        return NAVIGATE_NOT_FOUND;
    }

    // Stop at a package-store boundary instead of leaking into the parent
    // directory that happens to contain the store.
    if (!self->current_.rootPath.isEmpty() &&
        parent == self->current_.rootPath) {
        self->current_ = makeScopeRootNode(self->current_.scope, parent,
                                           self->current_.explicitSource);
        return NAVIGATE_SUCCESS;
    }

    self->current_.physicalPath = parent;
    if (self->current_.scope == Scope::External) {
        self->current_.kind = NodeKind::ExternalDirectory;
    } else {
        self->current_.kind = NodeKind::Directory;
    }
    return NAVIGATE_SUCCESS;
}

luarequire_NavigateResult WingLuauRequire::cbToChild(lua_State *, void *ctx,
                                                     const char *name) {
    auto *self = static_cast<WingLuauRequire *>(ctx);
    if (!self || !name) {
        return NAVIGATE_NOT_FOUND;
    }

    const QString component = QString::fromUtf8(name);
    if (component.isEmpty() || component == QLatin1String(".")) {
        return NAVIGATE_SUCCESS;
    }

    if (component == QLatin1String("..")) {
        return WingLuauRequire::cbToParent(nullptr, ctx);
    }

    const Candidate candidate = self->resolveChild(component);
    if (candidate.result == Candidate::Result::Ambiguous) {
        return NAVIGATE_AMBIGUOUS;
    }
    if (candidate.result != Candidate::Result::Success) {
        return NAVIGATE_NOT_FOUND;
    }

    self->current_ = candidate.node;
    return NAVIGATE_SUCCESS;
}

bool WingLuauRequire::cbIsModulePresent(lua_State *, void *ctx) {
    auto *self = static_cast<WingLuauRequire *>(ctx);
    if (!self) {
        return false;
    }
    switch (self->current_.kind) {
    case NodeKind::Builtin:
        return self->builtinModules_.contains(self->current_.builtinPath);
    case NodeKind::ModuleFile:
    case NodeKind::ExternalFile:
        return QFileInfo(self->current_.physicalPath).isFile();
    case NodeKind::ModuleDirectory:
    case NodeKind::ExternalDirectory:
        return self->directoryInitStatus(self->current_.physicalPath) ==
               InitStatus::Present;
    case NodeKind::ScopeRoot:
    case NodeKind::Directory:
        return false;
    }
    return false;
}

luarequire_WriteResult WingLuauRequire::writeString(const QString &value,
                                                    char *buffer,
                                                    size_t bufferSize,
                                                    size_t *sizeOut) {
    if (!buffer || !sizeOut) {
        return WRITE_FAILURE;
    }

    const QByteArray utf8 = value.toUtf8();
    const auto total = utf8.size();
    const size_t required = total + 1;

    if (bufferSize < required) {
        *sizeOut = required;
        return WRITE_BUFFER_TOO_SMALL;
    }

    memcpy(buffer, utf8.constData(), total);
    buffer[total] = '\0';
    *sizeOut = required;
    return WRITE_SUCCESS;
}

luarequire_WriteResult WingLuauRequire::cbGetChunkname(lua_State *, void *ctx,
                                                       char *buffer,
                                                       size_t bufferSize,
                                                       size_t *sizeOut) {
    auto *self = static_cast<WingLuauRequire *>(ctx);
    if (!self || !cbIsModulePresent(nullptr, self)) {
        return WRITE_FAILURE;
    }

    const QString chunkname = self->chunknameForNode(self->current_);
    if (chunkname.isEmpty()) {
        return WRITE_FAILURE;
    }

    self->knownChunks_.insert(chunkname, self->current_);
    return writeString(chunkname, buffer, bufferSize, sizeOut);
}

luarequire_WriteResult WingLuauRequire::cbGetLoadname(lua_State *, void *ctx,
                                                      char *buffer,
                                                      size_t bufferSize,
                                                      size_t *sizeOut) {
    auto *self = static_cast<WingLuauRequire *>(ctx);
    if (!self || !cbIsModulePresent(nullptr, self)) {
        return WRITE_FAILURE;
    }

    const QString loadName = self->loadPathForNode(self->current_);
    if (loadName.isEmpty()) {
        return WRITE_FAILURE;
    }

    return writeString(loadName, buffer, bufferSize, sizeOut);
}

luarequire_WriteResult WingLuauRequire::cbGetCacheKey(lua_State *, void *ctx,
                                                      char *buffer,
                                                      size_t bufferSize,
                                                      size_t *sizeOut) {
    auto *self = static_cast<WingLuauRequire *>(ctx);
    if (!self || !cbIsModulePresent(nullptr, self)) {
        return WRITE_FAILURE;
    }

    const QString cacheKey = self->cacheKeyForNode(self->current_);
    if (cacheKey.isEmpty()) {
        return WRITE_FAILURE;
    }

    return writeString(cacheKey, buffer, bufferSize, sizeOut);
}

luarequire_ConfigStatus WingLuauRequire::cbGetConfigStatus(lua_State *,
                                                           void *ctx) {
    auto *self = static_cast<WingLuauRequire *>(ctx);
    return self ? self->configStatus() : CONFIG_ABSENT;
}

luarequire_WriteResult WingLuauRequire::cbGetConfig(lua_State *, void *ctx,
                                                    char *buffer,
                                                    size_t bufferSize,
                                                    size_t *sizeOut) {
    auto *self = static_cast<WingLuauRequire *>(ctx);
    if (!self || !buffer || !sizeOut) {
        return WRITE_FAILURE;
    }

    const QString path = self->configPath();
    if (path.isEmpty()) {
        return WRITE_FAILURE;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return WRITE_FAILURE;
    }

    if (self->options_.maxConfigBytes >= 0 &&
        file.size() > self->options_.maxConfigBytes) {
        return WRITE_FAILURE;
    }

    const auto contentSize = file.size();
    if (self->options_.maxConfigBytes >= 0 &&
        contentSize > self->options_.maxConfigBytes) {
        return WRITE_FAILURE;
    }

    const size_t required = contentSize + 1;
    if (bufferSize < required) {
        *sizeOut = required;
        return WRITE_BUFFER_TOO_SMALL;
    }

    file.read(buffer, contentSize);
    buffer[contentSize] = '\0';
    *sizeOut = required;
    return WRITE_SUCCESS;
}

int WingLuauRequire::cbGetLuauConfigTimeout(lua_State *, void *ctx) {
    auto *self = static_cast<WingLuauRequire *>(ctx);
    return self ? self->options_.luauConfigTimeoutMs : 2000;
}

int WingLuauRequire::cbLoad(lua_State *L, void *ctx, const char *,
                            const char *chunkname, const char *loadname) {
    auto *self = static_cast<WingLuauRequire *>(ctx);
    if (!self || !loadname || !chunkname) {
        luaL_error(L, "invalid require loader context");
    }

    const QString loadName = QString::fromUtf8(loadname);
    if (loadName.startsWith(QStringLiteral("builtin:"), Qt::CaseInsensitive)) {
        const QString builtinPath = loadName.sliced(8);
        if (!self->builtinModules_.contains(builtinPath)) {
            luaL_error(L, "builtin module '%s' is not registered", loadname);
        }

        lua_getfield(L, LUA_REGISTRYINDEX, builtinRegistryKey());
        lua_getfield(L, -1, builtinPath.toUtf8().constData());

        if (lua_isnil(L, -1)) {
            lua_pop(L, 2);
            luaL_error(L, "builtin module '%s' is not available", loadname);
        }

        lua_remove(L, -2);
        return 1;
    }

    // module needs to run in a new thread, isolated from the rest
    // note: we create ML on main thread so that it doesn't inherit
    // environment of L
    lua_State *GL = lua_mainthread(L);
    lua_State *ML = lua_newthread(GL);
    lua_xmove(GL, L, 1);

    // new thread needs to have the globals sandboxed
    luaL_sandboxthread(ML);

    bool hadContents = false;
    int loadStatus = LUA_OK;

    // Handle C++ RAII objects in a scope which doesn't cause a Luau error
    {
        const auto loadFile = QString::fromUtf8(loadname);
        QFile file(loadFile);
        if (file.size() > DEFAULT_SCRIPT_FILE_SIZE_LIMIT) {
            luaL_error(L, "excessive module file size");
        }
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            const QByteArray source = file.readAll();

            auto *self = static_cast<WingLuauRequire *>(ctx);
            if (self) {
                const auto &cb = self->options_.onLuauFileLoading;
                if (cb) {
                    auto r = cb(L, ML, loadFile, source);
                    if (!r) {
                        luaL_error(L, "failed to load module '%s'", loadname);
                    }
                }
            }

            hadContents = true;
            const std::string sourceString(source.constData(), source.size());
            const auto bytecode = Luau::compile(sourceString);
            loadStatus =
                luau_load(ML, chunkname, bytecode.data(), bytecode.size(), 0);
        }
    }

    if (!hadContents) {
        luaL_error(L, "could not read file '%s'", loadname);
    }
    if (loadStatus != LUA_OK) {
        if (lua_isstring(ML, -1)) {
            luaL_error(L, "error loading module: %s", lua_tostring(ML, -1));
        }
        luaL_error(L, "unknown error loading module");
    }

    if (FFlag::LuauCyclicRequireShortCircuit && lua_usesexport(ML, -1) != 0) {
        luarequire_createplaceholder(L);
    }

    const int resumeStatus = lua_resume(ML, L, 0);

    if (resumeStatus == LUA_OK) {
        if (lua_gettop(ML) != 1)
            luaL_error(L, "module must return a single value");
    } else if (resumeStatus == LUA_YIELD) {
        luaL_error(L, "module can not yield");
    } else if (!lua_isstring(ML, -1)) {
        luaL_error(L, "unknown error while running module");
    } else {
        luaL_error(L, "error while running module: %s", lua_tostring(ML, -1));
    }

    // add ML result to L stack
    lua_xmove(ML, L, 1);

    // remove ML thread from L stack
    lua_remove(L, -2);

    // added one value to L stack: module result
    return 1;
}

void WingLuauRequire::configInit(luarequire_Configuration *config) {
    if (!config)
        return;

    std::memset(config, 0, sizeof(luarequire_Configuration));
    config->is_require_allowed = &WingLuauRequire::cbIsRequireAllowed;
    config->reset = &WingLuauRequire::cbReset;
    config->jump_to_alias = &WingLuauRequire::cbJumpToAlias;
    config->to_alias_override = &WingLuauRequire::cbToAliasOverride;
    config->to_alias_fallback = &WingLuauRequire::cbToAliasFallback;
    config->to_parent = &WingLuauRequire::cbToParent;
    config->to_child = &WingLuauRequire::cbToChild;
    config->is_module_present = &WingLuauRequire::cbIsModulePresent;
    config->get_chunkname = &WingLuauRequire::cbGetChunkname;
    config->get_loadname = &WingLuauRequire::cbGetLoadname;
    config->get_cache_key = &WingLuauRequire::cbGetCacheKey;
    config->get_config_status = &WingLuauRequire::cbGetConfigStatus;
    config->get_config = &WingLuauRequire::cbGetConfig;
    config->get_luau_config_timeout = &WingLuauRequire::cbGetLuauConfigTimeout;
    config->load = &WingLuauRequire::cbLoad;
}
