void do_tests() {
    write_file("/rename_test", "Hmm.");
    rename("/rename_test", "/ren_test");
    ASSERT(file_size("/rename_test") == -1);
    ASSERT(read_file("/ren_test") == "Hmm.");
    rename("/ren_test", "/single");
    ASSERT(file_size("/ren_test") == -1);
    ASSERT(read_file("/single/ren_test") == "Hmm.");
    rm("/single/ren_test");
}
