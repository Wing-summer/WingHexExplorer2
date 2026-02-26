// json_basic.as
// Test: json::fromJson and object access
void main() {
    string js = "{\"name\":\"as\",\"v\":1}";
    json::document doc = json::fromJson(js);
    assert(doc.isObject());
    json::value name = doc["name"];
    assert(name.isString());
    assert(name.toString() == "as");
    json::value v = doc["v"];
    assert(v.toInt() == 1);
    println("json_basic OK");
}
