void do_tests() {
#ifndef __OLD_ED__
    ASSERT_EQ(-1, query_ed_mode());
    ed_start("/ed_test");
    ASSERT_EQ(0, query_ed_mode());
    ed_cmd("");
    ASSERT_EQ(0, query_ed_mode());
    ed_cmd("2c");
    ASSERT_EQ(2, query_ed_mode());
    ed_cmd(".");
    ed_cmd("Q");
    ASSERT_EQ(-1, query_ed_mode());
#endif
}
