void do_tests() {
    mixed *x;

    ASSERT(deep_inherit_list(this_object()) == ({ }));
    x = deep_inherit_list(load_object("/clone/inh2.c"));
    ASSERT(sizeof(x) == 2);
    ASSERT(x[0] == "/clone/inh1.c");
    ASSERT(x[1] == "/clone/inh0.c");
}
