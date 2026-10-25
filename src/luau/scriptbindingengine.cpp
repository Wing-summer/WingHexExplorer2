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

#include "scriptbindingengine.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <QThread>

#include <lua.h>
#include <lualib.h>
#include <new>

namespace WingScriptBind {

namespace {
char objectMarker;
struct StackGuard {
    lua_State *state;
    int top;
    explicit StackGuard(lua_State *value)
        : state(value), top(lua_gettop(value)) {}
    ~StackGuard() { lua_settop(state, top); }
};
} // namespace

class ScriptBindingEngine::Impl final : public IRegistrationApi {
    struct Property {
        Callable getter;
        Callable setter;
        bool isStatic = false;
    };
    struct Function {
        QList<Callable> overloads;
        bool isStatic = false;
    };
    struct Scope {
        ScopeId parent = 0;
        QString name;
        bool isClass = false;
        TypeInfo type;
        QList<BaseInfo> bases;
        ClassFlags flags;
        int table = LUA_NOREF;
        int metatable = LUA_NOREF;
        QHash<QString, Function> functions;
        QHash<QString, Property> properties;
    };
    struct Control {
        Impl *engine = nullptr;
    };
    enum class Operation { Call, Index, NewIndex, Construct };
    struct Closure {
        std::shared_ptr<Control> control;
        ScopeId scope;
        QString name;
        Operation operation;
        bool instance = false;
    };

public:
    Impl(lua_State *state, ObjectCodec codec)
        : state_(state), codec_(std::move(codec)),
          thread_(QThread::currentThread()),
          control_(std::make_shared<Control>()) {
        if (!state_) {
            throw ScriptException(Exception::InvalidArgument);
        }
        control_->engine = this;
        Scope root;
        lua_pushvalue(state_, LUA_GLOBALSINDEX);
        root.table = lua_ref(state_, -1);
        lua_pop(state_, 1);
        scopes_.insert(0, root);
    }

    ScriptSymbolCatalog symbolCatalog() const {
        checkThread();
        ScriptSymbolCatalog result;
        for (auto it = scopes_.cbegin(); it != scopes_.cend(); ++it) {
            if (it.key() == 0) {
                continue;
            }
            const auto id = QStringLiteral("plugin:%1").arg(it.key());
            const auto parent =
                it->parent == 0 ? QString()
                                : QStringLiteral("plugin:%1").arg(it->parent);
            result.append({id, parent, it->name,
                           it->isClass ? ScriptSymbol::Kind::Class
                                       : ScriptSymbol::Kind::Module,
                           QStringLiteral("Native extension")});
            for (auto function = it->functions.cbegin();
                 function != it->functions.cend(); ++function) {
                result.append({id + '/' + function.key(), id, function.key(),
                               ScriptSymbol::Kind::Function,
                               QStringLiteral("%1 (%2 overloads)")
                                   .arg(function.key())
                                   .arg(function->overloads.size())});
            }
            for (auto property = it->properties.cbegin();
                 property != it->properties.cend(); ++property) {
                result.append(
                    {id + '/' + property.key(), id, property.key(),
                     ScriptSymbol::Kind::Property,
                     property->setter
                         ? QStringLiteral("Native property")
                         : QStringLiteral("Read-only native property")});
            }
            const auto constants = constants_.value(it.key());
            for (const auto &name : constants) {
                result.append({id + '/' + name,
                               id,
                               name,
                               ScriptSymbol::Kind::Constant,
                               {}});
            }
        }
        return result;
    }

    ~Impl() override {
        control_->engine = nullptr;
        for (const auto &scope : std::as_const(scopes_)) {
            if (scope.table != LUA_NOREF) {
                lua_unref(state_, scope.table);
            }
            if (scope.metatable != LUA_NOREF) {
                lua_unref(state_, scope.metatable);
            }
        }
    }

    ScopeId globalScope() const noexcept override { return 0; }

    ScopeId parentScope(ScopeId scope) const override {
        checkThread();
        return get(scope).parent;
    }

    void retainNamespaceScope(ScopeId root) noexcept override {
        if (QThread::currentThread() == thread_) {
            ++leases_[root];
        }
    }

