void do_tests() {
#ifdef __PRIVS__
    set_privs(this_object(), "foo");
    ASSERT(query_privs(this_object()) == "foo");
    set_privs(this_object(), 0);
    ASSERT(query_privs(this_object()) == 0);
#endif
}
