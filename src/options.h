/*
 * options.h: global defines for MudOS
 */

/* note: most of the configurable options are now set via the configuration
   file that is specified as the first argument to the driver.  See port.h
   for those #defines related to portability (compatibility).
*/

/* You _must_ choose exactly one of these mallocs.
 *
 * SYSMALLOC: builtin system malloc.  No wrappers, no statistics.  Can
 *   be used in conjunction with BSD or S malloc (in which case you will get
 *   malloc statistics if DO_MSTATS is also defined).  SYSMALLOC incurs no
 *   additional cpu or memory overhead.
 * WRAPPEDMALLOC: wrapper for builtin system malloc.  Provides limited
 *   statistics, can be used in conjunction with BSDMALLOC.  Limited
 *   additional cpu overhead and no additional memory overhead.
 * DEBUGMALLOC:  May be used in conjunction with BSD malloc.  Provides
 *   statistics on precisely how much memory has been malloc'd (as well
 *   as the stats provided by WRAPPEDMALLOC). Incurs a fair amount of
 *   overhead (in terms of memory and cpu usage).
 * GMALLOC: GNU malloc (not included: available from prep.ai.mit.edu:/pub/gnu).
 *   **has not been recently tested with MudOS.
 * GCMALLOC: garbage collecting malloc (not included).
 *   **has not been recently tested with MudOS.
 *
 * [NOTE: BSD malloc is also an option however it must be selected from the
 *  Makefile rather than here since it is an actual replacement for system
 *  malloc rather than a wrapper for system malloc as are SYSMALLOC and
 *  DEBUGMALLOC]
 */
#define SYSMALLOC
#undef WRAPPEDMALLOC
#undef DEBUGMALLOC

/* DO_MSTATS: do not define this unless BSD malloc or smalloc is chosen in the
   Makefile.  Defining this causes these replacement mallocs to keep
   statistics that the malloc_status() efun will print out (including total
   memory allocated/used).
*/
#undef DO_MSTATS

/* defining this (in addition to DEBUGMALLOC) enables the set_malloc_mask(int)
   and debugmalloc(string,int) efuns.  These two efuns basically allow you
   to cause certain malloc's and free's (with tags selected by a specified
   mask) to print debug information (addr, tag, description, size) to
   stdio (in the shell that invoked the driver) or to a file.  Not defining
   this does reduce the overhead of DEBUGMALLOC from 16 bytes per malloc
   down to 8.  This macro has no effect if DEBUGMALLOC isn't defined.
*/
#undef DEBUGMALLOC_EXTENSIONS

/* CONFIG_FILE_DIR specifies a directory in which the driver will search for
 * defaults config files.  If you don't wish to use this MACRO, you may
 * always specific a full path to the config file when starting the driver.
 */
#define CONFIG_FILE_DIR "/home/couns/sstock/src/MudOS/bin"

/* LAZY_RESETS: if this is defined, an object will only have reset()
   called in it when it is touched via apply_low() or move_object()
   (assuming enough time has passed since the last reset).  If LAZY_RESETS
   is #undef'd, then reset() will be called as always (which guaranteed that
   reset would always be called at least once).  The advantage of lazy
   resets is that reset doesn't get called in an object that is touched
   once and never again (which can save memory since some objects won't get
   reloaded that otherwise would).
*/
#define LAZY_RESETS

/* SANE_EXPLODE_STRING: define this if you want to prevent explode_string
   from stripping off more than one leading delimeters.  #undef it for the
   old behavior.
*/
#define SANE_EXPLODE_STRING

/* SET_EVAL_LIMIT:
   define this if you want to add the set_eval_limit() efun.  WARNING: do
   not define this unless you are prepared to either 1) have any object
   hang your mud with an infinite loop, or 2) use valid_override() in master
   and add a set_eval_limit() simul_efun to restrict the use of
   set_eval_limit().  *The intended use of this efun is to let certain
   privileged objects (e.g. daemons) that are trusted to not contain
   infinite loops up the eval limit to temporarily exceed the normal max
   eval limit and then set the limit back to normal before the next
   unprivileged object executes.
*/
#undef SET_EVAL_LIMIT