    void releaseNamespaceScope(ScopeId root) noexcept override {
        if (QThread::currentThread() != thread_) {
            return;
        }
        auto lease = leases_.find(root);
        if (lease == leases_.end() || --lease.value() > 0) {
            return;
        }
        leases_.erase(lease);
        prune(root);
    }

    ScopeId beginNamespace(ScopeId parent, QAnyStringView name) override {
        checkThread();
        if (get(parent).isClass) {
            throw ScriptException(Exception::InvalidScopeKind);
        }
        return child(parent, name, false, {}, {}, {});
    }

    ScopeId beginClass(ScopeId parent, const TypeInfo &type,
                       QAnyStringView name, const QList<BaseInfo> &bases,
                       ClassFlags flags) override {
        checkThread();
        if (get(parent).isClass) {
            throw ScriptException(Exception::InvalidScopeKind);
        }
        if (!type.rtti || !type.size || !type.alignment) {
            throw ScriptException(Exception::InvalidArgument);
        }
        if (flags.testFlag(ClassFlag::Extensible) ||
            flags.testFlag(ClassFlag::AllowOverridingMethods)) {
            throw ScriptException(
                Exception::InvalidArgument,
                QStringLiteral("Extensible class flags are not supported"));
        }
        for (const auto &base : bases) {
            if (!base.cast || classScope(base.type) == invalidScope) {
                throw ScriptException(Exception::TypeMismatch);
            }
        }
        const auto id = child(parent, name, true, type, bases, flags);
        StackGuard guard(state_);
        materialize(id);
        return id;
    }

    void registerFunction(ScopeId scope, QAnyStringView name,
                          const QList<Callable> &overloads,
                          bool isStatic) override {
        checkThread();
        const auto member = checkedName(name);
        if (overloads.isEmpty()) {
            throw ScriptException(Exception::InvalidArgument);
        }
        for (const auto &callable : overloads) {
            if (!callable) {
                throw ScriptException(Exception::InvalidArgument);
            }
        }
        checkMember(scope, member);
        if (get(scope).isClass && !isStatic &&
            (member == QStringLiteral("__index") ||
             member == QStringLiteral("__newindex") ||
             member == QStringLiteral("__gc") ||
             member == QStringLiteral("__metatable"))) {
            throw ScriptException(Exception::InvalidArgument);
        }
        StackGuard guard(state_);
        materialize(scope);
        scopes_[scope].functions.insert(member, {overloads, isStatic});
        if (!get(scope).isClass || isStatic) {
            lua_getref(state_, get(scope).table);
            pushClosure(scope, member, Operation::Call);
            lua_rawsetfield(state_, -2, member.toUtf8().constData());
        }
        if (member == QStringLiteral("__constructor") && get(scope).isClass) {
            lua_getref(state_, get(scope).table);
            lua_getmetatable(state_, -1);
            pushClosure(scope, member, Operation::Construct);
            lua_rawsetfield(state_, -2, "__call");
        } else if (member.startsWith(QStringLiteral("__")) &&
                   get(scope).isClass && !isStatic) {
            lua_getref(state_, get(scope).metatable);
            pushClosure(scope, member, Operation::Call, true);
            lua_rawsetfield(state_, -2, member.toUtf8().constData());
        }
    }

    void registerProperty(ScopeId scope, QAnyStringView name, Callable getter,
                          Callable setter, bool isStatic) override {
        checkThread();
        const auto member = checkedName(name);
        if (!getter) {
            throw ScriptException(Exception::InvalidArgument);
        }
        checkMember(scope, member);
        StackGuard guard(state_);
        materialize(scope);
        ensureTableMetatable(scope);
        scopes_[scope].properties.insert(
            member, {std::move(getter), std::move(setter), isStatic});
    }

    void registerVariable(ScopeId scope, QAnyStringView name,
                          const Value &value) override {
        checkThread();
        const auto member = checkedName(name);
        checkMember(scope, member);
        StackGuard guard(state_);
        materialize(scope);
        lua_getref(state_, get(scope).table);
        pushValue(state_, value, 0);
        lua_rawsetfield(state_, -2, member.toUtf8().constData());
        constants_[scope].insert(member);
    }

    void collectGarbage() {
        checkThread();
        lua_gc(state_, LUA_GCCOLLECT, 0);
    }

