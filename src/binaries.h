#ifndef BINARIES_H
#define BINARIES_H

#include "compiler.h"

extern char driver_name[];

FILE *crdir_fopen PROT((char *));
void init_binaries PROT((int, char **));
#ifdef LPC_TO_C
#define load_binary(x, y) int_load_binary(x, y)
program_t *int_load_binary PROT((char *, lpc_object_t *));
#else
#define load_binary(x, y) int_load_binary(x)
program_t *int_load_binary PROT((char *));
#endif
void save_binary PROT((program_t *, mem_block_t *, mem_block_t *));

#endif
