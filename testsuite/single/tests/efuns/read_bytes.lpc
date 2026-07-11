void do_tests() {
    string foo;
    ASSERT(cp("/crlf_test_file.lf", "/testfile"));

    foo = read_file("/testfile");

    ASSERT(!read_bytes("/does_not_exist"));
    ASSERT(!read_bytes("/does_not_exist", 1));
    ASSERT(!read_bytes("/does_not_exist", 1, 2));

    ASSERT(!read_bytes("/testfile", -100000, 10));
    ASSERT_EQ(foo[<10..<6], read_bytes("/testfile", -10, 5));
    ASSERT(read_bytes("/testfile", -10, 0) == foo[<10..]);
    ASSERT(read_bytes("/testfile", -10) == foo[<10..]);
    ASSERT(read_bytes("/testfile", -10, 100) == foo[<10..]);
    ASSERT_EQ(string_encode(read_bytes("/testfile"),"utf8"), string_encode(foo, "utf8"));
    ASSERT(!read_bytes("/testfile", 1000000, 10));
    ASSERT_EQ(read_bytes("/testfile", 1, 100), foo[1..199]);
    ASSERT_EQ(strlen(read_bytes("/testfile", 10, 1000)), strlen(foo) - 10);
}
