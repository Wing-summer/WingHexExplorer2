// json_doc_tests.as
// Test JsonDocument operations
void main() {
    Json::JsonDocument doc;
    Json::JsonArray arr;
    arr.append(Json::JsonValue(1));
    doc.setArray(arr);
    assert(doc.isArray());
    string j = doc.toJson(Json::JsonFormat::Compact);
    println("json compact: " + j);
    Json::JsonObject obj;
    obj.set("k", Json::JsonValue("v"));
    doc.setObject(obj);
    assert(doc.isObject());
    println("json_doc_tests OK");
}
