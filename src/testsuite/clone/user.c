// mudlib:   Lil 
// file:     user.c
// purpose:  is the representation of an interactive (user) in the MUD

#include <globals.h>

inherit BASE;

private string name;

// replace this with a functioning version.

string
process_input(string arg)
{
	// possible to modify player input here before driver parses it.
	return arg;
}

string
query_cwd()
{
	return "";
}

// logon: move this to /single/login.c when login.c gets written.

void
logon()
{
	write("Welcome to Lil.\n> ");
}

// query_name: called by various objects needing to know this user's name.

string
query_name()
{
	return name;
}

void
set_name(string arg)
{
//  may wish to add security to prevent just anyone from changing
//  someone else's name.
	name = arg;
}

// called by the present() efun (and some others) to determine whether
// an object is referred as an 'arg'.

int
id(string arg)
{
	return (arg == query_name()) || base::id(arg);
}

int
commandHook(string arg)
{
    string cmd_path;
    object cobj;

    cmd_path = COMMAND_PREFIX + query_verb();

    cobj = load_object(cmd_path);
    if (cobj) {
		return (int)cobj->main(arg);
    } else {
		// maybe call an emote/soul daemon here
    }
	return 0;
}

// init: called by the driver to give the object a chance to add some
// actions (see the MudOS "applies" documentation for a better description).

void
init()
{
	// using "" as the second argument to add_action() causes the driver
	// to call commandHook() for those user inputs not matched by other
	// add_action defined commands (thus 'commandHook' becomes the default
	// action for those verbs without an explicitly associated action).
	if (this_object() == this_player()) {
		add_action("commandHook", "", 1);
	}
}

// create: called by the driver after an object is compiled.

void
create()
{
	seteuid(0); // so that login.c can export uid to us
}

// receive_message: called by the message() efun.

void
receive_message(string newclass, string msg)
{
	// the meaning of 'class' is at the mudlib's discretion
	receive(msg);
}

// setup: used to configure attributes that aren't known by this_object()
// at create() time such as living_name (and so can't be done in create()).

void
setup()
{
    set_heart_beat(1);
	seteuid(getuid(this_object()));
    set_living_name(query_name());
	enable_commands();
	enable_wizard();
    add_action("commandHook", "", 1);
}

// net_dead: called by the gamedriver when an interactive player loses
// hir network connection to the mud.

void
net_dead()
{
	set_heart_beat(0);
    tell_room(environment(), query_name() + " is link-dead.\n");
}

// reconnect: called by the login.c object when a netdead player reconnects.

void
reconnect()
{
	set_heart_beat(1);
    tell_room(environment(), "Reconnected.\n");
    tell_room(environment(), query_name() + " has reconnected.\n");
}
