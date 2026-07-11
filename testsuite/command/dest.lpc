#include <command.h>

int
main(string arg)
{
	object obj;

	if (!arg) {
#ifndef __NO_ADD_ACTION__
		return notify_fail("usage: dest object\n");
#else
		write("usage: dest object\n");
		return 1;
#endif
	}
	obj = find_object(arg);
#ifndef __NO_ENVIRONMENT__
	if (!obj) {
		obj = present(arg);
		if (!obj) {
			obj = present(arg, environment(previous_object()));
		}
	}
#endif
	if (!obj) {
#ifndef __NO_ADD_ACTION__
		return notify_fail("can't find that.\n");
#else
		write("can't find that.\n");
		return 1;
#endif
	}
	obj->remove();
	if (obj) {
		destruct(obj);
	}
	return 1;
}