/* EACH: define this if you want the each() operator for mappings.  Undefining
   EACH save about 12 bytes per allocated mapping but will make the each()
   efun unavailable.
*/
#define EACH

/* STRICT_TYPE_CHECKING: define this if you wish to force formal parameter
   to include types.  If this is undef'd, then grossnesses like:
   func(obj) { string foo;  foo = allocate(3); } are allowed.
*/
#undef STRICT_TYPE_CHECKING

/* define NO_ANSI if you wish to disallow players (and wizards) from typing
   in commands that contain ANSI escape sequences.  Defining NO_ANSI causes
   all escapes (ANSI 27) to be replaced with a space ' '.
*/
#define NO_ANSI

/* OPC_PROF: define this if you wish to enable OPC profiling. 
   Allow a dump of the # of times each efun is invoked (via the opcprof() efun)
   Also enables the opcprof() efun.
*/
#define OPCPROF

/* TRAP_CRASHES define this if you want MudOS to call crash() in master.c
   and then shutdown when signals are received that would normally crash the
   driver.  This define only affects SIGINT and SIGTERM unless -DDEBUG is not
   defined (in the Makefile)
*/
#define TRAP_CRASHES

/* DROP_CORE: define this if you want the driver to attempt to create
   a core file when it crashes via the crash_MudOS() function.  This
   define only has an affect if -DDEBUG isn't defined in the makefile
   (except for the SIGINT and SIGTERM signals which are always trapped).
 
   Note: keep this undefined for now since it seems to hang some machines
   upon crashing (some DECstations apparently).  If you want to get a core
   file, undef'ing TRAP_CRASHES should work.
*/
#undef DROP_CORE

/* Define this if you wish this_player() to be useable from within
   call_out() callbacks.
*/
#define THIS_PLAYER_IN_CALL_OUT

/*
 * AUTO_SETEUID: when an object is created it's euid is automatically set to
 * the equivalent of seteuid(getuid(this_object())).  undef AUTO_SETEUID
 * if you would rather have the euid of the created object be set to 0.
 * The original LPmud behavior was to have the euid set to 0.
 */
#undef AUTO_SETEUID

/*
 * AUTO_TRUST_BACKBONE: define this if you want objects with the backbone
 * uid to automatically be trusted and to have their euid set to the uid of
 * the object that forced the object's creation.  This was the default behavior
 * of the driver, but if you #undef it, it will no longer trust backbone
 * automatically.
 */
#define AUTO_TRUST_BACKBONE

/* HEARTBEAT_INTERVAL: define heartbeat interval in microseconds (us).
 * 1,000,000 us = 1 second.  The value of this macro specifies
 * the frequency with which the heart_beat method will be called in
 * those LPC objects which have called set_heart_beat(1);
 * Note: if SYSV is defined, alarm() is used instead of ualarm().  Since
 * alarm() requires its argument in units of a second, we map 1 - 1,000,000 us
 * to an actual interval of one (1) second and 1,000,001 - 2,000,000 maps to
 * an actual interval of two (2) seconds, etc.
 */
#define HEARTBEAT_INTERVAL 2000000

/*
 * LARGEST_PRINTABLE_STRING is the size of the vsprintf() buffer in comm.c
 * add_message(). Instead of blindly making this value larger, mudlib
 * should be coded to not send huge strings to users.
 */
#define LARGEST_PRINTABLE_STRING 8192

/* MESSAGE_BUFFER_SIZE determines the size of the buffer for output that
   is sent to users.  If this buffer is filled before the outgoing socket
   becomes writeable, then output to the user will be truncated.  Be careful
   not to make this value too large since #(MAX_USERS) of these buffers are
   allocated.  (thus if MAX_USERS is 100, a value of 4kb for buffer size would
   cause 400kb of space to be allocated for outgoing buffers).  A minimum
   value of 1024 is recommended.
*/
#define MESSAGE_BUFFER_SIZE 4096

