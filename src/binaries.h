#ifndef BINARIES_H
#define BINARIES_H

#include "compiler.h"

extern char driver_name[];

FILE *crdir_fopen PROT((char *));
void init_binaries PROT((int, char **));
int load_binary PROT((char *, lpc_object_t *));
void save_binary PROT((program_t *, mem_block_t *, mem_block_t *));

#endif
