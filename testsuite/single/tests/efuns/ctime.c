void do_tests() {
    string tm = ctime();
    ASSERT(stringp(tm));
    ASSERT(strsrch(tm, '\n') == -1);

    tm = ctime(1602454371); // Sun, 11 Oct 2020 22:12:51 GMT
    ASSERT(stringp(tm));
    ASSERT(strsrch(tm, '\n') == -1);
}
