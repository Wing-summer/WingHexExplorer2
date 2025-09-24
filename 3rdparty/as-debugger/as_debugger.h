// MIT Licensed
// see https://github.com/Paril/angelscript-debugger

#pragma once

#include <angelscript.h>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include "as_helpers.h"

struct asIDBTypeId {
    int typeId = 0;
    asETypeModifiers modifiers = asTM_NONE;

    constexpr bool operator==(const asIDBTypeId &other) const {
        return typeId == other.typeId && modifiers == other.modifiers;
    }
};

template <class T>
inline void asIDBHashCombine(size_t &seed, const T &v) {
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <>
struct std::hash<asIDBTypeId> {
    inline std::size_t operator()(const asIDBTypeId &key) const {
        size_t h = std::hash<int>()(key.typeId);
        asIDBHashCombine(h, std::hash<asETypeModifiers>()(key.modifiers));
        return h;
    }
};

using asIDBTypeNameMap = std::unordered_map<asIDBTypeId, std::string>;

// a reference to a type ID + fixed address somewhere
// in memory that will always be alive as long as
// the debugger is currently broken on a frame.
struct asIDBVarAddr {
    int typeId = 0;
    bool constant = false;
    void *address = nullptr;

    asIDBVarAddr() = default;

    constexpr asIDBVarAddr(int typeId, bool constant, void *address)
        : typeId(typeId), constant(constant), address(address) {}
    asIDBVarAddr(const asIDBVarAddr &) = default;

    template <typename T>
    T *ResolveAs(bool reference = false) const {
        if (!address)
            return nullptr;
        else if (typeId & (asTYPEID_HANDLETOCONST | asTYPEID_OBJHANDLE)) {
            if (reference)
                return reinterpret_cast<T *>(address);
            return *reinterpret_cast<T **>(address);
        }
        return reinterpret_cast<T *>(address);
    }

    size_t GetSize(asIScriptEngine *engine) const {
        if (typeId == 0)
            return 0;
        else if (auto type = engine->GetTypeInfoById(typeId))
            return type->GetSize();
        else
            return engine->GetSizeOfPrimitiveType(typeId);
    }
};

// a variable name; ns is only non-blank for globals.
// to make the code simpler, "::" and "" should be equal.
struct asIDBVarName {
    std::string name;
    std::string ns;

    asIDBVarName() = default;

    template <typename T>
    asIDBVarName(T name) : name(name) {}

    template <typename Ta, typename Tb>
    asIDBVarName(Ta ns, Tb name) : name(name), ns(ns) {}

    inline bool operator<(const asIDBVarName &b) const {
        if (ns == b.ns)
            return asIDBNatILess()(name, b.name);

        return asIDBNatILess()(ns, b.ns);
    }

    inline std::string Combine() const {
        if (!ns.empty())
            return fmt::format("{}::{}", ns, name);
        return name;
    }
};

class asIDBDebugger;

// a variable for the debugger.
struct asIDBVariable {
    using Ptr = std::shared_ptr<asIDBVariable>;
    using WeakPtr = std::weak_ptr<asIDBVariable>;
    using WeakVector = std::vector<WeakPtr>;
    using Vector = std::vector<Ptr>;
    using Map = std::unordered_map<int64_t, WeakPtr>;
    using Set = std::unordered_set<Ptr>;

    struct Less {
        inline bool operator()(const Ptr &a, const Ptr &b) const {
            return *a < *b;
        }
    };

    inline bool operator<(const asIDBVariable &b) const {
        return identifier < b.identifier;
    }

    using SortedSet = std::set<Ptr, Less>;

    asIDBDebugger &dbg;
    WeakPtr ptr;

    asIDBVarName identifier;
    // if we are owned by another variable,
    // it's pointed to here.
    WeakPtr owner;

    // address will be non-null if we have a value
    // that can be retrieved. this might be null
    // for 'fake' variables or ones yet to be fetched.
    asIDBVarAddr address{};

    // these are only available after `evaluated` is true.
    bool evaluated = false;
    bool expandable = false;
    std::string value;
    std::string typeName;
    asIDBValue stackValue;

    // if it's a getter, this will be set.
    asIScriptFunction *getter = nullptr;
    Ptr get_evaluated;

    // automatically set after evaluation if
    // expandable is true.
    std::optional<int64_t> expandRefId{};

    // named properties & indexed variables
    bool expanded = false;
    SortedSet namedProps;
    Vector indexedProps;

    asIDBVariable(asIDBDebugger &dbg) : dbg(dbg) {}

    Ptr CreateChildVariable(asIDBVarName identifier, asIDBVarAddr address,
                            std::string_view typeName);

    void Evaluate();
    void Expand();

    // normally you don't need to call this directly
    // but if you're manually creating a variable you
    // may need to do this. This marks the object as
    // expandable and sets the ref id.
    void SetRefId();
};

// a local, fetched from GetVar
constexpr uint32_t SCOPE_SYSTEM = (uint32_t)-1;

// A scope contains variables.
struct asIDBScope {
    uint32_t offset; // offset in stack fetches (GetVar, etc)
    asIDBVariable::Ptr parameters;
    asIDBVariable::Ptr locals;
    asIDBVariable::Ptr registers; // "temporaries"

    std::unordered_map<uint32_t, asIDBVariable::WeakPtr> local_by_index;
    asIDBVariable::WeakPtr this_ptr;

    asIDBScope(asUINT offset, asIDBDebugger &dbg, asIScriptFunction *function);

private:
    void CalcLocals(asIDBDebugger &dbg, asIScriptFunction *function,
                    asIDBVariable::Ptr &container);
};

struct asIDBCallStackEntry {
    int64_t id; // unique id during debugging
    std::string declaration;
    std::string_view section;
    int row, column;
    asIDBScope scope;
};

using asIDBCallStackVector = std::vector<asIDBCallStackEntry>;

// This interface handles evaluation of asIDBVarAddr's.
// It is used when the debugger wishes to evaluate
// the value of, or the children/entries of, a var.
class asIDBTypeEvaluator {
public:
    // evaluate the given variable.
    virtual void Evaluate(asIDBVariable::Ptr var) const {}

    // for expandable objects, this is called when the
    // debugger requests it be expanded.
    virtual void Expand(asIDBVariable::Ptr var) const {}
};

// built-in evaluators you can extend for
// making custom evaluators.
template <typename T>
class asIDBPrimitiveTypeEvaluator : public asIDBTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override;
};

