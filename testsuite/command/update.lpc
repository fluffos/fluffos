#include <globals.h>

int
main(string file)
{
    object obj;

    // need to call resolve_path() and query_cwd()
    if (!file) {
#ifndef __NO_ADD_ACTION__
	return notify_fail("update what?\n");
#else
	write("update what?\n");
	return 1;
#endif
    }
    if (obj = find_object(file)) {
	destruct(obj);
    }
    load_object(file);
    return 1;
}
