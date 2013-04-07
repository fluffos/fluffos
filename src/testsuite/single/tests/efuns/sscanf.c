inherit "/inherit/tests";

void do_tests() {
    string s;
    
    ASSERT(catch(sscanf("you%", "%s%", s)));
    ASSERT(catch(sscanf("you%", "%su%", s)));
    sscanf("you%", "%s%%", s);
    ASSERT_EQ("you", s);
    sscanf("you%", "%su%%", s);
    ASSERT_EQ("yo", s);
}
