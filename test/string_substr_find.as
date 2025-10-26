// string_substr_find.as
// Test: substr, findFirstOf, contains, startsWith, endsWith
void main() {
    string s = "Hello AngelScript";
    string sub = s.substr(6, 11);
    assert(sub == "AngelScript");
    assert(s.findFirstOf("A") == 6);
    assert(s.findFirstOf("Z") == -1);
    assert(s.contains("Angel"));
    assert(s.startsWith("Hello"));
    assert(s.endsWith("Script"));
    println("string_substr_find OK");
}
