#include <stdio.h>
#include "config.h"
#include "lint.h"

extern char *strrchr();
extern int system();	/* Could be char* for sprintf, but... */

int
indent_program(name)
char *name;
{
    char buf[1000];
    char *p;

    p = strrchr(name, '/');
    if (!p)
	p = name;
    else
	p++;
#ifndef MSDOS
    (void) sprintf(buf, "%s/indent '%s' -lpc && rm '%s'.BAK", BIN_DIR, name,p);
    return system(buf) == 0;
#else
    {
	int ec;

	(void) sprintf(buf, "indent %s -lpc", name);
	ec = system(buf);
	unlink(p);
	return(ec == 0);
    }
#endif
}
