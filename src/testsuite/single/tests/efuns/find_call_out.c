void do_tests() {
    ASSERT(find_call_out("foo") == -1);
    call_out("foo", 10);
    ASSERT(find_call_out("foo") == 10);
    ASSERT(find_call_out("bar") == -1);
}
