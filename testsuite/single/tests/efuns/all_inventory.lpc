void do_tests() {
#ifndef __NO_ENVIRONMENT__
    int s = sizeof(all_inventory(this_object()));
    
    ASSERT(sizeof(all_inventory()) == s);
    
    for (int i = 0; i < 5; i++) {
	ASSERT(sizeof(all_inventory()) == s + i);
	clone_object(file_name());
    }
    ASSERT(filter(all_inventory(this_object()), (: !$1->is_all_inv_test() :)));
#endif
}

int is_all_inv_test() {
    return 1;
}

void
create() {
#ifndef __NO_ENVIRONMENT__
    if (clonep()) {
	string s = file_name();
	sscanf(s, "%s#%*d", s);
	move_object(s);
    }
#endif
}

	
