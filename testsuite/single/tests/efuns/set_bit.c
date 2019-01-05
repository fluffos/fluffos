void do_tests() {
    string str;
    
    ASSERT(catch(set_bit("", 10000000000000)));
    ASSERT(catch(set_bit("", -2)));

    str = set_bit("", 100);
    ASSERT(test_bit(str, 100));
    for (int i = 0; i < 200; i++) {
	if (i != 100)
	    ASSERT(!test_bit(str, i));
    }
    str = " ";
    str[0] = 5;
    ASSERT(catch(set_bit(str, 1)));
    str[0] = 255;
    ASSERT(catch(set_bit(str, 1)));
}
