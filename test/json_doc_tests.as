// json_doc_tests.as
// Test document operations
void main() {
    json::document doc;
    json::array arr;
    arr.append(json::jsonValue(1));
    doc.setArray(arr);
    assert(doc.isArray());
    string j = doc.tojson(json::jsonFormat::Compact);
    println("json compact: " + j);
    json::object obj;
    obj.set("k", json::jsonValue("v"));
    doc.setObject(obj);
    assert(doc.isObject());
    println("json_doc_tests OK");
}
