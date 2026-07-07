void do_tests() {
#ifndef __NO_ADD_ACTION__
    ASSERT(!sizeof(filter(livings(), (: !living($1) :))));
    if (this_player())
	ASSERT(member_array(this_player(), livings()) != -1);
#endif
}
