void do_tests() {
    ASSERT(member_array('c', "foo") == -1);
    ASSERT(member_array('b', "abar") == 1);
    ASSERT(member_array('y', "xyzzy") == 1);
    ASSERT(member_array('y', "xyzzy", 2) == 4);
    ASSERT(member_array(2, ({ 1, 2, 3 })) == 1);
    ASSERT(member_array("foo", ({ 1, "foo", 3 })) == 1);
}
