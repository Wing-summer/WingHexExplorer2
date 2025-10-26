// array_forloop.as
void main() {
    array<string> a;
    a.insertLast("x");
    a.insertLast("y");
    assert(a[0] == "x");
    assert(a[1] == 'y');
    foreach(auto item : a){
        println(item);
    }
    println("array_forloop OK");
}
