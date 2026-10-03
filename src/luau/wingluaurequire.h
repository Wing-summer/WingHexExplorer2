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

#ifndef WING_LUAU_REQUIRE_H
#define WING_LUAU_REQUIRE_H

// Design:
//   @<package>/...           -> normal Luau alias lookup, followed by host
//                              package fallback when no configured alias exists
//   @b/<module>              -> canonical explicit builtin spelling
//   @s/<package>/...         -> explicit system package and generated chunkname
//   @u/<package>/...         -> explicit user package and generated chunkname
//
// Package aliases are resolved from .luaurc/.config.luau before
// to_alias_fallback() checks builtin, user, and system package sources. If more
// than one source provides the package, lookup is ambiguous. Unprefixed paths
// are left to Luau.Require to reject; @s and @u select a package store, while
// @b selects a registered builtin directly.
//
// REPL input (=stdin) may use builtins and ordinary package lookup, but
// relative filesystem requires such as ./module or ../module are not allowed.
//
// The module loader itself follows the official Luau CLI structure:
// create an isolated thread, optionally sandbox it, compile the source,
// luau_load, resume it, and return exactly one module value.

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>

#include <cstring>
#include <functional>
#include <optional>

#include "Luau/Common.h"
#include "Luau/Require.h"

#include "lua.h"

LUAU_FASTFLAG(LuauCyclicRequireShortCircuit)

constexpr auto DEFAULT_SCRIPT_FILE_SIZE_LIMIT = 1024 * 1024;

class WingLuauRequire final {
public:
    struct Options {
        // The system package-store root. Packages are direct children.
        // Example:
        //   /opt/wing/packages/<package>/...
        QString systemRoot;

        // The user package-store root. Packages are direct children.
        // Example:
        //   ~/.wing/packages/<package>/...
        QString userRoot;

        // Runtime execution timeout for .config.luau.
        int luauConfigTimeoutMs = 2000;

        // Maximum configuration file size accepted by get_config(). This is a
        // host-side safety limit; Luau still applies its own execution timeout.
        qsizetype maxConfigBytes = DEFAULT_SCRIPT_FILE_SIZE_LIMIT;

        // called within cbLoad
        std::function<bool(lua_State *L, lua_State *ML, const QString &path,
                           const QByteArray &source)>
            onLuauFileLoading;

        inline Options() {}
    };

    explicit WingLuauRequire();
    ~WingLuauRequire() = default;

public:
    struct FileModuleResolution {
        enum class Kind {
            NotFound,
            File,
            ModuleDirectory,
            NamespaceDirectory,
            Ambiguous,
        };

        Kind kind = Kind::NotFound;
        QString path;
    };

    static FileModuleResolution resolveFileModule(const QString &basePath,
                                                  const QString &requestedPath);

    bool initRequire(lua_State *L, Options options);

    // Installs the native Luau require() into the global environment.
    //
    // The WingLuauRequire object must outlive the installed Lua closure
    // boecause the closure stores this object as a light-userdata context
    // pointer.
    //
    // A small adapter closure is installed in front of Luau's native require.
    // The native require implementation remains responsible for navigation,
    // configuration parsing, cache handling, and module execution.
    void installRequire();

    // Optional proxyrequire(path, chunkname) support.
    void installProxyRequire(const char *globalName = "proxyrequire");

    // Register an already-created Lua value as a builtin module.
    // The final canonical path is always under @b/.
    //
    // Example:
    //   lua_newtable(L);
    //   ... fill table ...
    //   requireSystem.registerBuiltinValue("json", -1);
    //
    // require("@b/json") selects this builtin directly. require("@json") uses
    // normal alias/package lookup and may be ambiguous with another source.
    bool registerBuiltinValue(const QString &moduleName, int valueIndex = -1);

    // Clears the entire native require cache.
    //
    // Builtin modules registered through luarequire_registermodule() are not
    // stored in the normal require cache and are therefore unaffected.
    void clearRequireCache();

    // Clears one normal require cache entry.
    void clearRequireCacheEntry(const char *cacheKey);

    const Options &options() const;

private:
    enum class Scope {
        System,
        User,
        External,
    };

    enum class NodeKind {
        ScopeRoot,
        Builtin,
        Directory,
        ModuleFile,
        ModuleDirectory,
        ExternalFile,
        ExternalDirectory,
    };

    struct Node {
        Scope scope = Scope::External;
        NodeKind kind = NodeKind::Directory;

        // Physical file/directory path. Empty for synthetic system/user
        // scope roots created by @s / @u before their first child is resolved.
        QString physicalPath;

        // The package-store root which bounds this node. Empty for external
        // nodes and synthetic roots. There is exactly one root per scope.
        QString rootPath;

        // Installed-package logical name, for example "core" in core/math.
        // Empty for external modules.
        QString packageName;

        // Logical builtin module path without the @b/ prefix, for example
        // "json" for @json or "qt/core" for @qt/core.
        QString builtinPath;

        // True when the node was entered through the explicit @s or @u host
        // alias. Normal package fallback keeps this false; chunknames still
        // retain the actual source prefix to remain reversible.
        bool explicitSource = false;
    };

    struct Candidate {
        enum class Result {
            NotFound,
            Success,
            Ambiguous,
        };

        Result result = Result::NotFound;
        Node node;
    };

    struct PackageLookup {
        Candidate::Result result = Candidate::Result::NotFound;
        Node node;
    };

    struct AliasSources {
        bool builtin = false;
        PackageLookup user;
        PackageLookup system;

        inline bool ambiguous() const {
            return user.result == Candidate::Result::Ambiguous ||
                   system.result == Candidate::Result::Ambiguous;
        }

        inline bool hasUser() const {
            return user.result == Candidate::Result::Success;
        }

