/* simple little program to link the right malloc c module to malloc.c */
/* needs to be a C program so that it can include config.h */

#include "config.h"

#ifdef SYSMALLOC
#define THE_MALLOC "sysmalloc.c"
#endif

#ifdef SMALLOC
#define THE_MALLOC "smalloc.c"
#endif

#ifdef SMALLOC2
#define THE_MALLOC "smalloc2.c"
#endif

#ifdef GCMALLOC
#define THE_MALLOC "gc.c"
#endif

#ifdef GMALLOC
#define THE_MALLOC "gmalloc.c"
#endif

main()
{
	unlink("malloc.c");
	link(THE_MALLOC,"malloc.c");
	return 0;
}
