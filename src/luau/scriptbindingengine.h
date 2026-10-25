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

#ifndef SCRIPTBINDINGENGINE_H
#define SCRIPTBINDINGENGINE_H

#include "WingPlugin/binding/registrationapi.h"

#include "scriptsymbol.h"
#include <QStringList>
#include <functional>
#include <memory>

struct lua_State;

namespace WingScriptBind {

class ScriptBindingEngine final : public IScriptEngine {
public:
    struct ObjectCodec {
        std::function<bool(lua_State *, int, ObjectValue &)> read;
        std::function<bool(lua_State *, const ObjectValue &)> push;
        // Host codecs may adapt scalar values and argument representations.
        std::function<bool(lua_State *, int, Value &)> readValue;
        std::function<bool(lua_State *, const Value &)> pushValue;
        std::function<void(QList<Value> &)> adaptArguments;
    };

    explicit ScriptBindingEngine(lua_State *state, ObjectCodec codec = {});
    ~ScriptBindingEngine() override;

public:
    IRegistrationApi &registrations() noexcept override;
    void collectGarbage() override;
    ScriptSymbolCatalog symbolCatalog() const;
    QStringList definitionFiles() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;

    Q_DISABLE_COPY_MOVE(ScriptBindingEngine)
};

} // namespace WingScriptBind
#endif
