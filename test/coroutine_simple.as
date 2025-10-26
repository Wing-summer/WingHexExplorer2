// coroutine_simple.as
// Test: createCoRoutine usage (requires host coroutine scheduler to resume)
funcdef void coro_func(dictionary@);

void my_coro(dictionary@ ctx) {
    println("coro started");
    yield();
    println("coro resumed");
}

void main() {
    dictionary d;
    createCoRoutine(@my_coro, @d);
    println("coroutine created; host must resume it to see 'coro resumed'");
}
