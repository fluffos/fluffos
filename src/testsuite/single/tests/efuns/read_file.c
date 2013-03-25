void do_tests() {
    string foo = read_bytes("/testfile");
    string mid = implode(explode(foo, "\n")[99..198], "\n") + "\n";
    
    ASSERT(!read_file("/single", 10, 10));
    ASSERT(read_file("/testfile") == foo);
    ASSERT(read_file("/testfile", 100, 100) == mid);
    // if we are asking more lines than in the file, return whole file
    ASSERT(read_file("/testfile", 100, 0x7fffffff) == mid);
    ASSERT(!read_file("/does_not_exist"));
    ASSERT(!read_file("/testfile", 10000, 1));
}
