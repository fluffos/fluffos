#ifndef FUNCTAB_TREE_H
#define FUNCTAB_TREE_H

#include "program.h"

/*
 * functab_tree.c
 */
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
int lookup_function PROT((function_t *, int, char *));
void add_function PROT((function_t *, unsigned short *, int));
#endif

#endif
