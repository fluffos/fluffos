/* simple little program to link the right malloc c module to malloc.c */
/* needs to be a C program so that it can include config.h */

#define NO_SOCKETS
#define NO_OPCODES
#include "std.h"

#ifdef SYSMALLOC
#  define THE_MALLOC "sysmalloc.c"
#endif
#ifdef SMALLOC
#  define THE_MALLOC "smalloc.c"
#endif
#ifdef BSDMALLOC
#  define THE_MALLOC "bsdmalloc.c"
#endif

#ifdef WRAPPEDMALLOC
#  define THE_WRAPPER "wrappedmalloc.c"
#endif

#ifdef DEBUGMALLOC
#  define THE_WRAPPER "debugmalloc.c"
#endif

#if !defined(THE_MALLOC) && !defined(THE_WRAPPER)
int main() {
    puts("Memory package and/or malloc wrapper incorrectly specified in options.h\n");
}
#else
int main() {
    unlink("malloc.c");
#ifdef THE_WRAPPER
    printf("Using memory allocation package: %s\n\t\tWrapped with: %s\n",
	   THE_MALLOC, THE_WRAPPER);
    link(THE_WRAPPER, "mallocwrapper.c");
#else
    printf("Using memory allocation package: %s\n", THE_MALLOC);
    link("plainwrapper.c", "mallocwrapper.c");
#endif
    link(THE_MALLOC, "malloc.c");
    return 0;
}
#endif
