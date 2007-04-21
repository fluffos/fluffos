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

#ifndef _FUNC_SPEC_
extern int debug_level;

void handle_debug_level (char *);
void debug_level_set (char *);
void debug_level_clear (char *);

#ifdef DEBUG_MACRO
#define debug(x,y) if (debug_level & DBG_##x) { printf("%s: ", #x); printf y; putchar('\n'); fflush(stdout); }
#else
#define debug(x,y)
#endif

/* Would be nice to have tons of these; should go to arbitrary bitsets */
#define DBG_call_out	 	1
#define DBG_addr_server		2
#define DBG_d_flag		4
#define DBG_connections		8
#define DBG_mapping		16
#define DBG_sockets		32
#define DBG_comp_func_tab	64
#define DBG_LPC			128
#define DBG_LPC_line		256

#endif
