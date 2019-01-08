void do_tests() {
#ifndef __OLD_ED__
    ASSERT(!in_edit(this_object()));
    ed_start("/ed_test");
    ASSERT(in_edit(this_object()) == "/ed_test");
    destruct(this_object());
#endif
}
