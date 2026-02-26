// json_value_variants.as
// Test json::value constructors and conversions
void main() {
    json::value v1(1);
    json::value v2(2.5);
    json::value v3("str");
    json::value v4(true);
    assert(v1.toInt() == 1);
    assert(v2.toDouble() == 2.5);
    assert(v3.toString() == "str");
    assert(v4.toBool() == true);
    println("json_value_variants OK");
}
