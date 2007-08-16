As MudOS is moving too slow to keep our driver hacks apart, we now call our own
FluffOS :)

Fluffos 2.7:
Fixes to compile with a Dead Souls config
Changed free_object() and free_program() to overwrite the freed address 
	(idea and first version by Pinkfish)
terminate string results of filter_string() 
disable run time type checking if strict types isn't on
allow adding objects to strings (adds filename)
fixed crasher in DEBUG mode
send too long eval errors to the mudlib error handler, there's no real reason 
     not to!
fixed crasher for 64 bit dead souls
possibly fixed some bugs in shuffling around code for free_object
allow string a="dsfsd"; a+=10; it used to give a parse error! the code to 
      handle it was there though
read_file() can now read compressed files.
write_file() now has an extra flag (2) that can be ORed with the existing one 
	     to write/append compressed files
add_ref() destruct objects with high ref counts (they're usually on the way to
	  wrap around to 0, which is where you crash).
Fluffos 2.6:
current_time is now 64 bit on 64 bit platforms, all ready for 2038 now, I think :)
fixed crasher in ed on big files with long lines (i hope)
new default arguments for destruct, virtualp, inherits (this_object) and ctime (time())
removed some code duplication in call_function_pointer()
switch(number){
	case ..1:
	  return "low";
	case 2:
	  return "middle";
	case 3..:
	  return "high";
}
now works!

new WOMBLES config option that disallows spaces in ({ ([ (: :) ]) }) when used 
	for arrays, mappings and functionals.

fixed 64 bit compiles so you don't need to use one specific compiler with no optimisations anymore

made the objects list double linked so destruct doesn't have to walk the whole
	list to find the previous object in the list.
fixed compiling without shadows support

Fluffos 2.5:
some pluralize() fixes (woom)
fixed zonetime() and is_daylight_savings_time()

Fluffos 2.4:
new efuns for sending telnet sequences:
	void request_term_type();
	void start_request_term_type();
	void request_term_size();
restore_object now calls restore_lost_variable() if a variable in the save file
	doesn't exist in the object, the arguments are the value and name of 
	the saved variable.
fixed crasher in event() when one of the earlier objects destructs an object
	that still would have gotten the event.

Fluffos 2.3:
fixed event efun (from 1.40)
added MAX_INT and SIZEOFINT predefines (from 1.40)
fixed leftover PROT1V macro use
replace is an efun now
virtual base obs are no longer seen as clones

FluffOS 2.2:
removed some more object swapping code
fixed memory leak in restoring gzipped save files
improved LPC function profiling
fixed ref counts in class/array efuns

FluffOS 2.1:
added str_to_arr, and arr_to_str efuns to convert between strings and UTF-32 arrays
added strwidth efun
fixed refs() for strings

FluffOS 2.0:
different from Fluffos 1.36:

Changed integer type to 64 bit, and some other 64 bit cleanups, as 64 bit ints
change the way things work for the LPC side of things, I changed the major
number to 2.
removed swapping, the OS can do it much better than us.
removed LPC_TO_C as it has been broken for as long as i can remember (MudOSv21
	times)
removed the annoying PROT P1 etc macros if you still use a K&R compiler it's 
	time to upgrade!
removed varargs.h support
removed warnings during compiling (with supplied localoptions file)
removed gdbm files as they couldn't even be used (no spec file!)
removed dw specific debugging code 
added iconv support 
      If activated:
      All text in the mud is UTF-8
      
      int set_encoding(string); //sets the encoding for this_players
      string to_utf8(string, string); //convert enc encoded str to UTF-8
      string utf8_to(string, string); //convert UTF-8 encoded str to enc

some additions to pluralize() (woom)
new functions in dwlib package (woom)
     mixed abs( int | float );
     mixed assemble_class( mixed * );
     mixed *disassemble_class( mixed );
     mixed fetch_class_member( mixed, int );
     void store_class_member( mixed, int, mixed );

