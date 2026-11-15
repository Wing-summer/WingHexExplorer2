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

#include "scriptpluginruntime.h"

#include <utility>

#include <QApplication>
#include <QScopeGuard>
#include <QScopedValueRollback>
#include <QThread>
#include <QTimer>

#include "qtluau/callback.hpp"
#include "qtluau/value_conversion.hpp"

#include "class/plugin/pluginsystem.h"
#include "class/settings/languagemanager.h"
#include "luau/runtime/scriptchunk.h"
#include "luau/runtime/scriptmachinebindings.h"
#include "plugin_protocol.hpp"
#include "qt_extensions.hpp"

namespace {
constexpr const char *runtimeKey = "WingHex.ScriptPluginRuntime";

// Protected lookup also contains user-provided __index implementations.
int lookup(lua_State *state) {
    lua_pushvalue(state, 2);
    lua_gettable(state, 1);
    return 1;
}
} // namespace

ScriptPluginRuntime::ScriptPluginRuntime(
    std::shared_ptr<ScriptPluginPackage> package,
    WingHex::IWingPluginBase *owner)
    : m_package(std::move(package)), m_owner(owner),
      m_resources(std::make_unique<QObject>()),
      m_io(std::make_shared<PluginIOContext>()) {
    m_io->devicePlugin = devicePlugin();
    m_threadData.mode = ScriptMachine::Background;
}

ScriptPluginRuntime::~ScriptPluginRuntime() { shutdown(); }

bool ScriptPluginRuntime::devicePlugin() const {
    return m_package->kind() == ScriptPluginPackage::Kind::Device;
}

ScriptPluginRuntime *ScriptPluginRuntime::self(lua_State *state) {
    lua_getfield(state, LUA_REGISTRYINDEX, runtimeKey);
    auto *result =
        static_cast<ScriptPluginRuntime *>(lua_touserdata(state, -1));
    lua_pop(state, 1);
    return result;
}

