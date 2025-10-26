// any_dictionaryValue.as
// Test any and dictionaryValue store/retrieve
void main() {
    any a;
    a.store(3.14);
    double d = 0.0;
    bool ok = a.retrieve(d);
    println("any retrieve double ok: " + ok);
    dictionaryValue dv;
    dv = 42;
    auto rout = int64(dv);
    println("dictionaryValue opConv -> " + rout);
    println("any_dictionaryValue OK");
}
