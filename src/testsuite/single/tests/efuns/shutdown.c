void do_the_nasty_deed() {
    shutdown(55);
    ASSERT(0);
}

void do_tests() {
    // This one is hard to test :-)
    call_out( (: do_the_nasty_deed :), 60);
}
