void do_tests() {
    int i = 5;
    float f = 6.7;
    
    i = to_int(f);
    ASSERT(i == 6);

    i = f;
    ASSERT(i == 6);

    i = to_int("7");
    ASSERT(i == 7);

    i = to_int("ajdfj");
    ASSERT(i == 0);
}
