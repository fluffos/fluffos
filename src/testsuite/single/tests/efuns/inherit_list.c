void do_tests() {
    mixed *x;
    ASSERT(inherit_list(this_object()) == ({}));
    x = inherit_list(load_object("/clone/inh2.c"));
    ASSERT(sizeof(x) == 1);
    ASSERT(x[0] == "/clone/inh1.c");
}
