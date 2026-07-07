void do_tests() {
    object x;
    
    foreach (x in children(__FILE__))
	if (x != this_object()) destruct(x);
    
    for (int i = 0; i < 5; i++)
	new(__FILE__);
    
    ASSERT(sizeof(children(__FILE__)) == 6);
}

    
