void do_tests() {
    object tp;
#ifndef __NO_ADD_ACTION__
    SAVETP;
    enable_commands();
    RESTORETP;
    ASSERT(!find_living("foo"));
    set_living_name("foo");
    ASSERT(find_living("foo") == this_object());
    disable_commands();
    ASSERT(!find_living("foo"));
    enable_commands();
    RESTORETP;
    set_living_name("bar");
    ASSERT(!find_living("foo"));
    ASSERT(find_living("bar") == this_object());
    destruct(this_object());
#endif
}
