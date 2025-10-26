// regex_basic.as
// Test: basic regex match and captures
void main() {
    regex::exp re("([0-9]+)-(abc)");
    assert(re.isValid());
    regex::match m = re.match("123-abc rest");
    assert(m.hasMatch());
    assert(m.captured(0) == "123-abc");
    assert(m.captured(1) == "123");
    assert(m.captured(2) == "abc");
    println("regex_basic OK");
}
