// print_variadic.as
// Test print/println with multiple types
void main() {
    print("a", 1, 2.5, true);
    println("line:", "ok", 123);
    warnprintln("warn", "test");
    infoprintln("info", 3.14);
    errprintln("err", 0);
    println("print_variadic OK");
}
