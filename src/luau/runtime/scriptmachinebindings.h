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

#ifndef QTLUAU_INTEGRATIONS_WINGHEX_SCRIPTMACHINEBINDINGS_H
#define QTLUAU_INTEGRATIONS_WINGHEX_SCRIPTMACHINEBINDINGS_H

#include <QDialog>
#include <QMainWindow>
#include <QUrl>

#include "qtluau/value_conversion.hpp"
#include "qtluau/value_ref.hpp"
#include "qtluau/widget_wrappers.hpp"

#include "generated_bindings.hpp"
#include "scriptfilesystem.h"
#include "scriptmachine.h"
#include "scriptwindowfactory.h"
#include "src/class/settings/settingmanager.h"

class ScriptMachineBindings {
    using Codec = WingScriptBind::ScriptBindingEngine::ObjectCodec;
    using Value = WingScriptBind::Value;
    using ObjectValue = WingScriptBind::ObjectValue;

    template <class T>
    inline static ObjectValue object(T value) {
        auto storage = std::make_shared<T>(std::move(value));
        return {WingScriptBind::typeInfo<T>(), storage, storage.get()};
    }

    template <class T>
    inline static bool readObject(const luabridge::LuaRef &value,
                                  ObjectValue &result) {
        if (auto native = value.cast<T>()) {
            result = object<T>(*native);
            return true;
        }
        return false;
    }

    template <class T>
    inline static bool pushObject(lua_State *L, const ObjectValue &value) {
        if (WingScriptBind::sameType(value.type,
                                     WingScriptBind::typeInfo<T>())) {
            qtluau::qtToLua(L, *static_cast<const T *>(value.pointer)).push();
            return true;
        }
        return false;
    }

