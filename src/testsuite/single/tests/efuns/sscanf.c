inherit "/inherit/tests";

void do_tests() {
    string s;
    
    ASSERT(catch(sscanf("you%", "%s%", s)));
    ASSERT(catch(sscanf("you%", "%su%", s)));
    sscanf("you%", "%s%%", s);
    ASSERT(s == "you");
    sscanf("you%", "%su%%", s);
    ASSERT(s == "yo");
}
