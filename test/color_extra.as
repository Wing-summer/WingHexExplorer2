// color_extra.as
// Test various color getters and conversions (non-destructive)
void main() {
    color c = color::fromRgb(10,20,30);
    int r,g,b;
    c.getRgb(r,g,b);
    assert(r == 10);
    color hsl = c.toHsl();
    color hsv = c.toHsv();
    println("color_extra OK");
}
