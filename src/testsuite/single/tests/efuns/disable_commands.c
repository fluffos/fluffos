void do_tests() {
#ifndef __NO_ADD_ACTION__
    object save_tp = this_player();
    
    disable_commands();
    ASSERT(!living(this_object()));
    enable_commands();
    ASSERT(living(this_object()));
    disable_commands();
    ASSERT(!living(this_object()));

    if (save_tp)
	evaluate(bind( (: enable_commands :), save_tp));
#endif
}