class asIDBObjectTypeEvaluator : public asIDBTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override;

    virtual void Expand(asIDBVariable::Ptr var) const override;

protected:
    // convenience function that queries the properties of the given
    // address (and object, if set) of the given type.
    void QueryVariableProperties(asIDBVariable::Ptr var) const;

    // convenience function that queries for getter property functions.
    void QueryVariableGetters(asIDBVariable::Ptr var) const;

    // convenience function to check the above two
    // to see if we have anything to expand.
    bool CanExpand(asIDBVariable::Ptr var) const;

    // convenience function to check if a function is
    // a compatible getter method
    bool IsCompatibleGetter(asIScriptFunction *function) const;

    // convenience function that iterates the opFor* of the given
    // address (and object, if set) of the given type. If positive,
    // a specific index will be used.
    void QueryVariableForEach(asIDBVariable::Ptr var, int index = -1) const;
};

// this class holds the cached state of stuff
// so that we're not querying things from AS
// every frame. You should only ever make one of these
// once you have a context that you are debugging.
// It should be destroyed once that context is
// destroyed.
class asIDBCache {
private:
    asIDBCache() = delete;
    asIDBCache(const asIDBCache &) = delete;
    asIDBCache &operator=(const asIDBCache &) = delete;

public:
    // the main context this cache is hooked to.
    // this will be reset to null if the context
    // is unhooked.
    asIScriptContext *ctx;

    // cache of type id+modifiers to names
    asIDBTypeNameMap type_names;

    // cached call stack
    asIDBCallStackVector call_stack;

    // cached globals
    asIDBVariable::Ptr globals;

    // cached set of variables
    asIDBVariable::Set variables;

    // cached map of var IDs to their variable.
    asIDBVariable::Map variable_refs;

    // ptr back to debugger
    asIDBDebugger &dbg;

    inline asIDBCache(asIDBDebugger &dbg, asIScriptContext *ctx)
        : dbg(dbg), ctx(ctx) {
        ctx->AddRef();
    }

    virtual ~asIDBCache() {
        ctx->ClearLineCallback();
        ctx->Release();
    }

    // restore data from the given cache that is
    // being replaced by this one.
    virtual void Restore(asIDBCache &cache);

    // caches all of the global properties in the context.
    virtual void CacheGlobals();

    // cache call stack entries
    virtual void CacheCallstack();

    // called when the debugger has broken and it needs
    // to refresh certain cached entries. This will only refresh
    // the state of active entries.
    virtual void Refresh();

    // get a safe view into a cached type string.
    virtual const std::string GetTypeNameFromType(asIDBTypeId id);

