#ifndef _OPTIONS_INTERNAL_H
#define _OPTIONS_INTERNAL_H

/* DO NOT MODIFY:  Everything in here is either very internal to the driver or
 * planned to be removed. You should not depends on any of the values here.
 */

/* You may optionally choose one (or none) of these malloc wrappers.  These
 * can be used in conjunction with any of the above malloc packages.
 *
 * DEBUGMALLOC:
 *   * Statistics on precisely how much memory has been malloc'd.
 *   * Incurs a fair amount of overhead (both memory and CPU)
 */
#undef DEBUGMALLOC

/* The following add certain bells and whistles to malloc: */

/* DEBUGMALLOC_EXTENSIONS: defining this (in addition to DEBUGMALLOC) enables
 * the set_malloc_mask(int) and debugmalloc(string,int) efuns.  These two
 * efuns basically allow you to cause certain malloc's and free's (with tags
 * selected by a specified mask) to print debug information (addr, tag,
 * description, size) to stdio (in the shell that invoked the driver) or to a
 * file.  Not defining this does reduce the overhead of DEBUGMALLOC from 16
 * bytes per malloc down to 8.  This macro has no effect if DEBUGMALLOC isn't
 * defined.
 */
#undef DEBUGMALLOC_EXTENSIONS

/* CHECK_MEMORY: defining this (in addition to DEBUGMALLOC and
 * DEBUGMALLOC_EXTENSIONS) causes the driver to check for memory
 * corruption due to writing before the start or end of a block.  This
 * also adds the check_memory() efun.  Takes a considerable ammount
 * more memory.  Mainly for debugging.
 */
#undef CHECK_MEMORY

/*
 * Keep statistics about allocated strings, etc.  Which can be viewed with
 * the mud_status() efun.  If this is off, mud_status() and memory_info()
 * ignore allocated strings, but string operations run faster.
 */
#define STRING_STATS
#define ARRAY_STATS
#define CLASS_STATS

/* APPLY_CACHE_BITS: defines the number of bits to use in the func lookup cache
 *   (in interpret.c).
 *
 *   22 bites : (1  << 22) * (3 * 8 + 2 * 2) ~= 112MB.
 */
#define APPLY_CACHE_BITS 22

/* CACHE_STATS: define this if you want call_other (apply_low) cache
 * statistics.  Causes HAS_CACHE_STATS to be defined in all LPC objects.
 */
#define CACHE_STATS

/* NONINTERACTIVE_STDERR_WRITE: if defined, all writes/tells/etc to
 *   noninteractive objects will be written to stderr prefixed with a ']'
 *   (old behavior).
 *
 * Compat status: Easy to support, and also on the "It's a bug!  No, it's
 * a feature!" religious war list.
 */
#undef NONINTERACTIVE_STDERR_WRITE

/* NO_BUFFER_TYPE: if this is #define'd then LPC code using the 'buffer'
 *   type won't be allowed to compile (since the 'buffer' type won't be
 *   recognized by the lexer).
 */
#undef NO_BUFFER_TYPE

/* ARRAY_RESERVED_WORD: If this is defined then the word 'array' can
 *   be used to define arrays, as in:
 *
 * int array x = ({ .... });
 *
 * A side effect is that 'array' cannot be a variable or function name.
 */
#undef ARRAY_RESERVED_WORD

/* REF_RESERVED_WORD: If this is defined then the word 'ref' can be
 *   used to pass arguments to functions by value.  Example:
 *
 * void inc(int ref x) {
 *     x++;
 * }
 *
 * ... y = 1; inc(ref y); ...
 *
 * A side effect is that 'ref' cannot be a variable or function name.
 *
 * Note: ref must be used in *both* places; this is intentional.  It protects
 * against passing references to routines which don't intend to return values
 * through their arguments, and against forgetting to pass a reference
 * to a function which wants one (or accidentally having a variable modified!)
 */
#define REF_RESERVED_WORD

/* use class keyword for lpc structs */
#define STRUCT_CLASS
/* use struct keyword for lpc structs */
#define STRUCT_STRUCT

