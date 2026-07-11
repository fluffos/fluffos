void do_tests() {
    // basic case
    write_file("/rename_test", "Hmm.");
    rename("/rename_test", "/ren_test");
    ASSERT_EQ(file_size("/rename_test"), -1);
    ASSERT_EQ("Hmm.", read_file("/ren_test"));
    // move into directory
    rename("/ren_test", "/single");
    ASSERT(file_size("/ren_test") == -1);
    ASSERT(read_file("/single/ren_test") == "Hmm.");
    rm("/single/ren_test");
    // Verify overwrite
    write_file("/rename_test", "1234");
    write_file("/rename_test1", "4321");
    rename("/rename_test", "/rename_test1");
    ASSERT_EQ(-1, file_size("/rename_test"));
    ASSERT_EQ("1234", read_file("/rename_test1"));
}