ScriptPluginResult ScriptPluginRuntime::prepare() {
    QString error;
    if (!qApp || QThread::currentThread() != qApp->thread() || m_state) {
        error = QStringLiteral("Invalid plugin runtime initialization");
        return {ScriptPluginError::Initialization, error, {}};
    }
    m_state = luaL_newstate();
    if (!m_state) {
        error = QStringLiteral("Cannot create plugin VM");
        return {ScriptPluginError::Initialization, error, {}};
    }
    m_active = true;
    m_translator = std::make_unique<QTranslator>();
    if (!m_translator->load(
            LanguageManager::instance().defaultLocale(),
            m_package->manifest().value(QStringLiteral("Id")).toString(),
            QStringLiteral("_"),
            QDir(rootPath()).filePath(QStringLiteral("translations")))) {
        m_translator.reset();
    }
    lua_pushlightuserdata(m_state, this);
    lua_setfield(m_state, LUA_REGISTRYINDEX, runtimeKey);
    lua_setthreaddata(m_state, &m_threadData);
    luaL_openlibs(m_state);
    luabridge::registerMainThread(m_state);
    qtluau::setLuaContextResolver(
        m_state, [](lua_State *state) -> const void * { return self(state); });
    WingLuauRequire::Options options;
    options.readRoot = m_package->rootPath();
    options.onLuauFileLoading = [this](lua_State *, lua_State *thread,
                                       const QString &, const QByteArray &,
                                       ScriptChunkFormat) {
        lua_setthreaddata(thread, &m_threadData);
        return m_active;
    };
    if (!m_require.initRequire(m_state, options)) {
        error = QStringLiteral("Cannot initialize plugin require");
        return {ScriptPluginError::Initialization, error, {}};
    }
    try {
        m_ioService = new PluginIOService(m_io, m_resources.get());
        ScriptMachineBindings::initialize(m_state, {}, m_require, true,
                                          *m_ioService);
        registerPluginApi();
        // Register Qt classes before sandboxing so all callback threads share
        // the same class namespace, including lazy module consumers.
        bindings::registerModule(m_state, "qt");
    } catch (const std::exception &exception) {
        error = QString::fromUtf8(exception.what());
        return {ScriptPluginError::Initialization, error, {}};
    }
    m_require.installRequire();
    m_require.installProxyRequire();
    lua_callbacks(m_state)->userthread = [](lua_State *parent,
                                            lua_State *thread) {
        lua_setthreaddata(thread, parent ? lua_getthreaddata(parent) : nullptr);
        if (parent) {
            luaL_sandboxthread(thread);
        }
    };
    qtluau::LuaCallbackHooks hooks;
    hooks.dispatch = [this](std::function<void()> job) {
        if (!m_active) {
            return;
        }
        if (m_busy) {
            fail(QStringLiteral("Reentrant plugin signal callback"));
            return;
        }
        m_busy = true;
        job();
        m_busy = false;
    };
    hooks.end = [this](const QString &message) {
        if (!message.isEmpty()) {
            fail(message);
        }
    };
    qtluau::setLuaCallbackHooks(m_state, std::move(hooks));
    luaL_sandboxthread(m_state);
    const auto entry = m_package->entryPath();
    const auto file = readScriptFile(entry);
    if (!file) {
        return {ScriptPluginError::Initialization, file.message, entry};
    }
    const auto result =
        loadScriptChunk(m_state, file.bytes, '@' + entry, file.format);
    if (!result) {
        return {ScriptPluginError::Initialization, result.message, entry};
    }
    m_busy = true;
    const auto status = lua_pcall(m_state, 0, 1, 0);
    m_busy = false;
    if (status != LUA_OK || !lua_istable(m_state, -1)) {
        error = status == LUA_OK
                    ? QStringLiteral("Plugin entry must return a table")
                    : QString::fromUtf8(lua_tostring(m_state, -1));
        lua_settop(m_state, 0);
        return {ScriptPluginError::Initialization, error, {}};
    }
    m_object = std::make_unique<luabridge::LuaRef>(
        luabridge::LuaRef::fromStack(m_state, -1));
    lua_pop(m_state, 1);
    for (const auto &descriptor : pluginCallbackDescriptors) {
        if ((descriptor.deviceOnly && !devicePlugin()) ||
            (descriptor.ordinaryOnly && devicePlugin())) {
            continue;
        }
        const auto *name = descriptor.name;
        auto callback = field(*m_object, name);
        if (!m_active) {
            error = m_error;
            return {ScriptPluginError::Initialization, error, {}};
        }
        if (!callback.isNil() && !callback.isFunction()) {
            error = QStringLiteral("Plugin callback is not a function: %1")
                        .arg(QString::fromUtf8(name));
            return {ScriptPluginError::Initialization, error, {}};
        }
        m_callbacks.insert(name, callback);
    }
    for (const auto &descriptor : pluginCallbackDescriptors) {
        if (descriptor.required && (!descriptor.deviceOnly || devicePlugin()) &&
            m_callbacks.value(descriptor.name, luabridge::LuaRef(m_state))
                .isNil()) {
            error = QStringLiteral("Missing required plugin callback: %1")
                        .arg(QString::fromUtf8(descriptor.name));
            return {ScriptPluginError::Initialization, error, {}};
        }
    }
    return {};
}

