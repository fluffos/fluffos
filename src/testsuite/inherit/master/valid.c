// file:   /inherit/master/valid.c
// mudlib: Lil

// separated out from master.c to reduce the complexity a bit.

#include <globals.h>



void create() {
}

int
valid_shadow(object ob)
{
#ifdef __PACKAGE_UIDS__
    if (getuid(ob) == ROOT_UID) {
        return 0;
    }
#endif
    if (ob->query_prevent_shadow(previous_object())) {
        return 0;
    }
	return 1;
}

int
valid_author(string)
{
	return 1;
}

// valid_override: controls which simul_efuns may be overridden with
// efun:: prefix and which may not.  This function is only called at
// object compile-time.
//
// returns: 1 if override is allowed, 0 if not.

int
valid_override(string file, string name)
{
	if (file == OVERRIDES_FILE) {
		return 1;
	}
	if ((name == "move_object") && (file != BASE))
		return 0;
//  may also wish to protect destruct, shutdown, snoop, and exec.
	return 1;
}

// valid_seteuid: determines whether an object ob can become euid str.
// returns: 1 if seteuid() may succeed, 0 if not.

int
valid_seteuid(object, string)
{
    return 1;
}

// valid_domain: decides if a domain may be created
// returns: 1 if domain may be created, 0 if not.

int
valid_domain(string)
{
	return 1;
} 

// valid_socket: controls access to socket efunctions
// return: 1 if access allowed, 0 if not.

int
valid_socket(object, string, mixed *)
{
	return 1;
}

// Write and Read privileges:
//
// valid_write: called with the file name, the object initiating the call,
//  and the function by which they called it. 
// return: 1 if access allowed, 0 if access not allowed.

int
valid_write(string, mixed, string)
{
    return 1;
}

// valid_read:  called exactly the same as valid_write()

int
valid_read(string, mixed, string)
{
    return 1;
}
 private int y = 1;
