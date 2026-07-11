void do_tests() {
#ifndef __OLD_ED__
    ASSERT(catch(ed_cmd("foo")));
    rm("/ed_test");
    write_file("/ed_test", "xyzzy\nxxx");
    ed_start("/ed_test");
    ASSERT(ed_cmd("1s/y/a/g"));
    ASSERT(ed_cmd("2s!x!b!"));
    ASSERT(catch(ed_start("foo")));
    ASSERT(ed_cmd("x"));
    ASSERT(read_file("/ed_test") == "xazza\nbxx\n");
#endif
}
