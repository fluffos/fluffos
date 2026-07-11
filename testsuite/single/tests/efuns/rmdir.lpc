void do_tests() {
    mkdir("/rm_dir");
    ASSERT(file_size("/rm_dir") == -2);
    rmdir("/rm_dir");
    ASSERT(file_size("/rm_dir") == -1);
}