    void registerDefinitionFile(ScopeId scope,
                                QAnyStringView fileName) override {
        checkThread();
        if (lua_getreadonly(state_, LUA_GLOBALSINDEX)) {
            throw ScriptException(
                Exception::RegistrationFailed,
                QStringLiteral("Definition registration has finished"));
        }
        if (get(scope).isClass) {
            throw ScriptException(Exception::InvalidScopeKind);
        }
        const auto name = fileName.toString();
        const bool resource = name.startsWith(QStringLiteral(":/"));
        const QFileInfo info(name);
        if (name.isEmpty() || name.contains(QChar(u'\0')) ||
            (!resource && !info.isAbsolute()) || !info.isFile()) {
            throw ScriptException(Exception::InvalidArgument, name);
        }
        const auto path =
            resource ? QDir::cleanPath(name) : info.canonicalFilePath();
        if (definitionPaths_.contains(path)) {
            return;
        }
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            throw ScriptException(Exception::InvalidArgument,
                                  file.errorString());
        }
        definitionPaths_.insert(path);
        definitionFiles_.append(path);
    }

    QStringList definitionFiles() const {
        checkThread();
        return definitionFiles_;
    }

private:
    static constexpr ScopeId invalidScope = std::numeric_limits<ScopeId>::max();

    void checkThread() const {
        if (QThread::currentThread() != thread_) {
            throw ScriptException(Exception::WrongThread);
        }
    }

    const Scope &get(ScopeId scope) const {
        auto found = scopes_.constFind(scope);
        if (found == scopes_.cend()) {
            throw ScriptException(Exception::InvalidScope);
        }
        return found.value();
    }

    QString checkedName(QAnyStringView name) const {
        auto result = name.toString();
        if (result.isEmpty() || result.contains(QChar(u'\0'))) {
            throw ScriptException(Exception::InvalidArgument);
        }
        return result;
    }

    ScopeId classScope(const TypeInfo &type) const {
        for (auto scope = scopes_.cbegin(); scope != scopes_.cend(); ++scope) {
            if (scope->isClass && sameType(scope->type, type)) {
                return scope.key();
            }
        }
        return invalidScope;
    }

    ScopeId child(ScopeId parent, QAnyStringView name, bool isClass,
                  const TypeInfo &type, const QList<BaseInfo> &bases,
                  ClassFlags flags) {
        const auto member = checkedName(name);
        for (auto scope = scopes_.cbegin(); scope != scopes_.cend(); ++scope) {
            if (scope.key() != 0 && scope->parent == parent &&
                scope->name == member) {
                if (scope->isClass != isClass) {
                    throw ScriptException(Exception::DuplicateName, member);
                }
                if (isClass && !sameType(scope->type, type)) {
                    throw ScriptException(Exception::TypeMismatch, member);
                }
                if (!bases.isEmpty()) {
                    throw ScriptException(
                        Exception::DerivedClassAlreadyInitialized, member);
                }
                return scope.key();
            }
        }
        if (isClass && classScope(type) != invalidScope) {
            throw ScriptException(Exception::DuplicateName, member);
        }
        const auto &record = get(parent);
        if (record.functions.contains(member) ||
            record.properties.contains(member) ||
            constants_.value(parent).contains(member)) {
            throw ScriptException(Exception::DuplicateName, member);
        }
        Scope recordValue;
        recordValue.parent = parent;
        recordValue.name = member;
        recordValue.isClass = isClass;
        recordValue.type = type;
        recordValue.bases = bases;
        recordValue.flags = flags;
        auto id = nextScope_++;
        scopes_.insert(id, std::move(recordValue));
        return id;
    }

    void checkMember(ScopeId scope, const QString &name) const {
        const auto &record = get(scope);
        if (record.functions.contains(name) ||
            record.properties.contains(name) ||
            constants_.value(scope).contains(name)) {
            throw ScriptException(Exception::DuplicateName, name);
        }
        for (auto child = scopes_.cbegin(); child != scopes_.cend(); ++child) {
            if (child.key() && child->parent == scope && child->name == name) {
                throw ScriptException(Exception::DuplicateName, name);
            }
        }
        if (record.table != LUA_NOREF) {
            StackGuard guard(state_);
            lua_getref(state_, record.table);
            if (lua_getreadonly(state_, -1)) {
                throw ScriptException(
                    Exception::RegistrationFailed,
                    QStringLiteral("Registration table is read-only"));
            }
            lua_rawgetfield(state_, -1, name.toUtf8().constData());
            if (!lua_isnil(state_, -1)) {
                throw ScriptException(Exception::DuplicateName, name);
            }
        }
    }

