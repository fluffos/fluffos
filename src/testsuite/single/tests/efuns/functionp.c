void do_tests() {
    ASSERT(!functionp(0));
    ASSERT(!functionp("foo"));
    ASSERT(functionp((: do_tests :)));
}
