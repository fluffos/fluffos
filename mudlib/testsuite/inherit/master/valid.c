// file:   /inherit/master/valid.c
// mudlib: Lil

// Separated out from master.c to reduce the complexity a bit.
// All functions return 1 if access/action is permitted, 0 if not permitted.
// Little to no security here, to give you the incentive to write your own.

#include <globals.h>

// valid_shadow: controls whether an object may be shadowed or not
int
valid_shadow( object ob )
{
    if (getuid(ob) == ROOT_UID) {
        return 0;
    }
    if (ob->query_prevent_shadow(previous_object())) {
        return 0;
    }
	return 1;
}

// valid_override: controls which simul_efuns may be overridden with
//   efun:: prefix and which may not.  This function is only called at
//   object compile-time
int
valid_override( string file, string name )
{
	if (file == OVERRIDES_FILE) {
		return 1;
	}
	if ((name == "move_object") && (file != BASE))
		return 0;
    //  may also wish to protect destruct, shutdown, snoop, and exec.
	return 1;
}

// valid_seteuid: determines whether an object ob can become euid str
int
valid_seteuid( object ob, string str )
{
    return 1;
}

// valid_socket: controls access to socket efunctions
int
valid_socket( object eff_user, string fun, mixed *info )
{
	return 1;
}

// valid_asm: controls whether or not an LPC->C compiled object can use
//   asm { }
int
valid_asm( string file )
{
    return 1;
}

// valid_compile_to_c: controls whether or not an object may be compiled
//   via LPC->C
int
valid_compile_to_c( string file )
{
    return 1;
}

// valid_hide: controls the use of the set_hide() efun, to hide objects or
//   see hidden objects
int
valid_hide( object who )
{
    return 1;
}

// valid_object: controls whether an object loaded by the driver should
//   exist
int
valid_object( object ob )
{
    return 1;
}

// valid_link: controls use of the link() efun for creating hard links
//   between paths
int
valid_link( string original, string reference )
{
    return 1;
}

// valid_save_binary: controls whether an object can save a binary
//   image of itself to the specified "save binaries directory"
//   (see config file)
int
valid_save_binary( string filename )
{
    return 1;
}

// valid_write: write privileges; called with the file name, the object
//   initiating the call, and the function by which they called it. 
int
valid_write( string file, mixed user, string func )
{
    return 1;
}

// valid_read: read privileges; called exactly the same as valid_write()
int
valid_read( string file, mixed user, string func )
{
    return 1;
}