/* number of bits to use in the call_other cache (in interpret.c).  Somewhere
   between six (6) and ten (10) is probably sufficient  */
#define APPLY_CACHE_BITS 10

/* define this if you want call_other (apply_low) cache statistics */
#define CACHE_STATS

/* define this if you want the printf() and sprintf() efuns.
 * They are left as an option because they are rather large and expensive.
 */
#define PRINTF

/*
 * define this if you want the ed() efun (the builtin line editor).
 */
#define ED

/*
 * define this if you want no heartbeats when there are no users logged in.
 * if this is not defined, heartbeats will be called always.
 */
#undef OLD_HB_BEHAVIOR

/* define VALID_CLONE if you want clone_object and load_object to check for 
 * valid_read ability before they work.  This will probably cause a lot of
 * mudlib headaches during initial conversions.
 */
#undef VALID_CLONE

/* define CAST_CALL_OTHERS if you want to require casting of call_other's
 * this was the default behavior of the driver prior to this addition.
 */
#undef CAST_CALL_OTHERS

/* INTERACTIVE_CATCH_TELL
   define this if you want catch_tell called on interactives as well as
   NPCs.  If this is defined, user.c will need a catch_tell(msg) method that
   calls receive(msg);
*/
#undef INTERACTIVE_CATCH_TELL

/*
 * TRACE_CODE
 * define this for to enable code tracing
 * (the driver will print out the previous lines of code to an error)
 */
#define TRACE_CODE

/*
 * undefine this if you want to use the old style of access control
 */
#undef ACCESS_RESTRICTED

/*
 * define this if you want comm debugging info in the status command
 */
#define COMM_STAT

/*
 * define this if you want restricted ed mode enabled
 */
#define RESTRICTED_ED

/*
 * define this if you want to disable shadows in your driver.
 */
#undef NO_SHADOWS

/*
 * Define LOG_SHOUT if you want all shouts to be logged in
 * mudlib/log/SHOUTS.
 */
#undef LOG_SHOUT

/* define OLD_PRESENT if you want the old present() efun behavior.  The
   old present(name,...) would return the object responding to id(name).
   The new present(name,...) may return the object returned by id(name).
   id(name) may also return an integer in which case present(name,...)
   behaves like the old present().

   #undef'ing OLD_PRESENT can cause crashes in certain scenarios (when
   and id() in an object contains a call to present() -- infinite recursion).
   Best to leave OLD_PRESENT defined for now.
*/

#define OLD_PRESENT

/* various string sizes */

#define SMALL_STRING_SIZE     100
#define LARGE_STRING_SIZE     1000
#define COMMAND_BUF_SIZE      2000

/* number of levels of nested datastructures allowed -- this limit prevents
   crashes from occuring when saving objects containing variables containing
   recursive datastructures (with circular references).
*/
#define MAX_SAVE_SVALUE_DEPTH 25

/*
 * these config options still need to be split out into the runtime config.
 */

/* MAX_LOCAL: maximum number of local variables allowed per LPC function */
#define MAX_LOCAL 20	          /* get_config_int(8) */
/* MAX_USERS: maximum number of simultaneous interactive users allowed */
#define MAX_USERS 40	          /* get_config_int(12) */
/* MAX_EFUN_SOCKS: maximum number of efun sockets */
#define MAX_EFUN_SOCKS 16	  /* get_config_int(24) */

#define EVALUATOR_STACK_SIZE 1000 /* get_config_int(4) */
#define COMPILER_STACK_SIZE 200   /* get_config_int(5) */
#define MAX_TRACE 30              /* get_config_int(6) */
#define LIVING_HASH_SIZE 101      /* get_config_int(20) */

/*
 * Current version of the driver.
 * (the patchlevel is automatically appended to the end)
 */
#define VERSION "0.9."
