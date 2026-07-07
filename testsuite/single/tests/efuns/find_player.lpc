void do_tests() {
#ifndef __NO_ADD_ACTION__
    object tp;
    object o1, o2;

    if (this_player()) {
	ASSERT(!find_player("bar"));
	SAVETP;
	enable_commands();
	set_living_name("bar");
	o1 = find_player("bar");
	exec(this_object(), tp);
	o2 = find_player("bar");
	exec(tp, this_object());
	disable_commands();
	RESTORETP;
	ASSERT(!o1);
	ASSERT(o2 == this_object());
    }
#endif
}
