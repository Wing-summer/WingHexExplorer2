// color_conversion.as
// Test: color conversion helpers
void main() {
    color c = color::fromHsv(180, 50, 50);
    color rgb = c.toRgb();
    assert(rgb.isValid());
    println("color_conversion OK");
}
