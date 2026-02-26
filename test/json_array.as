// json_array.as
void main() {
    json::array arr;
    arr.append(1);
    arr.append(json::value("two"));
    assert(arr.size() == 2);
    json::value a0 = arr.at(0);
    assert(a0.toInt() == 1);
    print(arr);
    println("json_array OK");
}
