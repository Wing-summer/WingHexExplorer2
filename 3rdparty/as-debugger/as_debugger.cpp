// MIT Licensed
// see https://github.com/Paril/angelscript-debugger

#include "as_debugger.h"
#include <array>
#include <bitset>
#include <charconv>
#include <string>

void asIDBVariable::Evaluate() {
    if (evaluated)
        return;
    // getters don't evaluate and are
    // just placeholders, but they need
    // a ref ID.
    else if (getter) {
        SetRefId();
        return;
    }

    auto var = ptr.lock();
    dbg.cache->GetEvaluator(var->address).Evaluate(var);
    evaluated = true;

    if (expandable)
        SetRefId();
}

void asIDBVariable::SetRefId() {
    if (expandRefId.has_value())
        return;

    auto &refs = dbg.cache->variable_refs;

    int64_t next_id = refs.size() + 1;
    expandRefId = next_id;
    refs.emplace(next_id, ptr);
}

void asIDBVariable::Expand() {
    Evaluate();

    if (expanded)
        return;
    else if (!expandRefId)
        return;

    expanded = true;

    auto var = ptr.lock();

    if (!getter) {
        dbg.cache->GetEvaluator(var->address).Expand(var);
        return;
    }

    // getters are a bit special; we have to fetch the variable
    // that our getter is linked to, & store the result in stack memory.
    auto ctx = dbg.cache->ctx;

    dbg.internal_execution = true;
    ctx->PushState();

    ctx->Prepare(getter);
    ctx->SetObject(this->owner.lock()->address.ResolveAs<void>());
    ctx->Execute();

    var->namedProps.clear();
    var->indexedProps.clear();

    if (ctx->GetState() != asEXECUTION_FINISHED) {
        var->get_evaluated = var->CreateChildVariable(var->identifier, {}, "");
        var->get_evaluated->value =
            fmt::format("Exception thrown ({})", ctx->GetExceptionString());
        var->get_evaluated->evaluated = true;
    } else {
        asDWORD returnFlags;
        int typeId = getter->GetReturnTypeId(&returnFlags);
        asIDBValue returnValue(ctx->GetEngine(), ctx->GetAddressOfReturnValue(),
                               typeId, (returnFlags & asTM_INOUTREF) != 0);

        asIDBVariable::Ptr child = var->CreateChildVariable(
            var->identifier, {typeId, (returnFlags & asTM_CONST) != 0, nullptr},
            dbg.cache->GetTypeNameFromType(
                {typeId, (asETypeModifiers)returnFlags}));
        child->stackValue = std::move(returnValue);
        child->address.address = child->stackValue.GetPointer<void>(true);
    }

    ctx->PopState();
    dbg.internal_execution = false;
}

asIDBVariable::Ptr asIDBVariable::CreateChildVariable(
    asIDBVarName identifier, asIDBVarAddr address, std::string_view typeName) {
    asIDBVariable::Ptr child = dbg.cache->CreateVariable();
    child->owner = ptr;
    child->identifier = identifier;
    child->address = address;
    child->typeName = typeName;
    if (identifier.name[0] == '[')
        indexedProps.push_back(child);
    else
        namedProps.insert(child);
    return child;
}

asIDBScope::asIDBScope(asUINT offset, asIDBDebugger &dbg,
                       asIScriptFunction *function)
    : offset(offset), parameters(dbg.cache->CreateVariable()),
      locals(dbg.cache->CreateVariable()),
      registers(dbg.cache->CreateVariable()) {
    CalcLocals(dbg, function, parameters);
    CalcLocals(dbg, function, locals);
    CalcLocals(dbg, function, registers);
}

void asIDBScope::CalcLocals(asIDBDebugger &dbg, asIScriptFunction *function,
                            asIDBVariable::Ptr &container) {
    if (!function || offset == SCOPE_SYSTEM)
        return;

    auto &cache = *dbg.cache.get();
    auto ctx = cache.ctx;
    asUINT numParams = function->GetParamCount();
    asUINT numLocals = ctx->GetVarCount(offset);

    asUINT start = 0, end = 0;

    if (container == parameters)
        end = numParams;
    else {
        start = numParams;
        end = numLocals;
    }

    if (container == locals) {
        if (auto thisPtr = ctx->GetThisPointer(offset)) {
            int thisTypeId = ctx->GetThisTypeId(offset);

            asIDBTypeId typeKey{thisTypeId, asTM_NONE};

            const auto viewType = cache.GetTypeNameFromType(typeKey);

            asIDBVarAddr idKey{thisTypeId, false, thisPtr};

            asIDBVariable::Ptr var =
                container->CreateChildVariable("this", idKey, viewType);

            this_ptr = var;
        }
    }

    for (asUINT n = start; n < end; n++) {
        const char *name;
        int typeId;
        asETypeModifiers modifiers;
        int stackOffset;
        ctx->GetVar(n, offset, &name, &typeId, &modifiers, 0, &stackOffset);

        bool isTemporary = (container != parameters) && (!name || !*name);

        if (!ctx->IsVarInScope(n, offset))
            continue;
        else if (isTemporary != (container == registers))
            continue;

        void *ptr = ctx->GetAddressOfVar(n, offset);

        asIDBTypeId typeKey{typeId, modifiers};

        std::string localName = (name && *name)
                                    ? fmt::format("{} (&{})", name, n)
                                    : fmt::format("&{}", n);

        const auto viewType = cache.GetTypeNameFromType(typeKey);

        asIDBVarAddr idKey{typeId, (modifiers & asTM_CONST) != 0, ptr};

        asIDBVariable::Ptr var = container->CreateChildVariable(
            std::move(localName), idKey, viewType);

        local_by_index.emplace(n, var);
    }

    container->evaluated = container->expanded = true;

    if (!container->namedProps.empty() || !container->indexedProps.empty())
        container->SetRefId();
}

