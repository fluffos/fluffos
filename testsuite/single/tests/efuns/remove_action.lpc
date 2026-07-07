int called = 0;

void func() { called = 1; }

void do_tests() {
#ifndef __NO_ADD_ACTION__
    object tp;

    SAVETP;
    enable_commands();
    add_action( "func", "bar" );
    ASSERT(remove_action( "func", "bar" ));
    RESTORETP;
    command("bar");
    ASSERT(!called);
#endif
}
