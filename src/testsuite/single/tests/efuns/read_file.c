void do_tests() {
    string foo = read_bytes("/testfile");
    string mid = implode(explode(foo, "\n")[99..198], "\n") + "\n";
    
    ASSERT(!read_file("/single", 10, 10));
    ASSERT(read_file("/testfile") == foo);
    ASSERT(read_file("/testfile", 100, 100) == mid);
    ASSERT(!read_file("/does_not_exist"));
    ASSERT(!read_file("/testfile", 10000, 1));
}
