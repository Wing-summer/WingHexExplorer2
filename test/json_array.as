// json_array.as
// Test: Json::JsonArray append/at
void main() {
    Json::JsonArray arr;
    arr.append(Json::JsonValue(1));
    arr.append(Json::JsonValue("two"));
    assert(arr.size() == 2);
    Json::JsonValue a0 = arr.at(0);
    assert(a0.toInt() == 1);
    println("json_array OK");
}
