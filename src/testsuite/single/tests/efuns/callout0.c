int i;

void test() {
    mixed error;

    error = catch(call_out("test", 0));
    i++;
    if (i < 9) {
       ASSERT_EQ(0, error);
    } else {
      ASSERT_EQ("*Nesting call_out(0) level limit exceeded: 10", error);
    }
}
void do_tests() {
    i = 0;
    test();
}
