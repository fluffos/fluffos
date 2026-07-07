inherit "/inherit/tests";

void do_tests() {
    string s, s1;
    int num;

    ASSERT(catch(sscanf("you%", "%s%", s)));
    ASSERT(catch(sscanf("you%", "%su%", s)));
    sscanf("you%", "%s%%", s);
    ASSERT_EQ("you", s);
    sscanf("you%", "%su%%", s);
    ASSERT_EQ("yo", s);

    sscanf("123sdkfwo1239405adsf", "%d", num);
    ASSERT_EQ(123, num);

    sscanf("+29375282", "%d", num);
    ASSERT_EQ(29375282, num);

    sscanf("  -19322", "%d", num);
    ASSERT_EQ(-19322, num);

    sscanf("FFFF", "%x", num);
    ASSERT_EQ(65535, num);

    sscanf("0x1293jsf", "%x", num);
    ASSERT_EQ(4755, num);

    ASSERT_EQ(2, sscanf("abc%abc", "%s%%%s", s, s1));
    ASSERT_EQ("abc", s);
    ASSERT_EQ("abc", s1);
}
