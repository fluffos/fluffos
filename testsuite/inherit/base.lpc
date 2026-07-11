// set to 'private static' so that inheritor won't be able to directly
// access this variable and so that save_object() won't save it to the .o file

private staticv string *ids;

void
remove()
{
	// add code here to prevent unwanted destructions.
	// update attributes of the container and the containees.
	destruct(this_object());
}

#ifndef __NO_ENVIRONMENT__
int
move(mixed dest)
{
	// add code here to prevent disallowed moves (target is full, object
	// is immovable, etc.).
	move_object(dest);
}
#endif

void
set_ids(string *arg)
{
	// probably want to add some security here.
	ids = arg;
}

int
id(string arg)
{
	if (!arg || !ids) {
		return 0;
	}
	return (member_array(arg, ids) != -1);
}
