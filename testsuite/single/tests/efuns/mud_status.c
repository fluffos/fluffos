void do_tests() {
    ASSERT(stringp(mud_status()));
    ASSERT(stringp(mud_status(1)));
}
