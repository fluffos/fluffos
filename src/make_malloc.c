/* simple little program to link the right malloc c module to malloc.c */
/* needs to be a C program so that it can include config.h */

#define NO_OPCODES
#define NO_SOCKETS
#include "std.h"

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
#if defined(LATTICE) || defined(OS2)
    {
	char cmd[100];

	sprintf(cmd, "copy %s malloc.c", THE_MALLOC);
	system(cmd);
    }
#else
    link(THE_MALLOC, "malloc.c");
#endif
    return 0;
}
