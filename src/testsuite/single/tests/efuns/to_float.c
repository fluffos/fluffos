inherit "/inherit/tests";

void do_tests() {
    int i = 5;
    float f = 6.7;
    
    f = to_float(i);
    ASSERT(f == 5.0);

    f = i;
    ASSERT(f == 5.0);

    f = to_float("1.23");
    ASSERT(f == 1.23);

    f = to_float("ajdfj");
    ASSERT(f == 0.00);
}
