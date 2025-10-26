// json_value_variants.as
// Test Json::JsonValue constructors and conversions
void main() {
    Json::JsonValue v1(1);
    Json::JsonValue v2(2.5);
    Json::JsonValue v3("str");
    Json::JsonValue v4(true);
    assert(v1.toInt() == 1);
    assert(v2.toDouble() == 2.5);
    assert(v3.toString() == "str");
    assert(v4.toBool() == true);
    println("json_value_variants OK");
}
