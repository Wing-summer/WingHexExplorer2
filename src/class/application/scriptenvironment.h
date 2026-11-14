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

#ifndef SCRIPTENVIRONMENT_H
#define SCRIPTENVIRONMENT_H

#include "luau/runtime/scriptmachine.h"

class MainWindow;

// Owns the plugin/VM lifetime for a running application mode.
class ScriptEnvironment {
public:
    inline ScriptEnvironment() = default;
    ~ScriptEnvironment();

public:
    void loadPlugins(MainWindow *window = nullptr);
    static bool initializeMachine(ScriptRuntimeOptions options = {});
    static bool finishRegistration();
    bool requestClose();
    void shutdown();

private:
    bool m_pluginsStarted = false;
    bool m_closingNotified = false;

private:
    Q_DISABLE_COPY_MOVE(ScriptEnvironment)
};

#endif // SCRIPTENVIRONMENT_H
