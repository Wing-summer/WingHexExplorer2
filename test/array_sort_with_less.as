// array_sort_with_less.as
// Test array<T>::sort with custom less comparator
funcdef bool less_str(const string&in a, const string&in b);
bool less_str_impl(const string&in a, const string&in b) {
    return a.length() < b.length();
}
void main() {
    array<string> a;
    a.insertLast("short");
    a.insertLast("loooooong");
    // Some hosts accept passing function pointer by name; we attempt both
    a.sort(@less_str_impl);
    assert(a[0] == "short");
    println("array_sort_with_less OK");
}