/*virtual*/ void asIDBCache::Refresh() {}

/*virtual*/ const std::string asIDBCache::GetTypeNameFromType(asIDBTypeId id) {
    if (auto f = type_names.find(id); f != type_names.end())
        return f->second;

    auto type = ctx->GetEngine()->GetTypeInfoById(id.typeId);
    std::string rawName;

    if (!type) {
        // a primitive
        switch (id.typeId & asTYPEID_MASK_SEQNBR) {
        case asTYPEID_BOOL:
            rawName = "bool";
            break;
        case asTYPEID_INT8:
            rawName = "int8";
            break;
        case asTYPEID_INT16:
            rawName = "int16";
            break;
        case asTYPEID_INT32:
            rawName = "int32";
            break;
        case asTYPEID_INT64:
            rawName = "int64";
            break;
        case asTYPEID_UINT8:
            rawName = "uint8";
            break;
        case asTYPEID_UINT16:
            rawName = "uint16";
            break;
        case asTYPEID_UINT32:
            rawName = "uint32";
            break;
        case asTYPEID_UINT64:
            rawName = "uint64";
            break;
        case asTYPEID_FLOAT:
            rawName = "float";
            break;
        case asTYPEID_DOUBLE:
            rawName = "double";
            break;
        default:
            rawName = "???";
            break;
        }
    } else {
        auto ns = type->GetNamespace();
        if (ns && strlen(ns)) {
            rawName = fmt::format("{}::{}", ns, type->GetName());
        } else {
            rawName = type->GetName();
        }
    }

    std::string name = fmt::format(
        "{}{}{}{}", (id.modifiers & asTM_CONST) ? "const " : "", rawName,
        (id.typeId & (asTYPEID_HANDLETOCONST | asTYPEID_OBJHANDLE)) ? "@" : "",
        ((id.modifiers & asTM_INOUTREF) == asTM_INOUTREF) ? "&"
        : ((id.modifiers & asTM_INOUTREF) == asTM_INREF)  ? "&in"
        : ((id.modifiers & asTM_INOUTREF) == asTM_OUTREF) ? "&out"
                                                          : "");

    type_names.emplace(id, name);
    return type_names.at(id);
}

void *asIDBCache::ResolvePropertyAddress(const asIDBVarAddr &id,
                                         int propertyIndex, int offset,
                                         int compositeOffset,
                                         bool isCompositeIndirect) {
    if (id.typeId & asTYPEID_SCRIPTOBJECT) {
        asIScriptObject *obj = id.ResolveAs<asIScriptObject>();
        return obj->GetAddressOfProperty(propertyIndex);
    }

    // indirect changes our ptr to
    // *(object + compositeOffset) + offset
    if (isCompositeIndirect) {
        void *propAddr = *reinterpret_cast<uint8_t **>(id.ResolveAs<uint8_t>() +
                                                       compositeOffset);

        // if we're null, leave it alone, otherwise point to
        // where we really need to be pointing
        if (propAddr)
            propAddr = reinterpret_cast<uint8_t *>(propAddr) + offset;

        return propAddr;
    }

    return id.ResolveAs<uint8_t>() + offset + compositeOffset;
}