void ScriptPluginRuntime::registerPluginApi() {
    if (devicePlugin()) {
        bindings::registerModule(m_state, "_plugin_device_types");
        qtluau::pushModule(m_state, "_plugin_device_types");
        lua_newtable(m_state);
        lua_getfield(m_state, -2, "IODevice");
        lua_rawsetfield(m_state, -2, "IODevice");
        lua_remove(m_state, -2);
        lua_setglobal(m_state, "plugin");
    } else {
        lua_newtable(m_state);
        lua_setglobal(m_state, "plugin");
    }
    if (!devicePlugin()) {
        bindings::registerModule(m_state, "_plugin_paint");
    }
    PluginSystem::instance().installPluginAPI(m_state, m_owner.data());
    lua_getglobal(m_state, "plugin");
    qtluau::variantToLua(m_state, m_package->manifest().toVariantMap()).push();
    lua_setreadonly(m_state, -1, true);
    lua_setfield(m_state, -2, "info");
    for (const auto &name : {QStringLiteral("reader"), QStringLiteral("ctl"),
                             QStringLiteral("log"), QStringLiteral("toast"),
                             QStringLiteral("service")}) {
        const auto key = name.toUtf8();
        if (name == QStringLiteral("toast") ||
            name == QStringLiteral("service")) {
            if (!qtluau::pushModule(m_state, key.constData())) {
                lua_pushnil(m_state);
            }
        } else {
            lua_getglobal(m_state, key.constData());
        }
        if (!devicePlugin() || (name != QStringLiteral("reader") &&
                                name != QStringLiteral("ctl"))) {
            lua_setfield(m_state, -2, key.constData());
        } else {
            lua_pop(m_state, 1);
        }
        lua_pushnil(m_state);
        lua_setglobal(m_state, key.constData());
    }
#include "plugin_helpers.inc"
    if (devicePlugin()) {
        if (!qtluau::pushModule(m_state, "_plugin_device_io")) {
            throw std::runtime_error("Missing device IO module");
        }
        lua_setfield(m_state, -2, "io");
    }
    lua_pop(m_state, 1);
    auto loaded = loadScriptChunk(m_state, pluginBaseSource, "=plugin bases",
                                  ScriptChunkFormat::Source);
    if (!loaded || lua_pcall(m_state, 0, 0, 0) != LUA_OK) {
        throw std::runtime_error("Cannot initialize plugin base tables");
    }
    lua_getglobal(m_state, "plugin");
    lua_pushnil(m_state);
    lua_rawsetfield(m_state, -2, devicePlugin() ? "Plugin" : "DevicePlugin");
    lua_pushnil(m_state);
    while (lua_next(m_state, -2)) {
        if (lua_istable(m_state, -1)) {
            lua_setreadonly(m_state, -1, true);
        }
        lua_pop(m_state, 1);
    }
    lua_setreadonly(m_state, -1, true);
    lua_pop(m_state, 1);
}

int ScriptPluginRuntime::resumeThread(LuauThread *thread, int arguments) {
    if (!m_active) {
        thread->requestStop = true;
        return LUA_OK;
    }
    Q_ASSERT(!m_busy);
    QScopedValueRollback<bool> executing(m_busy, true);
    return lua_resume(thread->state, nullptr, arguments);
}

int ScriptPluginRuntime::spawnTask(lua_State *state) {
    auto *runtime = self(state);
    if (!runtime || !runtime->m_active || !runtime->m_initialized) {
        luaL_error(state, "Plugin tasks are unavailable");
    }
    luaL_checktype(state, 1, LUA_TFUNCTION);
    auto task = std::make_shared<Task>();
    auto *thread = lua_newthread(runtime->m_state);
    task->thread.state = thread;
    task->thread.clearContextOnReset = false;
    task->thread.refID = lua_ref(runtime->m_state, -1);
    lua_pop(runtime->m_state, 1);
    task->thread.data = &task->data;
    task->data.parent = &task->thread;
    task->data.mode = ScriptMachine::Background;
    lua_setthreaddata(thread, &task->data);
    luabridge::LuaRef::fromStack(state, 1).push(thread);
    auto *runner =
        new LuauScheduler(*runtime, &task->thread, runtime->m_resources.get());
    runtime->m_tasks.append(task);
    QObject::connect(runner, &LuauScheduler::sigOnFinished, runner,
                     [runtime, task, runner](int status) {
                         if (status != LUA_OK && runtime->m_active) {
                             runtime->fail(runner->executeError());
                         }
                         lua_setthreaddata(task->thread.state,
                                           &runtime->m_threadData);
                         task->thread.destory();
                         runtime->m_tasks.removeOne(task);
                         runner->deleteLater();
                     });
    runner->start();
    return 0;
}

