void do_tests() {
    mixed *x;
    ASSERT(inherit_list(this_object()) == ({}));
    x = inherit_list(load_object("/single/tests/efuns/inh2.c"));
    ASSERT(sizeof(x) == 1);
    ASSERT(x[0] == "/single/tests/efuns/inh1.c");
}
