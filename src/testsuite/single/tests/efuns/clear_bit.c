void do_tests() {
    string str;
    
    ASSERT(catch(clear_bit("", 1000000000000)));
    ASSERT(catch(clear_bit("", -2)));
    
    str = "11111";
    str = clear_bit(str, 7);
    ASSERT(!test_bit(str, 7));
    str = clear_bit(set_bit(str, 12), 12);
    ASSERT(!test_bit(str, 7));
    
    ASSERT(str == clear_bit(str, 40));
}

