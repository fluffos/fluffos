void do_tests() {
#ifndef __NO_ENVIRONMENT__
    object ob;
    
    foreach (ob in deep_inventory(this_object()))
	if (ob)
	    destruct(ob);
    new(__FILE__, this_object());
    new(__FILE__, this_object(), 1);
    ASSERT(sizeof(deep_inventory(this_object())) == 4);
#endif
}

void create(object ob, int flag) {
#ifndef __NO_ENVIRONMENT__
    if (ob) move_object(ob);
    if (flag) {
	new(__FILE__, this_object());
	new(__FILE__, this_object());
    }
#endif
}
