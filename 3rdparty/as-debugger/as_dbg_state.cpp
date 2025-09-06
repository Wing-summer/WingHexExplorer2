#include "as_dbg_state.h"

as_dbg_state &as_dbg_state::instance() {
    static as_dbg_state ins;
    return ins;
}

void as_dbg_state::RegisterEvaluator(
    int typeId, std::unique_ptr<asIDBTypeEvaluator> evaluator) {
    typeId &= asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR;
    evaluators.insert_or_assign(typeId, std::move(evaluator));
}

void as_dbg_state::CheckDebugger(asIScriptContext *ctx) {
    auto &debugger_state = instance();

    // TODO
    // check if the debugger needs to be changed
    // if (debugger_state.active_type != debugger_state.cvar->integer) {
    //     debugger_state.debugger.reset();
    //     debugger_state.workspace.reset();
    //     debugger_state.active_type = debugger_state.cvar->integer;
    // }

    // we don't want debugging
    // if (!debugger_state.cvar->integer)
    //     return;

    // create the debugger
    if (!debugger || debugger_state.outdated) {
        debugger.reset();
        debugger_state.workspace.reset();

        // debugger_state.workspace = std::make_unique<asIDBFileWorkspace>(
        //     std::filesystem::path(Q2AS_ScriptPath()).generic_string(),
        //     std::initializer_list<asIScriptEngine *>{svas.engine,
        //     cgas.engine});

        // debugger = std::make_unique<q2as_asIDBDebuggerVSCode>(
        //     debugger_state.workspace.get());
        debugger_state.outdated = false;
    }

    // hook the context if the debugger
    // has work to do (breakpoints, etc)
    debugger->HookContext(ctx, debugger->HasWork());
}

void as_dbg_state::DebugBreak(asIScriptContext *ctx) {
    if (!ctx)
        ctx = asGetActiveContext();

    if (!debugger)
        CheckDebugger(ctx);

    if (debugger)
        debugger->DebugBreak(ctx);
}
