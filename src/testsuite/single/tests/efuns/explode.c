void do_tests() {
    string tmp = "this is a test";
    mixed *ret;
    
    ret = explode(tmp, "");
    ASSERT(sizeof(ret) == sizeof(filter(ret, (: strlen($1) == 1 :))));
    ret = explode(tmp, " ");
    ASSERT(sizeof(ret) == 4);
    ASSERT(ret[1] == "is");
    ASSERT(ret[3] == "test");

    ret = explode(" " + tmp, " ");
#ifndef __REVERSIBLE_EXPLODE_STRING__
    ASSERT(sizeof(ret) == 4);
    ASSERT(ret[1] == "is");
    ASSERT(ret[3] == "test");
#else
    ASSERT(sizeof(ret) == 5);
    ASSERT(ret[2] == "is");
    ASSERT(ret[4] == "test");
#endif

    ret = explode("     " + tmp, " ");
#ifndef __REVERSIBLE_EXPLODE_STRING__
#ifdef __SANE_EXPLODE_STRING__
    ASSERT(sizeof(ret) == 8);
    ASSERT(ret[5] == "is");
    ASSERT(ret[7] == "test");
#else
    ASSERT(sizeof(ret) == 4);
    ASSERT(ret[1] == "is");
    ASSERT(ret[3] == "test");
#endif
#else
    ASSERT(sizeof(ret) == 9);
    ASSERT(ret[6] == "is");
    ASSERT(ret[8] == "test");
#endif

    tmp = "this  is  a  test  ";
    ret = explode(tmp, "  ");
#ifndef __REVERSIBLE_EXPLODE_STRING__
    ASSERT(sizeof(ret) == 4);
#else
    ASSERT(sizeof(ret) == 5);
#endif
    ASSERT(ret[1] == "is");
    ASSERT(ret[3] == "test");

    ret = explode("  " + tmp, "  ");
#ifndef __REVERSIBLE_EXPLODE_STRING__
    ASSERT(sizeof(ret) == 4);
    ASSERT(ret[1] == "is");
    ASSERT(ret[3] == "test");
#else
    ASSERT(sizeof(ret) == 6);
    ASSERT(ret[2] == "is");
    ASSERT(ret[4] == "test");
#endif

    ret = explode("      " + tmp, "  ");
#ifndef __REVERSIBLE_EXPLODE_STRING__
#ifdef __SANE_EXPLODE_STRING__
    ASSERT(sizeof(ret) == 6);
    ASSERT(ret[3] == "is");
    ASSERT(ret[5] == "test");
#else
    ASSERT(sizeof(ret) == 4);
    ASSERT(ret[1] == "is");
    ASSERT(ret[3] == "test");
#endif
#else
    ASSERT(sizeof(ret) == 8);
    ASSERT(ret[4] == "is");
    ASSERT(ret[6] == "test");
#endif

}
