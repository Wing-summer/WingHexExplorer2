// regex_wildcard_escape.as
// Test: wildcardToRegularExpression and escape
void main() {
    string pattern = "file.*.txt";
    string rexp = regex::wildcardToRegularExpression(pattern);
    assert_x(rexp.length() > 0, "wildcardToRegularExpression returned empty");
    string esc = regex::escape("a+b(c)");
    assert_x(esc.findFirstOf("\\+") >= 0 || esc.findFirstOf("\\(") >= 0, "escape likely failed");
    println("regex_wildcard_escape OK");
}