/*virtual*/ asIDBExpected<asIDBVariable::WeakPtr>
asIDBCache::ResolveExpression(std::string_view expr,
                              std::optional<int> stack_index) {
    // just in case your IDE sends `@ent` or `&ent` for a hover
    if (!expr.empty() &&
        (expr[0] == '@' ||
         (expr.size() >= 2 && expr[0] == '&' && !isdigit(expr[1]))))
        expr.remove_prefix(1);

    if (expr.empty())
        return asIDBExpected("empty string");

    CacheCallstack();

    // isolate the variable name first
    size_t variable_end = expr.find_first_of(".[", 0);
    std::string_view variable_name = expr.substr(0, variable_end);

    if (variable_name.empty())
        return asIDBExpected("bad expression");

    asIDBExpected<asIDBVariable::WeakPtr> variable;
    asIDBCallStackEntry *stack = nullptr;

    if (stack_index.has_value()) {
        if (call_stack.size() > stack_index.value()) {
            stack = &call_stack[stack_index.value()];
        }
    }

    // if it starts with a & it has to be a local variable index
    if (stack && variable_name[0] == '&') {
        uint32_t offset;
        auto result = std::from_chars(
            &variable_name.front(),
            &variable_name.front() + variable_name.size(), offset);

        if (result.ec != std::errc())
            return asIDBExpected("invalid numerical offset");

        // check bounds
        int m = ctx->GetVarCount(stack_index.value());

        if (m < 0)
            return asIDBExpected("bad stack index");

        if (offset >= (asUINT)m)
            return asIDBExpected("stack offset out of bounds");

        if (!ctx->IsVarInScope(offset, stack_index.value()))
            return asIDBExpected("variable out of scope");

        if (auto varit = stack->scope.local_by_index.find(offset);
            varit != stack->scope.local_by_index.end())
            variable = varit->second;
        else
            return asIDBExpected("missing local index");
    }
    // check this
    else if (stack && variable_name == "this") {
        if (stack->scope.this_ptr.expired())
            return asIDBExpected("not a method");

        variable = stack->scope.this_ptr;
    } else {
        struct asIDBNamespacedVar {
            asIDBVariable::WeakPtr var;
            std::string_view name;
            std::string_view ns;
        };

        std::vector<asIDBNamespacedVar> matches;
        std::string_view variable_ns;

        if (auto ns_end = variable_name.find_last_of(':');
            ns_end != std::string_view::npos) {
            variable_ns = variable_name.substr(0, ns_end - 1);
            variable_name = variable_name.substr(ns_end + 1);
        }

        if (stack) {
            // not an offset; in order, check the following:
            // - local variables (in reverse order)
            // - function parameters
            // - class member properties (if appropriate)
            // - globals
            for (int i = ctx->GetVarCount(stack_index.value()) - 1; i >= 0;
                 i--) {
                if (!ctx->IsVarInScope(i, stack_index.value()))
                    continue;

                const char *name;
                int typeId;
                asETypeModifiers modifiers;
                ctx->GetVar(i, stack_index.value(), &name, &typeId, &modifiers);

                if (variable_name != name)
                    continue;

                if (auto varit = stack->scope.local_by_index.find(i);
                    varit != stack->scope.local_by_index.end())
                    matches.push_back({varit->second, name});

                break;
            }

            // check `this` parameters
            if (!stack->scope.this_ptr.expired()) {
                auto var = stack->scope.this_ptr.lock();
                var->Expand();

                for (auto &param : var->namedProps)
                    if (variable_name == param->identifier.name)
                        matches.push_back({param, param->identifier.name});
                for (auto &param : var->indexedProps)
                    if (variable_name == param->identifier.name)
                        matches.push_back({param, param->identifier.name});
            }
        }

        // check globals
        CacheGlobals();

        for (auto &global : globals->namedProps)
            if (variable_name == global->identifier.name)
                matches.push_back(
                    {global, global->identifier.name, global->identifier.ns});
        for (auto &global : globals->indexedProps)
            if (variable_name == global->identifier.name)
                matches.push_back(
                    {global, global->identifier.name, global->identifier.ns});

        if (matches.size() == 1)
            variable = matches[0].var;
        // if we didn't specify a ns but had multiple
        // matches, return an error
        else if (variable_ns.empty())
            return asIDBExpected(matches.empty() ? "can't find variable"
                                                 : "ambiguous variable name");
        else {
            for (auto &match : matches) {
                if (variable_ns == match.ns) {
                    variable = match.var;
                    break;
                }
            }
        }

        if (!variable)
            return asIDBExpected("can't find variable");
    }

    // variable_key should be non-null and with
    // a valid type ID here.
    return ResolveSubExpression(variable.value(),
                                variable_end == std::string_view::npos
                                    ? std::string_view{}
                                    : expr.substr(variable_end));
}

/*virtual*/ asIDBExpected<asIDBVariable::WeakPtr>
asIDBCache::ResolveSubExpression(asIDBVariable::WeakPtr var,
                                 const std::string_view rest) {
    // nothing left, so this is the result.
    if (rest.empty())
        return var;

    // make sure we're a type that supports properties
    auto varp = var.lock();

    varp->Evaluate();

    if (!varp->expandRefId)
        return asIDBExpected("invalid expression");

    varp->Expand();

    if (varp->namedProps.empty() && varp->indexedProps.empty())
        return asIDBExpected("no members");

    // check what kind of sub-evaluator to use
    size_t eval_start = rest.find_first_of(".[", 1);
    std::string_view eval_name = rest.substr(0, eval_start);

    if (eval_name[0] == '.')
        eval_name.remove_prefix(1);

    for (auto &child : varp->namedProps) {
        if (child->identifier.name == eval_name)
            return ResolveSubExpression(child,
                                        eval_start == std::string_view::npos
                                            ? std::string_view{}
                                            : rest.substr(eval_start));
    }

    for (auto &child : varp->indexedProps) {
        if (child->identifier.name == eval_name)
            return ResolveSubExpression(child,
                                        eval_start == std::string_view::npos
                                            ? std::string_view{}
                                            : rest.substr(eval_start));
    }

    return asIDBExpected("can't resolve sub-expression");
}

