inherit "/inherit/tests";

#ifndef __SENSIBLE_MODIFIERS__
static
#else
protected
#endif
int stat() {
}

private int priv() {
}

void do_tests() {
    ASSERT_NE(0, function_exists("stat", this_object(), 1));
    ASSERT_NE(0, function_exists("priv", this_object(), 1));

    ASSERT_EQ(0, function_exists("stat", this_object()));
    ASSERT_EQ(0, function_exists("priv", this_object()));

    ASSERT_EQ(0, function_exists("bar", this_object()));
    ASSERT_EQ(__FILE__, function_exists("do_tests", this_object()) + ".c");
    ASSERT_EQ("/inherit/tests.c", function_exists("describe_test", this_object()) + ".c");
}
