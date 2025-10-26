// fp_math_tests.as
// Test additional math helpers
void main() {
    float v = math::pow(2.0f, 8.0f);
    assert(math::closeTo(v, 256.0f, 1e-5f));
    float fr = math::fraction(3.75f);
    println("fraction(3.75) = " + fr);
    assert(math::closeTo(math::sqrt(9.0f), 3.0f));
    println("fp_math_tests OK");
}
