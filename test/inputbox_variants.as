// inputbox_variants.as
// Test inputbox variants, skipped in headless mode
void main() {
    bool ok = false;
    string t = inputbox::getText("this", "Test", inputbox::EchoMode::Normal, "", ok);
    println("status: " + ok, "got text: " + t);
}