int ScriptPluginRuntime::addDefinitionFile(lua_State *state) {
    auto *runtime = self(state);
    if (!runtime || !runtime->m_active || runtime->m_exportsFrozen) {
        luaL_error(
            state,
            "Definitions must be registered before initialization finishes");
    }
    try {
        const auto path =
            qtluau::qtFromLua<QString>(luabridge::LuaRef::fromStack(state, 1));
        if (!path.endsWith(QStringLiteral(".d.luau"), Qt::CaseInsensitive)) {
            throw std::runtime_error("Expected a .d.luau definition file");
        }
        const auto resource = runtime->m_package->resourcePath(path);
        if (!resource) {
            throw std::runtime_error(resource.message.toStdString());
        }
        if (!runtime->m_definitionFiles.contains(resource.path)) {
            runtime->m_definitionFiles.append(resource.path);
        }
        return 0;
    } catch (const std::exception &error) {
        luaL_error(state, "%s", error.what());
    }
}

void ScriptPluginRuntime::registerDefinitions(
    WingScriptBind::ScopeBuilder scope) const {
    for (const auto &file : m_definitionFiles) {
        scope.addDefinitionFile(file);
    }
}

int ScriptPluginRuntime::exportServices(lua_State *state) {
    auto *runtime = self(state);
    if (!runtime || !runtime->m_active || runtime->m_exportsFrozen) {
        luaL_error(state,
                   "Services must be exported before initialization finishes");
    }
    luaL_checktype(state, 1, LUA_TTABLE);
    lua_pushnil(state);
    while (lua_next(state, 1)) {
        if (lua_type(state, -2) != LUA_TSTRING || !lua_isfunction(state, -1)) {
            luaL_error(state,
                       "Exported services require string names and functions");
        }
        size_t size;
        const auto *key = lua_tolstring(state, -2, &size);
        runtime->m_exports.insert(QByteArray(key, qsizetype(size)),
                                  luabridge::LuaRef::fromStack(state, -1));
        lua_pop(state, 1);
    }
    return 0;
}

QVariant ScriptPluginRuntime::serviceValue(const QVariant &value) const {
    const std::weak_ptr<PluginIOContext> token = m_io;
    size_t nodes = 0;
    std::function<QVariant(const QVariant &, int)> transfer =
        [&](const QVariant &item, int depth) -> QVariant {
        if (++nodes > 100000 || depth > 64) {
            throw std::runtime_error("Service value exceeds conversion limits");
        }
        if (item.metaType() == QMetaType::fromType<qtluau::QObjectValue>()) {
            auto object = item.value<qtluau::QObjectValue>();
            if (!object.object ||
                qobject_cast<PluginIODevice *>(object.object.data()) ||
                qobject_cast<PluginIOService *>(object.object.data())) {
                throw std::runtime_error(
                    "Device-plugin IO cannot cross a VM boundary");
            }
            object.guard = [previous = object.guard, token] {
                if (previous) {
                    previous();
                }
                const auto context = token.lock();
                if (!context || !context->active) {
                    throw std::runtime_error("Exporting plugin is inactive");
                }
            };
            return QVariant::fromValue(object);
        }
        if (item.metaType().id() == QMetaType::QVariantList) {
            QVariantList result;
            for (const auto &child : item.toList()) {
                result.append(transfer(child, depth + 1));
            }
            return result;
        }
        if (item.metaType().id() == QMetaType::QVariantMap ||
            item.metaType().id() == QMetaType::QVariantHash) {
            QVariantMap result;
            const auto children = item.toMap();
            for (auto it = children.cbegin(); it != children.cend(); ++it) {
                result.insert(it.key(), transfer(it.value(), depth + 1));
            }
            return result;
        }
        return item;
    };
    return transfer(value, 0);
}

