#include <globals.h>

int
main(string file)
{
	// need to call resolve_path() and query_cwd()
#ifdef __OLD_ED__
	ed(file);
#else
	this_player()->start_ed(file);
#endif
	return 1;
}