    void prune(ScopeId scope) {
        QList<ScopeId> children;
        for (auto entry = scopes_.cbegin(); entry != scopes_.cend(); ++entry) {
            if (entry.key() && entry->parent == scope) {
                children.append(entry.key());
            }
        }
        for (auto child : children) {
            prune(child);
        }
        if (scope && get(scope).table == LUA_NOREF &&
            !leases_.contains(scope)) {
            scopes_.remove(scope);
        }
    }

    void materialize(ScopeId scope) {
        if (get(scope).table != LUA_NOREF) {
            return;
        }
        const auto record = get(scope);
        materialize(record.parent);
        lua_getref(state_, get(record.parent).table);
        if (lua_getreadonly(state_, -1)) {
            throw ScriptException(
                Exception::RegistrationFailed,
                QStringLiteral("Parent registration table is read-only"));
        }
        lua_rawgetfield(state_, -1, record.name.toUtf8().constData());
        if (lua_isnil(state_, -1)) {
            lua_pop(state_, 1);
            lua_newtable(state_);
            lua_pushvalue(state_, -1);
            lua_rawsetfield(state_, -3, record.name.toUtf8().constData());
        } else if (!lua_istable(state_, -1) || record.isClass) {
            throw ScriptException(Exception::DuplicateName, record.name);
        }
        scopes_[scope].table = lua_ref(state_, -1);
        lua_pop(state_, 2);
        if (record.isClass) {
            lua_newtable(state_);
            lua_pushlightuserdata(state_, &objectMarker);
            lua_rawsetfield(state_, -2, "__wing_binding");
            pushClosure(scope, {}, Operation::Index, true);
            lua_rawsetfield(state_, -2, "__index");
            pushClosure(scope, {}, Operation::NewIndex, true);
            lua_rawsetfield(state_, -2, "__newindex");
            if (!record.flags.testFlag(ClassFlag::VisibleMetatables)) {
                lua_pushboolean(state_, false);
                lua_rawsetfield(state_, -2, "__metatable");
            }
            scopes_[scope].metatable = lua_ref(state_, -1);
            lua_pop(state_, 1);
            ensureTableMetatable(scope);
        }
    }

    void ensureTableMetatable(ScopeId scope) {
        lua_getref(state_, get(scope).table);
        if (lua_getmetatable(state_, -1)) {
            lua_rawgetfield(state_, -1, "__wing_scope");
            const bool own = lua_touserdata(state_, -1) == control_.get();
            lua_pop(state_, 2);
            if (!own) {
                throw ScriptException(
                    Exception::RegistrationFailed,
                    QStringLiteral("Existing table has a foreign metatable"));
            }
            lua_pop(state_, 1);
            return;
        }
        lua_newtable(state_);
        lua_pushlightuserdata(state_, control_.get());
        lua_rawsetfield(state_, -2, "__wing_scope");
        pushClosure(scope, {}, Operation::Index);
        lua_rawsetfield(state_, -2, "__index");
        pushClosure(scope, {}, Operation::NewIndex);
        lua_rawsetfield(state_, -2, "__newindex");
        lua_pushboolean(state_, false);
        lua_rawsetfield(state_, -2, "__metatable");
        lua_setmetatable(state_, -2);
        lua_pop(state_, 1);
    }

    void pushClosure(ScopeId scope, const QString &name, Operation operation,
                     bool instance = false) {
        auto *memory = lua_newuserdatadtor(
            state_, sizeof(Closure), [](lua_State *, void *data) {
                static_cast<Closure *>(data)->~Closure();
            });
        new (memory) Closure{control_, scope, name, operation, instance};
        lua_pushcclosure(state_, &dispatch, "C++ binding", 1);
    }

