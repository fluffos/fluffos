void create() {
#ifndef __NO_ADD_ACTION__
    enable_commands();
#endif
}

void catch_tell(string str) {
}

void send(string str, mixed x) {
#ifndef __NO_ENVIRONMENT__
    if (x)
	say(str, x);
    else
	say(str);
#else
    shout(str);
#endif
}

void move(object ob) {
#ifndef __NO_ENVIRONMENT__
    move_object(ob);
#endif
}