    inline static void adaptText(Value &value) {
        if (value.kind == Value::Kind::Bytes) {
            value.object = object<QString>(
                QAnyStringView(QUtf8StringView(value.bytes)).toString());
            value.kind = Value::Kind::Object;
        } else if (value.kind == Value::Kind::Sequence) {
            for (auto &entry : value.sequence) {
                adaptText(entry);
            }
        } else if (value.kind == Value::Kind::Table) {
            for (auto &entry : value.entries) {
                adaptText(entry.first);
                adaptText(entry.second);
            }
        }
    }

public:
    inline static Codec initialize(lua_State *L, Codec fallback,
                                   WingLuauRequire &require, bool ui) {
        if (!ui) {
#include "headless_initializers.inc"
        } else {
            bindings::registerEagerModules(L);
        }
        const auto modules = bindings::moduleFiles();

        // The statically linked factory is registered by the host.
        const QStringList internalMods{"cffi"};
        for (auto it = modules.cbegin(); it != modules.cend(); ++it) {
            const auto name = it.key();
            if (name.isEmpty()) {
                continue;
            }
            if (internalMods.contains(name) ||
                (!ui && (name == QStringLiteral("qt") ||
                         name == QStringLiteral("dialogs") ||
                         name == QStringLiteral("clipboard") ||
                         name == QStringLiteral("toast")))) {
                continue;
            }
            const bool registered =
                require.registerBuiltinLoader(name, [name](lua_State *caller) {
                    return qtluau::loadRegisteredModule(
                        caller, name, &bindings::registerModule);
                });
            if (!registered) {
                throw std::runtime_error(
                    "Cannot register generated builtin loader");
            }
        }
#include SCRIPTMACHINE_LUAU_CALLBACKS
        if (ui) {
            qtluau::setUiWidgetResolver(&ScriptWindowFactory::create);
        }
        ScriptFileSystem::setFileSystemAccessPolicy(
            {[] {
                 return SettingManager::instance().scriptFileEnableOverwrite();
             },
             [] {
                 return SettingManager::instance().scriptFileSystemWrite();
             }});
        if (ui) {
            qtluau::setUiWidgetCreatedCallback(&ScriptWindowFactory::created);
            qtluau::setUiWidgetFinalizer(&ScriptWindowFactory::finalize);
            qtluau::setWidgetContentResolver(&ScriptWindowFactory::content);
            qtluau::setWidgetContentMutators(&ScriptWindowFactory::setContent,
                                             &ScriptWindowFactory::takeContent);
            qtluau::setWindowMenuAccessors(&ScriptWindowFactory::menu,
                                           &ScriptWindowFactory::setMenu);
        }
        Codec codec;
        codec.read = [previous = fallback.read](lua_State *state, int index,
                                                ObjectValue &result) {
            const auto value = luabridge::LuaRef::fromStack(state, index);
            if (auto text = value.cast<qtluau::QtValueRef<QString>>()) {
                result = object<QString>(text->value());
                return true;
            }
            if (auto box = value.cast<qtluau::VariantBox>()) {
                result = object<QVariant>(box->value);
                return true;
            }
            if (readObject<QColor>(value, result) ||
                readObject<QFont>(value, result) ||
                readObject<QUrl>(value, result) ||
                readObject<QChar>(value, result) ||
                readObject<WingHex::HexPosition>(value, result) ||
                readObject<WingHex::MetadataInfo>(value, result)) {
                return true;
            }
            return previous && previous(state, index, result);
        };
        codec.push = [previous = fallback.push](lua_State *state,
                                                const ObjectValue &value) {
            if (pushObject<QString>(state, value) ||
                pushObject<QColor>(state, value) ||
                pushObject<QFont>(state, value) ||
                pushObject<QUrl>(state, value) ||
                pushObject<QChar>(state, value) ||
                pushObject<QVariant>(state, value) ||
                pushObject<WingHex::HexPosition>(state, value) ||
                pushObject<WingHex::MetadataInfo>(state, value)) {
                return true;
            }
            return previous && previous(state, value);
        };
        codec.adaptArguments =
            [previous = fallback.adaptArguments](QList<Value> &values) {
                if (previous) {
                    previous(values);
                }
                for (auto &value : values) {
                    adaptText(value);
                }
            };
        codec.readValue = [previous = fallback.readValue](
                              lua_State *state, int index, Value &result) {
            if (lua_type(state, index) == LUA_TUSERDATA) {
                if (qtluau::isUnsignedInteger64(state, index)) {
                    std::uint64_t number = 0;
                    qtluau::readUnsignedInteger64(state, index, number);
                    result = WingScriptBind::detail::toValue(number);
                    return true;
                }
                if (qtluau::isInteger64(state, index)) {
                    std::int64_t number = 0;
                    qtluau::readInteger64(state, index, number);
                    result = WingScriptBind::detail::toValue(number);
                    return true;
                }
                auto value = luabridge::LuaRef::fromStack(state, index);
                if (auto bytes = value.cast<qtluau::QtValueRef<QByteArray>>()) {
                    result = WingScriptBind::detail::toValue(bytes->value());
                    return true;
                }
                if (auto box = value.cast<qtluau::VariantBox>()) {
                    const auto type = box->value.metaType().id();
                    if (type == QMetaType::ULongLong ||
                        type == QMetaType::ULong) {
                        result = WingScriptBind::detail::toValue(
                            box->value.toULongLong());
                        return true;
                    }
                    if (type == QMetaType::LongLong ||
                        type == QMetaType::Long) {
                        result = WingScriptBind::detail::toValue(
                            box->value.toLongLong());
                        return true;
                    }
                }
            }
            return previous && previous(state, index, result);
        };
        codec.pushValue = [previous = fallback.pushValue](lua_State *state,
                                                          const Value &value) {
            if (value.kind == Value::Kind::Signed &&
                (value.signedNumber < -qtluau::MaxSafeInteger ||
                 value.signedNumber > qtluau::MaxSafeInteger)) {
                qtluau::qtToLua(state, value.signedNumber).push();
                return true;
            }
            if (value.kind == Value::Kind::Unsigned &&
                value.unsignedNumber > quint64(qtluau::MaxSafeInteger)) {
                qtluau::qtToLua(state, value.unsignedNumber).push();
                return true;
            }
            return previous && previous(state, value);
        };
        return codec;
    }

    inline static QHash<QString, QString> moduleFiles() {
        return bindings::moduleFiles();
    }

    inline static QHash<QString, QString> definitions() {
        return bindings::definitionFiles();
    }
};

#endif // QTLUAU_INTEGRATIONS_WINGHEX_SCRIPTMACHINEBINDINGS_H