    bool readObject(lua_State *state, int index, ObjectValue &result) {
        if (lua_type(state, index) == LUA_TUSERDATA &&
            lua_getmetatable(state, index)) {
            lua_rawgetfield(state, -1, "__wing_binding");
            const bool own = lua_touserdata(state, -1) == &objectMarker;
            lua_pop(state, 2);
            if (own) {
                result =
                    *static_cast<ObjectValue *>(lua_touserdata(state, index));
                return true;
            }
        }
        if (codec_.read) {
            const int top = lua_gettop(state);
            const bool handled = codec_.read(state, index, result);
            if (lua_gettop(state) != top) {
                throw ScriptException(
                    Exception::RegistrationFailed,
                    QStringLiteral("Object reader changed the VM stack"));
            }
            return handled;
        }
        return false;
    }

    Value readValue(lua_State *state, int index, QSet<const void *> &tables,
                    int depth) {
        if (depth > 64 || !lua_checkstack(state, 4)) {
            throw ScriptException(Exception::InvalidArgument,
                                  QStringLiteral("Table nesting is too deep"));
        }
        Value result;
        if (codec_.readValue && codec_.readValue(state, index, result)) {
            return result;
        }
        index = lua_absindex(state, index);
        switch (lua_type(state, index)) {
        case LUA_TNIL:
            return result;
        case LUA_TBOOLEAN:
            return detail::toValue(bool(lua_toboolean(state, index)));
        case LUA_TNUMBER:
            return detail::toValue(lua_tonumber(state, index));
        case LUA_TINTEGER:
            return detail::toValue(lua_tointeger64(state, index, nullptr));
        case LUA_TSTRING: {
            size_t length = 0;
            const auto *bytes = lua_tolstring(state, index, &length);
            return detail::toValue(QByteArray(bytes, qsizetype(length)));
        }
        case LUA_TTABLE: {
            const void *identity = lua_topointer(state, index);
            if (tables.contains(identity)) {
                throw ScriptException(
                    Exception::InvalidArgument,
                    QStringLiteral("Cyclic table cannot be copied"));
            }
            tables.insert(identity);
            result.kind = Value::Kind::Table;
            lua_pushnil(state);
            while (lua_next(state, index)) {
                auto key = readValue(state, -2, tables, depth + 1);
                auto value = readValue(state, -1, tables, depth + 1);
                result.entries.append(
                    qMakePair(std::move(key), std::move(value)));
                lua_pop(state, 1);
            }
            tables.remove(identity);
            return result;
        }
        default:
            if (readObject(state, index, result.object)) {
                if (!result.object.pointer || !result.object.type.rtti) {
                    throw ScriptException(Exception::TypeMismatch);
                }
                result.kind = Value::Kind::Object;
                return result;
            }
            throw ScriptException(Exception::TypeMismatch);
        }
    }

    void pushValue(lua_State *state, const Value &value, int depth) {
        if (depth > 64 || !lua_checkstack(state, 4)) {
            throw ScriptException(Exception::InvalidArgument);
        }
        if (codec_.pushValue && codec_.pushValue(state, value)) {
            return;
        }
        switch (value.kind) {
        case Value::Kind::Nil:
            lua_pushnil(state);
            break;
        case Value::Kind::Boolean:
            lua_pushboolean(state, value.boolean);
            break;
        case Value::Kind::Signed:
            if (value.signedNumber >= -9007199254740991LL &&
                value.signedNumber <= 9007199254740991LL) {
                lua_pushnumber(state, double(value.signedNumber));
            } else {
                lua_pushinteger64(state, value.signedNumber);
            }
            break;
        case Value::Kind::Unsigned:
            if (value.unsignedNumber >
                quint64(std::numeric_limits<qint64>::max())) {
                throw ScriptException(
                    Exception::TypeMismatch,
                    QStringLiteral("Integer exceeds Luau int64 range"));
            }
            if (value.unsignedNumber <= 9007199254740991ULL) {
                lua_pushnumber(state, double(value.unsignedNumber));
            } else {
                lua_pushinteger64(state, qint64(value.unsignedNumber));
            }
            break;
        case Value::Kind::Number:
            lua_pushnumber(state, value.number);
            break;
        case Value::Kind::Bytes:
            lua_pushlstring(state, value.bytes.constData(),
                            size_t(value.bytes.size()));
            break;
        case Value::Kind::Object: {
            if (!value.object.pointer || !value.object.type.rtti) {
                throw ScriptException(Exception::TypeMismatch);
            }
            if (codec_.push) {
                int top = lua_gettop(state);
                bool handled = codec_.push(state, value.object);
                if (lua_gettop(state) != top + (handled ? 1 : 0)) {
                    throw ScriptException(
                        Exception::RegistrationFailed,
                        QStringLiteral(
                            "Object writer violated its stack contract"));
                }
                if (handled) {
                    break;
                }
            }
            const auto scope = classScope(value.object.type);
            if (scope == invalidScope) {
                throw ScriptException(
                    Exception::TypeMismatch,
                    QStringLiteral("No binding registered for object type"));
            }
            auto *memory = lua_newuserdatadtor(
                state, sizeof(ObjectValue), [](lua_State *, void *data) {
                    static_cast<ObjectValue *>(data)->~ObjectValue();
                });
            new (memory) ObjectValue(value.object);
            lua_getref(state, get(scope).metatable);
            lua_setmetatable(state, -2);
            break;
        }
        case Value::Kind::Sequence:
            lua_createtable(state, int(value.sequence.size()), 0);
            for (qsizetype index = 0; index < value.sequence.size(); ++index) {
                pushValue(state, value.sequence[index], depth + 1);
                lua_rawseti(state, -2, int(index + 1));
            }
            break;
        case Value::Kind::Table:
            lua_createtable(state, 0, int(value.entries.size()));
            for (const auto &entry : value.entries) {
                if (entry.first.kind == Value::Kind::Nil ||
                    (entry.first.kind == Value::Kind::Number &&
                     std::isnan(entry.first.number))) {
                    throw ScriptException(Exception::TypeMismatch);
                }
                pushValue(state, entry.first, depth + 1);
                pushValue(state, entry.second, depth + 1);
                lua_rawset(state, -3);
            }
            break;
        }
    }

