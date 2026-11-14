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

#include "scriptenvironment.h"

#include <QDebug>

#include "class/plugin/pluginsystem.h"

ScriptEnvironment::~ScriptEnvironment() { shutdown(); }

void ScriptEnvironment::loadPlugins(MainWindow *window) {
    m_pluginsStarted = true;
    auto &plugins = PluginSystem::instance();
    plugins.setMainWindow(window);
    plugins.loadAllPlugins();
}

bool ScriptEnvironment::initializeMachine(ScriptRuntimeOptions options) {
    return ScriptMachine::instance().init({}, options);
}

bool ScriptEnvironment::finishRegistration() {
    return PluginSystem::instance().doneRegisterScriptObj();
}

bool ScriptEnvironment::requestClose() {
    if (!PluginSystem::instance().dispatchAppClosingEvent()) {
        return false;
    }
    m_closingNotified = true;
    return true;
}

void ScriptEnvironment::shutdown() {
    if (!m_pluginsStarted) {
        return;
    }
    m_pluginsStarted = false;
    auto &plugins = PluginSystem::instance();
    try {
        if (!m_closingNotified && !plugins.dispatchAppClosingEvent()) {
            qWarning("Plugin shutdown veto ignored during process exit");
        }
    } catch (...) {
        qWarning("Plugin threw during shutdown notification");
    }
    ScriptMachine::instance().destoryMachine();
    plugins.destory();
}
