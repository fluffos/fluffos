void do_tests() {
    object tp = this_player();
    
    if (tp) {
	ASSERT(exec(this_object(), tp));
	ASSERT(userp(this_object()));
	ASSERT(exec(tp, this_object()));
    }
}

