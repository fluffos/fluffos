#define EDIT_SOURCE
#define NO_SOCKETS
#define NO_OPCODES

#include "std.h"
#include "hash.h"

unsigned int whashstr (const char * s)
{
	int i = 0;
    unsigned long __h = 0;
    for ( ; *s && i++ <100 ; ++s)
      __h = 37 * __h + *s;
    return __h;
}
