void do_tests() {
    string foo, mid, all;

    ASSERT(cp("/single/master.c", "/testfile"));

    rm("/testfile.zerolength");
    write_file("/testfile.zerolength", "");
    foo = read_file("/testfile.zerolength");
    ASSERT_EQ("", foo);

    foo = read_bytes("/testfile");
    mid = implode(explode(foo, "\n")[9..18], "\n") + "\n";
    all = implode(explode(foo, "\n")[9..<1], "\n") + "\n";

    ASSERT(!read_file("/single", 10, 10));
    ASSERT_EQ(foo, read_file("/testfile"));
    ASSERT_EQ(read_file("/testfile"), read_file("/testfile.crlf"));
    ASSERT_EQ(mid, read_file("/testfile", 10, 10));
    // if we are asking more lines than in the file, return whole file
    ASSERT(all == read_file("/testfile", 10, 0x7fffffff));
    ASSERT(!read_file("/does_not_exist"));
    ASSERT(!read_file("/testfile", 10000, 1));
}