/*virtual*/ void asIDBCache::CacheCallstack() {
    if (!ctx || !call_stack.empty())
        return;

    if (auto sysfunc = ctx->GetSystemFunction())
        call_stack.emplace_back(asIDBCallStackEntry{
            dbg.frame_offset++, sysfunc->GetDeclaration(true, false, true), "",
            0, 0, asIDBScope(SCOPE_SYSTEM, dbg, sysfunc)});

    for (asUINT n = 0; n < ctx->GetCallstackSize(); n++) {
        asIScriptFunction *func = nullptr;
        int column = 0;
        const char *section = "";
        int row = 0;

        // FIXME: check this, because this will skip GetFunction(n).
        // I think this is correct though...?
        if (n == 0 && ctx->GetState() == asEXECUTION_EXCEPTION) {
            func = ctx->GetExceptionFunction();
            if (func)
                row = ctx->GetExceptionLineNumber(&column, &section);
        } else {
            func = ctx->GetFunction(n);
            if (func)
                row = ctx->GetLineNumber(n, &column, &section);
        }

        std::string decl;

        if (func)
            decl = func->GetDeclaration(true, false, true);
        else
            decl = "???"; // FIXME: why does this happen?

        call_stack.push_back(asIDBCallStackEntry{
            dbg.frame_offset++, std::move(decl), section, row, column,
            asIDBScope(
                func && func->GetFuncType() == asFUNC_SYSTEM ? SCOPE_SYSTEM : n,
                dbg, func)});
    }
}

// restore data from the given cache that is
// being replaced by this one.
/*virtual*/ void asIDBCache::Restore(asIDBCache &cache) {}

/*virtual*/ void asIDBCache::CacheGlobals() {
    if (!ctx)
        return;

    if (!globals)
        globals = CreateVariable();

    if (globals->expanded)
        return;

    auto fn = ctx->GetFunction(0);
    if (fn == nullptr) {
        return;
    }

    auto main = fn->GetModule();
    if (main == nullptr) {
        return;
    }

    for (asUINT n = 0; n < main->GetGlobalVarCount(); n++) {
        const char *name;
        const char *nameSpace;
        int typeId;
        void *ptr;
        bool isConst;

        main->GetGlobalVar(n, &name, &nameSpace, &typeId, &isConst);
        ptr = main->GetAddressOfGlobalVar(n);

        asIDBTypeId typeKey{typeId, isConst ? asTM_CONST : asTM_NONE};
        const auto viewType = GetTypeNameFromType(typeKey);

        asIDBVarAddr idKey{typeId, isConst, ptr};

        globals->CreateChildVariable(
            asIDBVarName((nameSpace && nameSpace[0]) ? nameSpace : "", name),
            idKey, viewType);
    }

    for (asUINT n = 0; n < main->GetEngine()->GetGlobalPropertyCount(); n++) {
        const char *name;
        const char *nameSpace;
        int typeId;
        void *ptr;
        bool isConst;

        main->GetEngine()->GetGlobalPropertyByIndex(
            n, &name, &nameSpace, &typeId, &isConst, nullptr, &ptr);

        asIDBTypeId typeKey{typeId, isConst ? asTM_CONST : asTM_NONE};
        const auto viewType = GetTypeNameFromType(typeKey);

        asIDBVarAddr idKey{typeId, isConst, ptr};

        std::string localName = (nameSpace && nameSpace[0])
                                    ? fmt::format("{}::{}", nameSpace, name)
                                    : name;

        globals->CreateChildVariable(std::move(localName), idKey, viewType);
    }

    globals->evaluated = globals->expanded = true;

    if (!globals->namedProps.empty() || !globals->indexedProps.empty())
        globals->SetRefId();
}

class asIDBNullTypeEvaluator : public asIDBTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        var->value = "<null>";
    }
};

class asIDBUninitTypeEvaluator : public asIDBTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        var->value = "<uninit>";
    }
};

