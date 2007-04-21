#ifndef SPRINTF_H
#define SPRINTF_H

#include "lpc_incl.h"

void svalue_to_string (svalue_t *, outbuffer_t *, int, int, int);
char *string_print_formatted (const char *, int, svalue_t *);

#endif
