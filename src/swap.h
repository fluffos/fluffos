#ifndef SWAP_H
#define SWAP_H

#include "lpc_incl.h"

/*
 * swap.c
 */
int swap PROT((object_t *));
int swap_line_numbers PROT((program_t *));
void load_ob_from_swap PROT((object_t *));
void load_line_numbers PROT((program_t *));
void remove_swap_file PROT((object_t *));
void unlink_swap_file PROT((void));
void remove_line_swap PROT((program_t *));
int locate_in PROT((program_t *));
int locate_out PROT((program_t *));
void print_swap_stats PROT((outbuffer_t *));

#endif
