int called;
int xverb;

void do_tests() {
#ifndef __NO_ADD_ACTION__
    object tp;
    xverb = called = 0;
    SAVETP;
    enable_commands();
    add_action( (: called = 1 :), "foo");
    add_action( (: xverb = 1 :), "b", 1);
    RESTORETP;
    command("foo");
    command("bar");
    ASSERT(called);
    ASSERT(xverb);
#endif
}
