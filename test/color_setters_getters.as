// color_setters_getters.as
// Test setters and getters of color class
void main() {
    color c;
    c.setRgb(64,128,192);
    c.setHsl(120, 50, 50);
    int r,g,b;
    c.getRgb(r,g,b);
    println("color getters r: " + r);
    color dark = c.darker(150);
    color light = c.lighter(120);
    println("color_setters_getters OK");
}
