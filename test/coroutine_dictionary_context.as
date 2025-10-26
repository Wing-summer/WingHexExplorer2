// coroutine_dictionary_context.as
// Test coroutine created with context dictionary
funcdef void coro_func(dictionary@);

void coro_fn(dictionary@ ctx) {
    if (ctx !is null && ctx.exists("v")) {
        string val;
        ctx.get("v", val);
        println("coro context v: " + val);
    }
    yield();
}

void main() {
    dictionary d;
    d.set("v", "hello");
    createCoRoutine(@coro_fn, @d);
    println("coroutine_dictionary_context created");
}
