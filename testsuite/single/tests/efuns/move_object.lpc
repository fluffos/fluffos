void do_tests() {
#ifndef __NO_ENVIRONMENT__
    ASSERT(catch(move_object("foo")));
    ASSERT(catch(move_object(__FILE__)));
    ASSERT(catch(move_object(this_object())));
    move_object(master());
    ASSERT(environment(this_object()) == master());
    destruct(this_object());
    ASSERT(catch(move_object(master())));
#endif
}
