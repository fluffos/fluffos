/* simple little program to link the right malloc c module to malloc.c */
/* needs to be a C program so that it can include config.h */

#define NO_SOCKETS
#define NO_OPCODES
#include "std.h"

#if !defined(THE_MALLOC) && !defined(THE_WRAPPER)
int main() {
    puts("Memory package and/or malloc wrapper incorrectly specified in options.h\n");
}
#else
int main() {
    unlink("malloc.c");
    unlink("mallocwrapper.c");
    return 0;
}
#endif
