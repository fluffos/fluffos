void do_tests() {
    ASSERT(sizeof(({ 1, 2, 3 })) == 3);
    ASSERT(sizeof(allocate(50)) == 50);
    ASSERT(sizeof(allocate_mapping(5)) == 0);
    ASSERT(sizeof( ([ 1 : 2, 3 : 4 ]) ) == 2);
    ASSERT(sizeof("foo") == 3);
    ASSERT(sizeof((: sizeof :)) == 0);
    ASSERT(sizeof(6) == 0);
}
