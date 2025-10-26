// grid_tests.as
// Test grid<T> resize and indexing
void main() {
    grid<int> g;
    g.resize(4, 3);
    assert(g.width() == 4 && g.height() == 3);
    g.opIndex(1,1) = 42;
    int v = g.opIndex(1,1);
    assert(v == 42);
    println("grid_tests OK");
}
