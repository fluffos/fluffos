void do_tests() {
    string foo = read_file("/testfile");
    
    ASSERT(!read_bytes("/does_not_exist"));
    ASSERT(!read_bytes("/does_not_exist", 1));
    ASSERT(!read_bytes("/does_not_exist", 1, 2));
    
    ASSERT(!read_bytes("/testfile", -100000, 10));
    ASSERT(read_bytes("/testfile", -10, 5) == foo[<10..<6]);
    ASSERT(read_bytes("/testfile", -10, 0) == foo[<10..]);
    ASSERT(read_bytes("/testfile", -10) == foo[<10..]);
    ASSERT(read_bytes("/testfile", -10, 100) == foo[<10..]);
    ASSERT(read_bytes("/testfile") == foo);
    ASSERT(!read_bytes("/testfile", 1000000, 10));
    ASSERT(read_bytes("/testfile", 100, 100) == foo[100..199]);
    ASSERT(strlen(read_bytes("/testfile", 100, 100)) == 100);
}
