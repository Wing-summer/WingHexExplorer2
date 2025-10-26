// complex_extra.as
// Test complex getters/setters and arithmetic
void main() {
    math::complex z(3.0f, 4.0f);
    assert(math::closeTo(z.abs(), 5.0f));
    math::complex r = z.r;
    math::complex i = z.i;
    println(z);
    z.set_ri(r);
    z.set_ir(i);
    println(z);
    println("complex_extra OK");
}
