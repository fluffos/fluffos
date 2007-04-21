#ifndef BINARIES_H
#define BINARIES_H

/* lpc_object_t */
#include "object.h"

#include "compiler.h"

extern char driver_name[];

FILE *crdir_fopen (char *);
void init_binaries (int, char **);
#define load_binary(x, y) int_load_binary(x)
program_t *int_load_binary (char *);
void save_binary (program_t *, mem_block_t *, mem_block_t *);

#endif