bool ScriptPluginRuntime::invokeExport(const QByteArray &name,
                                       const QVariantList &arguments,
                                       QVariant &result) {
    if (!m_active || !m_initialized || m_busy || !m_exports.contains(name)) {
        return false;
    }
    try {
        // QObject values need their Qt wrapper types in the receiving VM.
        bindings::registerModule(m_state, "qt");
        QList<luabridge::LuaRef> values;
        for (const auto &argument : arguments) {
            values.append(qtluau::variantToLua(m_state, argument));
        }
        const auto returned =
            invoke(*m_object, m_exports.value(name, luabridge::LuaRef(m_state)),
                   values);
        if (!m_active) {
            return false;
        }
        result = serviceValue(qtluau::variantFromLua(returned));
        return true;
    } catch (const std::exception &exception) {
        fail(QString::fromUtf8(exception.what()));
        return false;
    }
}

int ScriptPluginRuntime::translateText(lua_State *state) {
    auto *runtime = self(state);
    if (!runtime || !runtime->m_active) {
        luaL_error(state, "Plugin is inactive");
    }
    try {
        const auto text =
            qtluau::qtFromLua<QString>(luabridge::LuaRef::fromStack(state, 1));
        const auto encoded = text.toUtf8();
        const auto context = runtime->m_package->manifest()
                                 .value(QStringLiteral("Id"))
                                 .toString()
                                 .toUtf8();
        const auto translated =
            runtime->m_translator
                ? runtime->m_translator->translate(context.constData(),
                                                   encoded.constData())
                : QString();
        qtluau::qtToLua(state, translated.isEmpty() ? text : translated).push();
        return 1;
    } catch (const std::exception &error) {
        lua_pushstring(state, error.what());
    }
    lua_error(state);
}

int ScriptPluginRuntime::resourceRead(lua_State *state) {
    auto *runtime = self(state);
    if (!runtime || !runtime->m_active) {
        luaL_error(state, "Plugin is inactive");
    }
    try {
        const auto relative =
            qtluau::qtFromLua<QString>(luabridge::LuaRef::fromStack(state, 1));
        if (QDir::isAbsolutePath(relative) ||
            relative != QDir::cleanPath(relative) ||
            relative.startsWith(QStringLiteral("../"))) {
            throw std::runtime_error("Invalid package resource path");
        }
        const auto bytes = runtime->m_package->readResource(relative);
        if (!bytes) {
            throw std::runtime_error("Package resource not found");
        }
        qtluau::qtToLua(state, *bytes).push();
        return 1;
    } catch (const std::exception &error) {
        lua_pushstring(state, error.what());
    }
    lua_error(state);
}

int ScriptPluginRuntime::resourcePath(lua_State *state) {
    auto *runtime = self(state);
    if (!runtime || !runtime->m_active) {
        luaL_error(state, "Plugin is inactive");
    }
    try {
        const auto relative =
            qtluau::qtFromLua<QString>(luabridge::LuaRef::fromStack(state, 1));
        if (QDir::isAbsolutePath(relative) ||
            relative != QDir::cleanPath(relative) ||
            relative.startsWith(QStringLiteral("../"))) {
            throw std::runtime_error("Invalid package resource path");
        }
        const auto resource = runtime->m_package->resourcePath(relative);
        if (!resource) {
            throw std::runtime_error(resource.message.toStdString());
        }
        qtluau::qtToLua(state, resource.path).push();
        return 1;
    } catch (const std::exception &error) {
        lua_pushstring(state, error.what());
    }
    lua_error(state);
}

int ScriptPluginRuntime::settingGet(lua_State *state) {
    auto *runtime = self(state);
    if (!runtime || !runtime->m_active || !runtime->m_settings) {
        lua_pushnil(state);
        return 1;
    }
    try {
        const auto key =
            qtluau::qtFromLua<QString>(luabridge::LuaRef::fromStack(state, 1));
        qtluau::variantToLua(state, runtime->m_settings->value(key)).push();
        return 1;
    } catch (const std::exception &exception) {
        lua_pushstring(state, exception.what());
    }
    lua_error(state);
}

