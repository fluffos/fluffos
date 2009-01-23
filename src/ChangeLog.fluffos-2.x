As MudOS is moving too slow to keep our driver hacks apart, we now call our own
FluffOS :), note: where it says Cratylus, I got it from his version, usually
someone else did the work, but I don't know how to find who did what there.

FluffOS 2.15:
IPV6 support
class stats (Skullslayer@Realms of the Dragon)
some console additions see 'help' in the console (Hamlet)
some 64 bit fixes (Woom)
compiles with C90 compilers (that's 19!!! years ago now) (Kalinash@lpmuds.net)
added string_difference(string, string) which returns the difference between strings as a number (Woom)
updated MySQL support (Shadyman@lpmuds.net)
fixed crasher when the master apply doesn't allow an object to be created


FluffOS 2.14:
fixed crasher in async writes.
fixed bug in switch/case when using more than 2GB memory
check 64 bit lib before 32 bit version for mysql
support classes in member_array
fixed copyright statement in regex file
fixed memory leak in large shared strings
fixed some memory leaks in async io
use clone_object sefun (if present) when cloning objects with new() (Kalinash)

FluffOS 2.13:
oh no! unlucky number
removed binaries support
fixed crasher in restore_string
fixed some new compiler warnings (gcc 4.3)
some cleanups in comm.c (Cratylus@Dead souls)
cygwin and other changes in build.FluffOS (Cratylus@Dead souls)
set program_t to {0} for silly OSes that don't clear BBS memory (Cratylus@Dead souls) 
    (I think there may be more places that could go wrong!)
changed locale to "C" (Cratylus@Dead souls) 
Added a console, mostly for debugging use.  If driver is started directly
        rather than through a script, add argument -C and it has a 
        command-line.  try 'help'.  HAS_CONSOLE must be defined in
        local_options. (hamlet)
fixed crasher where we did remove_interactive when people go netdead, bad idea!

FluffOS 2.12:
Crasher fixes in using a mudlib error handler (Cratylus@Dead souls)
some mingw fixes (Cratylus@Dead souls)
new localoptions.ds (Cratylus@Dead souls)
rework of ed to do larger output chunks, more configurability, and bugfixes:
	mixed receive_ed(string msg, string fname) apply in playerob to 
	doctor the text (return 0 to have ed output to screen, 1 to have ed
	output nothing, or return a new string for ed to output).  Need to
        define RECEIVE_ED in options.h.  ed_start() and ed() take optional 
        final arg specifying lines on user's screen.  Indentant fixed for 
        'foreach' and lines with only '//'.  z++ and z-- now work.  Optional 
        new compile-time defines: ED_INDENT_CASE (should we indent 'case' 
        after 'switch'?) and ED_INDENT_SPACES <num> for how far autoindent 
        will indent each level. (hamlet)
fixed several crashers (comm.c and simulate.c) related to using vsprintf
	instead of vsnprintf. (hamlet)
added CFG_MAX_GLOBAL_VARIABLES to options.h.  Old setting was an arbitrary 256.
        (hamlet)
async writes with compression 
fixed some memory leaks
stuff I forgot, it's been too long!
slightly useful sfuns if you use async:

void decode(object ob, int flag, function cb, string saved){
  string *lines = explode(saved, "\n");
  mixed vars = filter(variables(ob,1), (:strsrch($1[1], "nosave") == -1:));
  vars = map(vars, (:$1[0]:));
  if(!flag)
    map(vars, bind((:store_variable($1, 0):), ob));
  vars = allocate_mapping(vars, 1);

  foreach(string line in lines){
    if(line[0] == '#')
      continue;
    else {
      int i = strsrch(line, ' ');
      if(vars[line[0..i-1]])
        evaluate(bind((:store_variable, line[0..i-1], restore_variable(line[i+1\..]):), ob));
    }
  }
  evaluate(cb);
}

void restore_object_async(string name, int flag, function cb){
  async_read(name, (: decode, previous_object(), flag, cb :));
}

void save_object_async(string name, int flag, function cb){
  mixed vars = filter(variables(previous_object(),1), (:strsrch($1[1], "nosave"\) == -1:));
  string *lines = allocate(sizeof(vars)+1);
  vars = map(vars, (:$1[0]:));
  lines[0] = "#"+base_name(previous_object())+".c";
  for(int i = 0; i < sizeof(vars); i++){
    string val = save_variable(evaluate(bind((:fetch_variable, vars[i]:), previ\ous_object())));
    if(flag & 1 || val)
      lines[i+1] = vars[i] + " " + val;
  }
  async_write(name, implode(lines, "\n"), flag | 1, cb);
}

FluffOS 2.11:
stop eval_cost() adding to the time you're allowed to run. (libc return a time
     longer than the set time if you query the remaining time right after 
     restarting the timer!
reset_eval_cost() now stops working after 100*max eval cost.
hopefully fixed readfile with lines beyond max readsize.

FluffOS 2.10:
can be compiled with g++
fix bugs in using arrays as sets 
    int *a=({1<<31,0}); return a-a
fixed crash in the children efun (hamlet)

FluffOS 2.9:
removed amiga support.
included most DS changes, should work on windows now (except for over
	 evaluation errors)
changed alist_cmp to work on longs rather than int, and store program offsets
	as 32 bit ints (instead of 64 (with room for 32) which fixed crashing 
	with 64 bit on solaris and stopped crashing on linux when using over 
	2GB memory.
added package async for async read and write support.
removed some dead code
fixed crasher in read_file() (i hope)
fixed makefile so it doesn't rebuild everything all the time
slightly bigger buffers for lex.c so it can load files quicker.
made buffer handling in loading compressed save files a bit smarter
fixed bug in children()
actually calculate the hash values for objects again, oops.
fixed crasher in event()
moved some functions to package contrib:
int num_classes( object );
    returns the amount of class definitions in the object
mixed assemble_class( mixed * );
    returns a class with fields filled in from the array arg
mixed *disassemble_class( mixed );
    returns an array with all class elements
mixed fetch_class_member( mixed, int );
    same as disassemble_class(mixed)[int];
mixed store_class_member( mixed, int, mixed );
    set a class member by offset
mixed *shuffle(mixed *);
    shuffle array members around and return the array (original is changed as 
    well!)
mixed element_of(mixed *);
    same as array[random(sizeof(array))]
mixed max( mixed *, int | void );
    returns the max value in the array, or the index of that value if the
    second argument is present and not 0
mixed min( mixed *, int | void );
    see max, but then lowest
mixed abs( int | float );
    guess.
FluffOS 2.8:
use a hash table for the children() efun
set the usec field in the select timeout
fixed sending sending byte value 255 (this will break any code that tries to 
	send telnet code, but that shouldn't be needed anyway and it didn't work
	on UTF enabled drivers already.
removed a lot of casts to int, C does that by default, so it's not needed.
replaced whashstr with a simpler, faster (on modern CPUs) hash that isn't
	limited to 16 bit results
fixed to_int so it doesn't truncate to 32 bit on 64 bit systems
now fills in maxrss in rusage on linux
changed from SIGALRM to SIGVTALRM, so other processes can't mess up the eval
	time
added a __LINE__ predefine for the line number
changed mappings to use the hash function rather than pointer values for a 
	better spread over the hash table
the flag argument to functions() now uses 1 for extra info, 2 for limiting
	the returned functions to those that are not inherited, those can be
	combined (3)
replace_html and replace_mxp do the < to &lt; etc conversions
random() can now return larger than 32 bit numbers
commented out a recursion check in the parser so DS can parse its commands

FluffOS 2.7:
fixes to compile with a Dead Souls config
changed free_object() and free_program() to overwrite the freed address 
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
	  
FluffOS 2.6:
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

FluffOS 2.5:
some pluralize() fixes (woom)
fixed zonetime() and is_daylight_savings_time()

FluffOS 2.4:
new efuns for sending telnet sequences:
	void request_term_type();
	void start_request_term_type();
	void request_term_size();
restore_object now calls restore_lost_variable() if a variable in the save file
	doesn't exist in the object, the arguments are the value and name of 
	the saved variable.
fixed crasher in event() when one of the earlier objects destructs an object
	that still would have gotten the event.

FluffOS 2.3:
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

changed integer type to 64 bit, and some other 64 bit cleanups, as 64 bit ints
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

