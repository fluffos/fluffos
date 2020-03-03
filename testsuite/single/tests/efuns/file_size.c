void do_tests() {
    string str = "This is a test";
    rm("/test_file");

    // no such file returns -1
    ASSERT_EQ(-1, file_size("/test_file"));

    write_file("/test_file", str);
    ASSERT(file_size("test_file") == strlen(str));

    ASSERT_EQ(sizeof(read_bytes("/crlf_test_file.crlf")), file_size("/crlf_test_file.crlf"));

    // Directory returns -2
    ASSERT_EQ(-2, file_size("/single/tests"));
    ASSERT_EQ(-2, file_size("/single/tests/"));
}
