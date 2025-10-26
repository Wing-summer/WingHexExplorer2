// array_sort_reverse.as
// Test: array sort and reverse
void main() {
    array<string> a;
    a.insertLast("b");
    a.insertLast("a");
    a.insertLast("c");
    a.sortAsc();
    assert(a[0] == "a" && a[1] == "b" && a[2] == "c");
    a.reverse();
    assert(a[0] == "c");
    println("array_sort_reverse OK");
}
