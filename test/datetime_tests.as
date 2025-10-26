// datetime_tests.as
// Test datetime constructors and arithmetic
void main() {
    datetime dt(2020, 1, 1, 0, 0, 0);
    assert(dt.get_year() == 2020);
    dt += 3600; // add one hour
    println("datetime after add hour: " + dt.get_hour());
    datetime dt2 = dt + 60;
    auto cmp = dt == dt2;
    println("datetime cmp: " + cmp);
    println("datetime_tests OK");
}
