// string_number_parse.as
// Test: string::number and parseInt/parseDouble
void main() {
    string s = string::number(1234);
    assert(s == "1234");
    s = string::number(-42, 10);
    assert(s == "-42");
    s = string::number(3.14159f, string::DoubleFmt::DFSignificantDigits, 4);
    assert_x(s.length() > 0, "number(float) produced empty string");

    bool ok = false;
    int v = string::parseInt("42", 10, ok);
    assert(ok && v == 42);

    ok = false;
    double d = string::parseDouble("2.5", ok);
    assert(ok && closeTo(d, 2.5, 1e-9));

    println("string_number_parse OK");
}
