void do_tests() {
#ifndef __NO_ADD_ACTION__
    if (this_player())
	ASSERT(living(this_player()));
    ASSERT(!living(this_object()));
#endif
}
