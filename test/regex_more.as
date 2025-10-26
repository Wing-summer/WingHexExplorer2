// regex_more.as
// Tests regex::exp setters and properties
void main() {
    regex::exp re;
    re.setPattern("(?<name>[A-Za-z]+)-(\\d+)");
    re.setPatternOptions(regex::PatternOptions::CaseInsensitiveOption | regex::PatternOptions::MultilineOption);
    assert(re.isValid());
    assert(re.pattern().length() > 0);
    int cap = re.captureCount();
    println("captureCount = " + cap);
    array<string>@ names = re.namedCaptureGroups();
    if (names !is null) {
        println("named groups: " + names.length());
    }
    regex::match m = re.match("John-42");
    assert(m.hasMatch());
    println("regex_more OK");
}
