#define TESTFILE "/testfile"

void do_tests() {
    rm(TESTFILE);
    ASSERT(file_size(TESTFILE) == -1);
    ASSERT(cp("/single/master.c", TESTFILE));
    ASSERT(file_size(TESTFILE));
    ASSERT(read_file("/single/master.c") == read_file(TESTFILE));
}

