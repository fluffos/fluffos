int flag;

int action() {
    flag = 1;
    return 1;
}

void do_tests() {
#ifndef __NO_ADD_ACTION__
    object save_tp = this_player();
    mixed *comms;
    
    flag = 0;
    enable_commands();
    add_action( (: action :), "foo");
    comms = commands();
    disable_commands();
    if (save_tp)
	evaluate(bind( (: enable_commands :), save_tp));
    
    ASSERT(command("foo"));
    ASSERT(sizeof(comms) == 1);
    ASSERT(comms[0][0] == "foo");
    ASSERT(comms[0][2] == this_object());
    ASSERT(comms[0][3] == "<function>");
    ASSERT(flag);
    destruct(this_object());
#endif
}
