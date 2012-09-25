As MudOS is moving too slow to keep our driver hacks apart, we now call our own
FluffOS :), note: where it says Cratylus, I got it from his version, usually
someone else did the work, but I don't know how to find who did what there.

FluffOS 2.24
LPC floats are now C doubles
number of structs (or classes) allowed is now 127 
defer() now hopefully notices if you try to change this_player, so it won't
	then destroy the change after the deffered function finishes
changed something for the terminal_colours_replace apply, but can't remember
	what (need to release more often!)
added some flags in debug_info
added some argument checking to replace() (dwlib.c)
a fix in pcre (woom) (again, can't remember what was fixed)

FluffOS 2.23
added a terminal_colour_replace apply, this will be called with every string between two %^ delimiters, and will be replaced with whatever is returned.
fixed protocol number for GMCP
fixed sprintf code for MSSP uptime
added defer efun, it takes a function pointer that will be called when the current function ends (even if that was caused by a runtime)
the old range behaviour warning for negative array indexes is now optional
the driver can now be compiled to use either struct or class for structs, or even allow both
fixed crasher in uniq_array
fixed crasher in socket_status
added missing ',' in non iconv driver pcre support
FluffOS 2.22
fixed potential crasher in pcre_extract
removed limit for number of matches in pcre efuns
added classes() efun (woom)
	classes(ob) returns a list of class names available in ob
	classes(ob, 1) returns the same list, with all the fields in the class (type and name)
you can now have more than 256 Globals (qwer@lpmuds.net)
added postgres support (unagi@lpmuds.net)
fixed zmp crash
removed some obsolete malloc options
FluffOS 2.21
small cleanup in malloc32
added gmcp support
      gmcp_enable() gets called when a user has gmcp
      gmcp() will get called with any received message
      send_gmcp(string) will send the string as a gmcp message
      has_gmcp(object) returns if the object supports gmcp
fixed sorting when the compare function returns values that don't fit in a 32 bit int.
fixed memory leak in sorting
new deep_inventory functionality (tiz)
    it can now take an array of objects, and a function pointer that can return
    0 don't include this object or it's contents
    1 do include
    2 include this object but not it's contents
    3 don't include this object but do add the contents
    the function will be called with one object from the inventory (for each object looked at)
fixed memory leak in new deep_inventory
added pcre support (Volothamp@Final Realms)
      string pcre_version(void);
      version of the pcre lib
      mixed pcre_match(string | string *, string, void | int);
      like the regexp efun
      mixed *pcre_assoc(string, string *, mixed *, mixed | void);
      like reg_assoc
      string *pcre_extract(string, string);
      extract matching parts
      string pcre_replace(string, string, string *);
      string replace, one entry in the array for each match
      string pcre_replace_callback(string, string, string | function, ...);
      string replace uses a callback to get the replace string (called with the matched string and match number, starting with 0)
      mapping pcre_cache(void);
      returns content of the pcre cache (not all that useful)
fixed memory leaks in pcre efuns
fixed crashers in pcre efuns
small optimisation in reg_assoc and pcre_assoc
fixed memory leak in compiling files
restore_object will no longer randomly set 0 values as undefined
fixed crasher in asking for an unused config setting
sprintf buffer is now big enough for max string size
fixed crasher in async_db_exec (never seen for real, but it was possible!)
db_fetch will no longer randomly return some 0s as undefined
dwlib package now has a replace_dollars function which searches for patterns starting with a $ only, otherwise the same as replace() (but faster as it only scans once)

FluffOS 2.20
more error checks in malloc64
bigger arrays   (up to 2^31 elements)
bigger mappings (see arrays)
more efficient clean_up()
setting sockets to close on exec done in a more compatible way (only worked on rather new linux kernels)
no longer sends mccp messages when already compressed (fixes older cmud versions)
some cleanups for compiler warnings
new roulette_wheel() efun in the dwlib package (Woom)
new replace_objects() efun int the dwlib package (replaces all object references in the argument with filenames recursively)
check_valid_path apply now also used for compile paths (source files)
32BIT fix (Kalinash)
use less chars for string hashes (faster)
correctly do tables in (s)printf with utf-8 strings
use the already existing precalculated string hashes more often
save string length for bigger strings as well instead of using strlen on strings > 64k all the time
NetBSD IPV6 fix (Tiz)
fixed crasher in reference_allowed() (in dwlib.c)

FluffOS 2.19
attempt to fix string block alignments. hopefuly helps sparc64
open sockets as close on exec if available (so they don't end up in external programs started from the driver) 
fix conflict between ed and solaris (both used the same define!)
fix bug with freeing an object table in backend.c
some fixes for sparc64 (Kalinash)
added missing Mysql data types so they don't always get returned as 0 anymore
changed some optional efun args to default to 0 instead for slightly cleaner code (Woom)
new addition to pluralize() (diff from Cratylus)

FluffOS 2.18
compiles for netbsd (tiz)
make more empty arrays point to the_null_array, saves memory and allows 
     comparing with ({}) to see if arrays are empty (reported by Woom)
clear this_user etc when runtimes get us all the way back to backend()
fix the inherits() return value if the inherit was indirectly inherited
     (reported by Woom)
member_array now return -1 for failure if you search beyond the end of the array
     (reported by Woom)
no longer loops forever when adding a reference whole destructing things with
     too many references
fixed crasher in async db_exec
fixed crasher in filedescriptor leak fix
parser changed to be less strict (Cratylus)
stop wasting memory if repeat_string would exceed max string size (reported by woom)
fixed crasher in pragma optimize

FluffOS 2.17
math package updates: Added vector norm, dotprod, distance, angle.
     Also added log2() and round() which works on floats (surprisingly useful).
     Added int args to the efuns as apppropriate (Hamlet)
fixed above so the int args actually work without needing casts for the result
fixed 64bit malloc for large allocations (never happened on dw, so I doubt it was a problem!)
added 32bit malloc (malloc32) which is sysmalloc with realloc replaced by 
      malloc->memcpy->free, saves lots of memory
telnet environment support (Cratylus)
windows compile fix for add_action (Cratylus)
added dtrace support! just define DTRACE in local_options if you have it
zmp support
    zmp calls from the client result in an apply on the player object
    	zmp_command(string command, string *args);
    sending zmp is done with
    	send_zmp(string command, string *args);
    check if a player supports zmp, returns 1 if they do, 0 otherwise.
    	has_zmp(object player) 
    note: zmp protocol is just a way to transfer information for zmp packages,
    	  You'll still need to implement those in LPC
fixed the use of select()
compiles with C++ again (with dw's local_options anyway)
even more places to look for mysql libs
fixed profiling recursive functions
fixed profiling when a runtime error happens
fixed filedescriptor leak with compressed save files
fixed crasher in unloading object programs (this should have happened constantly
      , so there's probably a bug preventing this from actually happening most 
      of the time).
the driver now finishes all async IO before finishing shutdown()
blocked socket fix for lpc network sockets (Hamlet)
package async now does sql!
	async_db_exec(int db, string request, function callback);
	don't use the same database handle with this call if you also use it
	with db_exec(), just make an extra connection for your async sql.
new efun restore_from_string(string savedata), does what it says on the tin, 
    the string format is the same as a save file.
added optional int argument to request_term_size().  If 0, the client is asked 
     _not_ to offer any further term size updates (Hamlet, suggested by Detah)

FluffOS 2.16
improved single char mode support (Cratylus)
	 this includes some new efuns:
	      int query_charmode(object);
	      int remove_charmode(object);
	      int remove_get_char(object);
efun to send nullbytes (Raudhrskal)
     int send_nullbyte(object);
improved ed failure mess (Cratylus)
new no arguments version of save_object, which returns the save string
fix to stop iconv looping forever
faster hashing for big strings (now stops after 1000 chars)
some new predefines (Cratylus)
async io fixes, sadly now requires pthreads
parser update (Cratylus)
sqlite support (Ajandurah@Demonslair)
compile warning fixes (Ajandurah@Demonslair)
crypto and sha1 package (Ajandurah@Demonslair)
added MSSP support, the driver will call get_mud_stats() on the master ob, 
      which should return a mapping with the keys/values, if a value is an 
      array of strings they'll all be sent as values for that key. The driver
      send the NAME (from config file) PLAYERS and UPTIME values if the 
      function doesn't exist, if it does but didn't include one of those fields
      the driver will add the field as those are required.
new malloc option malloc64 which tries to avoid needing big copies on realloc by spreading all allocations a few MB apart in virtual memory.

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

