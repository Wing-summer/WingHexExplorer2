// datetime_set.as
// Test datetime setDate and setTime return values
void main() {
    datetime dt;
    bool ok = dt.setDate(2021, 12, 31);
    bool ok2 = dt.setTime(23, 59, 59);
    assert(ok && ok2);
    println("datetime_set OK");
}
