void do_tests() {
    mapping m = ([ "foo/" : 1, "bar" : 2, "bazz/foo" : 3, "foo/bar/bazz" : 4]);

    ASSERT(!match_path(m, "kajdfj"));
    ASSERT(match_path(m, "bar") == 2);
    ASSERT(match_path(m, "bar/") == 2);
    ASSERT(match_path(m, "bazz//foo") == 3);
    ASSERT(match_path(m, "bazz/foo//") == 3);
    ASSERT(match_path(m, "foo/bar") == 1);
    ASSERT(match_path(m, "foo/bar/bazz") == 4);
}