int ScriptPluginRuntime::settingSet(lua_State *state) {
    auto *runtime = self(state);
    if (!runtime || !runtime->m_active || !runtime->m_settings) {
        luaL_error(state, "Plugin settings are unavailable");
    }
    try {
        const auto key =
            qtluau::qtFromLua<QString>(luabridge::LuaRef::fromStack(state, 1));
        const auto converted =
            qtluau::variantFromLua(luabridge::LuaRef::fromStack(state, 2));
        runtime->m_settings->setValue(key, converted);
        return 0;
    } catch (const std::exception &exception) {
        lua_pushstring(state, exception.what());
    }
    lua_error(state);
}

luabridge::LuaRef
ScriptPluginRuntime::invoke(const luabridge::LuaRef &object,
                            const luabridge::LuaRef &function,
                            const QList<luabridge::LuaRef> &arguments) {
    if (!m_active || !function.isFunction()) {
        return luabridge::LuaRef(m_state);
    }
    if (m_busy) {
        fail(QStringLiteral("Reentrant plugin callback"));
        return luabridge::LuaRef(m_state);
    }
    const int top = lua_gettop(m_state);
    function.push();
    object.push();
    for (const auto &argument : arguments) {
        argument.push();
    }
    m_busy = true;
    const auto status = lua_pcall(m_state, arguments.size() + 1, 1, 0);
    m_busy = false;
    if (status != LUA_OK) {
        const auto error = QString::fromUtf8(lua_tostring(m_state, -1));
        lua_settop(m_state, top);
        fail(error);
        return luabridge::LuaRef(m_state);
    }
    auto result = luabridge::LuaRef::fromStack(m_state, -1);
    lua_settop(m_state, top);
    return result;
}

luabridge::LuaRef
ScriptPluginRuntime::call(const char *name,
                          const QList<luabridge::LuaRef> &arguments) {
    const auto callback = m_callbacks.value(name, luabridge::LuaRef(m_state));
    if (!m_object || !callback.isFunction()) {
        return luabridge::LuaRef(m_state);
    }
    const auto result = invoke(*m_object, callback, arguments);
    if (!m_active) {
        return result;
    }
    for (const auto &descriptor : pluginCallbackDescriptors) {
        if (QByteArrayView(descriptor.name) != name) {
            continue;
        }
        const QByteArrayView expected(descriptor.result);
        bool valid = true;
        if (expected == "boolean") {
            valid = result.isBool();
        } else if (expected == "number") {
            valid = bool(result.cast<unsigned int>());
        } else if (expected == "string | ustring") {
            try {
                qtluau::qtFromLua<QString>(result);
            } catch (const std::exception &) {
                valid = false;
            }
        }
        if (!valid) {
            fail(QStringLiteral("Invalid result from plugin callback: %1")
                     .arg(QString::fromUtf8(name)));
        }
        break;
    }
    return result;
}

luabridge::LuaRef ScriptPluginRuntime::field(const luabridge::LuaRef &object,
                                             const char *name) {
    if (!m_active) {
        return luabridge::LuaRef(m_state);
    }
    if (m_busy) {
        fail(QStringLiteral("Reentrant plugin lookup"));
        return luabridge::LuaRef(m_state);
    }
    const int top = lua_gettop(m_state);
    lua_pushcfunction(m_state, lookup, "plugin field lookup");
    object.push();
    lua_pushstring(m_state, name);
    m_busy = true;
    const auto status = lua_pcall(m_state, 2, 1, 0);
    m_busy = false;
    if (status != LUA_OK) {
        lua_settop(m_state, top);
        fail(QStringLiteral("Plugin field lookup failed"));
        return luabridge::LuaRef(m_state);
    }
    auto result = luabridge::LuaRef::fromStack(m_state, -1);
    lua_settop(m_state, top);
    return result;
}

luabridge::LuaRef
ScriptPluginRuntime::callObject(const luabridge::LuaRef &object,
                                const char *name,
                                const QList<luabridge::LuaRef> &arguments) {
    return invoke(object, field(object, name), arguments);
}

