// MIT Licensed
// see https://github.com/Paril/angelscript-debugger

#ifndef AS_DBG_STATE_H
#define AS_DBG_STATE_H

#include "as_debugger.h"

class as_dbg_state {
public:
    static as_dbg_state &instance();

public:
    // set to true to re-set the workspace.
    bool outdated = false;

    std::unique_ptr<asIDBDebugger> debugger;
    std::unique_ptr<asIDBWorkspace> workspace;

    uint8_t current_tid = 0;

    // evaluators don't take up much memory so we'll just
    // always keep them around.
    std::unordered_map<int, std::unique_ptr<asIDBTypeEvaluator>> evaluators;

    // Register an evaluator.
    void RegisterEvaluator(int typeId,
                           std::unique_ptr<asIDBTypeEvaluator> evaluator);

    // A quick shortcut to make a templated instantiation
    // of T from the given type name.
    template <typename T>
    void RegisterEvaluator(asIScriptEngine *engine, const char *name) {
        RegisterEvaluator(engine->GetTypeInfoByName(name)->GetTypeId(),
                          std::make_unique<T>());
    }

    int active_type; // active debugger type
    bool suspend_immediately = true;

    void CheckDebugger(asIScriptContext *ctx);
    void DebugBreak(asIScriptContext *ctx = nullptr);
};

#endif // AS_DBG_STATE_H
