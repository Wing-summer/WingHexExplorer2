
// complex_and_any.as
// Test: complex arithmetic and any storage/retrieve
void main() {
    math::complex a(1.0f, 2.0f);
    math::complex b(2.0f, -1.0f);
    math::complex c = a + b;
    assert(math::closeTo(c.r, 3.0f, 1e-6f));
    any anyv;
    anyv.store(123);
    int64 rout;
    bool ok = anyv.retrieve(rout);
    assert(ok && rout == 123);
    println("complex_and_any OK");
}
