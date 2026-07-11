void do_tests() {
    mapping m = ([ 1 : 2, "foo" : 3, this_object() : 5 ]);

    map_delete(m, 1);
    ASSERT(undefinedp(m[1]));
    map_delete(m, ({}));
    map_delete(m, "foo");
    ASSERT(undefinedp(m["foo"]));
    map_delete(m, this_object());
    ASSERT(undefinedp(m[this_object()]));
}
