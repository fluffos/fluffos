/*
 * config.h
 * global defines
 */

/*
 You _must_ choose one of these mallocs.  The malloc package is no longer
 to be specified in the Makefile.

 SYSMALLOC: default system malloc
 SMALLOC: satoria's malloc (included)
 GMALLOC: GNU malloc (not included)
 GCMALLOC: garbage collecting malloc (not included)

*/

#define SYSMALLOC

/*
 * Current version of the driver.
 * (the patchlevel is automatically appended to the end)
 */
#define VERSION "0.8."

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

/* CONFIG_FILE_DIR specifies a directory in which the driver will search for
   defaults config files.  If you don't wish to use this MACRO, you may
   always specific a full path to the config file when starting the driver.
*/

#define CONFIG_FILE_DIR "/usr/local/mud/etc"

/* INTERACTIVE_CATCH_TELL
   define this if you want catch_tell called on interactives as well as
   NPCs.  If this is defined, player.c will need a catch_tell(msg) method that
   calls receive(msg);
*/

#define INTERACTIVE_CATCH_TELL

/* ALL_CALL_EXIT
   define this if you wish move_object to call "exit" in objects in
   the inverse manner that move_object calls "init" in objects.  Note:
   this will make move_object slower.  And it could possibly make it
   impossible to leave a room if the room code contains an "exit"
   function that contains errors.  If you wish to have this behavior,
   its actually probably better to implement it in your move() lfun
   instead of in the driver (but then again, "init" should be called
   from the move() lfun too.   The desire for efficiency makes us do
   things we don't want to).
*/

#undef ALL_CALL_EXIT

/* ROOM_CALL_EXIT
   define this and not ALL_CALL_EXIT if you want "exit" to only be called
   in the room being exited and not in a manner which is an exact inverse
   of the way "init" is called.  (ROOM_CALL_EXIT is the behavior in 2.4.5)
*/

#undef ROOM_CALL_EXIT

/*
 * defined this if you want to use mudwho
 */

#undef MUDWHO 

/*
 * TRACE_CODE
 * define this for to enable code tracing
 * (the driver will print out the previous lines of code to an error)
 */
#define TRACE_CODE

/*
 * undefine this if you want to use the old style of access control
 */
#define ACCESS_RESTRICTED

/*
 * define this if you want comm debugging info in the status command
 */

#define COMM_STAT

/*
 * define this if your version of inet_ntoa works well.  It has a problem on
 * the sun 4.
 * NOTE: you must define this when compiling on a NeXT
 */
#define INET_NTOA_OK

/*
 * define this if you want restricted ed mode enabled
 */
#define RESTRICTED_ED

/*
 * define this if you have RCS on your system and want RCS functions
 * available through the driver
 * 
 * NOTE: on some systems this doesn't work! (not all the code is portable yet)
 */
/* #define RCS */

/*
 * define this if you want the printf() and sprintf() efuns.
 * They are left as an option because they are rather large and expensive.
 */
#define PRINTF

/* 
 * Define what random number generator to use.
 * If no one is specified, a guaranteed bad one will be used.
 * (make sure none is specified on hpux)
 #define DRAND48
 */
#define RANDOM

/*
 * define this if you want to disable shadows in your driver.
 */
#undef NO_SHADOWS

/*
 * Define LOG_SHOUT if you want all shouts to be logged in
 * mudlib/log/SHOUTS.
 */
#define LOG_SHOUT

/*
 * do you have berkeley style symlinks?
 */
#define SYMLINKS

/*
 * Define what ioctl to use against tty's.
 */
#define USE_TIOCGETP		/* BSD */
/* #define USE_TCGETA */ 	/* SYSV */


/*
 * Does the system have a getrusage call?
 */
#define RUSAGE

/*
 * Define SYSV if you are running system V with a lower release level than
 * Sys V.4.
 */
#undef SYSV

/*
 * Define FCHMOD_MISSING only if your system doesn't have fchmod().
 */
#undef FCHMOD_MISSING

/*
 * runtime config strings
 */

#define MUD_NAME                get_config_str(0)
#define MUD_LIB                 get_config_str(1)
#define BIN_DIR                 get_config_str(2)
#define SWAP_FILE               get_config_str(3)
#define CONFIG_DIR              get_config_str(4)
#define LOG_DIR                 get_config_str(5)
#define MASTER_FILE             get_config_str(6)
#define ACCESS_FILE             get_config_str(7)
#define ACCESS_LOG              get_config_str(8)
#define INCLUDE_DIRS            get_config_str(9)
#define SIMUL_EFUN              get_config_str(10)
#define GLOBAL_INCLUDE          get_config_str(11)

