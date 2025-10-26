// msgbox_buttons_variants.as
// Test msgbox variants but skip if headless environment variable set
void main() {
    msgbox::question("Q", "Question?", msgbox::buttons::Yes | msgbox::buttons::No);
    msgbox::warning("W", "Warning", msgbox::buttons::Ok);
    msgbox::critical("C", "Critical", msgbox::buttons::Ok);
    println("msgbox_buttons_variants invoked (interactive)");
}
