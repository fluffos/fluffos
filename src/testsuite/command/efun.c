#include <globals.h>

int
main(string fun)
{
	int result, which;
	string objname;

	if (!fun) {
		return notify_fail("efun what?\n");
	}
	objname = "/single/efuns/" + fun;
	which = 0;
	do {
		result = (int)call_other(objname, "test", which);
		if (result > -1) {
			write((string)call_other(objname, "query_test_info") + ": ");
			if (result == 1) {
				write("success.\n");
			} else if (result == 0) {
				write("failure.\n");
			}
		}
		which++;
	} while (result > -1);
	return 1;
}
