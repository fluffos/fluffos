#ifndef BINARIES_H
#define BINARIES_H

#include "incralloc.h"

extern char driver_name[];

FILE *crdir_fopen PROT((char *));
void init_binaries PROT((int, char **));
int load_binary PROT((char *));
void save_binary PROT((struct program *, struct mem_block *, struct mem_block *));

#endif