        inline bool hasSystem() const {
            return system.result == Candidate::Result::Success;
        }

        inline int count() const {
            if (ambiguous()) {
                return 2;
            }

            return int(builtin) + int(hasUser()) + int(hasSystem());
        }
    };

    enum class InitStatus {
        None,
        Present,
        Ambiguous,
    };

    lua_State *L_ = nullptr;
    Options options_;

    // NavigationContext is stateful. A single require() call mutates this
    // object while Navigator walks the path.
    Node current_;

    // Chunk names generated by this adapter are also cached here so reset()
    // can restore a module exactly even for externally supplied aliases.
    QHash<QString, Node> knownChunks_;

    // Builtins are registered with Luau under @b/<name>. These sets mirror the
    // registration names locally so shorthand and ordinary package lookups can
    // detect builtin namespaces without relying on Luau internals.
    QSet<QString> builtinModules_;
    QSet<QString> builtinNamespaces_;

    inline constexpr static const char *builtinRegistryKey() {
        return "_WING_LUAU_REQUIRE_BUILTINS";
    }

    static QString callerChunkname(lua_State *L);

    static QString normalizeRoot(const QString &input);

    static QString normalizeExistingPath(const QString &input);

    static QString joinPath(const QString &base, const QString &name);

    static bool pathEqualOrBelow(const QString &path, const QString &root);

    static QString relativePath(const QString &root, const QString &path);

    static QString stripModuleSuffix(QString path);

    static bool isFileModuleCandidate(const QFileInfo &info);

    static bool isSafeModulePath(const QString &input);

    static QString normalizeBuiltinName(const QString &input);

    static QString builtinChunknameFor(const QString &moduleName);

    static QString externalChunkname(const Node &node);

    static Node makeScopeRootNode(Scope scope, const QString &physicalRoot = {},
                                  bool explicitSource = false);

    bool isModuleNode(const Node &node) const;

    AliasSources findAliasSources(const QString &alias) const;

    QString sourcePrefixForNode(const Node &node) const;

    static InitStatus directoryInitStatus(const QString &directory,
                                          QString *loadPath = nullptr);

    Candidate makeCandidateFromBase(const QString &base, Scope scope,
                                    const QString &rootPath,
                                    const QString &component,
                                    const QString &packageName = {},
                                    bool explicitSource = false) const;

    Candidate resolveChildFromSingleRoot(const QString &root, Scope scope,
                                         const QString &component,
                                         bool explicitSource) const;

    Candidate resolveChild(const QString &component) const;

    QString rootForScope(Scope scope) const;

    PackageLookup findPackageInScope(Scope scope,
                                     const QString &packageName) const;

    PackageLookup findInstalledPackage(const QString &packageName) const;

    QString chunknameForNode(const Node &node) const;

    QString loadPathForNode(const Node &node) const;

    QString cacheKeyForNode(const Node &node) const;

    QString configDirectory() const;

    luarequire_ConfigStatus configStatus() const;

    QString configPath() const;

    // Maps an absolute path to the same logical Node representation used by
    // normal navigation. This is used for absolute alias targets supplied by
    // .luaurc or .config.luau.
    std::optional<Node> mapAbsolutePathToNode(Scope scope, const QString &root,
                                              const QString &physical,
                                              bool explicitSource) const;

    luarequire_NavigateResult resetToChunkname(const char *chunkname);

    luarequire_NavigateResult jumpToAbsolutePath(const char *path);

    // --- Runtime callbacks -------------------------------------------------

    static int cbRequireDispatch(lua_State *L);

    static bool cbIsRequireAllowed(lua_State *, void *ctx,
                                   const char *requirerChunkname);

    static luarequire_NavigateResult cbReset(lua_State *, void *ctx,
                                             const char *requirerChunkname);

    static luarequire_NavigateResult cbJumpToAlias(lua_State *, void *ctx,
                                                   const char *path);

    static luarequire_NavigateResult
    cbToAliasOverride(lua_State *, void *ctx, const char *aliasUnprefixed);

    static luarequire_NavigateResult
    cbToAliasFallback(lua_State *, void *ctx, const char *aliasUnprefixed);

    static luarequire_NavigateResult cbToParent(lua_State *, void *ctx);

    static luarequire_NavigateResult cbToChild(lua_State *, void *ctx,
                                               const char *name);

    static bool cbIsModulePresent(lua_State *, void *ctx);

    static luarequire_WriteResult writeString(const QString &value,
                                              char *buffer, size_t bufferSize,
                                              size_t *sizeOut);

    static luarequire_WriteResult cbGetChunkname(lua_State *, void *ctx,
                                                 char *buffer,
                                                 size_t bufferSize,
                                                 size_t *sizeOut);

    static luarequire_WriteResult cbGetLoadname(lua_State *, void *ctx,
                                                char *buffer, size_t bufferSize,
                                                size_t *sizeOut);

    static luarequire_WriteResult cbGetCacheKey(lua_State *, void *ctx,
                                                char *buffer, size_t bufferSize,
                                                size_t *sizeOut);

    static luarequire_ConfigStatus cbGetConfigStatus(lua_State *, void *ctx);

    static luarequire_WriteResult cbGetConfig(lua_State *, void *ctx,
                                              char *buffer, size_t bufferSize,
                                              size_t *sizeOut);

    static int cbGetLuauConfigTimeout(lua_State *, void *ctx);

    static int cbLoad(lua_State *L, void *ctx, const char *,
                      const char *chunkname, const char *loadname);

    static void configInit(luarequire_Configuration *config);

private:
    Q_DISABLE_COPY_MOVE(WingLuauRequire)
};

#endif // WING_LUAU_REQUIRE_H