    ScopeId findMember(ScopeId scope, const QString &name,
                       ObjectValue *receiver) {
        const auto &record = get(scope);
        if (record.functions.contains(name) ||
            record.properties.contains(name)) {
            return scope;
        }
        for (const auto &base : record.bases) {
            auto converted = receiver ? *receiver : ObjectValue{};
            if (receiver) {
                converted.pointer = base.cast(converted.pointer);
                converted.type = base.type;
            }
            auto found = findMember(classScope(base.type), name,
                                    receiver ? &converted : nullptr);
            if (found != invalidScope) {
                if (receiver) {
                    *receiver = std::move(converted);
                }
                return found;
            }
        }
        return invalidScope;
    }

    bool castReceiver(ObjectValue &receiver, ScopeId target) {
        if (sameType(receiver.type, get(target).type)) {
            return true;
        }
        const auto scope = classScope(receiver.type);
        if (scope == invalidScope) {
            return false;
        }
        for (const auto &base : get(scope).bases) {
            auto converted = receiver;
            converted.type = base.type;
            converted.pointer = base.cast(receiver.pointer);
            if (castReceiver(converted, target)) {
                receiver = std::move(converted);
                return true;
            }
        }
        return false;
    }

    int invoke(lua_State *state, const QList<Callable> &overloads,
               const ObjectValue &receiver, int first) {
        QList<Value> arguments;
        QSet<const void *> tables;
        const int top = lua_gettop(state);
        for (int index = first; index <= top; ++index) {
            arguments.append(readValue(state, index, tables, 0));
        }
        for (int attempt = 0; attempt < 2; ++attempt) {
            for (const auto &callable : overloads) {
                if (callable->accepts(arguments)) {
                    auto result = callable->invoke(receiver, arguments);
                    if (callable->returnsVoid()) {
                        return 0;
                    }
                    pushValue(state, result, 0);
                    return 1;
                }
            }
            if (attempt != 0 || !codec_.adaptArguments) {
                break;
            }
            codec_.adaptArguments(arguments);
        }
        throw ScriptException(Exception::TypeMismatch,
                              QStringLiteral("No matching overload"));
    }

