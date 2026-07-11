void do_tests() {
    mixed *a;
    string s = @END
xxx
yyy
END;
    ASSERT( s == "xxx\nyyy\n");
    a = @@END
xxx
yyy
END;
    ASSERT(sizeof(a) == 2);
}
