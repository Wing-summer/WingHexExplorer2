// json_basic.as
// Test: Json::fromJson and object access
void main() {
    string js = "{\"name\":\"as\",\"v\":1}";
    Json::JsonDocument doc = Json::fromJson(js);
    assert(doc.isObject());
    Json::JsonValue name = doc["name"];
    assert(name.isString());
    assert(name.toString() == "as");
    Json::JsonValue v = doc["v"];
    assert(v.toInt() == 1);
    println("json_basic OK");
}