class asIDBEnumTypeEvaluator : public asIDBTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        auto &dbg = var->dbg;

        // for enums where we have a single matched value
        // just display it directly; it might be a mask but that's OK.
        auto type =
            dbg.cache->ctx->GetEngine()->GetTypeInfoById(var->address.typeId);

        union {
            asINT64 v = 0;
            asQWORD uv;
        };

        switch (type->GetTypedefTypeId()) {
        case asTYPEID_INT8:
            v = *var->address.ResolveAs<const int8_t>();
            break;
        case asTYPEID_UINT8:
            uv = *var->address.ResolveAs<const uint8_t>();
            break;
        case asTYPEID_INT16:
            v = *var->address.ResolveAs<const int16_t>();
            break;
        case asTYPEID_UINT16:
            uv = *var->address.ResolveAs<const uint16_t>();
            break;
        case asTYPEID_INT32:
            v = *var->address.ResolveAs<const int32_t>();
            break;
        case asTYPEID_UINT32:
            uv = *var->address.ResolveAs<const uint32_t>();
            break;
        case asTYPEID_INT64:
            v = *var->address.ResolveAs<const int64_t>();
            break;
        case asTYPEID_UINT64:
            uv = *var->address.ResolveAs<const uint64_t>();
            break;
        }

        for (asUINT e = 0; e < type->GetEnumValueCount(); e++) {
            int ov = 0;
            const char *name = type->GetEnumValueByIndex(e, &ov);

            if (ov == v) {
                if (type->GetTypedefTypeId() >= asTYPEID_UINT8 &&
                    type->GetTypedefTypeId() <= asTYPEID_UINT64) {
                    var->value = fmt::format("{} ({})", name, uv);
                    return;
                }

                var->value = fmt::format("{} ({})", name, v);
                return;
            }
        }

        std::bitset<32> bits(v);

        if (bits.count() == 1) {
            if (type->GetTypedefTypeId() >= asTYPEID_UINT8 &&
                type->GetTypedefTypeId() <= asTYPEID_UINT64) {
                var->value = fmt::format("{}", uv);
                return;
            }

            var->value = fmt::format("{}", v);
            return;
        }

        var->value = fmt::format("{} bits", bits.count());
        var->expandable = true;
    }

    virtual void Expand(asIDBVariable::Ptr var) const override {
        auto &dbg = var->dbg;
        auto &cache = *dbg.cache;
        auto type =
            cache.ctx->GetEngine()->GetTypeInfoById(var->address.typeId);

        union {
            asINT64 v = 0;
            asQWORD uv;
        };

        switch (type->GetTypedefTypeId()) {
        case asTYPEID_INT8:
            v = *var->address.ResolveAs<const int8_t>();
            break;
        case asTYPEID_UINT8:
            uv = *var->address.ResolveAs<const uint8_t>();
            break;
        case asTYPEID_INT16:
            v = *var->address.ResolveAs<const int16_t>();
            break;
        case asTYPEID_UINT16:
            uv = *var->address.ResolveAs<const uint16_t>();
            break;
        case asTYPEID_INT32:
            v = *var->address.ResolveAs<const int32_t>();
            break;
        case asTYPEID_UINT32:
            uv = *var->address.ResolveAs<const uint32_t>();
            break;
        case asTYPEID_INT64:
            v = *var->address.ResolveAs<const int64_t>();
            break;
        case asTYPEID_UINT64:
            uv = *var->address.ResolveAs<const uint64_t>();
            break;
        }

        {
            std::string rawValue;

            if (type->GetTypedefTypeId() >= asTYPEID_UINT8 &&
                type->GetTypedefTypeId() <= asTYPEID_UINT64)
                rawValue = fmt::format("{}", uv);
            else
                rawValue = fmt::format("{}", v);

            auto child = var->CreateChildVariable("value", {}, "");
            child->value = std::move(rawValue);
            child->evaluated = true;
        }

        // find bit names
        int ov = 0;
        std::array<const char *, sizeof(ov) * 8> bit_names{};

        for (asUINT e = 0; e < type->GetEnumValueCount(); e++) {
            const char *name = type->GetEnumValueByIndex(e, &ov);
            std::bitset<sizeof(ov) * 8> obits(ov);

            // skip masks
            if (obits.count() != 1)
                continue;

            if (ov & v) {
                int p = 0;

                while (ov && !(ov & 1)) {
                    ov >>= 1;
                    p++;
                }

                // only take the first name, just incase
                // there's later overrides
                if (p <= (obits.size() - 1) && !bit_names[p])
                    bit_names[p] = name;
            }
        }

        // display bits
        for (asQWORD e = 0; e < bit_names.size(); e++) {
            if (v & (1ull << e)) {
                std::string bitEntry;

                if (bit_names[e])
                    bitEntry = bit_names[e];
                else
                    bitEntry = fmt::format("{}", 1 << e);

                auto child = var->CreateChildVariable(
                    fmt::format("[{:{}}]", e, type->GetSize() == 1 ? 1 : 2), {},
                    "");
                child->value = std::move(bitEntry);
                child->evaluated = true;
            }
        }
    }
};

class asIDBFuncDefTypeEvaluator : public asIDBTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        asIScriptFunction *ptr = var->address.ResolveAs<asIScriptFunction>();
        auto &dbg = var->dbg;
        var->value = ptr->GetName();
    }
};

/*virtual*/ void
asIDBObjectTypeEvaluator::Evaluate(asIDBVariable::Ptr var) const /*override*/
{
    auto &dbg = var->dbg;
    auto &cache = *dbg.cache;
    auto ctx = cache.ctx;
    auto type = ctx->GetEngine()->GetTypeInfoById(var->address.typeId);

    var->expandable = CanExpand(var);

    if (type == nullptr) {
        return;
    }

    var->typeName = cache.GetTypeNameFromType(var->address.typeId);

    if (ctx->GetState() != asEXECUTION_EXCEPTION) {
        asIDBObjectIteratorHelper it(type, var->address.ResolveAs<void>());

        if (!it) {
            if (!it.error.empty()) {
                var->value = std::string(it.error);
                return;
            }

            if (var->value.empty())
                var->value = fmt::format("{{{}}}", var->typeName);
        } else {
            dbg.internal_execution = true;
            size_t numElements = it.CalculateLength(ctx);
            dbg.internal_execution = false;

            if (var->value.empty())
                var->value = fmt::format("{} elements", numElements);

            if (numElements)
                var->expandable = true;
        }
    }
}

/*virtual*/ void
asIDBObjectTypeEvaluator::Expand(asIDBVariable::Ptr var) const /*override*/
{
    QueryVariableProperties(var);

    QueryVariableGetters(var);

    QueryVariableForEach(var);
}

