void do_tests() {
#ifndef __OLD_ED__
    ASSERT(query_ed_mode() == -1);
    ed_start("/ed_test");
    ASSERT(query_ed_mode() == 0);
    ed_cmd("h");
    ASSERT(query_ed_mode() == -2);
    ed_cmd("");
    ASSERT(query_ed_mode() == 0);
    ed_cmd("2c");
    ASSERT(query_ed_mode() == 2);
    ed_cmd(".");
    ed_cmd("Q");
    ASSERT(query_ed_mode() == -1);
#endif
}
