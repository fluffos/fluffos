void do_tests() {
    mixed *x;
    
    ASSERT(deep_inherit_list(this_object()) == ({ }));
    x = deep_inherit_list(load_object("/single/tests/efuns/inh2.c"));
    ASSERT(sizeof(x) == 2);
    ASSERT(x[0] == "/single/tests/efuns/inh1.c");
    ASSERT(x[1] == "/single/tests/efuns/inh0.c");
}