// convenience function that queries the properties of the given
// address (and object, if set) of the given type.
void asIDBObjectTypeEvaluator::QueryVariableProperties(
    asIDBVariable::Ptr var) const {
    auto &dbg = var->dbg;
    auto &cache = *dbg.cache;
    auto type = cache.ctx->GetEngine()->GetTypeInfoById(var->address.typeId);

    for (asUINT n = 0; n < type->GetPropertyCount(); n++) {
        const char *name;
        int propTypeId;
        void *propAddr = nullptr;
        int offset;
        int compositeOffset;
        bool isCompositeIndirect;
        bool isReadOnly;

        type->GetProperty(n, &name, &propTypeId, 0, 0, &offset, 0, 0,
                          &compositeOffset, &isCompositeIndirect, &isReadOnly);

        propAddr = cache.ResolvePropertyAddress(
            var->address, n, offset, compositeOffset, isCompositeIndirect);

        asIDBVarAddr propId{propTypeId, isReadOnly, propAddr};

        // TODO: variables that overlap memory space will
        // get culled by this. this helps in the case of
        // vec3_t::x and vec3_t::pitch for instance, but
        // causes some confusion for edict_t::number and
        // edict_t::s::number, where `s` is now just an empty
        // struct. it'd be ideal if, in this case, it prefers
        // the deeper nested ones. not sure how we'd express that
        // with the limited context we have, though.

        // TODO 2.0: this causes an issue with Watch variables
        // because of the way dereferencing works. For now, it
        // will add duplicates, and the old var state cache is gone.
        var->CreateChildVariable(
            name, propId,
            cache.GetTypeNameFromType(
                {propTypeId, isReadOnly ? asTM_CONST : asTM_NONE}));
    }
}

// convenience function that queries for getter property functions.
void asIDBObjectTypeEvaluator::QueryVariableGetters(
    asIDBVariable::Ptr var) const {
    auto &dbg = var->dbg;
    auto &cache = *dbg.cache;
    auto type = cache.ctx->GetEngine()->GetTypeInfoById(var->address.typeId);

    for (asUINT n = 0; n < type->GetMethodCount(); n++) {
        asIScriptFunction *function = type->GetMethodByIndex(n, true);

        if (!IsCompatibleGetter(function))
            continue;

        auto child = var->CreateChildVariable(
            std::string(std::string_view(function->GetName()).substr(4)), {},
            cache.GetTypeNameFromType(
                {function->GetReturnTypeId(), asTM_NONE}));
        child->getter = function;
        child->Evaluate();
    }
}

bool asIDBObjectTypeEvaluator::CanExpand(asIDBVariable::Ptr var) const {
    auto &dbg = var->dbg;
    auto &cache = *dbg.cache;
    auto type = cache.ctx->GetEngine()->GetTypeInfoById(var->address.typeId);

    if (type->GetPropertyCount())
        return true;

    for (asUINT n = 0; n < type->GetMethodCount(); n++) {
        asIScriptFunction *function = type->GetMethodByIndex(n, true);

        if (IsCompatibleGetter(function))
            return true;
    }

    return false;
}

bool asIDBObjectTypeEvaluator::IsCompatibleGetter(
    asIScriptFunction *function) const {
    return function->IsReadOnly() && function->IsProperty() &&
           function->GetParamCount() == 0;
}

// convenience function that iterates the opFor* of the given
// address (and object, if set) of the given type. If positive,
// a specific index will be used.
void asIDBObjectTypeEvaluator::QueryVariableForEach(asIDBVariable::Ptr var,
                                                    int index) const {
    auto &dbg = var->dbg;
    auto &cache = *dbg.cache;
    auto ctx = cache.ctx;

    if (ctx->GetState() == asEXECUTION_EXCEPTION)
        return;

    auto type = ctx->GetEngine()->GetTypeInfoById(var->address.typeId);

    dbg.internal_execution = true;
    asIDBObjectIteratorHelper it(type, var->address.ResolveAs<void>());

    if (!it) {
        dbg.internal_execution = false;
        return;
    }

    ctx->PushState();

    auto itValue = it.Begin(ctx);
    int elementId = 0;
    bool multiElement = index == -1 && it.opForValues.size() > 1;

    while (true) {
        if (it.End(ctx, itValue))
            break;

        asIDBVariable::Ptr indexVar;

        // if we're a multi-element, the root is fake
        // and just exists to store the element id.
        if (multiElement) {
            indexVar =
                var->CreateChildVariable(fmt::format("[{}]", elementId), {},
                                         "" // FIXME: could show types as tuple?
                );
            indexVar->expanded = indexVar->evaluated = true;
        }

        for (int offset = (index == -1 ? 0 : index), visibleOffset = 0;
             offset < (index == -1 ? it.opForValues.size() : index + 1);
             offset++, visibleOffset++) {
            it.Value(ctx, itValue, offset);

            asDWORD returnFlags;
            int typeId = it.opForValues[offset]->GetReturnTypeId(&returnFlags);
            asIDBValue returnValue(ctx->GetEngine(),
                                   ctx->GetAddressOfReturnValue(), typeId,
                                   (returnFlags & asTM_INOUTREF) != 0);

            auto child =
                (multiElement ? indexVar : var)
                    ->CreateChildVariable(
                        fmt::format("[{}]",
                                    multiElement ? visibleOffset : elementId),
                        {typeId, (returnFlags & asTM_CONST) != 0, nullptr},
                        dbg.cache->GetTypeNameFromType(
                            {typeId, (asETypeModifiers)returnFlags}));
            child->stackValue = std::move(returnValue);
            child->address.address = child->stackValue.GetPointer<void>(true);
        }

        itValue = it.Next(ctx, itValue);

        elementId++;
    }

    ctx->PopState();
    cache.dbg.internal_execution = false;
}

