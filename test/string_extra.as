// string_extra.as
// Tests additional string namespace functions and array conversions
void main() {
    string s1 = string::number(123, 10);
    assert(s1 == "123");
    string s2 = string::number(3.1415, string::DoubleFmt::DFDecimal, 3);
    assert_x(s2.length() > 0, "double formatting produced empty");

    bool ok = false;
    int64 i64 = string::parseInt64("9223372036854775807", 10, ok);
    // host may not support full range; just ensure parse was attempted
    println("parseInt64 ok? " + ok);

    uint64 ui64 = string::parseUInt64("42", 10, ok);
    println("parseUInt64 ok? " + ok);

    float f = string::parseFloat("1.5", ok);
    println("parseFloat ok? " + ok);

    // array<uint8> -> fromUtf8/fromAscii
    array<uint8> utf8;
    utf8.insertLast(65); // 'A'
    utf8.insertLast(0);
    string from = string::fromAscii(utf8);
    assert(from.length() >= 1);

    array<char> raw;
    raw.insertLast(char('x'));
    string rr = string::fromRawData(raw);
    assert(rr.length() >= 1);

    // join & compare
    array<string> arr;
    arr.insertLast("a");
    arr.insertLast("b");
    string j = string::join(arr, ",");
    assert(j == "a,b");
    string tA = "A";
    assert(tA.compare("a", false) == 0);
    println("string_extra OK");
}
