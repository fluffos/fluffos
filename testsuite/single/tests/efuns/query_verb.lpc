#ifndef __NO_ADD_ACTION__
void func() {
    ASSERT(query_verb() == "foo");
}
#endif

void do_tests() {
#ifndef __NO_ADD_ACTION__
    object tp;
    SAVETP;
    enable_commands();
    add_action( (: func :), "foo");
    RESTORETP;
    command("foo");
#endif
}
