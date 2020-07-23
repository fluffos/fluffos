void do_tests() {
    for (int i = 0; i < 100; i++) {
	ASSERT(secure_random(5) >= 0);
	ASSERT(secure_random(5) < 5);
    }
}
