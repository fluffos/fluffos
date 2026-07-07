string my_arg;

string get_arg() {
    return my_arg;
}

void create(string arg) {
    my_arg = arg;
}

void do_tests() {
    object ob;
#ifdef __PACKAGE_UIDS__
    seteuid(0);
    ASSERT(catch(new(__FILE__)));
    seteuid(getuid(this_object()));
#endif
    ASSERT(new(__FILE__)->get_arg() == 0);
    ASSERT((string)new(__FILE__, "foo")->get_arg() == "foo");
}

