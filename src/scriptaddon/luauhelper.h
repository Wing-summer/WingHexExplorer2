#ifndef LUAUHELPER_H
#define LUAUHELPER_H

#include "lua.h"

#include "LuaBridge/LuaBridge.h"

#include <QMetaEnum>

namespace LuauHelper {

template <typename T>
void registerLuauEnum(lua_State *L) {
    Q_ASSERT(L != nullptr);
    auto metaEnum = QMetaEnum::fromType<T>();
    auto enumName = metaEnum.name();
    auto &&n = luabridge::getGlobalNamespace(L).beginNamespace(enumName);
    auto total = metaEnum.keyCount();
    for (int i = 0; i < total; ++i) {
        auto value = metaEnum.value(i);
        n.addProperty(metaEnum.key(i), +[value]() { return value; });
    }
    n.endNamespace();
}

}; // namespace LuauHelper

#endif // LUAUHELPER_H