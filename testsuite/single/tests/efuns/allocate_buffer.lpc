void do_tests() {
    int i = random(100000);

    ASSERT(sizeof(allocate_buffer(i)) == i);
    ASSERT(catch(allocate_buffer(-20)));
}