const asIDBTypeEvaluator &
asIDBCache::GetEvaluator(const asIDBVarAddr &id) const {
    // the only way the base address is null is if
    // it's uninitialized.
    static constexpr const asIDBUninitTypeEvaluator uninitType;
    static constexpr const asIDBNullTypeEvaluator nullType;

    if (id.address == nullptr)
        return uninitType;
    else if (id.ResolveAs<void>() == nullptr)
        return nullType;

    auto type = ctx->GetEngine()->GetTypeInfoById(id.typeId);

    // we'll use the fall back evaluators.
    // check primitives first.
#define CHECK_PRIMITIVE_EVAL(asTypeId, cTypeName)                              \
    if (id.typeId == asTypeId) {                                               \
        static constexpr const asIDBPrimitiveTypeEvaluator<cTypeName>          \
            cTypeName##Type;                                                   \
        return cTypeName##Type;                                                \
    }

    CHECK_PRIMITIVE_EVAL(asTYPEID_BOOL, bool);
    CHECK_PRIMITIVE_EVAL(asTYPEID_INT8, int8_t);
    CHECK_PRIMITIVE_EVAL(asTYPEID_INT16, int16_t);
    CHECK_PRIMITIVE_EVAL(asTYPEID_INT32, int32_t);
    CHECK_PRIMITIVE_EVAL(asTYPEID_INT64, int64_t);
    CHECK_PRIMITIVE_EVAL(asTYPEID_UINT8, uint8_t);
    CHECK_PRIMITIVE_EVAL(asTYPEID_UINT16, uint16_t);
    CHECK_PRIMITIVE_EVAL(asTYPEID_UINT32, uint32_t);
    CHECK_PRIMITIVE_EVAL(asTYPEID_UINT64, uint64_t);
    CHECK_PRIMITIVE_EVAL(asTYPEID_FLOAT, float);
    CHECK_PRIMITIVE_EVAL(asTYPEID_DOUBLE, double);

#undef CHECK_PRIMITIVE_EVAL

    if (type->GetFlags() & asOBJ_ENUM) {
        static constexpr const asIDBEnumTypeEvaluator enumType;
        return enumType;
    } else if (type->GetFlags() & asOBJ_FUNCDEF) {
        static constexpr const asIDBFuncDefTypeEvaluator funcdefType;
        return funcdefType;
    }

    // finally, just return the base one.
    static constexpr const asIDBObjectTypeEvaluator objectType;
    return objectType;
}

#include <filesystem>
#include <fstream>

std::string asIDBFileWorkspace::PathToSection(const std::string_view v) const {
    return std::filesystem::relative(v, base_path).generic_string();
}

std::string asIDBFileWorkspace::SectionToPath(const std::string_view v) const {
    return (base_path + '/').append(v);
}

std::string asIDBFileWorkspace::SectionSource(const std::string_view v) const {
    std::filesystem::path p(SectionToPath(v));
    size_t size = std::filesystem::file_size(p);
    std::string data(size, '\0');
    std::ifstream in(p);
    in.read(data.data(), size);

    // not sure why this is necessary but here we go
    if (auto nul = data.find_first_of('\0'))
        data.erase(nul);

    return data;
}

void asIDBFileWorkspace::CompileScriptSources() {
    for (size_t i = 0; i < engine->GetModuleCount(); i++) {
        auto module = engine->GetModuleByIndex(i);

        for (size_t f = 0; f < module->GetFunctionCount(); f++) {
            const char *section;
            module->GetFunctionByIndex(f)->GetDeclaredAt(&section, nullptr,
                                                         nullptr);
            AddSection(section);
        }
    }
}

void asIDBFileWorkspace::CompileBreakpointPositions() {
    potential_breakpoints.clear();

    auto addFunctionBreakpointLocations = [&](asIScriptFunction *func) {
        asUINT total = func->GetLineEntryCount();
        for (asUINT i = 0; i < total; i++) {
            const char *section;
            int line, col;
            func->GetLineEntry(i, &line, &col, &section, nullptr);
            potential_breakpoints[section].insert(asIDBLineCol{line, col});
        }
    };

    for (size_t i = 0; i < engine->GetModuleCount(); i++) {
        asIScriptModule *module = engine->GetModuleByIndex(i);

        for (size_t f = 0; f < module->GetFunctionCount(); f++)
            addFunctionBreakpointLocations(module->GetFunctionByIndex(f));

        for (size_t t = 0; t < module->GetObjectTypeCount(); t++) {
            asITypeInfo *type = module->GetObjectTypeByIndex(t);

            for (size_t m = 0; m < type->GetMethodCount(); m++)
                addFunctionBreakpointLocations(
                    type->GetMethodByIndex(m, false));

            for (size_t m = 0; m < type->GetBehaviourCount(); m++)
                addFunctionBreakpointLocations(
                    type->GetBehaviourByIndex(m, nullptr));

            for (size_t m = 0; m < type->GetFactoryCount(); m++)
                addFunctionBreakpointLocations(type->GetFactoryByIndex(m));
        }
    }
}

