// colordlg_test.as
// Test colordlg.getColor (interactive; skip in headless)
void main() {
    color c = colordlg::getColor("Choose");
    println("colordlg returned valid: " + c.isValid());
    println('color: ' + c.name());
}
