#include <globals.h>

int main(string fun);

void recurse(string dir) {
    mixed leaks;
    
    foreach (string file in get_dir(dir + "*.c")) 
	main(dir + file);
    foreach (string subdir in map(filter(get_dir(dir + "*", -1),
					 (: $1[1] == -2 :)),
				  (: $1[0] :)) - ({ ".", ".." }))
	{
	    if (subdir == "fail") {
		foreach (string fn in get_dir(dir + "fail/*.c")) {
		    ASSERT2(catch(load_object(dir+"fail/"+fn)), "fail/" + fn + " loaded");
#if defined(__DEBUGMALLOC_EXTENSIONS__) && defined(__CHECK_MEMORY__)
		    leaks = check_memory();
		    if (sizeof(filter(explode(leaks, "\n"), (: $1 && $1[0] :))) != 1) {
			write("After trying to compile: " + dir + "fail/" + fn + "\n");
			write(leaks);
			error("LEAK\n");
		    }
#endif
		}
		rm("/log/compile");
	    } else
		recurse(dir + subdir + "/");
	}	
}

int
main(string fun)
{
    string leaks;
    object tp = this_player();
    
    if (!fun || fun == "") {
	recurse("/single/tests/");
	write("Checks succeeded.\n");
	return 1;
    }
    fun->do_tests();
    if (tp != this_player())
	error("Bad this_player() after calling " + fun + "\n");
#if defined(__DEBUGMALLOC_EXTENSIONS__) && defined(__CHECK_MEMORY__)
    leaks = check_memory();
    if (sizeof(filter(explode(leaks, "\n"), (: $1 && $1[0] :))) != 1) {
	write("After calling: " + fun + "\n");
	write(leaks);
	error("LEAK\n");
    }
#endif
    return 1;
}
