/* simple little program to link the right malloc c module to malloc.c */
/* needs to be a C program so that it can include config.h */

#include "config.h"
#ifdef NeXT
#include <libc.h>
#endif
#if defined(__386BSD__) || defined(SunOS_5)
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include "lint.h"
#endif

#ifdef SYSMALLOC
#define THE_MALLOC "sysmalloc.c"
#endif

#ifdef WRAPPEDMALLOC
#define THE_MALLOC "wrappedmalloc.c"
#endif

#ifdef DEBUGMALLOC
#define THE_MALLOC "debugmalloc.c"
#endif

int main(argc, argv)
      int argc;
      char *argv[];
{
	unlink("malloc.c");
	if (argc == 2) {
		printf("Using memory allocation package: %s\n", argv[1]);
	} else {
		printf("Using memory allocation package: %s\n", THE_MALLOC);
	}
#ifdef LATTICE
	{
		char cmd[100];
		sprintf(cmd,"copy %s malloc.c",THE_MALLOC);
		system(cmd);
	}
#else
	link(THE_MALLOC,"malloc.c");
#endif
	return 0;
}