/* HAS_STATUS_TYPE: old MudOS drivers had a 'status' type which was
 * identical to the 'int' type.  Define this to bring it back.
 *
 * Compat status: very archaic, but easy to support.
 */
#undef HAS_STATUS_TYPE

/* OLD_TYPE_BEHAVIOR: reintroduces a bug in type-checking that effectively
 * renders compile time type checking useless.  For backwards compatibility.
 *
 * Compat status: dealing with all the resulting compile errors can be
 * a huge pain even if they are correct, and the impact on the code is
 * small.
 */
#undef OLD_TYPE_BEHAVIOR

/* OLD_RANGE_BEHAVIOR: define this if you want negative indexes in string
 * or buffer range values (not lvalue, i.e. x[-2..-1]; for e.g. not
 * x[-2..-1] = foo, the latter is always illegal) to mean counting from the
 * end
 *
 * Compat status: Not horribly difficult to replace reliance on this, but not
 * trivial, and cannot be simulated.
 */
#undef OLD_RANGE_BEHAVIOR
/* define to get a warning for code that might use the old range behavior
 * when you're not actually using the old range behavior*/
#undef WARN_OLD_RANGE_BEHAVIOR

/* ARGUMENTS_IN_TRACEBACK: prints out function call arguments in error
 *   tracebacks, to aid in debugging.  Note: it prints the values of
 *   the arguments at the time of the error, not when the function
 *   was called.  It looks like this:
 *
 * Failed to load file: read_buffer
 * program: command/update.c, object: command/update line 15
 * '    commandHook' in '        clone/user.c' ('        clone/user#1')line 72
 * arguments were ("/read_buffer.c")
 * '           main' in '    command/update.c' ('      command/update')line 15
 * arguments were ("/read_buffer.c")
 *
 * The only down side is some people like their logs shorter
 */
#define ARGUMENTS_IN_TRACEBACK

/* LOCALS_IN_TRACEBACK: similar to ARGUMENTS_IN_TRACEBACK, but for local
 *   variables.  The output looks more or less like:
 *
 * locals: 1, "local_value"
 *
 * Same as above.  Tends to produce even longer logs, but very useful for
 * tracking errors.
 */
#define LOCALS_IN_TRACEBACK

/* supress warnings about unused arguments; only warn about unused local
 * variables.  Makes older code (where argument names were required) compile
 * more quietly.
 */
#define SUPPRESS_ARGUMENT_WARNINGS

/* TRAP_CRASHES:  define this if you want MudOS to call crash() in master.c
 *   and then shutdown when signals are received that would normally crash the
 *   driver.
 */
#define TRAP_CRASHES

/* This define has became default, define it has no value.*/
#define CALLOUT_HANDLES

/* Some maximum string sizes
 */
#define SMALL_STRING_SIZE 100
#define LARGE_STRING_SIZE 1000
#define COMMAND_BUF_SIZE 2000

/* Number of levels of nested datastructures allowed -- this limit prevents
 * crashes from occuring when saving objects containing variables containing
 * recursive datastructures (with circular references).
 */
#define MAX_SAVE_SVALUE_DEPTH 100

/* Miscellaneous config options that should probably be in the runtime
 * config file.
 */
/* MAX_LOCAL: maximum number of local variables allowed per LPC function */
#define CFG_MAX_LOCAL_VARIABLES 50

/* CFG_MAX_GLOBAL_VARIABLES: This value determines the maximum number of
 *   global variables per object.  The maximum value is 65536.  There is
 *   a marginal memory increase for a value over 256.
 */
#define CFG_MAX_GLOBAL_VARIABLES 65536

#define CFG_EVALUATOR_STACK_SIZE 6000
#define CFG_COMPILER_STACK_SIZE 600
#define CFG_MAX_CALL_DEPTH 150
/* This must be one of 4, 16, 64, 256, 1024, 4096 */
#define CFG_LIVING_HASH_SIZE 256

#endif
