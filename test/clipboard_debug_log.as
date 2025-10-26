// clipboard_debug_log.as
// Test: clipboard and debug/log helpers
void main() {
    string before = clipboard::text();
    clipboard::setText("AS clipboard test");
    assert(clipboard::text() == "AS clipboard test");
    debug::setBreak(); // debug break
    string bt = debug::backtrace();
    println("backtrace length: " + bt.length());
    log::info("log info from script");
    log::warn("log warn from script");
    println("clipboard_debug_log OK");
}
