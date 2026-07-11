int clean_up(int inherited) { return 0; }

void do_tests() {
    // This object defines clean_up(), so the request takes effect.
    ASSERT_EQ(1, request_clean_up());
    ASSERT_EQ(1, request_clean_up(this_object()));

    // The master object defines no clean_up(), so the request is refused.
    ASSERT_EQ(0, request_clean_up(find_object("/single/master")));
}
