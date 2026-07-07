void do_tests() {
    ASSERT(find_call_out("foo") == -1);
    call_out("foo", 10);
    ASSERT_EQ(10, find_call_out("foo"));
    ASSERT_EQ(-1, find_call_out("bar"));
    remove_call_out("foo");
}
