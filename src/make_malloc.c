/* simple little program to link the right malloc c module to malloc.c */
/* needs to be a C program so that it can include config.h */

#include "config.h"
#ifdef NeXT
#include <libc.h>
#endif

#ifdef SYSMALLOC
#define THE_MALLOC "sysmalloc.c"
#endif

#ifdef WRAPPEDMALLOC
#define THE_MALLOC "wrappedmalloc.c"
#endif

#ifdef GCMALLOC
#define THE_MALLOC "gc.c"
#endif

#ifdef GMALLOC
#define THE_MALLOC "gmalloc.c"
#endif

#ifdef DEBUGMALLOC
#define THE_MALLOC "debugmalloc.c"
#endif

int main()
{
	unlink("malloc.c");
	printf("Using memory allocation package: %s\n", THE_MALLOC);
	link(THE_MALLOC,"malloc.c");
	return 0;
}
