void do_tests() {
#ifndef __NO_ADD_ACTION__
    object ob;
    object tp;
    
    ASSERT(!find_living("bar"));
    SAVETP;
    enable_commands();
    set_living_name("bar");
    ob = find_living("bar");
    disable_commands();
    RESTORETP;
    ASSERT(ob == this_object());
    ASSERT(!find_living("bar"));
#endif
}
