void do_tests() {
    for (int i = 0; i < 10; i++) {
	ASSERT(sizeof(allocate(i)) == i);
	ASSERT(filter(allocate(i), (: $1 :)) == ({}));
    }
    ASSERT(allocate(0) == ({}));
    ASSERT(catch(allocate(-10)));
}
