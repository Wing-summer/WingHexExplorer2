// msgbox_calls.as
// Test: msgbox functions (interactive; may block in headless)
void main() {
    msgbox::about("Test About", "AngelScript test about");
    msgbox::information("Info", "This is info", msgbox::buttons::Ok);
    println("msgbox_calls invoked (may require manual confirmation)");
}
