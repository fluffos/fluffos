#define EDIT_SOURCE
#define NO_SOCKETS
#define NO_OPCODES

#include "std.h"
#include "hash.h"

unsigned int whashstr (const char * s)
{
    unsigned long __h = 0;
    for ( ; *s; ++s)
      __h = 37 * __h + *s;
    return __h;
}
