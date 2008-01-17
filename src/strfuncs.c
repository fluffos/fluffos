/*
 * ANSI C string library support/emulation
 *  (not complete)
 */

#include "std.h"

strcspn (register char * s, char * set)
{
    register char *t;
    register int count = 0;

    while (*s) {
	t = set;
	while (*t && (*s != *t))
	    t++;
	if (!*t) {
	    s++;
	    count++;
	} else
	    break;
    }

    return (count);
}


INLINE char *memset (char * s, int c, int n)
{
    if (c == 0)
	bzero(s, n);
    else {
	debug_message("Failed in memset\n");
	exit(1);
    }
}

INLINE char *memcpy (char * b, char * a, int s)
{
    bcopy(a, b, s);
    return b;
}
