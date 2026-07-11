#ifndef __NO_ENVIRONMENT__
string err = 0;
string yes = 0;

#define YES(x) ASSERT2((talker->send(x), yes == x), x); ASSERT2((talker->send("no:" + x, this_object()), !err), err); ASSERT2((talker->send("no:" + x, ({ this_object() })), !err), err)
#define NO(x) ASSERT2((talker->send(x), !err), err)

void catch_tell(string str) {
    if (str[0..2] != "yes")
	err = str;
    else
	yes = str;
}

void do_tests() {
    object tp;
    object talker = new(__DIR__ "talker");
    
#ifndef __NO_ADD_ACTION__
    SAVETP;
    enable_commands();
    RESTORETP;
#endif
    NO("no-noenv");
    talker->move(this_object());
    
    YES("yes-inside");
    talker->move(new("/single/void"));
    move_object(talker);
    YES("yes-outside");
}
#endif
