// dictionary_basic.as
// Test: dictionary basic set/get and keys
void main() {
    dictionary d;
    d.set("k1", 123);
    d.set("k2", "hello");
    int64 outInt;
    bool ok = d.get("k1", outInt);
    assert(ok && outInt == 123);
    assert(d.exists("k2"));
    array<string>@ keys = d.getKeys();
    assert(keys !is null && keys.length() >= 2);
    println("dictionary_basic OK");
}
