void do_tests() {
    string tm = ctime(0);
    ASSERT(stringp(tm));
    ASSERT(strsrch(tm, '\n') == -1);
}
