void do_tests() {
    ASSERT(mapp(([])));
    ASSERT(!mapp(0));
    ASSERT(!mapp("foo"));
}
