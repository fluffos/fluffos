#include <command.h>

int
main(string arg)
{
	object obj;

	if (!arg) {
		notify_fail("usage: dest object\n");
		return 0;
	}
	obj = find_object(arg);
	if (!obj) {
		obj = present(arg);
		if (!obj) {
			obj = present(arg, environment(previous_object()));
		}
	}
	if (!obj) {
		notify_fail("can't find that.\n");
		return 0;
	}
	obj->remove();
	if (obj) {
		destruct(obj);
	}
	return 1;
}
