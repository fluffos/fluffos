int executed = 1;

void test() {
    int h = call_out("test", 0);
    ASSERT_EQ(0, find_call_out(h));
    write(sprintf("callout0: %d\n", executed));
    executed++;;
}
void do_tests() {
    call_out("test", 0.1);
}
