// math_fp_conv.as
// Test: fpToIEEE / fpFromIEEE
void main() {
    float f = 3.14f;
    uint fp = math::fpToIEEE(f);
    float f2 = math::fpFromIEEE(fp);
    assert_x(math::closeTo(f, f2, 1e-6f), "fpTo/From IEEE mismatch");
    println("math_fp_conv OK");
}
