string id;

int id(string name) { return name == id; }

void create(string arg) {
#ifndef __NO_ENVIRONMENT__
    id = arg;
    if (!arg) {
	new(__FILE__, "foo");
    } else {
	move_object(previous_object());
    }	
#endif
}

void do_tests() {
#ifndef __NO_ENVIRONMENT__
    ASSERT(present("foo", this_object()));
    ASSERT(present("foo"));
#endif
}