/*
 * runtime config ints
 */

#define TIME_TO_CLEAN_UP        get_config_int(0)
#define TIME_TO_SWAP            get_config_int(1)
#define TIME_TO_RESET	        get_config_int(2)
#define	ALLOWED_ED_CMDS         get_config_int(3)
#define MAX_BITS                get_config_int(7)
#define MAX_COST                get_config_int(9)
#define MAX_ARRAY_SIZE          get_config_int(10)
#define MAX_LOG_SIZE            get_config_int(12)
#define READ_FILE_MAX_SIZE      get_config_int(13)
#define	MAX_CMDS_PER_BEAT       get_config_int(14)
#define MAX_BYTE_TRANSFER       get_config_int(15)
#define PORTNO                  get_config_int(16)
#define RESERVED_SIZE           get_config_int(17)
#define	HTABLE_SIZE             get_config_int(19)
#define OTABLE_SIZE             get_config_int(20)


/*
 * these still need to be split out
 */

#define EVALUATOR_STACK_SIZE	1000	/* get_config_int(4) */
#define COMPILER_STACK_SIZE	200	/* get_config_int(5) */
#define MAX_TRACE		30	/* get_config_int(6) */
#define MAX_LOCAL		20	/* get_config_int(8) */
#define MAX_PLAYERS		40	/* get_config_int(11) */
#define LIVING_HASH_SIZE	100	/* get_config_int(18) */

/*
 * What is the value of the first constant defined by yacc ? If you do not
 * know, compile, and look at y.tab.h.
 */
#define F_OFFSET		257


/*
 * common macros
 (use this one if you don't have unsigned char in your compiler)
 #define EXTRACT_UCHAR(p) (*p < 0 ? *p + 0x100 : *p)
 */
#define EXTRACT_UCHAR(p) (*(unsigned char *)p)

/************************************************************************/
/*	END OF CONFIG -- DO NOT ALTER ANYTHING BELOW THIS LINE		*/
/************************************************************************/

#ifndef INLINE
#if defined(__GNUC__) && !defined(lint)
#define INLINE inline
#else
#define INLINE
#endif
#endif

/*
 * probably should be added to config file
 */

#define SMALL_STRING_SIZE     100
#define LARGE_STRING_SIZE     1000
#define COMMAND_BUF_SIZE      2000

/*
 * some generic large primes used by various hash functions in different files
 * You can alter these if you know of a better set of numbers!  Be sure
 * they are primes...
 */

#define	P1		701	/* 3 large, different primes */
#define	P2		14009	/* There's a file of them here somewhere :-) */
#define	P3		54001

/*
   define MALLOC, FREE, and REALLOC depending upon what malloc package is
   is used.  This technique is used because overlaying system malloc
   with another function also named malloc doesn't work on most machines
   that have shared libraries.  Using the below method will let such machines
   (e.g. NeXTs and RS/6000's) use smalloc if they choose.  It will also
   let us keep malloc stats even when system malloc is used.
*/

#ifdef SMALLOC
#define MALLOC(x)     malloc(x)
#define FREE(x)       free(x)
#define REALLOC(x,y)  realloc(x,y)
#define CALLOC(x,y)   calloc(x,y)
#endif

#ifdef GMALLOC
#define MALLOC(x)  gmalloc(x)
#define FREE(x)    gfree(x)
#define REALLOC(x,y) grealloc(x,y)
#define CALLOC(x,y)   gcalloc(x,y)
#endif

#ifdef SYSMALLOC
#define MALLOC(x)  sysmalloc(x)
#define FREE(x)    sysfree(x)
#define REALLOC(x,y) sysrealloc(x,y)
#define CALLOC(x,y)   syscalloc(x,y)
#endif

#ifndef MALLOC
#define MALLOC(x) puts("You need to specify a malloc package in config.h");
#define FREE(x) puts("You need to specify a malloc package in config.h");
#define REALLOC(x) puts("You need to specify a malloc package in config.h");
#define CALLOC(x) puts("You need to specify a malloc package in config.h");
#endif