/*static*/ void asIDBDebugger::LineCallback(asIScriptContext *ctx,
                                            asIDBDebugger *debugger) {
    if (debugger->internal_execution)
        return;

    if (debugger->onLineCallBack) {
        auto r = debugger->onLineCallBack(ctx);
        if (!r) {
            // cancelled
            return;
        }
    }

    const char *section = nullptr;
    int col;
    int row = ctx->GetLineNumber(0, &col, &section);

    // we might not have an action - functions called from within
    // the debugger will never have this set.
    if (debugger->action != asIDBAction::None) {
        // Step Into just breaks on whatever happens to be next.
        if (debugger->action == asIDBAction::Pause ||
            debugger->action == asIDBAction::StepInto) {
            debugger->DebugBreak(ctx);
            return;
        }

        // Step Over breaks on the next line that is <= the
        // current stack level.
        else if (debugger->action == asIDBAction::StepOver) {
            if (ctx->GetCallstackSize() <= debugger->stack_size)
                debugger->DebugBreak(ctx);
            return;
        }
        // Step Out breaks on the next line that is < the
        // current stack level.
        else if (debugger->action == asIDBAction::StepOut) {
            if (ctx->GetCallstackSize() < debugger->stack_size)
                debugger->DebugBreak(ctx);
            return;
        }
    }

    // breakpoints are handled here. note that a single
    // breakpoint can be hit by multiple things on the same
    // line.
    bool break_from_bp = false;

    if (section) {
        std::scoped_lock lock(debugger->mutex);

        if (auto entries = debugger->breakpoints.find(section);
            entries != debugger->breakpoints.end()) {
            // Did we move into a new function?
            asIScriptFunction *func = ctx->GetFunction();

            if (debugger->_lastFunction != func) {
                // Check if any breakpoints need adjusting
                for (auto &bp : entries->second) {
                    // We need to check for a breakpoint at entering the
                    // function

                    // Check if a given breakpoint fall on a line with code or
                    // else adjust it to the next line
                    if (bp.needAdjust) {
                        int line = func->FindNextLineWithCode(bp.line);
                        if (line >= 0) {
                            bp.needAdjust = false;
                            if (line != bp.line) {
                                // Moving break point to next line with code
                                auto old = bp.line;
                                // Move the breakpoint to the next line
                                bp.line = line;

                                if (debugger->onAdjustBreakPoint) {
                                    debugger->onAdjustBreakPoint(old, line,
                                                                 section);
                                }
                            }
                        }
                    }
                }
                debugger->_lastFunction = func;
            }

            for (auto &bp : entries->second) {
                if (row == bp.line) {
                    if (!bp.column.has_value() || bp.column.value() == col) {
                        break_from_bp = true;
                        break;
                    }
                }
            }
        }

        if (!debugger->function_breakpoints.empty()) {
            int decl_row;
            auto func = ctx->GetFunction(0);

            if (func) {
                func->GetDeclaredAt(nullptr, &decl_row, nullptr);

                if (debugger->function_breakpoints.find(func->GetName()) !=
                        debugger->function_breakpoints.end() &&
                    row == func->FindNextLineWithCode(decl_row))
                    break_from_bp = true;
            }
        }
    }

    if (break_from_bp)
        debugger->DebugBreak(ctx);
}

/*static*/ void asIDBDebugger::ExceptionCallback(asIScriptContext *ctx,
                                                 asIDBDebugger *debugger) {
    if (!debugger->internal_execution)
        debugger->DebugBreak(ctx);
}

void asIDBDebugger::HookContext(asIScriptContext *ctx, bool has_work) {
    if (ctx == nullptr) {
        return;
    }

    if (!cache) {
        cache = CreateCache(ctx);
    }

    // TODO: is this safe to be called even if
    // the context is being switched?
    if (ctx->GetState() != asEXECUTION_EXCEPTION) {
        if (has_work)
            ctx->SetLineCallback(asFUNCTION(asIDBDebugger::LineCallback), this,
                                 asCALL_CDECL);
        ctx->SetExceptionCallback(asFUNCTION(asIDBDebugger::ExceptionCallback),
                                  this, asCALL_CDECL);
    }
}

void asIDBDebugger::DebugBreak(asIScriptContext *ctx) {
    if (ctx->GetEngine() != workspace->engine)
        return;

    {
        std::scoped_lock lock(mutex);
        action = asIDBAction::Pause;
        std::unique_ptr<asIDBCache> new_cache = CreateCache(ctx);

        if (cache)
            new_cache->Restore(*cache);

        std::swap(cache, new_cache);
    }

    const char *section = nullptr;
    int col;
    int row = ctx->GetLineNumber(0, &col, &section);

    if (onLineCallBackExec) {
        onLineCallBackExec(row, col, section);
    }

    // rehook
    HookContext(ctx, true);

    Suspend();
}

bool asIDBDebugger::HasWork() {
    std::scoped_lock lock(mutex);

    if (action != asIDBAction::None)
        return true;
    else if (!breakpoints.empty())
        return true;
    else if (!function_breakpoints.empty())
        return true;

    return false;
}

// debugger operations; these set the next breakpoint
// and call Resume.
void asIDBDebugger::SetAction(asIDBAction new_action) {
    // should never happen
    if (new_action == asIDBAction::None)
        return;

    if (new_action != asIDBAction::Continue) {
        std::scoped_lock lock(mutex);

        if (cache)
            stack_size = cache->ctx->GetCallstackSize();
    }
    action = new_action;

    Resume();
}
