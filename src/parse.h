#ifndef PARSE_H
#define PARSE_H

#include "lpc_incl.h"

int parse (const char *, svalue_t *, const char *, svalue_t *, int);
char *process_string (char *);
svalue_t *process_value (char *);
char *break_string (char *, int, svalue_t *);

#endif