    int execute(lua_State *state, const Closure &closure) {
        checkThread();
        if (closure.operation == Operation::Call ||
            closure.operation == Operation::Construct) {
            ObjectValue receiver;
            int first = 1;
            auto scope = closure.scope;
            if (closure.instance) {
                if (!readObject(state, 1, receiver)) {
                    detail::conversionError();
                }
                if (!castReceiver(receiver, scope)) {
                    detail::conversionError();
                }
                first = 2;
            } else if (closure.operation == Operation::Construct) {
                first = 2;
            }
            return invoke(state,
                          get(scope).functions.value(closure.name).overloads,
                          receiver, first);
        }

        if (lua_type(state, 2) != LUA_TSTRING) {
            detail::conversionError();
        }
        const QString name = QString::fromUtf8(lua_tostring(state, 2));
        ObjectValue receiver;
        auto scope = closure.scope;
        if (closure.instance && !readObject(state, 1, receiver)) {
            detail::conversionError();
        }
        if (closure.instance && !castReceiver(receiver, scope)) {
            detail::conversionError();
        }
        scope = findMember(scope, name, closure.instance ? &receiver : nullptr);
        if (scope != invalidScope) {
            const auto &record = get(scope);
            auto property = record.properties.constFind(name);
            if (property != record.properties.cend() &&
                (!record.isClass || property->isStatic != closure.instance)) {
                if (closure.operation == Operation::Index) {
                    return invoke(state, {property->getter}, receiver, 3);
                }
                if (!property->setter) {
                    throw ScriptException(
                        Exception::InvalidArgument,
                        QStringLiteral("Property is read-only"));
                }
                return invoke(state, {property->setter}, receiver, 3);
            }
            auto function = record.functions.constFind(name);
            if (function != record.functions.cend() && !function->isStatic &&
                closure.instance && closure.operation == Operation::Index) {
                auto *memory = lua_newuserdatadtor(
                    state, sizeof(Closure), [](lua_State *, void *data) {
                        static_cast<Closure *>(data)->~Closure();
                    });
                new (memory)
                    Closure{control_, scope, name, Operation::Call, true};
                lua_pushcclosure(state, &dispatch, "C++ method", 1);
                return 1;
            }
        }
        if (closure.operation == Operation::Index) {
            lua_pushnil(state);
            return 1;
        }
        if (!closure.instance && !get(closure.scope).isClass &&
            scope == invalidScope) {
            lua_pushvalue(state, 2);
            lua_pushvalue(state, 3);
            lua_rawset(state, 1);
            return 0;
        }
        throw ScriptException(Exception::InvalidArgument,
                              QStringLiteral("Unknown or read-only member"));
    }

    static int dispatch(lua_State *state) {
        const int top = lua_gettop(state);
        auto *closure =
            static_cast<Closure *>(lua_touserdata(state, lua_upvalueindex(1)));
        try {
            if (!closure->control->engine) {
                throw ScriptException(
                    Exception::InvalidScope,
                    QStringLiteral("Binding engine has been destroyed"));
            }
            return closure->control->engine->execute(state, *closure);
        } catch (const ScriptException &error) {
            lua_settop(state, top);
            auto bytes = error.getErrorString().toUtf8();
            lua_pushlstring(state, bytes.constData(), size_t(bytes.size()));
        } catch (const std::exception &error) {
            const QByteArray message(error.what());
            lua_settop(state, top);
            lua_pushlstring(state, message.constData(), size_t(message.size()));
        } catch (...) {
            lua_settop(state, top);
            lua_pushliteral(state, "C++ binding failed");
        }
        lua_error(state);
    }

    lua_State *state_;
    ObjectCodec codec_;
    QThread *thread_;
    std::shared_ptr<Control> control_;
    QHash<ScopeId, Scope> scopes_;
    QHash<ScopeId, QSet<QString>> constants_;
    QHash<ScopeId, int> leases_;
    QSet<QString> definitionPaths_;
    QStringList definitionFiles_;
    ScopeId nextScope_ = 1;
};

ScriptBindingEngine::ScriptBindingEngine(lua_State *state, ObjectCodec codec)
    : impl_(std::make_unique<Impl>(state, std::move(codec))) {}

ScriptBindingEngine::~ScriptBindingEngine() = default;

IRegistrationApi &ScriptBindingEngine::registrations() noexcept {
    return *impl_;
}

void ScriptBindingEngine::collectGarbage() { impl_->collectGarbage(); }

ScriptSymbolCatalog ScriptBindingEngine::symbolCatalog() const {
    return impl_->symbolCatalog();
}

QStringList ScriptBindingEngine::definitionFiles() const {
    return impl_->definitionFiles();
}

} // namespace WingScriptBind
