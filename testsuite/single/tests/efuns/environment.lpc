void do_tests() {
#ifndef __NO_ENVIRONMENT__
    object ob;
    
    ASSERT(environment() == 0);
    ASSERT(environment(this_object()) == 0);
    ob = new(__FILE__, 1);
    ASSERT(environment(ob) == this_object());
#endif
}

void create(int flag) {
#ifndef __NO_ENVIRONMENT__
    if (flag)
	move_object(previous_object());
#endif
}

	