    // for the given type + property data, fetch the address of the
    // value that this property points to.
    virtual void *ResolvePropertyAddress(const asIDBVarAddr &id,
                                         int propertyIndex, int offset,
                                         int compositeOffset,
                                         bool isCompositeIndirect);

    // fetch an evaluator for the given resolved address.
    // the built-in implementation only handles a few base evaluators.
    virtual const asIDBTypeEvaluator &
    GetEvaluator(const asIDBVarAddr &id) const;

    // resolve the given expression to a unique var state.
    // `expr` must contain a resolvable expression; it's a limited
    // form of syntax designed solely to resolve a variable.
    // The format is as follows (curly brackets indicates optional elements;
    // ellipses indicate supporting zero or more entries): var{selector...}
    // `var` must be either:
    // - the name of a local, parameter, class member, or global. if there are
    // multiple
    //   matches, they will be selected in that same defined order.
    // - a fully qualified name to a local, parameter, class member, global, or
    //   `this`. This follows the same rules for qualification that the compiler
    //   does (`::` can be used to refer to the global scope).
    // - a stack variable index, prefixed with &. This can be used to
    // disambiguate
    //   in the rare case where you have a collision in parameters. It can also
    //   be used to select temporaries, if necessary.
    // `selector` must be one or more of the following:
    // - a valid property of the left hand side, in the format:
    //     .name
    // - an iterator index, in the format:
    //     [n{, o}]
    //   Only uint indices are supported. You may also optionally select which
    //   value to retrieve from multiple opValue implementations; if not
    //   specified it will default to zero (that is to say, [0] and [0,0] are
    //   equivalent).
    virtual asIDBExpected<asIDBVariable::WeakPtr>
    ResolveExpression(std::string_view expr, std::optional<int> stack_index);

    // Resolve the remainder of a sub-expression; see ResolveExpression
    // for the syntax.
    virtual asIDBExpected<asIDBVariable::WeakPtr>
    ResolveSubExpression(asIDBVariable::WeakPtr var,
                         const std::string_view rest);

    // Create a variable container. Generally you don't call
    // this directly, unless you need a blank variable.
    asIDBVariable::Ptr CreateVariable() {
        asIDBVariable::Ptr ptr = std::make_shared<asIDBVariable>(dbg);
        ptr->ptr = ptr;
        return *variables.emplace(ptr).first;
    }
};

struct asIDBBreakpoint {
    int line;
    std::optional<int> column;
    bool needAdjust;
};

using asIDBSectionBreakpoints = std::vector<asIDBBreakpoint>;

// TODO: class/namespace specifier
using asIDBSectionFunctionBreakpoints = std::unordered_set<std::string>;

enum class asIDBAction : uint8_t {
    None,
    Pause, // pause on next line, whatever it is
    StepInto,
    StepOver,
    StepOut,
    Continue
};

struct asIDBLineCol {
    int line, col;

    constexpr bool operator<(const asIDBLineCol &o) const {
        return line == o.line ? col < o.col : line < o.line;
    }
};

struct asIDBSource {
    std::string section;
    uint64_t ref;

    asIDBSource(std::string_view v, uint64_t ref) : section(v), ref(ref) {}

    struct LessComparator {
        using is_transparent = std::true_type;

        bool operator()(const std::string_view &a, const asIDBSource &b) const {
            return a < b.section;
        }

        bool operator()(const asIDBSource &a, const std::string_view &b) const {
            return a.section < b;
        }

        bool operator()(const asIDBSource &a, const asIDBSource &b) const {
            return a.section < b.section;
        }
    };

    inline bool operator<(const std::string_view &a) const {
        return section < a;
    }
};

using asIDBSectionSet = std::set<asIDBSource, asIDBSource::LessComparator>;
using asIDBPotentialBreakpointMap =
    std::unordered_map<std::string_view, std::set<asIDBLineCol, std::less<>>>;

// The workspace is contains information about the
// "project" that the debugger is operating within.
/*abstract*/ class asIDBWorkspace {
public:
    // sections that this workspace is working with
    asIDBSectionSet sections;

    // list of engines that can be hooked.
    asIScriptEngine *engine;

    // map of breakpoint positions
    asIDBPotentialBreakpointMap potential_breakpoints;

    // source ref id
    uint64_t ref_id = 1;

    asIDBWorkspace(asIScriptEngine *engine) { this->engine = engine; }

    virtual ~asIDBWorkspace() {}

    virtual void AddSection(std::string_view section) {
        if (auto it = sections.find(section); it == sections.end())
            sections.emplace(section, ref_id++);
    }

    // convert section names to physical path and vice versa
    virtual std::string PathToSection(const std::string_view v) const = 0;
    virtual std::string SectionToPath(const std::string_view v) const = 0;

    // check if a section is virtual.
    virtual bool SectionIsVirtual(const std::string_view v) const = 0;

    // return the full section source of the given section.
    virtual std::string SectionSource(const std::string_view v) const = 0;
};

