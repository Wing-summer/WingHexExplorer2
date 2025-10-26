// color_construction.as
// Test: color construction and fromString/colorNames
void main() {
    color c1 = color::fromRgb(255);
    assert(c1.isValid());
    color c2 = color::fromRgb(10, 20, 30);
    int r, g, b;
    c2.getRgb(r, g, b);
    assert(r == 10 && g == 20 && b == 30);
    color c3 = color::fromString("red");
    assert(c3.isValid());
    array<string>@ names = color::colorNames();
    assert(names !is null && names.length() > 0);
    println("color_construction OK");
}
