#include <globals.h>

// needs fixed to handle passwords

void
logon()
{
	object user;

	write("Welcome to Lil!\n\n");
	cat("/etc/motd");
	write("\n> ");
    seteuid(getuid(this_object()));
    user= new("/clone/user");
    user->set_name("stuf" + getoid(user));
	exec(user, this_object());
    user->setup();
	tell_room(VOID_OB, (string)user->query_name() + " enters this reality\n");
    user->move(VOID_OB);
	destruct(this_object());
}
