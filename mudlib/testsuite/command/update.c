#include <globals.h>

int
main(string file)
{
	object obj;

	// need to call resolve_path() and query_cwd()
	if (!file) {
		return notify_fail("update what?\n");
	}
	if (obj = find_object(file)) {
		destruct(obj);
                if (obj)  return 1;
                file_size("???");
        }
        catch(file->dummy_apply_load());
	return 1;
}
