#ifndef PARSE_H
#define PARSE_H

#include "lpc_incl.h"

int parse PROT((const char *, svalue_t *, const char *, svalue_t *, int));
char *process_string PROT((char *));
svalue_t *process_value PROT((char *));
char *break_string PROT((char *, int, svalue_t *));

#endif
