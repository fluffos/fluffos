void do_tests() {
    ASSERT_EQ("*hi\n", catch(error("hi")));
    ASSERT_EQ("*hi\n", catch(error("hi\n")));
}
