/* debug.h: added by Truilkan: 92/02/08 */

/* this include file adds some nice debugging capabilities.  You can
   use -DDEBUG_MACRO in CFLAGS in the Makefile to let the debug code
   be compiled in.  If DEBUG_MACRO is not defined then the debug code
   is removed by the C preprocessor.  The global variable debug_level
   is defined in main.c.  The debug_level typically starts out at zero
   (0) and slowly grows larger over the life of the program.  The main
   advantage to this debug macro is that you can turn various sets of
   debug statements of and on by changing the value of debug_level.
   Say for example that debug_level is set to 7; then, all debug()
   statements with a first argument of 1, 2, or 4 would be printed. If
   debug_level is 6, then only debug()'s with 2 or 4 as the first arg
   would be printed.  Sample usage is as follows:

   debug(4,("module.c: simulate(): x = %d, y = %d\n",x,y));
*/

extern int debug_level;

#ifdef DEBUG_MACRO
#define debug(x,y) if (x & debug_level) { printf y; fflush(stdout); }
#else
#define debug(x,y)
#endif
