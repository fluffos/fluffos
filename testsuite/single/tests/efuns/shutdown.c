void do_the_nasty_deed() {
    write("all tests finished, shutting down.\n");
    shutdown(0);
    ASSERT(0);
}

void do_tests() {
    write("shutdown in 15 seconds.\n");
    // This one is hard to test :-)
    call_out( (: do_the_nasty_deed :), 15);
}