// A basic workspace that expects the sources go into
// the code unmodified and are just read directly from
// the filesystem.
class asIDBFileWorkspace : public asIDBWorkspace {
    // base path for the workspace
    std::string base_path;

public:
    asIDBFileWorkspace(std::string_view base_path, asIScriptEngine *engine)
        : asIDBWorkspace(engine), base_path(base_path) {
        this->engine = engine;
        CompileScriptSources();
        CompileBreakpointPositions();
    }

    virtual std::string PathToSection(const std::string_view v) const override;
    virtual std::string SectionToPath(const std::string_view v) const override;

    virtual bool SectionIsVirtual(const std::string_view v) const override {
        return false;
    }

    virtual std::string SectionSource(const std::string_view v) const override;

private:
    void CompileScriptSources();
    void CompileBreakpointPositions();
};

using asIDBBreakpointMap =
    std::unordered_map<std::string, asIDBSectionBreakpoints>;

// This is the main class for interfacing with
// the debugger. This manages the debugger thread
// and the 'state' of the debugger itself. The debugger
// only needs to be kept alive if it still has work to do,
// but be careful about destroying the debugger if any
// contexts are still attached to it.
/*abstract*/ class asIDBDebugger {
public:
    // mutex for shared state, like the cache and breakpoints.
    std::recursive_mutex mutex;

    // next action to perform
    asIDBAction action = asIDBAction::None;
    asUINT stack_size = 0; // for certain actions (like Step Over) we have to
                           // know the size of the old stack.

    // if true, line callback will not execute
    // (used to prevent infinite loops)
    std::atomic_bool internal_execution = false;

    asIDBWorkspace *workspace;
    asIDBBreakpointMap breakpoints;
    asIDBSectionFunctionBreakpoints function_breakpoints;

    // cache for the current active broken state.
    // the cache is only kept for the duration of
    // a broken state; resuming in any way destroys
    // the cache.
    std::unique_ptr<asIDBCache> cache;

    // current frame offset for use by the cache
    std::atomic_int64_t frame_offset = 0;

    std::function<bool(asIScriptContext *)> onLineCallBack;
    std::function<void(int, int, const char *)> onLineCallBackExec;
    std::function<void()> onDebugBreak;
    std::function<void(int, int, const char *)> onAdjustBreakPoint;

    asIDBDebugger(asIDBWorkspace *workspace) : workspace(workspace) {}

    virtual ~asIDBDebugger() {}

    // hooks the context onto the debugger; this will
    // reset the cache, and unhook the previous context
    // from the debugger. You'll want to call this if
    // HasWork() returns true and you're requesting
    // a new context / executing code from a context
    // that isn't already hooked.
    void HookContext(asIScriptContext *ctx, bool has_work);

    // break on the current context. Creates the cache
    // and then suspends. Note that the cache will
    // add a reference to this context, preventing it
    // from being deleted until the cache is reset.
    void DebugBreak(asIScriptContext *ctx);

    // check if we have any work left to do.
    // it is only safe to destroy asIDBDebugger
    // if this returns false. If it returns true,
    // a context still has a linecallback set
    // using this debugger.
    virtual bool HasWork();

    // debugger operations; these set the next breakpoint,
    // clear the cache context and call Resume.
    virtual void SetAction(asIDBAction new_action);

protected:
    // called when the debugger is being asked to pause.
    // don't call directly, use DebugBreak.
    virtual void Suspend() = 0;

    // called when the debugger is being asked to resume.
    // don't call directly, use Continue.
    virtual void Resume() = 0;

    // create a cache for the given context.
    virtual std::unique_ptr<asIDBCache> CreateCache(asIScriptContext *ctx) = 0;

    static void LineCallback(asIScriptContext *ctx, asIDBDebugger *debugger);
    static void ExceptionCallback(asIScriptContext *ctx,
                                  asIDBDebugger *debugger);

protected:
    asIScriptFunction *_lastFunction = nullptr;
};

template <typename T>
/*virtual*/ void asIDBPrimitiveTypeEvaluator<T>::Evaluate(
    asIDBVariable::Ptr var) const /*override*/
{
    var->value = fmt::format("{}", *var->address.ResolveAs<const T>());
}
