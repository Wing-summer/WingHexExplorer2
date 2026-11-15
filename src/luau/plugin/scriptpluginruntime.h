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

#ifndef SCRIPTPLUGINRUNTIME_H
#define SCRIPTPLUGINRUNTIME_H

#include <QHash>
#include <QPointer>
#include <QSettings>
#include <QStringList>
#include <QTranslator>
#include <QWidget>

#include "lua.h"
#include "lualib.h"

#include <LuaBridge/LuaBridge.h>

#include "luau/runtime/luauscheduler.h"
#include "luau/wingluaurequire.h"
#include "pluginiodevice.h"
#include "scriptpluginpackage.h"

namespace WingHex {
class IWingPluginBase;
}

namespace WingScriptBind {
class ScopeBuilder;
}

class ScriptPluginRuntime final : public LuauExecutor {
public:
    explicit ScriptPluginRuntime(std::shared_ptr<ScriptPluginPackage> package,
                                 WingHex::IWingPluginBase *owner);
    ~ScriptPluginRuntime() override;

public:
    ScriptPluginResult prepare();
    bool initialize(const QSettings *settings);
    void stop();
    void shutdown();

public:
    QVariant serviceValue(const QVariant &value) const;
    bool invokeExport(const QByteArray &name, const QVariantList &arguments,
                      QVariant &result);

    inline bool active() const { return m_active; }

    inline QString error() const { return m_error; }

    inline lua_State *state() const { return m_state; }

    inline QString rootPath() const { return m_package->rootPath(); }

    bool devicePlugin() const;

    inline bool isExecuting() const override { return m_busy; }

    int resumeThread(LuauThread *thread, int arguments) override;
    void registerDefinitions(WingScriptBind::ScopeBuilder scope) const;
    luabridge::LuaRef call(const char *name,
                           const QList<luabridge::LuaRef> &arguments = {});
    luabridge::LuaRef
    callObject(const luabridge::LuaRef &object, const char *name,
               const QList<luabridge::LuaRef> &arguments = {});

    template <class T>
    inline luabridge::LuaRef value(const T &value) const {
        return luabridge::LuaRef(m_state, value);
    }

    void trackWidget(QWidget *widget);
    QString grantIODevice(QIODevice *device, bool allowClose);
    void revokeIODevice(const QString &handle);

    inline QObject *resources() const { return m_resources.get(); }

    luabridge::LuaRef field(const luabridge::LuaRef &object, const char *name);

    luabridge::LuaRef
    callFunction(const luabridge::LuaRef &object,
                 const luabridge::LuaRef &function,
                 const QList<luabridge::LuaRef> &arguments = {}) {
        return invoke(object, function, arguments);
    }

    void fail(const QString &message);
    void enqueueEvent(const char *name,
                      const QList<luabridge::LuaRef> &arguments = {},
                      bool coalesce = false);

private:
    static ScriptPluginRuntime *self(lua_State *state);
    static int spawnTask(lua_State *state);
    static int exportServices(lua_State *state);
    static int addDefinitionFile(lua_State *state);
    static int translateText(lua_State *state);
    static int resourcePath(lua_State *state);
    static int resourceRead(lua_State *state);
    static int settingGet(lua_State *state);
    static int settingSet(lua_State *state);
    void registerPluginApi();
    void dispatchEvent();
    luabridge::LuaRef invoke(const luabridge::LuaRef &object,
                             const luabridge::LuaRef &function,
                             const QList<luabridge::LuaRef> &arguments);

private:
    std::shared_ptr<ScriptPluginPackage> m_package;
    QPointer<WingHex::IWingPluginBase> m_owner;
    std::unique_ptr<QObject> m_resources;
    std::unique_ptr<QSettings> m_settings;
    std::unique_ptr<QTranslator> m_translator;
    std::shared_ptr<PluginIOContext> m_io;
    QPointer<PluginIOService> m_ioService;
    QList<QPointer<QWidget>> m_widgets;
    WingLuauRequire m_require;
    lua_State *m_state = nullptr;
    std::unique_ptr<luabridge::LuaRef> m_object;
    QHash<QByteArray, luabridge::LuaRef> m_callbacks;
    QHash<QByteArray, luabridge::LuaRef> m_exports;
    QStringList m_definitionFiles;

private:
    struct Task {
        LuauThread thread;
        LuauThreadData data;
    };

    QList<std::shared_ptr<Task>> m_tasks;
    LuauThreadData m_threadData;

    struct Event {
        QByteArray name;
        QList<luabridge::LuaRef> arguments;
    };

private:
    QList<Event> m_events;
    bool m_eventScheduled = false;
    bool m_active = false;
    bool m_busy = false;
    bool m_initialized = false;
    bool m_exportsFrozen = false;
    QString m_error;
};
#endif // SCRIPTPLUGINRUNTIME_H
