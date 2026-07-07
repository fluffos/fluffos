void do_tests() {
    write_file("/rm_test", "foo");
    ASSERT(file_size("/rm_test"));
    rm("/rm_test");
    ASSERT(file_size("/rm_test") == -1);
}