bool ScriptPluginRuntime::initialize(const QSettings *settings) {
    if (settings) {
        m_settings = std::make_unique<QSettings>(settings->fileName(),
                                                 settings->format());
    }
    m_initialized = true;
    if (m_callbacks.value("init", luabridge::LuaRef(m_state)).isNil()) {
        m_exportsFrozen = true;
        return m_active;
    }
    const auto result = call("init");
    const auto ok = result.cast<bool>();
    if (!ok || !*ok) {
        fail(QStringLiteral("Plugin initialization failed"));
        return false;
    }
    m_exportsFrozen = true;
    return m_active;
}

void ScriptPluginRuntime::enqueueEvent(
    const char *name, const QList<luabridge::LuaRef> &arguments,
    bool coalesce) {
    if (!m_active || !m_resources) {
        return;
    }
    if (coalesce) {
        for (auto &event : m_events) {
            if (event.name == name) {
                event.arguments = arguments;
                return;
            }
        }
    }
    m_events.append({name, arguments});
    if (!m_eventScheduled) {
        m_eventScheduled = true;
        QTimer::singleShot(0, m_resources.get(), [this] { dispatchEvent(); });
    }
}

void ScriptPluginRuntime::dispatchEvent() {
    m_eventScheduled = false;
    if (!m_active || m_events.isEmpty() || !m_resources) {
        return;
    }
    if (!m_busy) {
        const auto event = m_events.takeFirst();
        call(event.name.constData(), event.arguments);
    }
    if (m_active && !m_events.isEmpty() && m_resources) {
        m_eventScheduled = true;
        QTimer::singleShot(0, m_resources.get(), [this] { dispatchEvent(); });
    }
}

void ScriptPluginRuntime::fail(const QString &message) {
    m_error = message;
    m_active = false;
    m_io->active = false;
    if (m_resources) {
        QTimer::singleShot(0, m_resources.get(), [this] { stop(); });
    }
    qCritical().noquote()
        << QStringLiteral("Plugin [%1]: %2")
               .arg(
                   m_package->manifest().value(QStringLiteral("Id")).toString(),
                   message);
}

void ScriptPluginRuntime::trackWidget(QWidget *widget) {
    const auto owner = quintptr(m_owner.data());
    const auto previous =
        widget->property("__WING_SCRIPT_PLUGIN__").value<quintptr>();
    if (previous && previous != owner) {
        throw std::runtime_error("Widget belongs to another plugin");
    }
    widget->setProperty("__WING_SCRIPT_PLUGIN__", owner);
    if (!m_widgets.contains(widget)) {
        m_widgets.append(widget);
    }
}

QString ScriptPluginRuntime::grantIODevice(QIODevice *device, bool allowClose) {
    if (!m_active || !devicePlugin() || !m_ioService) {
        throw std::runtime_error("Device plugin is unavailable");
    }
    return m_ioService->grant(device, allowClose);
}

void ScriptPluginRuntime::revokeIODevice(const QString &handle) {
    if (m_ioService) {
        m_ioService->revoke(handle);
    }
}

void ScriptPluginRuntime::stop() {
    if (!m_state) {
        return;
    }
    if (m_active && m_initialized) {
        call("unload");
    }
    PluginSystem::instance().deactivatePlugin(m_owner.data());
    m_initialized = false;
    m_events.clear();
    m_eventScheduled = false;
    m_active = false;
    m_io->active = false;
    if (m_ioService) {
        m_ioService->invalidate();
    }
    qtluau::clearLuaContext(m_state);
    m_resources.reset();
    const auto widgets = std::exchange(m_widgets, {});
    for (const auto &widget : widgets) {
        if (widget) {
            delete widget.data();
        }
    }
    for (const auto &task : std::as_const(m_tasks)) {
        lua_setthreaddata(task->thread.state, &m_threadData);
        task->thread.destory();
    }
    m_tasks.clear();
    qtluau::clearLuaCallbacks(m_state);
}

void ScriptPluginRuntime::shutdown() {
    if (!m_state) {
        return;
    }
    stop();
    m_exports.clear();
    m_callbacks.clear();
    m_object.reset();
    lua_close(m_state);
    m_state = nullptr;
}
