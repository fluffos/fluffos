void do_tests() {
    string str = "This is a test";
    rm("/test_file");
    write_file("/test_file", str);
    ASSERT(file_size("test_file") == strlen(str));
}
